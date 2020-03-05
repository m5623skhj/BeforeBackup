#pragma once
#include "Session.h"
#include "LockFreeStack.h"
#include "LockFreeQueue.h"

// Post �Լ��� ������ ���Ͽ� ������ �̹� ���� ó����
#define POST_RETVAL_ERR_SESSION_DELETED		0
// Post �Լ����� ������ �߻��Ͽ��� ������ ���� ����ִ� ������
#define POST_RETVAL_ERR						1
// Post �Լ��� ���� ���� ó���Ǿ���
#define POST_RETVAL_COMPLETE				2

#define dfNUM_OF_THREAD						5

struct st_Error;

class CNetServerSerializationBuf;

class CMMOServer
{
public :
	CMMOServer();
	virtual ~CMMOServer();

	UINT GetAcceptTPSAndReset();
	UINT GetRecvTPSAndReset();
	UINT GetSendTPSAndReset();
	UINT GetNumOfUserInfoAlloc()
	{
		return m_pUserInfoMemoryPool->GetAllocChunkCount();
	}
	UINT GetUserInfoUse()
	{
		return m_pUserInfoMemoryPool->GetUseNodeCount();
	}

	UINT GetSendThreadFPSAndReset();
	UINT GetAuthThreadFPSAndReset();
	UINT GetGameThreadFPSAndReset();

	UINT *GetAuthFPSPtr() { return &AuthThreadLoop; }
	UINT *GetGameFPSPtr() { return &GameThreadLoop; }
	UINT *GetNumOfAllSession() { return (UINT*)&NumOfTotalPlayer; }
	UINT *GetNumOfAuthSession() { return &NumOfAuthPlayer; }
	UINT *GetNumOfGameSession() { return &NumOfGamePlayer; }
	UINT *GetNumOfWaitRoom() { return NULL; }
	UINT *GetNumOfPlayRoom() { return NULL; }

	int GetRecvCompleteQueueTotalNodeCount();

public :
	UINT						NumOfAuthPlayer;
	UINT						NumOfGamePlayer;
	UINT64						NumOfTotalPlayer;

	UINT64						NumOfTotalAccept;

	UINT						RecvTPS;
	UINT						SendTPS;
	UINT						AcceptTPS;

	UINT						SendThreadLoop;
	UINT						AuthThreadLoop;
	UINT						GameThreadLoop;
	
protected :
	bool Start(const WCHAR *szMMOServerOptionFileName);
	void Stop();

	/////////////////////////////////////////////
	// MMOServer Start �Լ� ȣ�� ������ �ݵ�� ȣ�� �� �����
	// ���ǰ��� ������ ���ؼ� ���� �������� �Լ��� ȣ���ؾ� ��
	// InitMMOServer -> �� �÷��̾�� LinkSession -> CMMOServer::Start
	/////////////////////////////////////////////
	bool InitMMOServer(UINT NumOfMaxClient);

	// ���� �������� �� �����Ӹ��� ȣ���
	virtual void OnAuth_Update() = 0;
	// ������Ʈ �������� �� �����Ӹ��� ȣ���
	virtual void OnGame_Update() = 0;
	// ���� ��Ȳ �߻��� ȣ�� ��
	virtual void OnError(st_Error *pError) = 0;

	////////////////////////////////////////////////////////////////////
	// �ٱ����� �̸� Session �� ��ӹ��� ��ü�� ������� �ִ� ���¿���
	// MMOServer �� �����Ű�� ���Ͽ� ��� ��ü�鿡 ����
	// ������ ȣ��Ǿ�� ��
	// �ʱ�ȭ�ÿ��� ȣ�� �� �ְ� ���ķδ� ȣ�� ���� ����
	// �� �Լ��� MMOServer �� ��ӹ��� ��ü�� ������ ��
	// Start �Լ��� ȣ���ϱ� ������ �ݵ�� ���� ������� ��
	// �ݺ������� ȣ���ϱ� �����ϸ� 0 ~ NumOfMaxClient - 1 ������ ȣ�� �� ��� ��
	////////////////////////////////////////////////////////////////////
	void LinkSession(CSession *PlayerPointer, UINT PlayerIndex);

private :
	/////////////////////////////////////////////
	// Accept Thread
	// accept �Լ����� ȣ���Ͽ� ����ڿ� ������ ����
	// Accept �Ϸ� ť�� �־���
	/////////////////////////////////////////////
	static UINT __stdcall AcceptThreadStartAddress(LPVOID CMMOServerPointer);
	UINT AcceptThread();

	/////////////////////////////////////////////
	// Worker Thread
	// Recv �Ϸ����� �� ������ ���� �� 
	// CompleteRecvPacket �� �����
	// Send �Ϸ����� �� Send �� �Ϸ�� ��Ŷ���� ������
	// IOCount �� 0 �� �Ǿ �ٷ� �������� �ʰ� �÷��׸� ���������ν�
	// �ܺο��� ����� ������
	/////////////////////////////////////////////
	static UINT __stdcall WorkerThreadStartAddress(LPVOID CMMOServerPointer);
	UINT WorkerThread();

	/////////////////////////////////////////////
	// Send Thread
	// ���� �ֱ⸶�� ����� 
	// ��� ���ǿ� ���Ͽ� �������� �����Ѵٸ�
	// WSASend �� ��û��
	/////////////////////////////////////////////
	static UINT __stdcall SendThreadStartAddress(LPVOID CMMOServerPointer);
	UINT SendThread();
	
	/////////////////////////////////////////////
	// Auth Thread
	// ���� �ֱ⸶�� �����
	// ���� ������ ����
	/////////////////////////////////////////////
	static UINT __stdcall AuthThreadStartAddress(LPVOID CMMOServerPointer);
	UINT AuthThread();

	/////////////////////////////////////////////
	// Game Thread
	// ���� �ֱ⸶�� �����
	// ���� ������ ����
	/////////////////////////////////////////////
	static UINT __stdcall GameThreadStartAddress(LPVOID CMMOServerPointer);
	UINT GameThread();

	/////////////////////////////////////////////
	// Release Thread
	// ���� �ֱ⸶�� �����
	// SessionMode �� �α׾ƿ� ������̸� �� ������ �ʱ�ȭ ��
	/////////////////////////////////////////////
	static UINT __stdcall ReleaseThreadStartAddress(LPVOID CMMOServerPointer);
	UINT ReleaseThread();

	char RecvPost(CSession *pSession);

	bool OptionParsing(const WCHAR *szMMOServerOptionFileName);

private :
	// ������ ���� �̺�Ʈ �ڵ� ��ȣ
	enum en_EVENT_NUMBER
	{
		EVENT_SEND = 0,
		EVENT_AUTH,
		EVENT_GAME,
		EVENT_RELEASE,
		NUM_OF_EVENT
	};

	BYTE												m_byNumOfWorkerThread;
	BYTE												m_byNumOfUsingWorkerThread;
	BOOL												m_bIsNagleOn;
	WORD												m_wSendThreadSleepTime;
	WORD												m_wAuthThreadSleepTime;
	WORD												m_wGameThreadSleepTime;
	WORD												m_wReleaseThreadSleepTime;
	WORD												m_wLoopAuthHandlingPacket;
	WORD												m_wLoopGameHandlingPacket;

	WORD												m_wPort;

	// �� ������ Send�� �õ��ϴ� �ִ� Ƚ��
	WORD												m_wNumOfSendProgress;
	// �� ������ �ִ�� �����ϴ� �ִ� ����
	WORD												m_wNumOfAuthProgress;
	// �� ������ ���� ������� �Ѿ���� ������ �ִ� ����
	WORD												m_wNumOfGameProgress;

	UINT												m_uiSendThreadLoopStartValue;

	UINT												m_uiNumOfMaxClient;
	UINT												m_uiNumOfUser;
	SOCKET												m_ListenSock;
	WCHAR												m_IP[16];

	HANDLE												*m_pWorkerThreadHandle;
	HANDLE												m_hAcceptThread;
	HANDLE												m_hSendThread[2];
	HANDLE												m_hAuthThread;
	HANDLE												m_hGameThread;
	HANDLE												m_hReleaseThread;

	HANDLE												m_hWorkerIOCP;

	// ������ ����� �̺�Ʈ �迭
	// �� ��ȣ�� en_EVENT_NUMBER �� ���� �Ǿ�����
	HANDLE												m_hThreadExitEvent[en_EVENT_NUMBER::NUM_OF_EVENT];

	// ���� ���� ����
	//CLockFreeStack<CSession::AcceptUserInfo*>			m_SessionInfoStack;
	CLockFreeStack<WORD>								m_SessionInfoStack;
	// Accept �� �������� ���� ������ ������� ť
	CLockFreeQueue<CSession::AcceptUserInfo*>			m_AcceptUserInfoQueue;

	CTLSMemoryPool<CSession::AcceptUserInfo>			*m_pUserInfoMemoryPool;
	
	// ���� ������ �迭
	// �ܺ� �������� �����͸� ���� �迭�� �־������
	CSession											**m_ppSessionPointerArray;
};