#include "stdafx.h"
#include <cv.h>
#include <cxcore.h>
#include <cvaux.h>
#include <highgui.h>
#include <string.h>
#include <windows.h>
#include <iostream>
using namespace std;
///////////����ģ�����ݽṹ

const int MAX_STEP = 200;
typedef struct blockTrack
{
	bool flag;	//��ʶ��ǰ֡���޼�⵽ǰһ֡���ٵ�����
	bool crsAear;	//�Ƿ��ڶ�����ڲ�	
	int step;	//��¼�˶�����
	CvPoint pt[MAX_STEP];	//��¼�˶�·��
	CvRect rect;	//��¼��ǰλ��
}blockTrack;

//���־����������
extern int t;	//��������±�
extern int a[][2];	//�������������
extern CvPoint **ptx;	//ptx[0]�����ĵ��λ�ã���ΪcvPolyLine�ĵ�����Ҫ�õ�ָ���ָ��

///////��ʼ�������ʽ
extern CvFont font1;
extern CvPoint ptext;
extern int linetype;
extern string msg[];

extern const int CONTOUR_MAX_AERA ;
extern CvMemStorage *stor;
//CvFilter filter = CV_GAUSSIAN_5x5;
extern int nFrmNum ;	//��ǰ֡��	
extern bool matchValue;
extern const int MAX_X_STEP;	//match����
extern const int MAX_Y_STEP;	//match����

extern const double MHI_DURATION;
extern const double MATCH_RATIO;
extern const int N;
extern IplImage **buf;
extern int last;
extern IplImage *mhi; // MHI: motion history image

extern std::vector<CvRect> objDetRect;	//�������򣬱��浱ǰ֡��⵽�����������
extern std::vector<blockTrack> trackBlock;	 //�����������	

extern void onMouse(int Event,int x,int y,int flags,void* param);
extern void m_Detect(IplImage* img, IplImage* dst, int diff_threshold);
extern bool match(CvRect rect1, CvRect rect2);
extern bool ptInPolygon(CvPoint** ptx,int t,CvPoint pt);