#pragma once
#include <unordered_map>
#include "Ringbuffer.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"
#include "ServerCommon.h"

// �ش� ������ �۽��߿� �ִ��� �ƴ���
#define NONSENDING	0
#define SENDING		1
// ���� ID �� ���ؼ� ���� �ε����� ã�� ���� ����Ʈ ��
#define SESSION_INDEX_SHIFT 48
// Recv / Send Post ��ȯ ��
#define POST_RETVAL_ERR_SESSION_DELETED		0
#define POST_RETVAL_ERR						1
#define POST_RETVAL_COMPLETE				2

#define ONE_SEND_WSABUF_MAX					200

#define df_RELEASE_VALUE					0x100000000

struct st_Error;

class CNetServerSerializationBuf;

class CNetClient
{
private:
	BYTE		m_byNumOfWorkerThread;
	BYTE		m_byNumOfUsingWorkerThread;
	BOOL		m_bIsNagleOn;
	WORD		m_wPort;
	UINT		m_uiNumOfUser;
	UINT		m_uiMaxClient;
	UINT		m_uiAcceptTotal;
	UINT		m_uiAcceptTPS;

	UINT64		m_iIDCount;
	WCHAR		m_IP[16];

	HANDLE		*m_pWorkerThreadHandle;
	HANDLE		m_hAcceptThread;
	HANDLE		m_hWorkerIOCP;

	struct OVERLAPPEDIODATA
	{
		WORD		wBufferCount;
		UINT		IOMode;
		OVERLAPPED  Overlapped;
		CRingbuffer RingBuffer;
	};

	struct OVERLAPPED_SEND_IO_DATA
	{
		LONG											lBufferCount;
		UINT											IOMode;
		OVERLAPPED										Overlapped;

		CLockFreeQueue<CNetServerSerializationBuf*>		SendQ;
	};

	WORD						m_wNumOfReconnect;
	UINT						m_IOCount;
	SOCKET						m_sock;
	UINT64						m_SessionID;
	OVERLAPPEDIODATA			m_RecvIOData;
	OVERLAPPED_SEND_IO_DATA		m_SendIOData;

	CNetServerSerializationBuf*	m_pSeirializeBufStore[ONE_SEND_WSABUF_MAX];

	bool ReleaseSession();

	char RecvPost();
	char SendPost();

	UINT Worker();
	static UINT __stdcall WorkerThread(LPVOID pNetServer);

	bool NetServerOptionParsing(const WCHAR *szOptionFileName);

	bool SessionAcquireLock();
	void SessionAcquireUnLock();

protected:
	bool Start(const WCHAR *szOptionFileName);
	void Stop();

public:
	CNetClient();
	virtual ~CNetClient();

	bool SendPacket(CNetServerSerializationBuf *pSerializeBuf);

	// ������ Connect �� �Ϸ� �� ��
	virtual void OnConnectionComplete() = 0;

	// ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(CNetServerSerializationBuf *OutReadBuf) = 0;
	// ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend(int sendsize) = 0;

	// ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadBegin() = 0;
	// ��Ŀ������ 1���� ���� ��
	virtual void OnWorkerThreadEnd() = 0;
	// ����� ���� ó�� �Լ�
	virtual void OnError(st_Error *OutError) = 0;

	int checksum = 0;
	int payloadOver = 0;
	int HeaderCode = 0;
};
