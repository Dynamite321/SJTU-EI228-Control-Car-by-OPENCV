#pragma once
#ifndef CONTROLCAR_H
#define CONTROLCAR_H

#include <iostream>
#include <cmath>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "SerialPort.h"
#include "Image.h"

using namespace std;

#define PortNum 4

const double distance_error = 35;
const double angle_error = 20;
const double PI = 3.1415926535;

double Distance(CvPoint2D32f src, CvPoint2D32f dst);
double Angle(CvPoint2D32f src, CvPoint2D32f dst);
CvPoint2D32f nextPoint(CvPoint2D32f carPos, CvPoint2D32f* points, int pNum, bool* mask);
bool car_center(IplImage* src, CvPoint2D32f &car, CvPoint2D32f &car_head, CvPoint2D32f &car_tail);
int moveCar(CSerialPort& port, CvPoint2D32f car, CvPoint2D32f car_head, CvPoint2D32f car_tail, CvPoint2D32f nextPoint);

 
#endif 
