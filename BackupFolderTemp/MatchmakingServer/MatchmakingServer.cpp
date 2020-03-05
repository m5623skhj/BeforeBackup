#include "PreCompile.h"
#include "MatchmakingServer.h"
#include "MatchmakingLanClient.h"
#include "MatchmakingMonitoringClient.h"

#include "Log.h"
#include "DBConnector.h"

#include "Protocol/CommonProtocol.h"

CMatchmakingServer::CMatchmakingServer()
{

}

CMatchmakingServer::~CMatchmakingServer()
{

}

bool CMatchmakingServer::MatchmakingServerStart(const WCHAR *szMatchmakingNetServerOptionFile, const WCHAR *szMatchmakingLanClientOptionFile, const WCHAR *szMatchmakingMonitoringClientOptionFile)
{
	m_uiNumOfLoginCompleteUser = 0;
	m_uiLoginSuccessTPS = 0;
	m_uiNumOfConnectUser = 0;
	m_uiUserNoCount = 0;

	InitializeCriticalSection(&m_csUserMapLock);
	InitializeCriticalSection(&m_csSessionMapLock);

	m_pUserMemoryPool = new CTLSMemoryPool<st_USER>(5, false);

	m_pMatchmakingLanclient = new CMatchmakingLanClient();
	if(m_pMatchmakingLanclient == NULL)
		return false;
	wprintf_s(L"LanClient On\n");

	m_pMatchmakingMonitoringClient = new CMatchmakingMonitoringClient();
	if(m_pMatchmakingMonitoringClient == NULL)
		return false;
	wprintf_s(L"Monitoring Client On\n");

	if (!Start(szMatchmakingNetServerOptionFile))
		return false;

	if (!MatchmakingNetServerOptionParsing(szMatchmakingNetServerOptionFile))
		return false;
	if (!m_pMatchmakingLanclient->MatchmakingLanClientStart(this, m_wServerNo, szMatchmakingLanClientOptionFile))
		return false;
	if (!m_pMatchmakingMonitoringClient->MatchmakingMonitoringClientStart(szMatchmakingMonitoringClientOptionFile, &m_uiNumOfConnectUser, &m_uiNumOfLoginCompleteUser))
		return false;

	m_hDBSendThreadHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSendEventHandle[en_USER_CONNECT_MAX_IN_DB_SAVE_TIME] = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hSendEventHandle[en_SEND_THREAD_EXIT] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hDBSendThreadHandle == NULL || m_hSendEventHandle[en_USER_CONNECT_MAX_IN_DB_SAVE_TIME] == NULL
		|| m_hSendEventHandle[en_SEND_THREAD_EXIT] == NULL)
		return false;

	m_hDBSendThreadHandle = (HANDLE)_beginthreadex(NULL, 0, shDBSendThreadAddress, NULL, 0, NULL);
	if (m_hDBSendThreadHandle == NULL)
		return false;

	wprintf_s(L"Matchmaking Server On\n");

	return true;
}

void CMatchmakingServer::MatchmakingServerStop()
{
	m_pMatchmakingLanclient->MatchmakingLanClientStop();
	m_pMatchmakingMonitoringClient->MatchmakingMonitoringClientStop();
	Stop();

	SetEvent(m_hSendEventHandle[en_SEND_THREAD_EXIT]);
	WaitForSingleObject(m_hDBSendThreadHandle, INFINITE);

	CloseHandle(m_hSendEventHandle[en_SEND_THREAD_EXIT]);
	CloseHandle(m_hSendEventHandle[en_USER_CONNECT_MAX_IN_DB_SAVE_TIME]);
	CloseHandle(m_hDBSendThreadHandle);

	DeleteCriticalSection(&m_csUserMapLock);
	DeleteCriticalSection(&m_csSessionMapLock);

	auto UserMapIterEnd = m_UserMap.end();
	for (auto iter = m_UserMap.begin(); iter != UserMapIterEnd; ++iter)
	{
		m_pUserMemoryPool->Free(iter->second);
		iter = m_UserMap.erase(iter);
		--iter;
	}

	auto SessionIDMapIterEnd = m_SessionIDMap.end();
	for (auto iter = m_SessionIDMap.begin(); iter != SessionIDMapIterEnd; ++iter)
	{
		iter = m_SessionIDMap.erase(iter);
		--iter;
	}

	delete m_pMatchmakingLanclient;
	delete m_pMatchmakingMonitoringClient;
	delete m_pUserMemoryPool;
}

void CMatchmakingServer::OnClientJoin(UINT64 ClientID)
{
	InterlockedIncrement(&m_uiNumOfConnectUser);
	st_USER *userInfo = m_pUserMemoryPool->Alloc();
	
	// UserMap �Ӱ迵�� ����
	EnterCriticalSection(&m_csUserMapLock);
	
	m_UserMap.insert({ ClientID, userInfo });

	// UserMap �Ӱ迵�� ��
	LeaveCriticalSection(&m_csUserMapLock);
}

void CMatchmakingServer::OnClientLeave(UINT64 ClientID)
{
	// UserMap �Ӱ迵�� ����
	EnterCriticalSection(&m_csUserMapLock);

	unordered_map<UINT64, st_USER*>::iterator UserMapIter = m_UserMap.find(ClientID);
	if (UserMapIter == m_UserMap.end())
	{
		g_Dump.Crash();
	}
	st_USER *pLeaveUser = UserMapIter->second;

	// �ش� ������ �α��� �� �������
	if (pLeaveUser->byStatus != en_STATUS_CREATE)
	{
		// �α��� �� ������ ���� ������
		InterlockedDecrement(&m_uiNumOfLoginCompleteUser);
		// �ش� ������ ��Ʋ ������ ���� ���߿� ������ �����Ͽ��ٸ� 
		if (pLeaveUser->byStatus == en_STATUS_GO_TO_BATTLE)
			// �� ��Ȳ�� �����Ϳ��� �˷��־� �����Ͱ� �� ���� �ο��� +1 �ϵ��� ������Ű�� ���Ͽ�
			// ������ ��Ʋ ������ ���°��� �����ߴٰ� �˷���
			m_pMatchmakingLanclient->SendToMaster_GameRoomEnterFail(pLeaveUser->uiClientKey);
	}

	m_UserMap.erase(ClientID);

	// UserMap �Ӱ迵�� ��
	LeaveCriticalSection(&m_csUserMapLock);

	m_pUserMemoryPool->Free(pLeaveUser);

	// SessionIDMap �Ӱ迵�� ����
	EnterCriticalSection(&m_csSessionMapLock);

	m_SessionIDMap.erase(ClientID);

	// SessionIDMap �Ӱ迵�� ��
	LeaveCriticalSection(&m_csSessionMapLock);

	InterlockedDecrement(&m_uiNumOfConnectUser);
}

bool CMatchmakingServer::OnConnectionRequest(const WCHAR *IP)
{
	return true;
}

void CMatchmakingServer::OnRecv(UINT64 ReceivedSessionID, CNetServerSerializationBuf *OutReadBuf)
{
	WORD Type;
	*OutReadBuf >> Type;

	switch (Type)
	{
	case en_PACKET_CS_MATCH_REQ_LOGIN:
		LoginProc(ReceivedSessionID, OutReadBuf);
		break;
	case en_PACKET_CS_MATCH_REQ_GAME_ROOM:
	{
		// UserMap �Ӱ迵�� ����
		EnterCriticalSection(&m_csUserMapLock);

		unordered_map<UINT64, st_USER*>::iterator UserMapIter = m_UserMap.find(ReceivedSessionID);
		if (UserMapIter == m_UserMap.end())
		{
			g_Dump.Crash();
		}
		st_USER &UserInfo = *UserMapIter->second;

		// UserMap �Ӱ迵�� ��
		LeaveCriticalSection(&m_csUserMapLock);

		m_pMatchmakingLanclient->SendToMaster_GameRoom(UserInfo.uiClientKey, UserInfo.iAccountNo);
		UserInfo.byStatus = en_STATUS_ACQUIRE_ROOM;
		break;
	}
	case en_PACKET_CS_MATCH_REQ_GAME_ROOM_ENTER:
	{
		WORD BattleServerNo;
		int RoomNo;
		if (OutReadBuf->GetUseSize() != 6)
			break;
		*OutReadBuf >> BattleServerNo >> RoomNo;

		// UserMap �Ӱ迵�� ����
		EnterCriticalSection(&m_csUserMapLock);

		unordered_map<UINT64, st_USER*>::iterator UserMapIter = m_UserMap.find(ReceivedSessionID);
		if (UserMapIter == m_UserMap.end())
		{
			g_Dump.Crash();
		}
		st_USER &UserInfo = *UserMapIter->second;

		// UserMap �Ӱ迵�� ��
		LeaveCriticalSection(&m_csUserMapLock);

		// ������ �� ���忡 �����Ͽ����� �����Ϳ��� �˷���
		m_pMatchmakingLanclient->SendToMaster_GameRoomEnterSuccess(BattleServerNo, RoomNo,
			UserInfo.uiClientKey);
		UserInfo.byStatus = en_STATUS_ENTER_BATTLE_COMPLETE;

		CNetServerSerializationBuf &SendBuf = *CNetServerSerializationBuf::Alloc();

		Type = en_PACKET_CS_MATCH_RES_GAME_ROOM_ENTER;
		SendBuf << Type;

		// ������ ������ ��ġ����ŷ �������� ������ ���°��� �����ϱ� ���Ͽ�
		// ���� ��Ŷ�� ȸ������
		SendPacket(ReceivedSessionID, &SendBuf);
		break;
	}
	default:
		break;
	}
}

void CMatchmakingServer::OnSend(UINT64 ClientID, int sendsize)
{

}

void CMatchmakingServer::OnWorkerThreadBegin()
{

}

void CMatchmakingServer::OnWorkerThreadEnd()
{

}

void CMatchmakingServer::OnError(st_Error *OutError)
{
	if (OutError->GetLastErr != WSAECONNRESET)
	{
		_LOG(LOG_LEVEL::LOG_DEBUG, L"CHATSERVERERR ", L"%d\n%d\n%d\n", OutError->GetLastErr, OutError->ServerErr, OutError->Line);
		printf_s("==============================================================\n");
	}
}

void CMatchmakingServer::SendToClient_GameRoom(CSerializationBuf *pRecvBuf)
{
	WORD Type = en_PACKET_CS_MATCH_RES_GAME_ROOM;
	UINT64 ClientKey;
	BYTE Status;

	CNetServerSerializationBuf &SendBuf = *CNetServerSerializationBuf::Alloc();
	CSerializationBuf &RecvBuf = *pRecvBuf;

	RecvBuf >> ClientKey >> Status;

	SendBuf << Type << Status;
	
	// ���� ���� �ޱ⿡ �����ߴٸ�
	if (Status == 0)
	{
		// m_SessionIDMap �Ӱ迵�� ����
		EnterCriticalSection(&m_csSessionMapLock);

		unordered_map<UINT64, UINT64>::iterator SessionMapIter = m_SessionIDMap.find(ClientKey);
		if (SessionMapIter == m_SessionIDMap.end())
		{
			g_Dump.Crash();
		}

		// Ŭ���̾�Ʈ���� ���и� �뺸��
		SendPacket(SessionMapIter->second, &SendBuf);

		// m_SessionIDMap �Ӱ迵�� ��
		EnterCriticalSection(&m_csSessionMapLock);

		return;
	}

	WORD BattleServerNo, Port, ChatServerPort;
	int RoomNo;
	char ConnectToken[32], EnterToken[32];
	WCHAR IP[16], ChatServerIP[16];

	RecvBuf >> BattleServerNo;
	RecvBuf.ReadBuffer((char*)IP, sizeof(IP));
	RecvBuf >> Port >> RoomNo;
	RecvBuf.ReadBuffer((char*)ConnectToken, sizeof(ConnectToken));
	RecvBuf.ReadBuffer((char*)EnterToken, sizeof(EnterToken));
	RecvBuf.ReadBuffer((char*)ChatServerIP, sizeof(ChatServerIP));
	RecvBuf >> ChatServerPort;

	SendBuf << BattleServerNo;
	SendBuf.WriteBuffer((char*)IP, sizeof(IP));
	SendBuf << Port << RoomNo;
	SendBuf.WriteBuffer((char*)ConnectToken, sizeof(ConnectToken));
	SendBuf.WriteBuffer((char*)EnterToken, sizeof(EnterToken));
	SendBuf.WriteBuffer((char*)ChatServerIP, sizeof(ChatServerIP));
	SendBuf << ChatServerPort;

	// m_SessionIDMap �Ӱ迵�� ����
	EnterCriticalSection(&m_csSessionMapLock);

	unordered_map<UINT64, UINT64>::iterator SessionMapIter = m_SessionIDMap.find(ClientKey);
	if (SessionMapIter == m_SessionIDMap.end())
	{
		g_Dump.Crash();
	}

	// Ŭ���̾�Ʈ���� ���и� �뺸��
	SendPacket(SessionMapIter->second, &SendBuf);

	// m_SessionIDMap �Ӱ迵�� ��
	EnterCriticalSection(&m_csSessionMapLock);
}

UINT __stdcall CMatchmakingServer::shDBSendThreadAddress(LPVOID MatchmakingNetServer)
{
	return ((CMatchmakingServer*)MatchmakingNetServer)->shDBSendThread();
}

UINT CMatchmakingServer::shDBSendThread()
{
	CDBConnector::InitTLSDBConnet(m_DBIP, m_DBUserName, m_DBPassWord, m_DBName, m_wDBPort);
	CDBConnector &DBConnector = *CDBConnector::GetDBConnector();

	UINT &NumOfConnectUser = m_uiNumOfConnectUser;

	DWORD WaitObjectRetval = 99999;

	DWORD StandardTime = m_wshDBSaveTime;

	// ���� ������ ���� ������ �ڽ��� ������ �ִ� ������ �̸� DB �� �ݿ����� ����
	// ���� DB �� �ش� ������ ���ٸ� INSERT ��Ŵ
	if (!DBConnector.SendQuery_Save(L"INSERT INTO matchmaking_status.server (`ip`, `port`, `connectuser`, `heartbeat`) VALUES(\'%s\', %d, %d, NOW()) ONDUPLICATE KEY UPDATE `ip`=\"%s\", `port`=%d, `connectuser`=%d, `heartbeat`=NOW()", m_DBIP, m_wDBPort, NumOfConnectUser))
		g_Dump.Crash();

	// �ش� ������ serverno �� ������ ���Ͽ� SELECT ������ ����
	if (!DBConnector.SendQuery(L"SELECT matchmaking_status.server WHERE `ip`=\'%s\' AND `port`=%d"), m_DBIP, m_wDBPort)
		g_Dump.Crash();
	int ServerNo = atoi(DBConnector.FetchRow()[0]);
	DBConnector.FreeResult();

	ULONGLONG NowTime, TotalTime;
	ULONGLONG BeforeTime = GetTickCount64();

	// StandardTime ��ŭ���� DB �� ��ġ����ŷ ������ ���¸� �����ش�
	// ���� �ο��� �̻��� StandardTime ���� �Ѳ����� ���Դٸ�
	// �� ���� DB �� ���¸� ������
	// ���� ó�������� �����°��� 3�ʰ� �зȴٸ� �� �� �� ������
	while (1)
	{
		NowTime = GetTickCount64();
		// WAIT_TIMEOUT �� ��� StandardTime ��ŭ ���� �������Ƿ� �� �ð��� ���� ����Ѵ�
		if (WaitObjectRetval == WAIT_TIMEOUT)
			TotalTime += NowTime - BeforeTime - StandardTime;
		else
			TotalTime += NowTime - BeforeTime;
		
		// ���� �ð��� ���� �ð����� ���ٸ�
		if (StandardTime < TotalTime)
		{
			// DB �� �ٽ� �� �� ��ġ����ŷ ������ ���¸� �ݿ����ش�
			DBConnector.SendQuery_Save(L"UPDATE matchmaking_status.server SET `connectuser`=%d, `heartbeat`=NOW() WHERE serverno=%d", NumOfConnectUser, ServerNo);
			TotalTime -= StandardTime;
		}
		
		WaitObjectRetval = WaitForMultipleObjects(2, m_hSendEventHandle, FALSE, StandardTime);
		// StandardTime ��ŭ �����ų� Ȥ�� �� �ð� �̳��� ���س��� �ο� �̻��� ������ ���
		if (WaitObjectRetval == WAIT_TIMEOUT || WaitObjectRetval == WAIT_OBJECT_0)
			// DB �� ���� ��ġ����ŷ ������ ���¸� �����ش�
			DBConnector.SendQuery_Save(L"UPDATE matchmaking_status.server SET `connectuser`=%d, `heartbeat`=NOW() WHERE serverno=%d", NumOfConnectUser, ServerNo);
		// ���� ��Ȳ�� �ƴҰ��
		else
			// �ܺο��� �� �����带 �����Ŵ
			break;
	}

	return 0;
}

UINT CMatchmakingServer::GetLoginSuccessTPSAndReset()
{
	UINT LoginSuccessTPS = m_uiLoginSuccessTPS;
	InterlockedExchange(&m_uiLoginSuccessTPS, 0);

	return LoginSuccessTPS;
}

UINT CMatchmakingServer::GetAcceptTPSAndReset()
{
	UINT AcceptTPS = m_uiAcceptTPS;
	InterlockedExchange(&m_uiAcceptTPS, 0);

	return AcceptTPS;
}

bool CMatchmakingServer::MatchmakingNetServerOptionParsing(const WCHAR *szOptionFileName)
{
	_wsetlocale(LC_ALL, L"Korean");

	CParser parser;
	WCHAR cBuffer[BUFFER_MAX];

	FILE *fp;
	_wfopen_s(&fp, szOptionFileName, L"rt, ccs=UNICODE");

	int iJumpBOM = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int iFileSize = ftell(fp);
	fseek(fp, iJumpBOM, SEEK_SET);
	int FileSize = (int)fread_s(cBuffer, BUFFER_MAX, sizeof(WCHAR), BUFFER_MAX / 2, fp);
	int iAmend = iFileSize - FileSize; // ���� ���ڿ� ���� ����� ���� ������
	fclose(fp);

	cBuffer[iFileSize - iAmend] = '\0';
	WCHAR *pBuff = cBuffer;

	if (!parser.GetValue_String(pBuff, L"DATABASE", L"DB_IP", m_DBIP))
		return false;
	if (!parser.GetValue_String(pBuff, L"DATABASE", L"DB_USERNAME", m_DBUserName))
		return false;
	if (!parser.GetValue_String(pBuff, L"DATABASE", L"DB_PASSWORD", m_DBPassWord))
		return false;
	if (!parser.GetValue_String(pBuff, L"DATABASE", L"DB_NAME", m_DBName))
		return false;
	if (!parser.GetValue_Short(pBuff, L"DATABASE", L"DB_PORT", (short*)&m_wDBPort))
		return false;

	if (!parser.GetValue_Short(pBuff, L"MATCHMAKING_SERVER", L"DB_SAVETIME", (short*)&m_wshDBSaveTime))
		return false;
	if (!parser.GetValue_Int(pBuff, L"MATCHMAKING_SERVER", L"VERSION_CODE", (int*)&m_uiServerVersionCode))
		return false;
	if (!parser.GetValue_Int(pBuff, L"MATCHMAKING_SERVER", L"SERVER_NO", &m_wServerNo))
		return false;

	return true;
}
