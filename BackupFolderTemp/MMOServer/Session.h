#pragma once
#include "Ringbuffer.h"
#include "LockFreeQueue.h"
#include "Queue.h"

// �ش� ������ �۽����� �ƴ�
#define NONSENDING	0
// �ش� ������ �۽�����
#define SENDING		1

// WSASend �� ���� ���� �� �ִ� �ִ� ��
#define dfONE_SEND_WSABUF_MAX									500

class CNetServerSerializationBuf;
class CMMOServer;

///////////////////////////////////////////////////
// �� ���̺귯���� �̿��Ϸ��� �ش� Ŭ������ ��ӹ��� �迭�� �̸� �Ҵ��ϰ�
// �Ҵ�� �迭�� ������ MMOServer �� LinkSession �Լ��� ȣ���Ͽ�
// MMOServer �� ���������߸� ��
///////////////////////////////////////////////////
class CSession
{
public :
	// �ش� �������� ��Ŷ�� ������
	void SendPacket(CNetServerSerializationBuf *pSendPacket);
	// �ش� ������ ������ ����
	void DisConnect();
	// �ش� ������ ���¸� ���� ���¿��� ���� ���·� ������
	// �� �Լ��� ȣ�� ���Ѿ� ������ ��μ� ���ӿ� ������
	bool ChangeMode_AuthToGame();

protected :
	CSession();
	virtual ~CSession();

	// ���� �����忡 ������ ������
	virtual void OnAuth_ClientJoin() = 0;
	// ���� �����忡�� ������ ����
	virtual void OnAuth_ClientLeave(bool IsClientLeaveServer) = 0;
	// ���� �����忡 �ִ� ������ ��Ŷ�� ����
	virtual void OnAuth_Packet(CNetServerSerializationBuf *pRecvPacket) = 0;
	// ���� �����忡 ������ ������
	virtual void OnGame_ClientJoin() = 0;
	// ���� �����忡 ������ ����
	virtual void OnGame_ClientLeave() = 0;
	// ���� �����忡 �ִ� ������ ��Ŷ�� ����
	virtual void OnGame_Packet(CNetServerSerializationBuf *pRecvPacket) = 0;
	// ���������� ������ ������ ��
	virtual void OnGame_ClientRelease() = 0;

private :
	// ���� �ʱ�ȭ �Լ�
	void SessionInit();

private :
	friend CMMOServer;

	// ������ ���� � ��������� ���� ǥ��
	enum en_SESSION_MODE
	{
		MODE_NONE = 0,			 // ���� �̻��
		MODE_AUTH,				 // ���� �� ������� ����
		MODE_AUTH_TO_GAME,		 // ���� ���� ��ȯ
		MODE_GAME,				 // ���� �� ���Ӹ�� ����
		MODE_LOGOUT_IN_AUTH, 	 // �����غ�
		MODE_LOGOUT_IN_GAME,	 // �����غ�
		MODE_WAIT_LOGOUT		 // ���� ����
	};

	// CMMOServer ���� �������� ������
	// �� ���ǵ��� �� ����ü�� �����͸� �� �Ʒ��� �������� ������
	struct AcceptUserInfo
	{
		WORD												wSessionIndex;
		WORD												wAcceptedPort;
		SOCKET												AcceptedSock;
		WCHAR												AcceptedIP[16];
	};

	struct st_RECV_IO_DATA
	{
		UINT												uiBufferCount;
		OVERLAPPED											RecvOverlapped;
		CRingbuffer											RecvRingBuffer;
	};

	struct st_SEND_IO_DATA
	{
		UINT												uiBufferCount;
		OVERLAPPED											SendOverlapped;
		//CLockFreeQueue<CNetServerSerializationBuf*>			SendQ;
		CListBaseQueue<CNetServerSerializationBuf*>			SendQ;
	};

	// Auth �����忡�� Game ������� �Ѿ �� ���� �� �ִ� ��
	// �ܺ��� ȣ�⿡ ���Ͽ� �����
	bool													m_bAuthToGame;
	// m_uiIOCount �� 0�� �Ǿ��� �� true �� ��
	// �ش� ���� true �� �Ǹ� Game �����忡�� ���������� ������ �����Ѵ�
	bool													m_bLogOutFlag;
	// �ش� ������ ��� ���¿� �ִ����� ���� ��
	// �� ���µ��� en_SESSION_MODE �� ���� �Ǿ�����
	BYTE													m_byNowMode;
	// �ش� ������ Send ������ �ƴ����� ���� ��
	UINT													m_uiSendIOMode;
	// �ش� ������ IO Count
	// 0�� �Ǹ� m_bLogOutFlag �� true �� ��
	UINT													m_uiIOCount;
	// �ش� ������ ���� �ĺ���
	UINT64													m_uiSessionID;
	// �ش� ������ Socket
	// m_pUserNetworkInfo �� ���ԵǾ��ִ� ���̳�
	// ������ �����Ƿ� ���� ��������
	SOCKET													m_Socket;
	// Recv �� �Ϸ�Ǹ� �ش� ť�� ���� ����ְ� �ǰ�
	// ���� Auth Ȥ�� Game �����尡 �� ť���� ��Ŷ�� �̾�
	// ����ڿ��� ó���� �ѱ�
	CListBaseQueue<CNetServerSerializationBuf*>				m_RecvCompleteQueue;

	AcceptUserInfo											*m_pUserNetworkInfo;
	st_RECV_IO_DATA											m_RecvIOData;
	st_SEND_IO_DATA											m_SendIOData;

	// WSASend �� ����ȭ ������ ������ ���Ͽ� �� �����͸� �־�� ����
	CNetServerSerializationBuf*								pSeirializeBufStore[dfONE_SEND_WSABUF_MAX];
};