#pragma once
#include <unordered_map>
#include "Ringbuffer.h"
//#include "Stack.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"
#include "ServerCommon.h"

// �ش� ������ �۽��߿� �ִ��� �ƴ���
#define NONSENDING	0
#define SENDING		1
// Recv / Send Post ��ȯ ��
#define POST_RETVAL_ERR_SESSION_DELETED		0
#define POST_RETVAL_ERR						1
#define POST_RETVAL_COMPLETE				2

#define ONE_SEND_WSABUF_MAX					200

#define df_RELEASE_VALUE					0x100000000

struct st_Error;

class CSerializationBuf;

class CLanClient
{
private:
	BYTE		m_byNumOfWorkerThread;
	BYTE		m_byNumOfUsingWorkerThread;
	BOOL		m_bIsNagleOn;
	WORD		m_wNumOfReconnect;
	WORD		m_wPort;
	SOCKET		m_sock;

	WCHAR		m_IP[16];

	HANDLE		*m_pWorkerThreadHandle;
	HANDLE		m_hWorkerIOCP;

	struct OVERLAPPEDIODATA
	{
		WORD		wBufferCount;
		OVERLAPPED  Overlapped;
		CRingbuffer RingBuffer;
	};

	struct OVERLAPPED_SEND_IO_DATA
	{
		LONG										lBufferCount;
		UINT										IOMode;
		OVERLAPPED									Overlapped;
		CLockFreeQueue<CSerializationBuf*>			SendQ;
	};

	UINT						m_IOCount;
	UINT64						m_SessionID;
	OVERLAPPEDIODATA			m_RecvIOData;
	OVERLAPPED_SEND_IO_DATA		m_SendIOData;

	CSerializationBuf*	pSeirializeBufStore[ONE_SEND_WSABUF_MAX];

	char RecvPost();
	char SendPost();

	UINT Worker();
	static UINT __stdcall WorkerThread(LPVOID pLanClient);

	bool LanClientOptionParsing(const WCHAR *szOptionFileName);

	bool ReleaseSession();

public:
	CLanClient();
	virtual ~CLanClient();

	bool Start(const WCHAR *szOptionFileName);
	void Stop();

	bool SendPacket(CSerializationBuf *pSerializeBuf);
	// ������ Connect �� �Ϸ� �� ��
	virtual void OnConnectionComplete() = 0;

	// ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(CSerializationBuf *OutReadBuf) = 0;
	// ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend() = 0;

	// ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadBegin() = 0;
	// ��Ŀ������ 1���� ���� ��
	virtual void OnWorkerThreadEnd() = 0;
	// ����� ���� ó�� �Լ�
	virtual void OnError(st_Error *OutError) = 0;
};
