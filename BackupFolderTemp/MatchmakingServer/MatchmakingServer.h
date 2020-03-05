#pragma once

#include "NetServer.h"
#include <unordered_map>

#include "NetServerSerializeBuffer.h"
#include "LanServerSerializeBuf.h"

using namespace std;

//class CNetServerSerializationBuf;
//class CSerializationBuf;
class CMatchmakingLanClient;
class CMatchmakingMonitoringClient;

class CMatchmakingServer : CNetServer
{
	friend CMatchmakingLanClient;

public:
	CMatchmakingServer();
	virtual ~CMatchmakingServer();

	bool MatchmakingServerStart(const WCHAR *szMatchmakingNetServerOptionFile, const WCHAR *szMatchmakingLanClientOptionFile, const WCHAR *szMatchmakingMonitoringClientOptionFile);
	void MatchmakingServerStop();

	//----------------------------------------------------------------------------
	// main ������ ����� ������
	UINT GetLoginSuccessTPSAndReset();
	UINT GetAcceptTPSAndReset();
	UINT GetNumOfTotalAccept() { return m_uiUserNoCount; }

	int GetUserInfoAlloc() { return m_pUserMemoryPool->GetAllocChunkCount(); }
	int GetUserInfoUse() { return m_pUserMemoryPool->GetUseChunkCount(); }
	int GetNumOfAllocLanSerializeChunk() { return CSerializationBuf::GetAllocSerializeBufChunkCount(); }
	int GetNumOfUsingLanSerializeBuf() { return CSerializationBuf::GetUsingSerializeBufNodeCount(); }
	int GetNumOfUsingLanSerializeChunk() { return CSerializationBuf::GetUsingSerializeBufChunkCount(); }
	int GetNumOfAllocSerializeChunk() { return CSerializationBuf::GetAllocSerializeBufChunkCount(); }
	int GetNumOfUsingSerializeBuf() { return CSerializationBuf::GetUsingSerializeBufNodeCount(); }
	int GetNumOfUsingSerializeChunk() { return CSerializationBuf::GetUsingSerializeBufChunkCount(); }
	//----------------------------------------------------------------------------

public :
	// �ش� ������ ������ �� �� ���� �α��ο� �����Ͽ������� ���� ��
	UINT											m_uiNumOfLoginCompleteUser;

	// �ش� ������ �� ���� ������ ���� �Ǿ��ִ����� ���� ��
	UINT											m_uiNumOfConnectUser;
	
private :
	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 ClientID/* Client ���� / ClientID / ��Ÿ��� */);
	// Disconnect �� ȣ��
	virtual void OnClientLeave(UINT64 ClientID);
	// Accept ���� IP ���ܵ��� ���� �뵵
	virtual bool OnConnectionRequest(const WCHAR *IP);

	// ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(UINT64 ReceivedSessionID, CNetServerSerializationBuf *OutReadBuf);
	// ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend(UINT64 ClientID, int sendsize);

	// ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadBegin();
	// ��Ŀ������ 1���� ���� ��
	virtual void OnWorkerThreadEnd();
	// ����� ���� ó�� �Լ�
	virtual void OnError(st_Error *OutError);
	
	// shDBSendThread �� ȣ���Ű�� ���� static �Լ�
	static UINT __stdcall	shDBSendThreadAddress(LPVOID MatchmakingNetServer);
	// ��ġ����ŷ ������ ������ DB �� ���� ������
	UINT shDBSendThread();

	// ������ �������� �� ������ �䱸�ϱ� ���� ȣ���ϴ� �Լ�
	// MatchmakingNetServer ������ ȣ���� �����ϵ��� private �Լ��� ����
	void SendToClient_GameRoom(CSerializationBuf *pRecvBuf);

	// Matchmaking NetServer �ɼ� �Ľ� �Լ�
	bool MatchmakingNetServerOptionParsing(const WCHAR *szOptionFileName);

	// ---------------------------------------------
	// ���ν��� �Լ�
	// ---------------------------------------------
	void LoginProc(UINT64 ReceivedSessionID, CNetServerSerializationBuf *OutReadBuf);

private :
	// Send Thread �� ���ϰ� �ִ� �ڵ���� ��
	enum en_SEND_THREAD_EVENT
	{
		// ���� ���Ͽ��� ������ ���� �� �̻����� �ο��� �ް� �Ǿ��ٸ� 
		// �� �ڵ��� �ñ׳� ���·� �ٲ��ְ� 
		// shDBSendThread �� �� ������ �����ϰ� �ȴ�
		en_USER_CONNECT_MAX_IN_DB_SAVE_TIME = 0,

		// shDBSendThread �� ���� �̺�Ʈ �ڵ�
		en_SEND_THREAD_EXIT,
	};

	enum en_USER_STATUS
	{
		// ������ ó�� ������ �� ����
		en_STATUS_CREATE = 0,
		// ������ �α��� ������ ���� ����
		en_STATUS_LOGIN_COMPLETE,
		// ������ ���� ��û���� ����
		en_STATUS_ACQUIRE_ROOM,
		// ������ ��Ʋ������ ���� ����
		en_STATUS_GO_TO_BATTLE,
		// ������ ��Ʋ������ ���ִ� ����
		en_STATUS_ENTER_BATTLE_COMPLETE,
	};

	struct st_USER
	{
		st_USER()
		{
			byStatus = en_STATUS_CREATE;
			uiClientKey = -1;
			iAccountNo = -1;
		}
		BYTE										byStatus;
		// ��ġ����ŷ ������ ���Ͽ� ������� Ŭ���̾�Ʈ ���θ����� ������
		UINT64										uiClientKey;
		// Ŭ���̾�Ʈ�� ������ ���� ���� �ĺ� ��ȣ
		// ��, �� ������ ������ �ĺ��ϰԵȴٸ�
		// ������ �����ӽ� �ߺ��� ��ȣ�� ���� �� �� ����
		// => ex) ���� 1���� ��û �� �� ������ �ޱ� ���� ��������
		// => ���� ���͵� �� ������ 1�������� ��û���� ���� ������ �ް� �� �� ����
		INT64										iAccountNo;
	};

	// st_USER �� �޸� Ǯ
	CTLSMemoryPool<st_USER>							*m_pUserMemoryPool;

	// ���� �ش� ������ ���� �Ǿ��ִ� �������� �����ϴ� map
	// key �� worker thread ���� ���� ClientID ����
	unordered_map<UINT64, st_USER*>					m_UserMap;

	// m_UserMap ���� Critical Section
	CRITICAL_SECTION								m_csUserMapLock;

	// Lan ���� Net ���� �������� �ѱ� �� �ش� ��Ŷ�� ������������ �� �ʿ䰡 ����
	// ���� �ش� ���� ����Ͽ� ������ ��Ŷ������ �Ǵ���
	// Key : ClientKey , Value : SessionID
	unordered_map<UINT64, UINT64>					m_SessionIDMap;

	// m_UserMap ���� Critical Section
	CRITICAL_SECTION								m_csSessionMapLock;
		
	// DB �� �� ms ���� ������ ���ΰ��� ���� �� (�̸� ������ ����)
	WORD											m_wshDBSaveTime;

	// shDBSaveTime ���� �ִ�� �� �� �ִ� ������ ��
	// �ð����� ���� �ο����� �� �� ���� ������ ��� 
	// shDBSendThread ���� �߰��� �� �� �� DB �� ������ ����
	WORD											m_wUserConnectMaxInshDBSaveTime;

	// �ش� ������ DB No
	int 											m_wServerNo;

	// �� ��ġ����ŷ �������� ���ϴ� ������ ��
	// Ŭ���̾�Ʈ�� �� ���� ��� �ش� ��ġ����ŷ ������ ����
	// �װ��� ���Ͽ� �� ������ �� Ŭ���̾�Ʈ�� �´��� Ȯ����
	// ���� ������ ClientKey �� ���� �� �� ���� ��
	// ClientKey 8 ����Ʈ �߿��� ���� 4 ����Ʈ�� �߰��Ͽ�
	// ��� ������ ���Ͽ� ������ ���� �����س�
	UINT											m_uiServerVersionCode;

	// �������� ���� �� �ϳ��� �����ϴ� ��
	// ���� ������ ClientKey �� ���� �� �� ���� ��
	// ClientKey 8 ����Ʈ �߿��� ���� 4 ����Ʈ�� �߰��Ͽ�
	// ��� ������ ���Ͽ� ������ ���� �����س�
	UINT											m_uiUserNoCount;

	// �ش� �������� 1 �� ���� �� ���� ������ �α��ο� �����Ͽ������� ���� ��
	UINT											m_uiLoginSuccessTPS;

	// �ش� �������� 1 �� ���� �� ���� ������ �� ������ �����Ͽ������� ���� ��
	UINT											m_uiAcceptTPS;

	// ������ �������� ����� ���� LanClient
	// �� �����͸� ���Ͽ� private ��� �Լ��� 
	// SendToMaster_GameRoom �� SendToMaster_GameRoomEnter �� �����Ѵ�
	CMatchmakingLanClient							*m_pMatchmakingLanclient;

	// ����͸� �������� ����� ���� MonitoringLanClient
	CMatchmakingMonitoringClient					*m_pMatchmakingMonitoringClient;
	
	// shDBSendThread ���� ���� �̺�Ʈ �ڵ�
	// en_SEND_THREAD_EVENT �� �� �ڵ��� �ѹ��� ���� �Ǿ�����
	HANDLE											m_hSendEventHandle[2];

	// shDBSendThread �� �ڵ�
	HANDLE											m_hDBSendThreadHandle;

	// ---------------------------------------------------------------------------
	// ���� ����
	WORD											m_wServerPort;
	WCHAR											m_ServerIP[16];
	// ---------------------------------------------------------------------------

	// ---------------------------------------------------------------------------
	// DB ����Ʈ ����
	WORD											m_wDBPort;
	WCHAR											m_DBIP[16];
	WCHAR											m_DBUserName[32];
	WCHAR											m_DBPassWord[32];
	WCHAR											m_DBName[32];
	// ---------------------------------------------------------------------------

};