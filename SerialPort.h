//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, 华中科技大学tickTick Group  （版权声明）
/// All rights reserved.
/// 
/// @file    SerialPort.h  
/// @brief   串口通信类头文件
///
/// 本文件蛠E纱谕ㄐ爬嗟纳丒
///
/// @version 1.0   
/// @author  卢俊 
/// @E-mail：lujun.hust@gmail.com
/// @date    2010/03/19
///
///  修订说明：
//////////////////////////////////////////////////////////////////////////

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include <Windows.h>

/** 串口通信纴E
 *   
 *  本类实现了对串口的基本操讈E
 *  例如监听发到指定串口的数据、发送指定数据到串口
 */
class CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);

public:
	
	/** 初始化串口函数
	 *
	 *  @param:  UINT portNo 串口编号,默认值为1,即COM1,注襾E尽量不要大于9
	 *  @param:  UINT baud   波特率,默认为9600
	 *  @param:  char parity 是否进行奇偶校褋E'Y'柄婢需要奇偶校褋E'N'柄婢不需要奇偶校褋E
	 *  @param:  UINT databits 数据位的个数,默认值为8个数据位
	 *  @param:  UINT stopsbits 停止位使用格式,默认值为1
	 *  @param:  DWORD dwCommEvents 默认为EV_RXCHAR,即只要收发任意一个字穪E则产生一个事件
	 *  @return: bool  初始化是否成功
	 *  @note:   在使用其他本类提供的函数前,莵E鹊饔帽竞写诘某跏蓟?
	 *　　　　　 \n本函数提供了一些常用的串口参数设置,若需要自行设置蟻E傅腄CB参数,可使用重载函数
	 *           \n本串口类析构时会自动关闭串口,无需尔殁执行关闭串口
	 *  @see:    
	 */
	bool InitPort( UINT  portNo = 1,UINT  baud = CBR_9600,char  parity = 'N',UINT  databits = 8, 
		           UINT  stopsbits = 1,DWORD dwCommEvents = EV_RXCHAR);

	/** 串口初始化函数
	 *
	 *  本函数提供直接根据DCB参数设置串口参数
	 *  @param:  UINT portNo
	 *  @param:  const LPDCB & plDCB
	 *  @return: bool  初始化是否成功
	 *  @note:   本函数提供用户自定义地串口初始化参数
	 *  @see:    
	 */
	bool InitPort( UINT  portNo ,const LPDCB& plDCB );

	/** 开启监听线程
	 *
	 *  本监听线程蛠E啥源谑莸募嗵?并将接收到的数据打印到屏幕输硜E
	 *  @return: bool  操作是否成功
	 *  @note:   当线程已经处于开启状态时,返回flase
	 *  @see:    
	 */
	bool OpenListenThread();

	/** 关闭监听线程
	 *
	 *  
	 *  @return: bool  操作是否成功
	 *  @note:   调用本函数簛E监听串口的线程将会被关闭
	 *  @see:    
	 */
	bool CloseListenTread();

    /** 向串口写数据
	 *
	 *  将缓冲区中的数据写葋E酱?
	 *  @param:  unsigned char * pData 指向需要写葋E诘氖莼撼迩丒
	 *  @param:  unsigned int length 需要写葋E氖莩ざ?
	 *  @return: bool  操作是否成功
	 *  @note:   length不要大于pData所指向缓冲区的大小
	 *  @see:    
	 */
	bool WriteData(unsigned char* pData, unsigned int length);

	/** 获取串口缓冲区中的字节数
	 *
	 *  
	 *  @return: UINT  操作是否成功
	 *  @note:   当串口缓冲区中无数据时,返回0
	 *  @see:    
	 */
	UINT GetBytesInCOM();

	/** 读取串口接收缓冲区中一个字节的数据
	 *
	 *  
	 *  @param:  char & cRecved 存放读取数据的字符变量
	 *  @return: bool  读取是否成功
	 *  @note:   
	 *  @see:    
	 */
	bool ReadChar(char &cRecved);

private:

	/** 打开串口
	 *
	 *  
	 *  @param:  UINT portNo 串口设备号
	 *  @return: bool  打开是否成功
	 *  @note:   
	 *  @see:    
	 */
	bool openPort( UINT  portNo );

	/** 关闭串口
	 *
	 *  
	 *  @return: void  操作是否成功
	 *  @note:   
	 *  @see:    
	 */
	void ClosePort();
	
	/** 串口监听线程
	 *
	 *  监听来自串口的数据和信息
	 *  @param:  void * pParam 线程参数
	 *  @return: UINT WINAPI 线程返回值
	 *  @note:   
	 *  @see:    
	 */
	static UINT WINAPI ListenThread(void* pParam);

private:

	/** 串口句眮E*/ 
	HANDLE  m_hComm;

	/** 线程退出眮E颈淞?*/ 
	static bool s_bExit;

	/** 线程句眮E*/ 
	volatile HANDLE    m_hListenThread;

	/** 同步互硜E临界区保护 */ 
	CRITICAL_SECTION   m_csCommunicationSync;       //!< 互斥操作串口

};

#endif //SERIALPORT_H_