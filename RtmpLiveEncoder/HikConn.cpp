// HiKConn.cpp : implementation file
//

#include "stdafx.h"
#include "HiKConn.h"
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
#include "Thread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow ();
LONG lPort; //ȫ�ֵĲ��ſ�port��
LONG m_iPort;
 

void CALLBACK DecCBFun(long nPort,char * pBuf,long nSize, FRAME_INFO * pFrameInfo, long nReserved1,long /*nReserved2*/) //YV12(YUV��һ��)-->IplImage
{
	if(pFrameInfo->dwFrameNum>0)
	{
		int index = nReserved1 ;
		HANDLE h[2] = {g_hEmptySemaphore[index],g_hMutex[index]};
		if(WaitForMultipleObjects(2,h,TRUE,0)==WAIT_TIMEOUT)
			return;
		
		//TRACE("%d %d", pFrameInfo->nWidth, pFrameInfo->nHeight);
		char * imgData = new char[640*480*1.5];
		memcpy(imgData, pBuf, 640*480*1.5);
		imagesData[index][in[index]] = imgData;
		
		in[index] =(in[index]+1)%SIZE_OF_BUFFER;
		ReleaseMutex(g_hMutex[index]);
		ReleaseSemaphore(g_hFullSemaphore[index],1,NULL);
		//delete[] imgData;

	}
}
void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize,void* dwUser)
{
	HWND hWnd= GetConsoleWindow();
	int* index = (int*)dwUser;
	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD: //ϵͳͷ

		if (!PlayM4_GetPort(&lPort))  //��ȡ���ſ�δʹ�õ�ͨ����
		{
			break;
		}
		m_iPort = lPort; //��һ�λص�����ϵͳͷ������ȡ�Ĳ��ſ�port�Ÿ�ֵ��ȫ��port���´λص�����ʱ��ʹ�ô�port�Ų���
		if (dwBufSize > 0)
		{
			if (!PlayM4_SetStreamOpenMode(lPort, STREAME_REALTIME))  //����ʵʱ������ģʽ
			{
				break;
			}

			if (!PlayM4_OpenStream(lPort, pBuffer, dwBufSize, 1024*1024)) //�����ӿ�
			{
				break;
			}
			PlayM4_SetDecCallBack(lPort,DecCBFun);
			if (!PlayM4_Play(lPort, hWnd)) //���ſ�ʼ
			{
				break;
			}
		}
	case NET_DVR_STREAMDATA:   //��������
		if (dwBufSize > 0 && lPort != -1)
		{
		
			if (!PlayM4_InputData(lPort, pBuffer, dwBufSize))
			{
				break;
			}
		}
	}

}

HiKConn::HiKConn(HWND hwnd,HWND pichwnd,int index,CString DeviceIp, CString m_csUser, CString m_csPWD, UINT m_nDevPort):hwnd(hwnd),pichwnd(pichwnd),index(index),DeviceIp(DeviceIp),m_csUser(m_csUser),m_csPWD(m_csPWD),m_nDevPort(m_nDevPort)
{
}

HiKConn::~HiKConn(void)
{
}

BOOL HiKConn::StartHikCamera(){
	if(!DoLogin())
		return FALSE;
	DoGetDeviceResoureCfg();  //��ȡ�豸��Դ��Ϣ	
	return TRUE;
}

BOOL HiKConn::DoLogin()
{
	//ת������
	USES_CONVERSION;
	
	NET_DVR_DEVICEINFO_V30 DeviceInfoTmp;
	memset(&DeviceInfoTmp,0,sizeof(NET_DVR_DEVICEINFO_V30));
		
	m_lDecID = NET_DVR_Login_V30(T2A(DeviceIp),m_nDevPort,T2A(m_csUser), T2A(m_csPWD), &DeviceInfoTmp);
	if(m_lDecID == -1)
	{
		return FALSE;
	}
    m_struDeviceInfo.lLoginID = m_lDecID;
	m_struDeviceInfo.iDeviceChanNum = DeviceInfoTmp.byChanNum;
    m_struDeviceInfo.iIPChanNum = DeviceInfoTmp.byIPChanNum;
    m_struDeviceInfo.iStartChan  = DeviceInfoTmp.byStartChan;

	return TRUE;
}

/*************************************************
������:    	DoGetDeviceResoureCfg
��������:	��ȡ�豸��ͨ����Դ
�������:   
�������:   			
����ֵ:		
**************************************************/
void HiKConn::DoGetDeviceResoureCfg()
{
	NET_DVR_IPPARACFG IpAccessCfg;
	memset(&IpAccessCfg,0,sizeof(IpAccessCfg));	
	DWORD  dwReturned;

	m_struDeviceInfo.bIPRet = NET_DVR_GetDVRConfig(m_struDeviceInfo.lLoginID,NET_DVR_GET_IPPARACFG,0,&IpAccessCfg,sizeof(NET_DVR_IPPARACFG),&dwReturned);

	int i;
    if(!m_struDeviceInfo.bIPRet)   //��֧��ip����,9000�����豸��֧�ֽ���ģ��ͨ��
	{
		for(i=0; i<MAX_ANALOG_CHANNUM; i++)
		{
			if (i < m_struDeviceInfo.iDeviceChanNum)
			{
				sprintf(m_struDeviceInfo.struChanInfo[i].chChanName,"camera%d",i+m_struDeviceInfo.iStartChan);
				m_struDeviceInfo.struChanInfo[i].iChanIndex=i+m_struDeviceInfo.iStartChan;  //ͨ����
				m_struDeviceInfo.struChanInfo[i].bEnable = TRUE;
				
			}
			else
			{
				m_struDeviceInfo.struChanInfo[i].iChanIndex = -1;
				m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				sprintf(m_struDeviceInfo.struChanInfo[i].chChanName, "");	
			}
		}
	}
	else        //֧��IP���룬9000�豸
	{
		for(i=0; i<MAX_ANALOG_CHANNUM; i++)  //ģ��ͨ��
		{
			if (i < m_struDeviceInfo.iDeviceChanNum)
			{
				sprintf(m_struDeviceInfo.struChanInfo[i].chChanName,"camera%d",i+m_struDeviceInfo.iStartChan);
				m_struDeviceInfo.struChanInfo[i].iChanIndex=i+m_struDeviceInfo.iStartChan;
				if(IpAccessCfg.byAnalogChanEnable[i])
				{
					m_struDeviceInfo.struChanInfo[i].bEnable = TRUE;
				}
				else
				{
					m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				}
				
			}
			else//clear the state of other channel
			{
				m_struDeviceInfo.struChanInfo[i].iChanIndex = -1;
				m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				sprintf(m_struDeviceInfo.struChanInfo[i].chChanName, "");	
			}		
		}

		//����ͨ��
		for(i=0; i<MAX_IP_CHANNEL; i++)
		{
			if(IpAccessCfg.struIPChanInfo[i].byEnable)  //ipͨ������
			{
				m_struDeviceInfo.struChanInfo[i+MAX_ANALOG_CHANNUM].bEnable = TRUE;
                m_struDeviceInfo.struChanInfo[i+MAX_ANALOG_CHANNUM].iChanIndex = IpAccessCfg.struIPChanInfo[i].byChannel;
				sprintf(m_struDeviceInfo.struChanInfo[i+MAX_ANALOG_CHANNUM].chChanName,"IP Camera %d",IpAccessCfg.struIPChanInfo[i].byChannel);

			}
			else
			{
               m_struDeviceInfo.struChanInfo[i+MAX_ANALOG_CHANNUM].bEnable = FALSE;
			    m_struDeviceInfo.struChanInfo[i+MAX_ANALOG_CHANNUM].iChanIndex = -1;
			}
		}		
	}

}

/*************************************************
������:    	StartPlay
��������:	��ʼһ·����
�������:   ChanIndex-ͨ����
�������:   			
����ֵ:		
**************************************************/
BOOL HiKConn::StartPlay(int iChanIndex)
{
	int m_iCurChanIndex = 0;
	NET_DVR_CLIENTINFO ClientInfo;
	ClientInfo.hPlayWnd     = pichwnd;
	ClientInfo.lChannel     = m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex;
	ClientInfo.lLinkMode    = 0;
    ClientInfo.sMultiCastIP = NULL;
	m_lPlayHandle = NET_DVR_RealPlay_V30(m_struDeviceInfo.lLoginID,&ClientInfo,g_RealDataCallBack_V30,NULL,FALSE);
	TRACE("*********%d",m_lPlayHandle);
	if(-1 == m_lPlayHandle)
	{
		return FALSE;
	}
}

/*************************************************
������:    	StopPlay
��������:	ֹͣ����
�������:   
�������:   			
����ֵ:		
**************************************************/
void HiKConn::StopPlay()
{
	if(m_lPlayHandle != -1)
	{
		NET_DVR_StopRealPlay(m_lPlayHandle);
	}
}


