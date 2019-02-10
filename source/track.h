#pragma once
#ifndef TRACK_H
#define TRACK_H

#include <iostream>

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

using namespace std;


extern IplImage *grayImage;
extern IplImage *binaryImage;

void trans_mouse(int mouseevent, int x, int y, int flags, void* param);
void on_trackbar1(int pos);
void transfer(IplImage* img1, IplImage* img2, CvMat* transmat);
void auto_perspective(IplImage* img1, IplImage* img2, CvMat* transmat);
void thinImage(IplImage* src, IplImage* dst, int maxIterations);
void cvThin(IplImage* img1, IplImage*img2);
void findPoint(IplImage* src, CvPoint2D32f* corners, int& pNum);
void track(IplImage* img, CvMat* transmat, CvPoint2D32f* points, int &pNum);

#endif
