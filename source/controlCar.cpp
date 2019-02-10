#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <iostream>
#include <ctime>
#include "controlCar.h"

char go_ahead = 'A';
char go_back = 'B';
char turn_left = 'L';
char turn_right = 'R';
char stop = 'P';


double Distance(CvPoint2D32f src, CvPoint2D32f dst)
{
	double distance = sqrtl((src.x - dst.x)*(src.x - dst.x) + (src.y - dst.y)*(src.y - dst.y));
	return distance;
}

double Angle(CvPoint2D32f src, CvPoint2D32f dst)
{
	double angle;
	double d = Distance(src, dst);
	if (dst.y <= src.y)
	{
		angle = acosl((dst.x - src.x) / d) / PI * 180;
	}
	else
	{
		angle = 360 - acosl((dst.x - src.x) / d) / PI * 180;
	}
	return angle;
}

CvPoint2D32f nextPoint(CvPoint2D32f carPos, CvPoint2D32f* points, int pNum, bool* mask)
{
	double min_distance = 1000.0, dist;
	int nearest = -1;
	for (int i = 0; i < pNum; ++i)
	{
		if (mask[i]) 
			continue;
		dist = Distance(carPos, points[i]);
		if (min_distance > dist)
		{
			min_distance = dist;
			nearest = i;
		}
	}
	mask[nearest] = true;
	return points[nearest];
}

int moveCar(CSerialPort& port, CvPoint2D32f car, CvPoint2D32f car_head, CvPoint2D32f car_tail, CvPoint2D32f nextPoint)
{
	double direction_car = Angle(car_tail, car_head);
	double direction_target = Angle(car, nextPoint);
	double distance = Distance(nextPoint, car);
	unsigned char *command;
	int flag = 1;
	int op = 3;
	if (distance>distance_error) 
	{
		if (fabs(direction_car - direction_target) <= 180)
		{
			if (fabs(direction_car - direction_target) > angle_error)
			{
				if (fabs(direction_car - direction_target) < 90)
				{
					if ((direction_car - direction_target) > 0)
					{
						op = 1; //right
					}
					else
					{
						op = 2; //left
					}
				}
				else op = 4; //backward
			}
			else
			{
				op = 3; //forward
			}
		}
		else
		{
			if (fabs(360.0 - fabs(direction_car - direction_target)) > angle_error)
			{
				if (direction_car < direction_target)
					direction_car += 360.0;
				else
					direction_target += 360.0;
				if (fabs(direction_car - direction_target) < 90)
				{
					if ((direction_car - direction_target) > 0)
					{
						op = 1; //right
					}
					else
					{
						op = 2; //left
					}
				}
				else op = 4; //backward
			}
			else
			{
				op = 3;
			}
		}
	}
	else 
	{
		op = 5; //stop
		flag = 0;
	}

	switch (op)
	{
	case 1:
		command = reinterpret_cast<unsigned char*>(&(turn_right));
		if (!port.WriteData(command, 1))  
			return -1;
		command = reinterpret_cast<unsigned char*>(&(stop));
		if (!port.WriteData(command, 1))
			return -1;
		Sleep(300);
		break;
	case 2:
		command = reinterpret_cast<unsigned char*>(&(turn_left));
		if (!port.WriteData(command, 1))
			return -1;
		command = reinterpret_cast<unsigned char*>(&(stop));
		if (!port.WriteData(command, 1))
			return -1;
		Sleep(300);
		break;
	case 3:
		command = reinterpret_cast<unsigned char*>(&(go_ahead));
		if (!port.WriteData(command, 1))
			return -1;
		command = reinterpret_cast<unsigned char*>(&(stop));
		if (!port.WriteData(command, 1))
			return -1;
		Sleep(100);
		break;
	case 4:
		command = reinterpret_cast<unsigned char*>(&(go_back));
		if (!port.WriteData(command, 1))
			return -1;
		command = reinterpret_cast<unsigned char*>(&(stop));
		if (!port.WriteData(command, 1))
			return -1;
		Sleep(150);
		break;
	case 5:
		command = reinterpret_cast<unsigned char*>(&(stop));
		if (!port.WriteData(command, 1))
			return -1;
		Sleep(100);
		break;
	}
	return flag;
}






