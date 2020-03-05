#include "PreCompile.h"
#include "ChatServer.h"
#include "NetServerSerializeBuffer.h"
#include "CommonProtocol.h"
#include "Log.h"
#include "ChattingLanClient.h"


/////////////////////////////////////////////////////////////////////
// Update ������ Packet �Լ� ���� cpp
// pMessage ���� Packet�� ���� Packet ���� Type �� ������ ���� ���·� ����
/////////////////////////////////////////////////////////////////////

bool CChatServer::PacketProc_PlayerJoin(UINT64 SessionID)
{
	st_USER &NewClient = *m_pUserMemoryPool->Alloc();
	NewClient.uiSessionID = SessionID;
	NewClient.SectorX = dfINIT_SECTOR_VALUE;
	NewClient.SectorY = dfINIT_SECTOR_VALUE;
	NewClient.bIsLoginUser = FALSE;
	NewClient.uiAccountNO = dfACCOUNT_INIT_VALUE;
	NewClient.uiBeforeRecvTime = dfHEARTBEAT_TIMEMAX;

	m_UserSessionMap.insert({ SessionID, &NewClient });
	++m_uiNumOfSessionAll;

	return true;
}

bool CChatServer::PacketProc_PlayerLeave(UINT64 SessionID)
{
	--m_uiNumOfSessionAll;
	--m_uiNumOfLoginCompleteUser;

	st_USER &LeaveClient = *m_UserSessionMap.find(SessionID)->second;
	if (&LeaveClient == m_UserSessionMap.end()->second)
		return false;

	// ������ �ش� ������ ������ ���� ����� Ȯ����
	if (LeaveClient.byNumOfOverWriteSessionKey > 0)
	{
		// ���� �������� �̹� �α��� �� �� �������Ƿ�
		// �÷��׸� ��� ��ȯ��
		--LeaveClient.byNumOfOverWriteSessionKey;
		return false;
	}

	//// SessionKeyMap �Ӱ迵�� ����
	//EnterCriticalSection(&m_SessionKeyMapCS);
	//st_SessionKey *pSessionKey = m_UserSessionKeyMap.find(LeaveClient.uiAccountNO)->second;
	//if (pSessionKey != NULL)
	//{
	//	if (m_UserSessionKeyMap.erase(LeaveClient.uiAccountNO) != 0)
	//		m_pSessionKeyMemoryPool->Free(pSessionKey);
	//	else
	//		g_Dump.Crash();
	//}
	//else
	//	g_Dump.Crash();
	//// SessionKeyMap �Ӱ迵�� ��
	//LeaveCriticalSection(&m_SessionKeyMapCS);

	m_UserSessionMap.erase(SessionID);
	if (LeaveClient.SectorX > dfMAX_SECTOR_X || LeaveClient.SectorY > dfMAX_SECTOR_Y)
	{
		m_pUserMemoryPool->Free(&LeaveClient);
		return true;
	}

	if (m_UserSectorMap[LeaveClient.SectorY][LeaveClient.SectorX].erase(SessionID) == 0)
	{
		st_Error Err;
		Err.GetLastErr = 0;
		Err.ServerErr = NOT_IN_USER_SECTOR_MAP_ERR;
		OnError(&Err);
		m_pUserMemoryPool->Free(&LeaveClient);
		return false;
	}

	m_pUserMemoryPool->Free(&LeaveClient);
	return true;
}

bool CChatServer::PacketProc_PlayerLoginFromLoginServer(CNetServerSerializationBuf *pRecvPacket)
{   
	UINT64 AccountNo, SessionKeyAddress;
	st_SessionKey *pSessionKey;

	*pRecvPacket >> AccountNo >> SessionKeyAddress;
	pSessionKey = (st_SessionKey*)SessionKeyAddress;

	// SessionKeyMap �Ӱ迵�� ����
	EnterCriticalSection(m_pSessionKeyMapCS);
	
	// insert �ϸ鼭 AccountNo �� ���� Ű �ʿ� �����ִٸ� 
	// ���� ������ ���� ������ ���� ��Ȳ�̶�� �Ǵ���
	if ((m_UserSessionKeyMap.insert({ AccountNo, pSessionKey })).second == false)
	{
		// ����Ű�� ����� ���Ͽ� ���� ����Ű�� ����
		st_SessionKey *NowSessionKey = m_UserSessionKeyMap.find(AccountNo)->second;
		// ���� ����Ű�� ����
		m_pSessionKeyMemoryPool->Free(NowSessionKey);
		// ���� �޾ƿ� ����Ű�� ���� ������ ���� ��
		NowSessionKey = pSessionKey;
		st_USER &OverWirteUser = *m_UserSessionMap.find(AccountNo)->second;

		// ���� Leave ���� ���� ������ �������� ���ϵ��� �÷��׸� �ø�
		++OverWirteUser.byNumOfOverWriteSessionKey;
		// ������ ���� �ʿ� �ڽ��� ������ ��� �ڽ��� ����
		if(OverWirteUser.SectorX < dfMAX_SECTOR_X && OverWirteUser.SectorY < dfMAX_SECTOR_Y)
			m_UserSectorMap[OverWirteUser.SectorY][OverWirteUser.SectorX].erase(AccountNo);
	}

	// SessionKeyMap �Ӱ迵�� ��
	LeaveCriticalSection(m_pSessionKeyMapCS);

	return true;
}

bool CChatServer::PacketProc_SectorMove(CNetServerSerializationBuf *pRecvPacket, CNetServerSerializationBuf *pOutSendPacket, UINT64 SessionID)
{
	INT64 AccountNo;
	WORD SectorX, SectorY;
	if (pRecvPacket->GetUseSize() != dfPACKETSIZE_SECTOR_MOVE)
	{
		st_Error err;
		err.GetLastErr = 0;
		err.Line = __LINE__;
		err.ServerErr = SERIALIZEBUF_SIZE_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}
	
	*pRecvPacket >> AccountNo >> SectorX >> SectorY;

	if (SectorX >= dfMAX_SECTOR_X || SectorY >= dfMAX_SECTOR_Y)
	{
		st_Error err;
		err.GetLastErr = 0;
		err.Line = __LINE__;
		err.ServerErr = SECTOR_RANGE_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	auto &User = *m_UserSessionMap.find(SessionID)->second;
	if (&User == m_UserSessionMap.end()->second)
	{
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = NOT_IN_USER_MAP_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	if (User.SectorX < dfMAX_SECTOR_X && User.SectorY < dfMAX_SECTOR_Y)
	{
		if (m_UserSectorMap[User.SectorY][User.SectorX].erase(SessionID) == 0)
		{
			st_Error err;
			err.Line = __LINE__;
			err.ServerErr = NOT_IN_USER_SECTOR_MAP_ERR;
			OnError(&err);
			DisConnect(SessionID);
			return false;
		}
	}

	if (User.bIsLoginUser == FALSE)
	{
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = NOT_LOGIN_USER_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	if (User.uiAccountNO != AccountNo)
	{
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = INCORRECT_ACCOUNTNO;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	User.uiBeforeRecvTime = dfHEARTBEAT_TIMEMAX;

	User.SectorX = SectorX;
	User.SectorY = SectorY;
	m_UserSectorMap[SectorY][SectorX].insert({ SessionID, &User });

	WORD Type = en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	*pOutSendPacket << Type << AccountNo << SectorX << SectorY;

	CNetServerSerializationBuf::AddRefCount(pOutSendPacket);
	SendPacket(SessionID, pOutSendPacket);

	return true;
}

bool CChatServer::PacketProc_ChatMessage(CNetServerSerializationBuf *pRecvPacket, CNetServerSerializationBuf *pOutSendPacket, UINT64 SessionID)
{
	INT64 AccountNo;
	WORD MessageLength;

	int RecvedSize = pRecvPacket->GetUseSize();
	if (RecvedSize < dfPACKETSIZE_CHAT_MESSAGE)
	{
		st_Error err;
		err.GetLastErr = 0;
		err.Line = __LINE__;
		err.ServerErr = SERIALIZEBUF_SIZE_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}
	*pRecvPacket >> AccountNo >> MessageLength;

	if (RecvedSize != MessageLength + dfPACKETSIZE_CHAT_MESSAGE)
	{
		st_Error err;
		err.GetLastErr = 0;
		err.Line = __LINE__;
		err.ServerErr = SERIALIZEBUF_SIZE_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	auto &User = *m_UserSessionMap.find(SessionID)->second;
	if (&User == m_UserSessionMap.end()->second)
	{
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = NOT_IN_USER_MAP_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	if (User.bIsLoginUser == FALSE)
	{
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = NOT_LOGIN_USER_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	if (User.uiAccountNO != AccountNo)
	{
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = INCORRECT_ACCOUNTNO;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	if (User.SectorX > dfMAX_SECTOR_X || User.SectorY > dfMAX_SECTOR_Y)
	{
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = SECTOR_RANGE_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	User.uiBeforeRecvTime = dfHEARTBEAT_TIMEMAX;

	WORD Type = en_PACKET_CS_CHAT_RES_MESSAGE;

	CNetServerSerializationBuf &SendBuf = *pOutSendPacket;
	SendBuf << Type << AccountNo;
	SendBuf.WriteBuffer((char*)&User.szID, sizeof(WCHAR) * dfID_NICKNAME_LENGTH);
	SendBuf << MessageLength;

	SendBuf.WriteBuffer(pRecvPacket->GetReadBufferPtr(), MessageLength);
	
	BroadcastSectorAroundAll(User.SectorX, User.SectorY, &SendBuf);

	return true;
}

void CChatServer::PacketProc_HeartBeat(CNetServerSerializationBuf *pRecvPacket, CNetServerSerializationBuf *pOutSendPacket)
{

}

void CChatServer::PacketProc_SessionKeyHeartBeat(CNetServerSerializationBuf *pRecvPacket)
{
	EnterCriticalSection(m_pSessionKeyMapCS);

	std::unordered_map<UINT64, st_SessionKey*>::iterator TravelIter = m_UserSessionKeyMap.begin();
	std::unordered_map<UINT64, st_SessionKey*>::iterator Enditer = m_UserSessionKeyMap.end();

	for (; TravelIter != Enditer; ++TravelIter)
	{
		UINT64 &SessionKeyLifeTime = TravelIter->second->SessionKeyLifeTime;
		SessionKeyLifeTime--;

		if (SessionKeyLifeTime <= 0)
		{
			m_pSessionKeyMemoryPool->Free(TravelIter->second);
			TravelIter = m_UserSessionKeyMap.erase(TravelIter);
			--TravelIter;
		}
	}

	LeaveCriticalSection(m_pSessionKeyMapCS);
}

bool CChatServer::PacketProc_Login(CNetServerSerializationBuf *pRecvPacket, CNetServerSerializationBuf *pOutSendPacket, UINT64 SessionID)
{
	INT64 AccountNo;
	CNetServerSerializationBuf &RecvPacket = *pRecvPacket;
	if (RecvPacket.GetUseSize() != dfPACKETSIZE_LOGIN)
	{
		st_Error err;
		err.GetLastErr = 0;
		err.Line = __LINE__;
		err.ServerErr = SERIALIZEBUF_SIZE_ERR;
		OnError(&err);
		DisConnect(SessionID);
		return false;
	}

	RecvPacket >> AccountNo;

	BYTE Status;
	CNetServerSerializationBuf &SendBuf = *pOutSendPacket;
	WORD Type = en_PACKET_CS_CHAT_RES_LOGIN;
	
	auto UserIter = m_UserSessionMap.find(SessionID);
	if (UserIter != m_UserSessionMap.end())
		Status = TRUE;
	else
	{
		Status = FALSE;
		SendBuf << Type << Status << AccountNo;
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = NOT_IN_USER_MAP_ERR;
		OnError(&err);

		CNetServerSerializationBuf::AddRefCount(&SendBuf);
		SendPacketAndDisConnect(SessionID, &SendBuf);

		//SendPacket(SessionID, &SendBuf);

		//DisConnect(SessionID);
		return false;
	}

	st_USER &User = *UserIter->second;

	User.uiAccountNO = AccountNo;
	RecvPacket.ReadBuffer((char*)&User.szID, sizeof(WCHAR) * dfID_NICKNAME_LENGTH);
	RecvPacket.ReadBuffer((char*)&User.SessionKey, sizeof(User.SessionKey));
	
	// SessionKeyMap �Ӱ迵�� ����
	EnterCriticalSection(m_pSessionKeyMapCS);

	auto SessionKeyIter = m_UserSessionKeyMap.find(AccountNo);
	if (SessionKeyIter == m_UserSessionKeyMap.end())
	{
		Status = FALSE;
		SendBuf << Type << Status << AccountNo;
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = SESSION_KEY_IS_NULL;
		OnError(&err);

		CNetServerSerializationBuf::AddRefCount(&SendBuf);
		SendPacketAndDisConnect(SessionID, &SendBuf);

		// SessionKeyMap �Ӱ迵�� ��
		LeaveCriticalSection(m_pSessionKeyMapCS);

		//SendPacket(SessionID, &SendBuf);

		//DisConnect(SessionID);
		return false;
	}

	st_SessionKey *pSessionKey = SessionKeyIter->second;
	if (memcmp(pSessionKey, User.SessionKey, dfSESSIONKEY_SIZE) != 0)
		//if (pSessionKey == m_UserSessionKeyMap.end()->second)
	{
		m_iNumOfSessionKeyMiss++;
		Status = FALSE;
		SendBuf << Type << Status << AccountNo;
		st_Error err;
		err.Line = __LINE__;
		err.ServerErr = INCORRECT_SESSION_KEY_ERR;
		OnError(&err);
		
		CNetServerSerializationBuf::AddRefCount(&SendBuf);
		SendPacketAndDisConnect(SessionID, &SendBuf);
		
		// SessionKeyMap �Ӱ迵�� ��
		LeaveCriticalSection(m_pSessionKeyMapCS);

		//SendPacket(SessionID, &SendBuf);

		//DisConnect(SessionID);
		return false;
	}

	m_pSessionKeyMemoryPool->Free(pSessionKey);
	m_UserSessionKeyMap.erase(AccountNo);

	// SessionKeyMap �Ӱ迵�� ��
	LeaveCriticalSection(m_pSessionKeyMapCS);

	SendBuf << Type  << Status << AccountNo;
	
	User.bIsLoginUser = TRUE;
	User.uiBeforeRecvTime = dfHEARTBEAT_TIMEMAX;

	CNetServerSerializationBuf::AddRefCount(&SendBuf);
	SendPacket(SessionID, &SendBuf);
	++m_uiNumOfLoginCompleteUser;

	return true;
}
