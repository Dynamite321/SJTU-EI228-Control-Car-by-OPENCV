//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, ���пƼ���ѧtickTick Group  ����Ȩ������
/// All rights reserved.
/// 
/// @file    SerialPort.h  
/// @brief   ����ͨ����ͷ�ļ�
///
/// ���ļ�́Eɴ���ͨ�������ÁE
///
/// @version 1.0   
/// @author  ¬�� 
/// @E-mail��lujun.hust@gmail.com
/// @date    2010/03/19
///
///  �޶�˵����
//////////////////////////////////////////////////////////////////////////

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#include <Windows.h>

/** ����ͨ����E
 *   
 *  ����ʵ���˶Դ��ڵĻ�����ׁE
 *  �����������ָ�����ڵ����ݡ�����ָ�����ݵ�����
 */
class CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);

public:
	
	/** ��ʼ�����ں���
	 *
	 *  @param:  UINT portNo ���ڱ��,Ĭ��ֵΪ1,��COM1,עҁE������Ҫ����9
	 *  @param:  UINT baud   ������,Ĭ��Ϊ9600
	 *  @param:  char parity �Ƿ������żУсE'Y'�����Ҫ��żУсE'N'��澲���Ҫ��żУсE
	 *  @param:  UINT databits ����λ�ĸ���,Ĭ��ֵΪ8������λ
	 *  @param:  UINT stopsbits ֹͣλʹ�ø�ʽ,Ĭ��ֵΪ1
	 *  @param:  DWORD dwCommEvents Ĭ��ΪEV_RXCHAR,��ֻҪ�շ�����һ���ַ�E�����һ���¼�
	 *  @return: bool  ��ʼ���Ƿ�ɹ�
	 *  @note:   ��ʹ�����������ṩ�ĺ���ǰ,ǁEȵ��ñ��������д��ڵĳ�ʼ�?
	 *���������� \n�������ṩ��һЩ���õĴ��ڲ�������,����Ҫ��������ρE���DCB����,��ʹ�����غ���
	 *           \n������������ʱ���Զ��رմ���,�������ִ�йرմ���
	 *  @see:    
	 */
	bool InitPort( UINT  portNo = 1,UINT  baud = CBR_9600,char  parity = 'N',UINT  databits = 8, 
		           UINT  stopsbits = 1,DWORD dwCommEvents = EV_RXCHAR);

	/** ���ڳ�ʼ������
	 *
	 *  �������ṩֱ�Ӹ���DCB�������ô��ڲ���
	 *  @param:  UINT portNo
	 *  @param:  const LPDCB & plDCB
	 *  @return: bool  ��ʼ���Ƿ�ɹ�
	 *  @note:   �������ṩ�û��Զ���ش��ڳ�ʼ������
	 *  @see:    
	 */
	bool InitPort( UINT  portNo ,const LPDCB& plDCB );

	/** ���������߳�
	 *
	 *  �������߳�́EɶԴ������ݵļ��?�������յ������ݴ�ӡ����Ļ�䳁E
	 *  @return: bool  �����Ƿ�ɹ�
	 *  @note:   ���߳��Ѿ����ڿ���״̬ʱ,����flase
	 *  @see:    
	 */
	bool OpenListenThread();

	/** �رռ����߳�
	 *
	 *  
	 *  @return: bool  �����Ƿ�ɹ�
	 *  @note:   ���ñ�������E�������ڵ��߳̽��ᱻ�ر�
	 *  @see:    
	 */
	bool CloseListenTread();

    /** �򴮿�д����
	 *
	 *  ���������е�����дȁE����?
	 *  @param:  unsigned char * pData ָ����ҪдȁE��ڵ����ݻ���ǁE
	 *  @param:  unsigned int length ��ҪдȁE����ݳ��?
	 *  @return: bool  �����Ƿ�ɹ�
	 *  @note:   length��Ҫ����pData��ָ�򻺳����Ĵ�С
	 *  @see:    
	 */
	bool WriteData(unsigned char* pData, unsigned int length);

	/** ��ȡ���ڻ������е��ֽ���
	 *
	 *  
	 *  @return: UINT  �����Ƿ�ɹ�
	 *  @note:   �����ڻ�������������ʱ,����0
	 *  @see:    
	 */
	UINT GetBytesInCOM();

	/** ��ȡ���ڽ��ջ�������һ���ֽڵ�����
	 *
	 *  
	 *  @param:  char & cRecved ��Ŷ�ȡ���ݵ��ַ�����
	 *  @return: bool  ��ȡ�Ƿ�ɹ�
	 *  @note:   
	 *  @see:    
	 */
	bool ReadChar(char &cRecved);

private:

	/** �򿪴���
	 *
	 *  
	 *  @param:  UINT portNo �����豸��
	 *  @return: bool  ���Ƿ�ɹ�
	 *  @note:   
	 *  @see:    
	 */
	bool openPort( UINT  portNo );

	/** �رմ���
	 *
	 *  
	 *  @return: void  �����Ƿ�ɹ�
	 *  @note:   
	 *  @see:    
	 */
	void ClosePort();
	
	/** ���ڼ����߳�
	 *
	 *  �������Դ��ڵ����ݺ���Ϣ
	 *  @param:  void * pParam �̲߳���
	 *  @return: UINT WINAPI �̷߳���ֵ
	 *  @note:   
	 *  @see:    
	 */
	static UINT WINAPI ListenThread(void* pParam);

private:

	/** ���ھ䱁E*/ 
	HANDLE  m_hComm;

	/** �߳��˳���E����?*/ 
	static bool s_bExit;

	/** �߳̾䱁E*/ 
	volatile HANDLE    m_hListenThread;

	/** ͬ������E�ٽ������� */ 
	CRITICAL_SECTION   m_csCommunicationSync;       //!< �����������

};

#endif //SERIALPORT_H_