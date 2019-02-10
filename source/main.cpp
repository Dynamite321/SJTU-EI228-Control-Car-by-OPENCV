#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>    
#include <opencv2/imgproc/imgproc.hpp>    
#include <opencv2/core/core.hpp>   
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <iostream>
#include <vector>
#include <ctime>
#include "track.h"
#include "SerialPort.h"
#include "controlCar.h"

CvPoint2D32f car, car_head, car_tail;
CvMat* transmat = cvCreateMat(3, 3, CV_32FC1);
CvPoint2D32f* points = new CvPoint2D32f[200];
bool *mask;
IplImage *img = NULL, *transimg = NULL;
int pNum = 0;
CSerialPort port;
IplImage *hsv1 = 0, *hue1 = 0, *maskimg1 = 0, *backproject1 = 0;
IplImage *hsv2 = 0, *hue2 = 0, *maskimg2 = 0, *backproject2 = 0;
CvHistogram *hist1 = 0;
CvHistogram *hist2 = 0;
int backproject_mode = 0;
int select_object = 0;
int track_object = 0;
CvPoint origin;
CvRect select_red, select_green;
CvRect track_window_red, track_window_green;
CvBox2D track_box1, track_box2;
CvConnectedComp track_comp1, track_comp2;
int hdims = 30;
float hranges_arr[] = { 0,180 };
float* hranges = hranges_arr;
int vmin = 10, vmax = 256, smin = 30;

inline void create_find()
{
	hsv1 = cvCreateImage(cvGetSize(transimg), 8, 3);
	hue1 = cvCreateImage(cvGetSize(transimg), 8, 1);
	maskimg1 = cvCreateImage(cvGetSize(transimg), 8, 1);
	backproject1 = cvCreateImage(cvGetSize(transimg), 8, 1);
	hist1 = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
	hsv2 = cvCreateImage(cvGetSize(transimg), 8, 3);
	hue2 = cvCreateImage(cvGetSize(transimg), 8, 1);
	maskimg2 = cvCreateImage(cvGetSize(transimg), 8, 1);
	backproject2 = cvCreateImage(cvGetSize(transimg), 8, 1);
	hist2 = cvCreateHist(1, &hdims, CV_HIST_ARRAY, &hranges, 1);
}

inline void find_center()
{
	cvCvtColor(transimg, hsv1, CV_BGR2HSV);
	int _vmin = vmin, _vmax = vmax;
	cvInRangeS(hsv1, cvScalar(0, smin, MIN(_vmin, _vmax), 0),
		cvScalar(180, 256, MAX(_vmin, _vmax), 0), maskimg1);
	cvSplit(hsv1, hue1, 0, 0, 0);
	cvCalcBackProject(&hue1, backproject1, hist1);
	cvAnd(backproject1, maskimg1, backproject1, 0);
	cvCamShift(backproject1, track_window_red,
		cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 1),
		&track_comp1, &track_box1);
	track_window_red = track_comp1.rect;
	cvCvtColor(transimg, hsv2, CV_BGR2HSV);
	cvInRangeS(hsv2, cvScalar(0, smin, MIN(_vmin, _vmax), 0),
		cvScalar(180, 256, MAX(_vmin, _vmax), 0), maskimg2);
	cvSplit(hsv2, hue2, 0, 0, 0);
	cvCalcBackProject(&hue2, backproject2, hist2);
	cvAnd(backproject2, maskimg2, backproject2, 0);
	cvCamShift(backproject2, track_window_green,
		cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 1),
		&track_comp2, &track_box2);
	track_window_green = track_comp2.rect;

	car_head.x = track_window_red.x + track_window_red.width / 2;
	car_head.y = track_window_red.y + track_window_red.height / 2;
	car_tail.x = track_window_green.x + track_window_green.width / 2;
	car_tail.y = track_window_green.y + track_window_green.height / 2;
	car.x = (car_head.x + car_tail.x) / 2;
	car.y = (car_head.y + car_tail.y) / 2;
}

void on_mouse_red(int event, int x, int y, int flags, void* param)
//鼠标回调函数,该函数用鼠标进行跟踪目标的选择  
{
	if (!img)
		return;

	if (img->origin)
		y = img->height - y;
	//如果图像原点坐标在左下,则将其改为左上  

	if (select_object)
		//select_object为1,表示在用鼠标进行目标选择  
		//此时对矩形类select_red用当前的鼠标位置进行设置  
	{
		select_red.x = MIN(x, origin.x);
		select_red.y = MIN(y, origin.y);
		select_red.width = select_red.x + CV_IABS(x - origin.x);
		select_red.height = select_red.y + CV_IABS(y - origin.y);

		select_red.x = MAX(select_red.x, 0);
		select_red.y = MAX(select_red.y, 0);
		select_red.width = MIN(select_red.width, img->width);
		select_red.height = MIN(select_red.height, img->height);
		select_red.width -= select_red.x;
		select_red.height -= select_red.y;
	}

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		//鼠标按下,开始点击选择跟踪物体  
		origin = cvPoint(x, y);
		select_red = cvRect(x, y, 0, 0);
		select_object = 1;
		break;
	case CV_EVENT_LBUTTONUP:
		//鼠标松开,完成选择跟踪物体  
		select_object = 0;
		if (select_red.width > 0 && select_red.height > 0)
			//如果选择物体有效，则打开跟踪功能  
			track_object = 2;
		break;
	}
}

void on_mouse_green(int event, int x, int y, int flags, void* param)
//鼠标回调函数,该函数用鼠标进行跟踪目标的选择  
{
	if (!img)
		return;

	if (img->origin)
		y = img->height - y;
	//如果图像原点坐标在左下,则将其改为左上  

	if (select_object)
		//select_object为1,表示在用鼠标进行目标选择  
		//此时对矩形类select_red用当前的鼠标位置进行设置  
	{
		select_green.x = MIN(x, origin.x);
		select_green.y = MIN(y, origin.y);
		select_green.width = select_green.x + CV_IABS(x - origin.x);
		select_green.height = select_green.y + CV_IABS(y - origin.y);

		select_green.x = MAX(select_green.x, 0);
		select_green.y = MAX(select_green.y, 0);
		select_green.width = MIN(select_green.width, img->width);
		select_green.height = MIN(select_green.height, img->height);
		select_green.width -= select_green.x;
		select_green.height -= select_green.y;
	}

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		//鼠标按下,开始点击选择跟踪物体  
		origin = cvPoint(x, y);
		select_green = cvRect(x, y, 0, 0);
		select_object = 1;
		break;
	case CV_EVENT_LBUTTONUP:
		//鼠标松开,完成选择跟踪物体  
		select_object = 0;
		if (select_green.width > 0 && select_green.height > 0)
			//如果选择物体有效，则打开跟踪功能  
			track_object = -1;
		break;
	}
}

void start_findcar()
{
	int _vmin = vmin, _vmax = vmax;
	cvCvtColor(transimg, hsv1, CV_BGR2HSV);
	cvInRangeS(hsv1, cvScalar(0, smin, MIN(_vmin, _vmax), 0),
		cvScalar(180, 256, MAX(_vmin, _vmax), 0), maskimg1);
	cvSplit(hsv1, hue1, 0, 0, 0);

	float max_val = 0.f;
	cvSetImageROI(hue1, select_red);
	cvSetImageROI(maskimg1, select_red);
	cvCalcHist(&hue1, hist1, 0, maskimg1);
	cvGetMinMaxHistValue(hist1, 0, &max_val, 0, 0);
	cvConvertScale(hist1->bins, hist1->bins, max_val ? 255. / max_val : 0., 0);
	cvResetImageROI(hue1);
	cvResetImageROI(maskimg1);
	track_window_red = select_red;
	cvCvtColor(transimg, hsv2, CV_BGR2HSV);
	cvInRangeS(hsv2, cvScalar(0, smin, MIN(_vmin, _vmax), 0),
		cvScalar(180, 256, MAX(_vmin, _vmax), 0), maskimg2);
	cvSplit(hsv2, hue2, 0, 0, 0);
	cvSetImageROI(hue2, select_green);
	cvSetImageROI(maskimg2, select_green);
	cvCalcHist(&hue2, hist2, 0, maskimg2);
	cvGetMinMaxHistValue(hist2, 0, &max_val, 0, 0);
	cvConvertScale(hist2->bins, hist2->bins, max_val ? 255. / max_val : 0., 0);
	cvResetImageROI(hue2);
	cvResetImageROI(maskimg2);
	track_window_green = select_green;
}

inline void destroy()
{
	//cvReleaseImage(&img);
	cvReleaseImage(&transimg);
	cvReleaseMat(&transmat);
	cvReleaseImage(&hsv1);
	cvReleaseImage(&hsv2);
	cvReleaseImage(&hue1);
	cvReleaseImage(&hue2);
	cvReleaseImage(&maskimg1);
	cvReleaseImage(&maskimg2);
	cvReleaseImage(&backproject1);
	cvReleaseImage(&backproject2);
	delete[] points;
	delete[] mask;
}
int main()
{
	cv::Mat Img;
	int pNum = 0;
	if (!port.InitPort(PortNum))
	{
		cout << "initPort fail!" << endl;
		return 0;
	}
	cout << "调整摄像头" << endl;
	cv::VideoCapture capture(0);
	cvNamedWindow("Camera Ajusting", CV_WINDOW_AUTOSIZE);

	while (true) //调整摄像头
	{
		capture >> Img;
		img = &IplImage(Img);
		cvShowImage("Camera Ajusting", img);
		if (cvWaitKey(15) > 0)
			break;
	}
	cvDestroyWindow("Camera Ajusting");
	track(img, transmat, points, pNum);
	mask = new bool[pNum + 1];
	for (int i = 0; i < pNum; ++i)
	{
		mask[i] = false;
	}
	transimg = cvCreateImage(cvSize(640, 400), IPL_DEPTH_8U, 3);
	create_find();
	cvNamedWindow("monitor");
	while (true) //调整摄像头
	{
		capture >> Img;
		img = &IplImage(Img);
		auto_perspective(img, transimg, transmat);
		cvShowImage("monitor", transimg);
		if (cvWaitKey(15) > 0)
			break;
	}
	while (true)
	{
		capture >> Img;
		img = &IplImage(Img);
		auto_perspective(img, transimg, transmat);
		if (track_object == 0)
		{
			cvShowImage("monitor", transimg);
			cvSetMouseCallback("monitor", on_mouse_red);
			cvWaitKey(0);
			cvSetMouseCallback("monitor", on_mouse_green);
			cvWaitKey(0);
			cvRectangle(transimg, cvPoint(select_red.x, select_red.y), 
				cvPoint(select_red.x+select_red.width, select_red.y+select_red.height), 
				cvScalar(0, 0, 255), 3, 4, 0);
			cvRectangle(transimg, cvPoint(select_green.x, select_green.y),
				cvPoint(select_green.x + select_green.width, select_green.y + select_green.height),
				cvScalar(0, 255, 255), 3, 4, 0);
			cvShowImage("monitor", transimg);
			cvWaitKey(0);
		}
		cvShowImage("monitor", transimg);
		if (cvWaitKey(15) > 0)
			break;
	}
	start_findcar();
	cout << "find car" << endl;
	while (true)
	{
		capture >> Img;
		img = &IplImage(Img);
		auto_perspective(img, transimg, transmat);
		find_center();
		cvCircle(transimg, cvPointFrom32f(car_head), 6, CV_RGB(255, 255, 0), 2);
		cvCircle(transimg, cvPointFrom32f(car_tail), 6, CV_RGB(0, 255, 255), 2);
		for (int i = 0; i < pNum; ++i)
		{
			cvCircle(transimg, cvPointFrom32f(points[i]), 6, CV_RGB(0, 0, 255), 2);
		}
		cvShowImage("monitor", transimg);
		if (cvWaitKey(15) > 0)
			break;
	}

	find_center();
	cvCircle(transimg, cvPointFrom32f(car_head), 6, CV_RGB(255, 0, 0), 2);
	cvCircle(transimg, cvPointFrom32f(car_tail), 6, CV_RGB(0, 255, 255), 2);
	cout << "Start from: " << car.x << ", " << car.y << endl;
	CvPoint2D32f next_point;
	for (int i = 0; i < pNum; ++i)
	{
		next_point = nextPoint(car_head, points, pNum, mask);
		cvCircle(transimg, cvPointFrom32f(next_point), 6, CV_RGB(0, 255, 0), 2);
		cout << next_point.x << ", " << next_point.y << endl;
		do
		{
			capture >> Img;
			img = &IplImage(Img);
			auto_perspective(img, transimg, transmat);
			find_center();
			cvCircle(transimg, cvPointFrom32f(car_head), 6, CV_RGB(255, 0, 0), 2);
			cvCircle(transimg, cvPointFrom32f(car_tail), 6, CV_RGB(0, 255, 255), 2);
			for (int i = 0; i < pNum; ++i)
			{
				cvCircle(transimg, cvPointFrom32f(points[i]), 6, CV_RGB(0, 0, 255), 2);
			}
			cvCircle(transimg, cvPointFrom32f(next_point), 6, CV_RGB(0, 255, 0), 2);
			cvShowImage("monitor", transimg);
			int state = moveCar(port, car, car_head, car_tail, next_point);
			if (state < 0)
				cout << "signal error!\n";
			else if (state == 0)
			{
				cout << "Finished TurnPoint " << i << endl;

				break;
			}
		} while (cvWaitKey(1)<0);
	}

	cvWaitKey();
	cvDestroyWindow("monitor");
	Img.release();
	destroy();
	return 0;
}
