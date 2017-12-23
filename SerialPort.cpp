//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, ���пƼ���ѧtickTick Group  ����Ȩ������
/// All rights reserved.
/// 
/// @file    SerialPort.cpp  
/// @brief   ����ͨ�����ʵ���ļ�
///
/// ���ļ�Ϊ����ͨ�����ʵ�ִ�E
///
/// @version 1.0   
/// @author  ¬��  
/// @E-mail��lujun.hust@gmail.com
/// @date    2010/03/19
/// 
///
///  �޶�˵����
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerialPort.h"
#include <process.h>
#include <iostream>

/** �߳��˳���E?*/ 
bool CSerialPort::s_bExit = false;
/** ������������ʱ,sleep���´β�ѯ�����ʱ��E��λ:ÁE*/ 
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

	/** ��ʱ����,���ƶ�����ת��Ϊ�ַ�����ʽ,�Թ�ԁECB�ṹ */ 
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,����ǁE�Ҫ�ӱ��?*/ 
	if (!openPort(portNo))
	{
		return false;
	}

	/** ��ȁEٽ�?*/ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ƿ��д���ɁE*/ 
	BOOL bIsSuccess = TRUE;

    /** �ڴ˿���������ȁE���Ļ��������?���������,��ϵͳ������Ĭ��ֵ.
	 *  �Լ����û�������Сʱ,Ҫע�������Դ�һЩ,���⻺�����糁E
	 */
	/*if (bIsSuccess )
	{
		bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** ���ô��ڵĳ�ʱʱ��E����Ϊ0,��澲�ʹ�ó�ʱ���� */
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
		// ��ANSI�ַ���ת��ΪUNICODE�ַ���
		DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, szDCBparam, -1, NULL, 0);
		wchar_t *pwText = new wchar_t[dwNum] ;
		if (!MultiByteToWideChar (CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
		{
			bIsSuccess = TRUE;
		}

		/** ��ȡ��ǰ�������ò���,���ҹ�ԁE���DCB���� */ 
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb) ;
		/** ����RTS flow���� */ 
		dcb.fRtsControl = RTS_CONTROL_ENABLE; 

		/** �ͷ��ڴ�ռ�E*/ 
		delete [] pwText;
	}

	if ( bIsSuccess )
	{
		/** ʹ��DCB�������ô���״̬ */ 
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}
		
	/**  ��մ��ڻ���ǁE*/
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** ��E��ٽ�?*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return bIsSuccess==TRUE;
}

bool CSerialPort::InitPort( UINT portNo ,const LPDCB& plDCB )
{
	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,����ǁE�Ҫ�ӱ��?*/ 
	if (!openPort(portNo))
	{
		return false;
	}
	
	/** ��ȁEٽ�?*/ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** ���ô��ڲ��� */ 
	if (!SetCommState(m_hComm, plDCB))
	{
		return false;
	}

	/**  ��մ��ڻ���ǁE*/
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** ��E��ٽ�?*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

void CSerialPort::ClosePort()
{
	/** ����д��ڱ��򿪣��ر�ˁE*/
	if( m_hComm != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hComm );
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

bool CSerialPort::openPort( UINT portNo )
{
	/** ��ȁEٽ�?*/ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ѵ��ڵı��ת��Ϊ�豸ÁE*/ 
    char szPort[50];
	sprintf_s(szPort, "COM%d", portNo);

	/** ��ָ���Ĵ��� */ 
	m_hComm = CreateFileA(szPort,		                /** �豸ÁECOM1,COM2�� */ 
						 GENERIC_READ | GENERIC_WRITE,  /** ����ģʽ,��ͬʱ��д */ 
						 0,                             /** �����ʽ,0��澲���ρE*/ 
					     NULL,							/** ��ȫ������,һ��ʹ��NULL */ 
					     OPEN_EXISTING,					/** �ò�������豸��ЁE��?���򴴽�ʧ�� */ 
						 0,    
						 0);    

	/** �����ʧ�ܣ��ͷ���Դ������ */ 
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** �˳��ٽ�ǁE*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

bool CSerialPort::OpenListenThread()
{
	/** ��E��߳��Ƿ��Ѿ������?*/ 
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** �߳��Ѿ���ƁE*/ 
		return false;
	}

	s_bExit = false;
	/** �߳�ID */ 
	UINT threadId;
	/** �����������ݼ����߳� */ 
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread)
	{
		return false;
	}
	/** �����̵߳����ȼ�,������ͨ�߳� */ 
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
		/** ֪ͨ�߳��˳�E*/ 
		s_bExit = true;

		/** �ȴ��߳��˳�E*/ 
		Sleep(10);

		/** ���߳̾����Ч */ 
		CloseHandle( m_hListenThread );
		m_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}

UINT CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;	/** ����E*/ 
	COMSTAT  comstat;   /** COMSTAT�ṹ́E��¼ͨ���豸��״̬��Ϣ */ 
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
	/** �ڵ���ReadFile��WriteFile֮ǰ,ͨ�������������ǰ�����Ĵ���E?*/ 
	if ( ClearCommError(m_hComm, &dwError, &comstat) )
	{
		BytesInQue = comstat.cbInQue; /** ��ȡ����ȁE������е��ֽ��?*/ 
	}

	return BytesInQue;
}

UINT WINAPI CSerialPort::ListenThread( void* pParam )
{
	/** �õ������ָՁE*/ 
	CSerialPort *pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	// �߳�ѭ��,��ѯ��ʽ��ȡ��������
	while (!pSerialPort->s_bExit) 
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();
		/** ���������ȁE������������?����Ϣһ���ٲ�ѯ */ 
		if ( BytesInQue == 0 )
		{
			Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}

		/** ��ȡ��ȁE������е����ݲ�������?*/
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

	/** �ٽ������� */ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** �ӻ�������ȡһ���ֽڵ����� */ 
	bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, NULL);
	if ((!bResult))
	{ 
		/** ��ȡ����E���Ը��ݸô���E������ԭҁE*/ 
		DWORD dwError = GetLastError();

		/** ��մ��ڻ���ǁE*/ 
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** ��E��ٽ�ǁE*/ 
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

	/** �ٽ������� */ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** �򻺳���дȁE����������?*/ 
	bResult = WriteFile(m_hComm, pData, length, &BytesToSend, NULL);
	if (!bResult)  
	{
		DWORD dwError = GetLastError();
		/** ��մ��ڻ���ǁE*/ 
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** ��E��ٽ�ǁE*/ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}


