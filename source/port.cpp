#include <iostream>
#include <string>
#include"SerialPort.h"

using namespace std;

#define PortNum 4 

int main()
{
	CSerialPort port;
	if (!port.InitPort(PortNum))
	{
		cout << "initPort fail!" << endl;
		return 0;
	}
	unsigned char *data;
	char a = 'l';
	data = reinterpret_cast<unsigned char*>(&a);
	if (!port.WriteData(data, 1))
		cout << "write fail" << endl;
}
