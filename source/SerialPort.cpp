//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, 华中科技大学tickTick Group  （版权声明）
/// All rights reserved.
/// 
/// @file    SerialPort.cpp  
/// @brief   串口通信类的实现文件
///
/// 本文件为串口通信类的实现代聛E
///
/// @version 1.0   
/// @author  卢俊  
/// @E-mail：lujun.hust@gmail.com
/// @date    2010/03/19
/// 
///
///  修订说明：
//////////////////////////////////////////////////////////////////////////

#include "SerialPort.h"
#include <process.h>
#include <iostream>

/** 线程退出眮E?*/ 
bool CSerialPort::s_bExit = false;
/** 当串口无数据时,sleep至下次查询间隔的时紒E单位:脕E*/ 
const UINT SLEEP_TIME_INTERVAL = 5;

CSerialPort::CSerialPort(void)
: m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hListenThread = INVALID_HANDLE_VALUE;

	InitializeCriticalSection(&m_csCommunicationSync);

}

CSerialPort::~CSerialPort(void)
{
	CloseListenTread();
	ClosePort();
	DeleteCriticalSection(&m_csCommunicationSync);
}

bool CSerialPort::InitPort( UINT portNo /*= 1*/,UINT baud /*= CBR_9600*/,char parity /*= 'N'*/,
						    UINT databits /*= 8*/, UINT stopsbits /*= 1*/,DWORD dwCommEvents /*= EV_RXCHAR*/ )
{

	/** 临时变量,将制定参数转化为字符串形式,以构詠ECB结构 */ 
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

	/** 打开指定串口,该函数内部已经有临界区保护,上面莵E灰颖；?*/ 
	if (!openPort(portNo))
	{
		return false;
	}

	/** 进葋E俳缍?*/ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** 是否有代牦发蓙E*/ 
	BOOL bIsSuccess = TRUE;

    /** 在此可以设置输葋E涑龅幕撼迩笮?如果不设置,则系统会设置默认值.
	 *  自己设置缓冲区大小时,要注意设置稍大一些,避免缓冲区溢硜E
	 */
	/*if (bIsSuccess )
	{
		bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** 设置串口的超时时紒E均设为0,柄婢不使用超时限制 */
	COMMTIMEOUTS  CommTimeouts;
	CommTimeouts.ReadIntervalTimeout         = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier  = 0;
	CommTimeouts.ReadTotalTimeoutConstant    = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	CommTimeouts.WriteTotalTimeoutConstant   = 0; 
	if ( bIsSuccess)
	{
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}

	DCB  dcb;
	if ( bIsSuccess )
	{
		// 将ANSI字符串转换为UNICODE字符串
		DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, szDCBparam, -1, NULL, 0);
		wchar_t *pwText = new wchar_t[dwNum] ;
		if (!MultiByteToWideChar (CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
		{
			bIsSuccess = TRUE;
		}

		/** 获取当前串口配置参数,并且构詠E贒CB参数 */ 
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb) ;
		/** 开启RTS flow控制 */ 
		dcb.fRtsControl = RTS_CONTROL_ENABLE; 

		/** 释放内存空紒E*/ 
		delete [] pwText;
	}

	if ( bIsSuccess )
	{
		/** 使用DCB参数配置串口状态 */ 
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}
		
	/**  清空串口缓冲莵E*/
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** 纴E俳缍?*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return bIsSuccess==TRUE;
}

bool CSerialPort::InitPort( UINT portNo ,const LPDCB& plDCB )
{
	/** 打开指定串口,该函数内部已经有临界区保护,上面莵E灰颖；?*/ 
	if (!openPort(portNo))
	{
		return false;
	}
	
	/** 进葋E俳缍?*/ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** 配置串口参数 */ 
	if (!SetCommState(m_hComm, plDCB))
	{
		return false;
	}

	/**  清空串口缓冲莵E*/
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** 纴E俳缍?*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

void CSerialPort::ClosePort()
{
	/** 如果有串口被打开，关闭藖E*/
	if( m_hComm != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hComm );
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

bool CSerialPort::openPort( UINT portNo )
{
	/** 进葋E俳缍?*/ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** 把串口的编号转换为设备脕E*/ 
    char szPort[50];
	sprintf_s(szPort, "COM%d", portNo);

	/** 打开指定的串口 */ 
	m_hComm = CreateFileA(szPort,		                /** 设备脕ECOM1,COM2等 */ 
						 GENERIC_READ | GENERIC_WRITE,  /** 访问模式,可同时读写 */ 
						 0,                             /** 共销啵式,0柄婢不共蟻E*/ 
					     NULL,							/** 安全性设置,一般使用NULL */ 
					     OPEN_EXISTING,					/** 该参数柄婢设备必衼E嬖?否则创建失败 */ 
						 0,    
						 0);    

	/** 如果打开失败，释放资源并返回 */ 
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** 退出临界莵E*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

bool CSerialPort::OpenListenThread()
{
	/** 紒E庀叱淌欠褚丫袅?*/ 
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** 线程已经开苼E*/ 
		return false;
	}

	s_bExit = false;
	/** 线程ID */ 
	UINT threadId;
	/** 开启串口数据监听线程 */ 
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread)
	{
		return false;
	}
	/** 设置线程的优先级,高于普通线程 */ 
	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL))
	{
		return false;
	}

	return true;
}

bool CSerialPort::CloseListenTread()
{	
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** 通知线程退硜E*/ 
		s_bExit = true;

		/** 等待线程退硜E*/ 
		Sleep(10);

		/** 置线程句柄无效 */ 
		CloseHandle( m_hListenThread );
		m_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}

UINT CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;	/** 代牦聛E*/ 
	COMSTAT  comstat;   /** COMSTAT结构虂E记录通信设备的状态信息 */ 
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
	/** 在调用ReadFile和WriteFile之前,通过本函数清除以前遗留的代牦眮E?*/ 
	if ( ClearCommError(m_hComm, &dwError, &comstat) )
	{
		BytesInQue = comstat.cbInQue; /** 获取在输葋E撼迩械淖纸谑?*/ 
	}

	return BytesInQue;
}

UINT WINAPI CSerialPort::ListenThread( void* pParam )
{
	/** 得到本类的指諄E*/ 
	CSerialPort *pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	// 线程循环,轮询方式读取串口数据
	while (!pSerialPort->s_bExit) 
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();
		/** 如果串口输葋E撼迩形奘?则休息一会再查询 */ 
		if ( BytesInQue == 0 )
		{
			Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}

		/** 读取输葋E撼迩械氖莶⑹涑鱿允?*/
		char cRecved = 0x00;
		do
		{
			cRecved = 0x00;
			if(pSerialPort->ReadChar(cRecved) == true)
			{
				std::cout << cRecved ; 
				continue;
			}
		}while(--BytesInQue);
	}

	return 0;
}

bool CSerialPort::ReadChar( char &cRecved )
{
	BOOL  bResult     = TRUE;
	DWORD BytesRead   = 0;
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/** 临界区保护 */ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** 从缓冲区读取一个字节的数据 */ 
	bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, NULL);
	if ((!bResult))
	{ 
		/** 获取代牦聛E可以根据该代牦聛E槌龃笤丒*/ 
		DWORD dwError = GetLastError();

		/** 清空串口缓冲莵E*/ 
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** 纴E俳缜丒*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return (BytesRead == 1);

}

bool CSerialPort::WriteData( unsigned char* pData, unsigned int length )
{
	BOOL   bResult     = TRUE;
	DWORD  BytesToSend = 0;
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/** 临界区保护 */ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** 向缓冲区写葋E付康氖?*/ 
	bResult = WriteFile(m_hComm, pData, length, &BytesToSend, NULL);
	if (!bResult)  
	{
		DWORD dwError = GetLastError();
		/** 清空串口缓冲莵E*/ 
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** 纴E俳缜丒*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}


