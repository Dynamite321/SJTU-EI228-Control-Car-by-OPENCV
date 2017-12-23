#include "stdafx.h"
#include "track.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <iostream>

#define cvCopyImage( src, dst )         cvCopy( src, dst, 0 )

vector<vector<int>> trans_point; //透视变化取点
IplImage *grayImage = NULL;
IplImage *binaryImage = NULL;

void trans_mouse(int mouseevent, int x, int y, int flags, void* param) //透视变化鼠标回调函数
{
	if (mouseevent == CV_EVENT_LBUTTONDOWN)
	{
		trans_point.push_back({ x,y });
	}
}

void on_trackbar1(int pos)
{

	cvThreshold(grayImage, binaryImage, pos, 255, CV_THRESH_BINARY);
	cvShowImage("二值图", binaryImage);
}

void transfer(IplImage *img1, IplImage* img2, CvMat* transmat) //透视变化
{
	const char *binaryTitle = "二值图";
	const char *winTitle = "track";
	const char *toolBarName = "二值图阈值";

	cvShowImage(winTitle, img1); //显示图像
	cvSetMouseCallback(winTitle, trans_mouse); //设置回调函数
	cvWaitKey(); //等待按任意键结束程序
	IplImage* transimg = cvCreateImage(cvSize(640, 400), IPL_DEPTH_8U, 3);
	CvPoint2D32f originpoints[4]; //保存四个点的原始坐标
	CvPoint2D32f newpoints[4]; //保存这四个点的新坐标
	originpoints[0] = cvPoint2D32f(trans_point[0][0], trans_point[0][1]);
	newpoints[0] = cvPoint2D32f(0, 0);
	originpoints[1] = cvPoint2D32f(trans_point[1][0], trans_point[1][1]);
	newpoints[1] = cvPoint2D32f(640, 0);
	originpoints[2] = cvPoint2D32f(trans_point[2][0], trans_point[2][1]);
	newpoints[2] = cvPoint2D32f(0, 400);
	originpoints[3] = cvPoint2D32f(trans_point[3][0], trans_point[3][1]);
	newpoints[3] = cvPoint2D32f(640, 400);
	cvGetPerspectiveTransform(originpoints, newpoints, transmat);
	cvWarpPerspective(img1, transimg, transmat); //根据变换矩阵计算图像的变换
	cvNamedWindow("透视修正", CV_WINDOW_AUTOSIZE);
	cvShowImage("透视修正", transimg); //显示变换后的图像
	cvWaitKey(0);

	grayImage = cvCreateImage(cvGetSize(transimg), IPL_DEPTH_8U, 1);
	cvCvtColor(transimg, grayImage, CV_BGR2GRAY);

	binaryImage = cvCreateImage(cvGetSize(grayImage), IPL_DEPTH_8U, 1);
	cvNamedWindow(binaryTitle, CV_WINDOW_AUTOSIZE);
	int nThreshold = 0;
	cvCreateTrackbar(toolBarName, binaryTitle, &nThreshold, 254, on_trackbar1);
	on_trackbar1(1);
	cvWaitKey(0);
	cvThreshold(grayImage, binaryImage, nThreshold, 255, CV_THRESH_BINARY);
	cvXorS(binaryImage, cvScalarAll(255), binaryImage);//黑白互换
	cvSaveImage("binary_image.png", binaryImage);
	cvWaitKey(0);

	cvDestroyWindow(binaryTitle);
	cvDestroyWindow(winTitle);
	cvDestroyWindow("透视修正");
	cvReleaseImage(&grayImage);
	cvReleaseImage(&transimg);
}

void auto_perspective(IplImage* img1, IplImage* img2, CvMat* transmat)
{
	cvWarpPerspective(img1, img2, transmat);
}

void thinImage(IplImage* src, IplImage* dst, int maxIterations = -1)
{
	CvSize size = cvGetSize(src);
	cvCopy(src, dst);
	int count = 0;
	while (true)
	{
		count++;
		if (maxIterations != -1 && count > maxIterations)
			break;
		vector<pair<int, int> > mFlag;
		for (int i = 0; i<size.height; ++i)
		{
			for (int j = 0; j<size.width; ++j)
			{
				int p1 = CV_IMAGE_ELEM(dst, uchar, i, j);
				int p2 = (i == 0) ? 0 : CV_IMAGE_ELEM(dst, uchar, i - 1, j);
				int p3 = (i == 0 || j == size.width - 1) ? 0 : CV_IMAGE_ELEM(dst, uchar, i - 1, j + 1);
				int p4 = (j == size.width - 1) ? 0 : CV_IMAGE_ELEM(dst, uchar, i, j + 1);
				int p5 = (i == size.height - 1 || j == size.width - 1) ? 0 : CV_IMAGE_ELEM(dst, uchar, i + 1, j + 1);
				int p6 = (i == size.height - 1) ? 0 : CV_IMAGE_ELEM(dst, uchar, i + 1, j);
				int p7 = (i == size.height - 1 || j == 0) ? 0 : CV_IMAGE_ELEM(dst, uchar, i + 1, j - 1);
				int p8 = (j == 0) ? 0 : CV_IMAGE_ELEM(dst, uchar, i, j - 1);
				int p9 = (i == 0 || j == 0) ? 0 : CV_IMAGE_ELEM(dst, uchar, i - 1, j - 1);
				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1)
					{
						if (p2*p4*p6 == 0)
						{
							if (p4*p6*p8 == 0)
							{
								mFlag.push_back(make_pair(i, j));
							}
						}
					}
				}
			}
		}
		for (vector<pair<int, int> >::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
		{
			CV_IMAGE_ELEM(dst, uchar, i->first, i->second) = 0;
		}
		if (mFlag.size() == 0)
		{
			break;
		}
		else
		{
			mFlag.clear();
		}
		for (int i = 0; i<size.height; ++i)
		{
			for (int j = 0; j<size.width; ++j)
			{
				int p1 = CV_IMAGE_ELEM(dst, uchar, i, j);
				if (p1 != 1) continue;
				int p2 = (i == 0) ? 0 : CV_IMAGE_ELEM(dst, uchar, i - 1, j);
				int p3 = (i == 0 || j == size.width - 1) ? 0 : CV_IMAGE_ELEM(dst, uchar, i - 1, j + 1);
				int p4 = (j == size.width - 1) ? 0 : CV_IMAGE_ELEM(dst, uchar, i, j + 1);
				int p5 = (i == size.height - 1 || j == size.width - 1) ? 0 : CV_IMAGE_ELEM(dst, uchar, i + 1, j + 1);
				int p6 = (i == size.height - 1) ? 0 : CV_IMAGE_ELEM(dst, uchar, i + 1, j);
				int p7 = (i == size.height - 1 || j == 0) ? 0 : CV_IMAGE_ELEM(dst, uchar, i + 1, j - 1);
				int p8 = (j == 0) ? 0 : CV_IMAGE_ELEM(dst, uchar, i, j - 1);
				int p9 = (i == 0 || j == 0) ? 0 : CV_IMAGE_ELEM(dst, uchar, i - 1, j - 1);

				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1)
					{
						if (p2*p4*p8 == 0)
						{
							if (p2*p6*p8 == 0)
							{
								mFlag.push_back(make_pair(i, j));
							}
						}
					}
				}
			}
		}
		for (vector<pair<int, int> >::iterator i = mFlag.begin(); i != mFlag.end(); ++i)
		{
			CV_IMAGE_ELEM(dst, uchar, i->first, i->second) = 0;
		}
		if (mFlag.size() == 0)
		{
			break;
		}
		else
		{
			mFlag.clear();
		}
	}
}

void cvThin(IplImage* img1, IplImage*img2)
{
	img1 = cvLoadImage("binary_image.png", CV_LOAD_IMAGE_GRAYSCALE);
	IplImage *pTemp = cvCreateImage(cvGetSize(img1), img1->depth, img1->nChannels);
	//将原图像转换为二值图像  
	cvThreshold(img1, pTemp, 128, 1, CV_THRESH_BINARY);
	//图像细化  
	thinImage(pTemp, img2);
	for (int i = 0; i<img2->height; ++i)
	{
		for (int j = 0; j<img2->width; ++j)
		{
			if (CV_IMAGE_ELEM(img2, uchar, i, j) == 1)
				CV_IMAGE_ELEM(img2, uchar, i, j) = 255;
		}
	}

	cvWaitKey(0);
	cvReleaseImage(&pTemp);
}

void findPoint(IplImage* src, CvPoint2D32f* corners, int& pNum)
{
	double qualityLevel = 0.05;
	double minDistance = 50;
	IplImage *corners1 = 0, *corners2 = 0;
	corners1 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	corners2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);

	cvGoodFeaturesToTrack(src, corners1, corners2, corners, &pNum, qualityLevel, minDistance, 0);

	if (pNum>0)
	{
		for (int k = 0; k <pNum; ++k)
		{
			cout << corners[k].x << ' ' << corners[k].y << endl;
		}
	}

	cvReleaseImage(&corners1);
	cvReleaseImage(&corners2);
}

void track(IplImage* img, CvMat* transmat, CvPoint2D32f* points, int &pNum)
{
	cvNamedWindow("track");
	IplImage* img1 = cvCreateImage(cvSize(640, 400), IPL_DEPTH_8U, 1);
	IplImage* img2 = cvCreateImage(cvSize(640, 400), IPL_DEPTH_8U, 1);
	IplImage* img3 = cvCreateImage(cvSize(640, 400), IPL_DEPTH_8U, 3);
	IplImage* img4 = NULL;
	transfer(img, img1, transmat);
	cvWaitKey(0);
	cvThin(img4, img2);
	cvShowImage("track", img2);
	cvWaitKey(0);
	findPoint(img2, points, pNum);
	cvCvtColor(img2, img3, CV_GRAY2BGR);
	if (pNum>0)
	{
		for (int k = 0; k < pNum; ++k)
		{
			cvCircle(img3, cvPointFrom32f(points[k]), 6, CV_RGB(255, 0, 0), 2);
		}
	}
	cvShowImage("track", img3);
	cvWaitKey(0);
	cvDestroyWindow("track");
	cvReleaseImage(&img1);
	cvReleaseImage(&img2);
	cvReleaseImage(&img3);
	cvReleaseImage(&img4);
}