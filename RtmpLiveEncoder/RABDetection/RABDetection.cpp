// RABDetection.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "stdafx.h"
#include "RABDetection.h"

int t=0;	//��������±�
int a[100][2];	//�������������
CvPoint **ptx = new CvPoint*[1];	//ptx[0]�����ĵ��λ�ã���ΪcvPolyLine�ĵ�����Ҫ�õ�ָ���ָ��

///////��ʼ�������ʽ
CvFont font1;
CvPoint ptext;
int linetype=CV_AA;
string msg[16]={"Alarm!!!","01","02","03","04","05","06","07","08","09","10","11","12","13","14","15"};
const int CONTOUR_MAX_AERA = 16;
CvMemStorage *stor;
int nFrmNum = 0;	//��ǰ֡��	
bool matchValue = false;
const int MAX_X_STEP = 10;	//match����
const int MAX_Y_STEP = 20;	//match����

const double MHI_DURATION = 0.5;
const double MATCH_RATIO = 0.5;
const int N = 3;
IplImage **buf = 0;
int last = 0;
IplImage *mhi = 0; // MHI: motion history image

std::vector<CvRect> objDetRect;	//�������򣬱��浱ǰ֡��⵽�����������
std::vector<blockTrack> trackBlock;	 //�����������	

void onMouse(int Event,int x,int y,int flags,void* param)   //��¼�����������
{ 
	if(Event == CV_EVENT_LBUTTONDOWN)
	{	
		a[t][0]=x;
		a[t][1]=y;            
		t++;
	} 
}

void  m_Detect( IplImage* img, IplImage* dst, int diff_threshold )		//Ŀ���⺯�����õ���rect���浽������
{
	double timestamp = clock()/100.; // get current time in seconds
	CvSize size = cvSize(img->width,img->height); // get current frame size
	int i, idx1, idx2;
	IplImage* silh;
	IplImage* pyr = cvCreateImage( cvSize((size.width & -2)/2, (size.height & -2)/2), 8, 1 );
	CvMemStorage *stor;
	CvSeq *cont;

	if( !mhi || mhi->width != size.width || mhi->height != size.height ) 
	{
		if( buf == 0 ) 
		{
			buf = (IplImage**)malloc(N*sizeof(buf[0]));		
			memset( buf, 0, N*sizeof(buf[0]));
		}

		for( i = 0; i < N; i++ ) 
		{
			cvReleaseImage( &buf[i] );
			buf[i] = cvCreateImage( size, IPL_DEPTH_8U, 1 );
			cvZero( buf[i] );
		}
		cvReleaseImage( &mhi );
		mhi = cvCreateImage( size, IPL_DEPTH_32F, 1 );
		cvZero( mhi ); // clear MHI at the beginning
	} // end of if(mhi)

	cvCvtColor( img, buf[last], CV_BGR2GRAY ); // convert frame to grayscale

	idx1 = last;
	idx2 = (last + 1) % N; // index of (last - (N-1))th frame 
	last = idx2;

	silh = buf[idx2];
	cvAbsDiff( buf[idx1], buf[idx2], silh ); // ��֡��

	cvThreshold( silh, silh, diff_threshold, 255, CV_THRESH_BINARY ); // �Բ�ͼ������ֵ��

	cvUpdateMotionHistory( silh, mhi, timestamp, MHI_DURATION ); // update MHI
	cvCvtScale( mhi, dst, 255./MHI_DURATION, 
		(MHI_DURATION - timestamp)*255./MHI_DURATION );    
	cvCvtScale( mhi, dst, 255./MHI_DURATION, 0 );    

	cvSmooth( dst, dst, CV_MEDIAN, 3, 0, 0, 0 );// ��ֵ�˲�������С������	
	
	cvPyrDown( dst, pyr, 7 );// ���²�����ȥ������
	cvDilate( pyr, pyr, 0, 1 );  // �����Ͳ���������Ŀ��Ĳ������ն�
	cvPyrUp( pyr, dst, 7 );

	// Create dynamic structure and sequence.
	stor = cvCreateMemStorage(0);
	cont = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint) , stor);

	cvFindContours( dst, stor, &cont, sizeof(CvContour), 
		CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

	// ֱ��ʹ��CONTOUR�еľ�����������
	for(;cont;cont = cont->h_next)
	{
		CvRect r = ((CvContour*)cont)->rect;
		if(r.height * r.width > CONTOUR_MAX_AERA) // ȥ������
			objDetRect.push_back(r);	//�Ѽ�⵽�����rect����	
	}

	cvReleaseMemStorage(&stor);		//�ͷ��ڴ�
	cvReleaseImage( &pyr );
}

bool match(CvRect rect1, CvRect rect2)	//ƥ���⵽������͸�����������ƶ�
{//���������ƶ�����
	CvPoint pt1,pt2;
	pt1 = cvPoint(rect1.x+rect1.width/2,rect1.y+rect1.height/2);
	pt2 = cvPoint(rect2.x+rect2.width/2,rect2.y+rect2.height/2);
	if (abs(pt1.x-pt2.x)<MAX_X_STEP&&abs(pt1.y-pt2.y)<MAX_Y_STEP)
		return TRUE;		

	return FALSE;
}

bool ptInPolygon(CvPoint** ptx,int t,CvPoint pt)
{
	int nCross = 0; 
	for (int i = 0; i < t; i++) 
	{ 
		CvPoint p1 = ptx[0][i]; 
		CvPoint p2 = ptx[0][(i + 1) % t]; 
		// ��� y=p.y �� p1p2 �Ľ��� 
		if ( p1.y == p2.y ) // p1p2 �� y=p0.yƽ�� 
			continue; 
		if ( pt.y < min(p1.y, p2.y) ) // ������p1p2�ӳ����� 
			continue; 
		if ( pt.y >= max(p1.y, p2.y) ) // ������p1p2�ӳ����� 
			continue; 
		// �󽻵�� X ���� 
		double x = (double)(pt.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) + p1.x; 
		if ( x > pt.x ) 
			nCross++; // ֻͳ�Ƶ��߽��� 
	} 
	// ���߽���Ϊż�������ڶ����֮��
	return (nCross % 2 == 1); 
}



