#include "PreCompile.h"

#include "MasterToBattle.h"
#include "MasterServer.h"

#include "LanServerSerializeBuf.h"
#include "Log.h"

#include "Protocol/CommonProtocol.h"

void CMasterToBattleServer::MasterToBattleServerOnProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf)
{
	CSerializationBuf &RecvBuf = *OutReadBuf;

	if (RecvBuf.GetUseSize() != dfSIZE_BATTLE_ON)
	{
		DisConnect(ReceivedSessionID);
		return;
	}

	st_BATTLE_SERVER_INFO *BattleServerInfo = *m_pBattleServerInfoMemoryPool->Alloc();
	char MasterToken[32];

	RecvBuf.ReadBuffer((char*)BattleServerInfo->ServerIP, sizeof(BattleServerInfo->ServerIP));
	RecvBuf >> BattleServerInfo->wPort;
	RecvBuf.ReadBuffer((char*)BattleServerInfo->ConnectTokenNow, sizeof(BattleServerInfo->ConnectTokenNow));
	RecvBuf.ReadBuffer(MasterToken, sizeof(MasterToken));
	memcpy(BattleServerInfo->ConnectTokenNow, BattleServerInfo->ConnectTokenBefore, sizeof(BattleServerInfo->ConnectTokenBefore));
	InitializeSRWLock(&BattleServerInfo->ConnectTokenSRWLock);

	if (memcmp(MasterToken, m_pMasterServer->m_MasterToken, sizeof(MasterToken)) != 0)
	{
		// ������ Ű�� �ٸ��� �ٸ� �뺸 ���� �������
		DisConnect(ReceivedSessionID);
		return;
	}

	RecvBuf.ReadBuffer((char*)BattleServerInfo->ChatServerIP, sizeof(BattleServerInfo->ChatServerIP));
	RecvBuf >> BattleServerInfo->wPort;

	// ��Ʋ ������ ��ȣ�� �ο���
	int BattleServerNo = InterlockedIncrement(&m_iBattleServerNo);
	BattleServerInfo->iServerNo = BattleServerNo;

	// m_BattleClientMap �Ӱ迵�� ����(����)
	AcquireSRWLockExclusive(&m_BattleServerInfoMapSRWLock);
	//EnterCriticalSection(&m_csBattleServerInfoLock);

	m_BattleServerInfoMap.insert({ ReceivedSessionID , BattleServerInfo });

	// m_BattleClientMap �Ӱ迵�� ��  (����)
	ReleaseSRWLockExclusive(&m_BattleServerInfoMapSRWLock);
	//LeaveCriticalSection(&m_csBattleServerInfoLock);

	CSerializationBuf &SendBuf = *CSerializationBuf::Alloc();

	WORD Type;

	Type = en_PACKET_BAT_MAS_RES_SERVER_ON;

	SendBuf << Type << BattleServerInfo->iServerNo;
	SendPacket(ReceivedSessionID, &SendBuf);
}

void CMasterToBattleServer::MasterToBattleConnectTokenProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf)
{
	CSerializationBuf &RecvBuf = *OutReadBuf;

	if (RecvBuf.GetUseSize() != dfSIZE_BATTLE_REISSUE_TOKEN)
	{
		DisConnect(ReceivedSessionID);
		return;
	}

	// m_BattleServerInfoMap �Ӱ迵�� ����
	AcquireSRWLockShared(&m_BattleServerInfoMapSRWLock);

	map<UINT64, st_BATTLE_SERVER_INFO*>::iterator BattleServerIter = m_BattleServerInfoMap.find(ReceivedSessionID);
	if (BattleServerIter == m_BattleServerInfoMap.end())
	{
		DisConnect(ReceivedSessionID);
		return;
	}
	st_BATTLE_SERVER_INFO *pBattleServerInfo = BattleServerIter->second;

	// ConnectToken �Ӱ迵�� ���� (����)
	AcquireSRWLockExclusive(&pBattleServerInfo->ConnectTokenSRWLock);

	memcpy(pBattleServerInfo->ConnectTokenBefore, pBattleServerInfo->ConnectTokenNow, dfSIZE_CONNECT_TOKEN);
	RecvBuf.ReadBuffer(pBattleServerInfo->ConnectTokenNow, dfSIZE_CONNECT_TOKEN);

	// ConnectToken �Ӱ迵�� ��   (����)
	ReleaseSRWLockExclusive(&pBattleServerInfo->ConnectTokenSRWLock);

	// m_BattleServerInfoMap �Ӱ迵�� ��
	ReleaseSRWLockShared(&m_BattleServerInfoMapSRWLock);

	UINT ReqSequence;
	RecvBuf >> ReqSequence;

	CSerializationBuf &SendBuf = *CSerializationBuf::Alloc();

	WORD Type;

	Type = en_PACKET_BAT_MAS_RES_CONNECT_TOKEN;

	SendBuf << Type << ReqSequence + 1;
	SendPacket(ReceivedSessionID, &SendBuf);
}

void CMasterToBattleServer::MasterToBattleLeftUserProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf)
{
	CSerializationBuf &RecvBuf = *OutReadBuf;

	if (RecvBuf.GetUseSize() != dfSIZE_LEFT_USER)
	{
		DisConnect(ReceivedSessionID);
		return;
	}

	// m_BattleServerInfoMap �Ӱ迵�� ����
	AcquireSRWLockShared(&m_BattleServerInfoMapSRWLock);

	map<UINT64, st_BATTLE_SERVER_INFO*>::iterator BattleServerIter = m_BattleServerInfoMap.find(ReceivedSessionID);
	if (BattleServerIter == m_BattleServerInfoMap.end())
	{
		DisConnect(ReceivedSessionID);
		return;
	}
	st_BATTLE_SERVER_INFO *pBattleServerInfo = BattleServerIter->second;

	UINT64 MapFindKey = pBattleServerInfo->iServerNo;
	MapFindKey = MapFindKey << dfSHIFT_RIGHT_SERVERNO;

	// m_BattleServerInfoMap �Ӱ迵�� ��
	ReleaseSRWLockShared(&m_BattleServerInfoMapSRWLock);

	int RoomNo;
	INT64 ClientKey;
	UINT ReqSequence;

	RecvBuf >> RoomNo >> ClientKey >> ReqSequence;
	MapFindKey += RoomNo;

	bool RoomFind = false;
	st_ROOM_INFO *pUserInRoom;

	// �켱������ �� ����Ʈ�� front ���� Ȯ����
	// Ǯ �� �� ���� ���캼 ��� ������ �� ���� ����Ʈ���� ������ �Ѿ������
	// �ش� �ο��� ���� �� �� ���� ���ɼ��� �����ϱ� ������
	// m_RoomInfoList �Ӱ迵�� ����
	EnterCriticalSection(&m_csRoomInfoLock);

	pUserInRoom = m_RoomInfoList.front();

	if (pUserInRoom->iRoomNo == RoomNo)
	{
		RoomFind = true;
		
		// pUserInRoom->UserClientKeySet �Ӱ迵�� ����(����)
		//EnterCriticalSection(&pUserInRoom->UserSetLock);

		if (pUserInRoom->UserClientKeySet.erase(ClientKey) != 0)
		{
			pUserInRoom->iMaxUser++;
		}

		// pUserInRoom->UserClientKeySet �Ӱ迵�� ��  (����)
		//LeaveCriticalSection(&pUserInRoom->UserSetLock);
	}

	// m_RoomInfoList �Ӱ迵�� ��
	LeaveCriticalSection(&m_csRoomInfoLock);

	// m_RoomInfoList �� ����� �����ٸ��� ���� ��� �ʿ� �������� �����Ƿ� Ȯ����
	if (!RoomFind)
	{
		// m_PlayStandbyRoomMap �Ӱ迵�� ����
		EnterCriticalSection(&m_csPlayStandbyRoomLock);
	
		map<UINT64, st_ROOM_INFO*>::iterator UserRoomMapIter = m_PlayStandbyRoomMap.find(MapFindKey);
		if (UserRoomMapIter != m_PlayStandbyRoomMap.end())
		{
			pUserInRoom = UserRoomMapIter->second;
			if (pUserInRoom->UserClientKeySet.erase(ClientKey) != 0)
			{
				pUserInRoom->iMaxUser++;
			}
		}

		// m_PlayStandbyRoomMap �Ӱ迵�� ��
		LeaveCriticalSection(&m_csPlayStandbyRoomLock);
	}

	CSerializationBuf &SendBuf = *CSerializationBuf::Alloc();

	WORD Type;

	Type = en_PACKET_BAT_MAS_RES_LEFT_USER;

	SendBuf << Type << RoomNo << ReqSequence + 1;
	SendPacket(ReceivedSessionID, &SendBuf);
}

void CMasterToBattleServer::MasterToBattleCreatedRoomProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf)
{
	CSerializationBuf &RecvBuf = *OutReadBuf;

	AcquireSRWLockShared(&m_BattleServerInfoMapSRWLock);
	//EnterCriticalSection(&m_csBattleServerInfoLock);
	map<UINT64, st_BATTLE_SERVER_INFO*>::iterator BattleServerIter = m_BattleServerInfoMap.find(ReceivedSessionID);
	if (BattleServerIter == m_BattleServerInfoMap.end())
	{
		DisConnect(ReceivedSessionID);
		return;
	}
	st_BATTLE_SERVER_INFO *pBattleServerInfo = BattleServerIter->second;

	if (pBattleServerInfo == NULL)
	{
		DisConnect(ReceivedSessionID);
		return;
	}

	if (RecvBuf.GetUseSize() != dfSIZE_CREATE_ROOM)
	{
		DisConnect(ReceivedSessionID);
		return;
	}

	ReleaseSRWLockShared(&m_BattleServerInfoMapSRWLock);
	//LeaveCriticalSection(&m_csBattleServerInfoLock);

	// �� �Ҵ� �ޱ�
	st_ROOM_INFO &RoomInfo = *m_pRoomInfoMemoryPool->Alloc();
	int ReqSequence;

	RecvBuf >> RoomInfo.iBattleServerNo >> RoomInfo.iRoomNo >> RoomInfo.iMaxUser;
	//RoomInfo.iRemainingUser = RoomInfo.iMaxUser;
	RecvBuf.ReadBuffer(RoomInfo.EnterToken, sizeof(RoomInfo.EnterToken));
	RecvBuf >> ReqSequence;

	// m_RoomInfoList �Ӱ迵�� ����
	EnterCriticalSection(&m_csRoomInfoLock);

	// �Ҵ� ���� ���� �ο��� �� �� ���� ���̹Ƿ� ����Ʈ ���� �ڿ� ����
	m_RoomInfoList.push_back(&RoomInfo);
	
	// m_RoomInfoList �Ӱ迵�� ��
	LeaveCriticalSection(&m_csRoomInfoLock);

	CSerializationBuf &SendBuf = *CSerializationBuf::Alloc();

	WORD Type;

	Type = en_PACKET_BAT_MAS_RES_CREATED_ROOM;

	SendBuf << Type << RoomInfo.iBattleServerNo << ReqSequence + 1;
	SendPacket(ReceivedSessionID, &SendBuf);
}

void CMasterToBattleServer::MasterToBattleClosedRoomProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf)
{
	CSerializationBuf &RecvBuf = *OutReadBuf;

	if (RecvBuf.GetUseSize() != dfSIZE_CLOSE_ROOM)
	{
		DisConnect(ReceivedSessionID);
		return;
	}
	
	// m_BattleServerInfoMap �Ӱ迵�� ����
	AcquireSRWLockShared(&m_BattleServerInfoMapSRWLock);

	map<UINT64, st_BATTLE_SERVER_INFO*>::iterator BattleServerIter = m_BattleServerInfoMap.find(ReceivedSessionID);
	if (BattleServerIter == m_BattleServerInfoMap.end())
	{
		DisConnect(ReceivedSessionID);
		return;
	}
	st_BATTLE_SERVER_INFO *pBattleServerInfo = BattleServerIter->second;

	UINT64 MapFindKey = pBattleServerInfo->iServerNo;
	MapFindKey = MapFindKey << dfSHIFT_RIGHT_SERVERNO;

	// m_BattleServerInfoMap �Ӱ迵�� ��
	ReleaseSRWLockShared(&m_BattleServerInfoMapSRWLock);

	int RoomNo;
	UINT ReqSequence;

	RecvBuf >> RoomNo >> ReqSequence;
	MapFindKey += RoomNo;

	// �� ����� ó��
	// ���� ��� �� �� �ȿ� �ִ� �͸��� ������� �˻���
	// ����Ʈ�� �Ⱥ��� ������ �� ����� ó�� ��Ŷ�� ��������
	// �ش� ���� ������ �󿡼� �� ���̶� �� �����ϱ� ������

	// m_PlayStandbyRoomMap �Ӱ迵�� ����
	EnterCriticalSection(&m_csPlayStandbyRoomLock);

	map<UINT64, st_ROOM_INFO*>::iterator UserRoomMapIter = m_PlayStandbyRoomMap.find(MapFindKey);
	if (UserRoomMapIter == m_PlayStandbyRoomMap.end())
	{
		// �� ����� ��û�� �޾����� �̹� ���¹���
		// ���� ��Ȳ
		g_Dump.Crash();
	}

	st_ROOM_INFO *pDeleteRoom = UserRoomMapIter->second;

	pDeleteRoom->UserClientKeySet.clear();
	m_PlayStandbyRoomMap.erase(MapFindKey);

	// m_PlayStandbyRoomMap �Ӱ迵�� ��
	LeaveCriticalSection(&m_csPlayStandbyRoomLock);

	m_pRoomInfoMemoryPool->Free(pDeleteRoom);

	CSerializationBuf &SendBuf = *CSerializationBuf::Alloc();

	WORD Type;

	Type = en_PACKET_BAT_MAS_RES_CLOSED_ROOM;

	SendBuf << Type << RoomNo << ReqSequence + 1;
	SendPacket(ReceivedSessionID, &SendBuf);
}

void CMasterToBattleServer::MatchToMasterRequireRoom(CSerializationBuf *OutSendBuf)
{
	st_ROOM_INFO *pUserEnterRoom = NULL;
	BYTE Status;

	// m_PlayStandbyRoomMap �Ӱ迵�� ����
	EnterCriticalSection(&m_csPlayStandbyRoomLock);

	map<UINT64, st_ROOM_INFO*>::iterator MapEndIter = m_PlayStandbyRoomMap.end();

	// �� ��ü�� ��ȸ�Ͽ� �ο��� ���ڶ� ���� ã�ƺ���
	for (auto MapTravelIter = m_PlayStandbyRoomMap.begin(); MapTravelIter != MapEndIter; ++MapTravelIter)
	{
		if (MapTravelIter->second->iMaxUser != 0)
		{
			// �� ������ pUserEnterRoom �� ����
			pUserEnterRoom = MapTravelIter->second;
			// �� ���� ���� ���� �ο����� 0 �� �ƴ� ���� �����Ѵٸ�
			// �ش� ���� ���� ���� �ο����� 1 ���δ�
			pUserEnterRoom->iMaxUser--;
			int RoomNo = pUserEnterRoom->iRoomNo;
			int ServerNo = pUserEnterRoom->iBattleServerNo;


			// m_PlayStandbyRoomMap �Ӱ迵�� ��
			LeaveCriticalSection(&m_csPlayStandbyRoomLock);
			
			// m_BattleServerInfoMap �Ӱ迵�� ����(�б�)
			AcquireSRWLockShared(&m_BattleServerInfoMapSRWLock);

			map<UINT64, st_BATTLE_SERVER_INFO*>::iterator EndIter = m_BattleServerInfoMap.end();
			map<UINT64, st_BATTLE_SERVER_INFO*>::iterator TravelIter = m_BattleServerInfoMap.begin();

			for (; TravelIter != EndIter; ++TravelIter)
			{
				if (TravelIter->second->iServerNo == ServerNo)
					break;
			}

			if (TravelIter == EndIter)
			{
				// ������ ������ ���� ã������ �� �濡 �ش��ϴ� ������ ���ٸ�
				// �� ���� ���и� ������ ������ �ٽ� ���� ��û�ϵ��� ������
				Status = 0;
				*OutSendBuf << Status;
				
				return;
			}

			st_BATTLE_SERVER_INFO &BattleServer = *TravelIter->second;

			Status = 1;
			*OutSendBuf << Status << ServerNo;
			OutSendBuf->WriteBuffer((char*)BattleServer.ServerIP, sizeof(BattleServer.ServerIP));
			*OutSendBuf << BattleServer.wPort << RoomNo;
			AcquireSRWLockShared(&BattleServer.ConnectTokenSRWLock);
			OutSendBuf->WriteBuffer(BattleServer.ConnectTokenNow, sizeof(BattleServer.ConnectTokenNow));
			AcquireSRWLockShared(&BattleServer.ConnectTokenSRWLock);
			OutSendBuf->WriteBuffer(pUserEnterRoom->EnterToken, sizeof(pUserEnterRoom->EnterToken));
			OutSendBuf->WriteBuffer((char*)BattleServer.ChatServerIP, sizeof(BattleServer.ChatServerIP));
			*OutSendBuf << BattleServer.wChatServerPort;

			// m_BattleServerInfoMap �Ӱ迵�� ��  (�б�)
			ReleaseSRWLockShared(&m_BattleServerInfoMapSRWLock);

			return;
		}
	}

	// m_PlayStandbyRoomMap �Ӱ迵�� ��
	LeaveCriticalSection(&m_csPlayStandbyRoomLock);

	if (pUserEnterRoom == NULL)
	{
		// m_RoomInfoList �Ӱ迵�� ����
		EnterCriticalSection(&m_csRoomInfoLock);

		// �ʿ� �ο��� ���� �� ���ִٸ� ����Ʈ�� ���� ó�� �κп��ٰ� �ο��� �Ҵ� �� �� �ִ��� Ȯ����
		if (m_RoomInfoList.begin() != m_RoomInfoList.end())
		{
			// ����Ʈ�� �ο��� �߰��� �� �ִٸ�
			// �� ������ pUserEnterRoom �� ����
			pUserEnterRoom = m_RoomInfoList.front();
			// ����Ʈ ���� ���� ���� �ο����� 0 �� �ƴ� ���� �����Ѵٸ�
			// �ش� ���� ���� ���� �ο����� 1 ���δ�
			pUserEnterRoom->iMaxUser--;
			int RoomNo = pUserEnterRoom->iRoomNo;
			int ServerNo = pUserEnterRoom->iBattleServerNo;


			// m_RoomInfoList �Ӱ迵�� ��
			LeaveCriticalSection(&m_csRoomInfoLock);

			// m_BattleServerInfoMap �Ӱ迵�� ����(�б�)
			AcquireSRWLockShared(&m_BattleServerInfoMapSRWLock);

			map<UINT64, st_BATTLE_SERVER_INFO*>::iterator EndIter = m_BattleServerInfoMap.end();
			map<UINT64, st_BATTLE_SERVER_INFO*>::iterator TravelIter = m_BattleServerInfoMap.begin();

			for (; TravelIter != EndIter; ++TravelIter)
			{
				if (TravelIter->second->iServerNo == ServerNo)
					break;
			}

			if (TravelIter == EndIter)
			{
				// ������ ������ ���� ã������ �� �濡 �ش��ϴ� ������ ���ٸ�
				// �� ���� ���и� ������ ������ �ٽ� ���� ��û�ϵ��� ������
				Status = 0;
				*OutSendBuf << Status;

				return;
			}

			st_BATTLE_SERVER_INFO &BattleServer = *TravelIter->second;

			Status = 1;
			*OutSendBuf << Status << ServerNo;
			OutSendBuf->WriteBuffer((char*)BattleServer.ServerIP, sizeof(BattleServer.ServerIP));
			*OutSendBuf << BattleServer.wPort << RoomNo;
			AcquireSRWLockShared(&BattleServer.ConnectTokenSRWLock);
			OutSendBuf->WriteBuffer(BattleServer.ConnectTokenNow, sizeof(BattleServer.ConnectTokenNow));
			AcquireSRWLockShared(&BattleServer.ConnectTokenSRWLock);
			OutSendBuf->WriteBuffer(pUserEnterRoom->EnterToken, sizeof(pUserEnterRoom->EnterToken));
			OutSendBuf->WriteBuffer((char*)BattleServer.ChatServerIP, sizeof(BattleServer.ChatServerIP));
			*OutSendBuf << BattleServer.wChatServerPort;

			// m_BattleServerInfoMap �Ӱ迵�� ��  (�б�)
			ReleaseSRWLockShared(&m_BattleServerInfoMapSRWLock);

			return;
		}

		// m_RoomInfoList �Ӱ迵�� ��
		LeaveCriticalSection(&m_csRoomInfoLock);
	}

	// ���̳� ����Ʈ�� ������ ���� �� �ִ� ������ ���� �������� �ʴ´ٸ�
	// �� ���� ��� ���� ��Ŷ�� ����
	Status = 0;
	*OutSendBuf << Status;
}

void CMasterToBattleServer::MatchToMasterEnterRoomSuccess(UINT64 ClientKey, int BattleServerNo, int BattleRoomNo)
{
	// m_RoomInfoList �Ӱ迵�� ����
	EnterCriticalSection(&m_csRoomInfoLock);

	st_ROOM_INFO &RoomInfoListFront = *m_RoomInfoList.front();

	if (RoomInfoListFront.UserClientKeySet.find(ClientKey) == RoomInfoListFront.UserClientKeySet.end())
	{
		// m_RoomInfoList �Ӱ迵�� ��
		LeaveCriticalSection(&m_csRoomInfoLock);
		
		return;
	}

	// m_RoomInfoList �Ӱ迵�� ��
	LeaveCriticalSection(&m_csRoomInfoLock);

	UINT64 MapFindKey = BattleServerNo;
	MapFindKey = MapFindKey << dfSHIFT_RIGHT_SERVERNO;
	MapFindKey += BattleRoomNo;

	// m_PlayStandbyRoomMap �Ӱ迵�� ����
	EnterCriticalSection(&m_csPlayStandbyRoomLock);

	map<UINT64, st_ROOM_INFO*>::iterator RoomInfoMapIter = m_PlayStandbyRoomMap.find(MapFindKey);
	
	if (RoomInfoMapIter != m_PlayStandbyRoomMap.end())
	{
		st_ROOM_INFO &ClientKeySetIter = *RoomInfoMapIter->second;

		if (ClientKeySetIter.UserClientKeySet.find(ClientKey) != ClientKeySetIter.UserClientKeySet.end())
		{
			// m_PlayStandbyRoomMap �Ӱ迵�� ��
			LeaveCriticalSection(&m_csPlayStandbyRoomLock);
			return;
		}
	}

	// m_PlayStandbyRoomMap �Ӱ迵�� ��
	LeaveCriticalSection(&m_csPlayStandbyRoomLock);
}

void CMasterToBattleServer::MatchToMasterEnterRoomFail(UINT64 ClientKey, UINT64 BattleRoomInfo)
{
	// m_pMasterToBattleServer �� m_RoomInfoList �Ӱ迵�� ����
	EnterCriticalSection(&m_csRoomInfoLock);

	st_ROOM_INFO &ListIter = *m_RoomInfoList.front();

	if (ListIter.iRoomNo == (int)BattleRoomInfo)
	{
		// �ش� Ŭ���̾�Ʈ Ű�� ������ �ִ� ��带 ����
		if (ListIter.UserClientKeySet.erase(ClientKey) != 0)
		{
			// Ŭ���̾�Ʈ Ű �¿��� ����� ���� �����Ͽ��ٸ�
			// �ش� ���� ���� ���� �ο��� 1 ������Ŵ
			ListIter.iMaxUser++;
		}

		// m_pMasterToBattleServer �� m_RoomInfoList �Ӱ迵�� ��
		LeaveCriticalSection(&m_csRoomInfoLock);
		return;
	}

	// m_pMasterToBattleServer �� m_RoomInfoList �Ӱ迵�� ��
	LeaveCriticalSection(&m_csRoomInfoLock);


	// m_pMasterToBattleServer �� m_BattleServerInfoMap �Ӱ迵�� ����
	EnterCriticalSection(&m_csPlayStandbyRoomLock);

	map<UINT64, CMasterToBattleServer::st_ROOM_INFO*>::iterator RoomInfoIter = m_PlayStandbyRoomMap.find(BattleRoomInfo);
	
	if (RoomInfoIter != m_PlayStandbyRoomMap.end())
	{
		// �ش� Ŭ���̾�Ʈ Ű�� ������ �ִ� ��带 ����
		if (RoomInfoIter->second->UserClientKeySet.erase(ClientKey) != 0)
		{
			// Ŭ���̾�Ʈ Ű �¿��� ����� ���� �����Ͽ��ٸ�
			// �ش� ���� ���� ���� �ο��� 1 ������Ŵ
			RoomInfoIter->second->iMaxUser++;
		}

		// m_pMasterToBattleServer �� m_BattleServerInfoMap �Ӱ迵�� ��
		LeaveCriticalSection(&m_csPlayStandbyRoomLock);
		return;
	}

	// m_pMasterToBattleServer �� m_BattleServerInfoMap �Ӱ迵�� ��
	LeaveCriticalSection(&m_csPlayStandbyRoomLock);
}

int CMasterToBattleServer::GetRoomInfoMemoryPoolChunkAllocSize()
{
	return m_pRoomInfoMemoryPool->GetAllocChunkCount();
}

int CMasterToBattleServer::GetRoomInfoMemoryPoolChunkUseSize()
{
	return m_pRoomInfoMemoryPool->GetUseChunkCount();
}

int CMasterToBattleServer::GetRoomInfoMemoryPoolBufUseSize()
{
	return m_pRoomInfoMemoryPool->GetUseNodeCount();
}

int CMasterToBattleServer::GetBattleStanbyRoomSize()
{
	return (int)(m_PlayStandbyRoomMap.size() + m_RoomInfoList.size());
}