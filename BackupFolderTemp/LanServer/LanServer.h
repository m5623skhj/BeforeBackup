#pragma once
#include <WinSock2.h>
#include <unordered_map>
#include "Ringbuffer.h"
//#include "Stack.h"
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

#define df_LANSERVER_HEADER_SIZE			2

//////////////////////////////////////////////////////////////////////////////////////
// LANSERVER_ERR
// CLanServer ������ �߻��ϴ� ���� �޽������� ����
// ���ο� ������ �ʿ��� ��� �߰��� ��
//////////////////////////////////////////////////////////////////////////////////////
//enum LANSERVER_ERR
//{
//	NO_ERR = 0,
//	WSASTARTUP_ERR,
//	LISTEN_SOCKET_ERR,
//	LISTEN_BIND_ERR,
//	LISTEN_LISTEN_ERR,
//	BEGINTHREAD_ERR,
//	SETSOCKOPT_ERR,
//	WORKERIOCP_NULL_ERR,
//	SESSION_NULL_ERR,
//	ACCEPT_ERR,
//	WSARECV_ERR,
//	WSASEND_ERR,
//	OVERLAPPED_NULL_ERR,
//	SERIALIZEBUF_NULL_ERR,
//	RINGBUFFER_MAX_SIZE_ERR,
//	RINGBUFFER_MIN_SIZE_ERR,
//	INCORRECT_SESSION,
//	END_OF_ERR
//};

struct st_Error;

class CSerializationBuf;

class CLanServer
{
private:
	BYTE		m_byNumOfWorkerThread;
	BYTE		m_byNumOfUsingWorkerThread;
	BOOL		m_bIsNagleOn;
	WORD		m_wPort;
	SOCKET		m_ListenSock;
	UINT		m_uiNumOfUser;
	UINT		m_uiMaxClient;

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
		LONG										lBufferCount;
		UINT										IOMode;
		OVERLAPPED									Overlapped;
		CLockFreeQueue<CSerializationBuf*>			SendQ;
	};

	struct Session
	{
		UINT						IOCount;
		UINT						IsUseSession;
		SOCKET						sock;
		UINT64						SessionID;
		OVERLAPPEDIODATA			RecvIOData;
		OVERLAPPED_SEND_IO_DATA		SendIOData;

		CSerializationBuf*			pSeirializeBufStore[ONE_SEND_WSABUF_MAX];
	};

	Session					*m_pSessionArray;
	CLockFreeStack<WORD>	*m_pSessionIndexStack;

	bool ReleaseSession(Session *pSession);

	char RecvPost(Session *pSession);
	char SendPost(Session *pSession);

	UINT Accepter();
	UINT Worker();
	static UINT __stdcall AcceptThread(LPVOID pLanServer);
	static UINT __stdcall WorkerThread(LPVOID pLanServer);
protected:

public:
	CLanServer();
	virtual ~CLanServer();

	bool Start(const WCHAR *szOptionFileName);
	void Stop();

	bool LanServerOptionParsing(const WCHAR *szOptionFileName);

	bool DisConnect(UINT64 SessionID);
	bool SendPacket(UINT64 SessionID, CSerializationBuf *pSerializeBuf);

	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 OutClientID/* Client ���� / ClientID / ��Ÿ��� */) = 0;
	// Disconnect �� ȣ��
	virtual void OnClientLeave(UINT64 ClientID) = 0;
	// Accept ���� IP ���ܵ��� ���� �뵵
	virtual bool OnConnectionRequest() = 0;

	// ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf) = 0;
	// ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend(UINT64 ClientID, int sendsize) = 0;

	// ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadBegin() = 0;
	// ��Ŀ������ 1���� ���� ��
	virtual void OnWorkerThreadEnd() = 0;
	// ����� ���� ó�� �Լ�
	virtual void OnError(st_Error *OutError) = 0;

	UINT GetNumOfUser();
	UINT GetStackRestSize();

	//LONG g_ULLConuntOfNew = 0;
	//LONG g_ULLConuntOfDel = 0;
};
