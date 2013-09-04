#pragma once
#include "stdafx.h"
#include "highgui.h"
#include "cvaux.h"

typedef struct
{
	HWND hHwnd;
	int index;
	int procType;
	CString type;	//����ͷ����
}threadParam;

typedef struct
{
	int m_port;
	int index;
}HikPort;

extern const unsigned short SIZE_OF_BUFFER; //����������
extern const unsigned short SIZE_OF_THREAD;
extern bool g_continue[]; //���Ƴ������
extern bool g_continueAxis[];
extern IplImage* images[][100];//�������Ǹ�ѭ������

char * imagesData[][100];

extern IplImage* depthImages[][10];	//�������֡ͼ��
extern unsigned short in[]; //��Ʒ��������ʱ�Ļ������±�
extern unsigned short out[]; //��Ʒ��������ʱ�Ļ������±�
extern HANDLE g_hMutex[]; //�����̼߳�Ļ���
extern HANDLE g_hFullSemaphore[]; //����������ʱ��ʹ�����ߵȴ�
extern HANDLE g_hEmptySemaphore[]; //����������ʱ��ʹ�����ߵȴ�
//extern HANDLE hThread[];
extern CWinThread* m_pThread[];
extern CWinThread* m_pAxisThread[];

extern HikPort g_CamOperate[];

extern UINT  RemoveNight(LPVOID lpParameter);
extern UINT  OrdinaryCount(LPVOID lpParameter);
extern UINT  ShowHikImg(LPVOID lpParameter);
extern UINT  getKinectFrame(LPVOID lpParameter);
extern UINT  KinectPedestrianCount(LPVOID lpParameter);
extern UINT  KinectTopCount(LPVOID lpParameter);
extern UINT  resolveAxisFrame(LPVOID lpParameter);
extern void stopThread(int i);
extern void stopAxisThread(int i);
extern void init(int i);

extern IplImage* YV12_To_IplImage(IplImage* pImage,unsigned char* pYV12, int width, int height);
extern bool YV12_To_BGR24(unsigned char* pYV12, unsigned char* pRGB24,int width, int height);

#define PARAM_NOPROC 1
#define PARAM_REMOVENIGHT 2
#define PARAM_PEOPLECLUSTERING 3
#define PARAM_UNUSUAL 4
#define PARAM_XIE_COUNT 5
#define PARAM_VERTICAL_COUNT 6


