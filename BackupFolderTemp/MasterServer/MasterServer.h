#pragma once

#define dfSIZE_SERVER_ON					36
#define dfSIZE_GAME_ROOM					16
#define dfSIZE_ENTER_SUCCESS				14
#define dfSIZE_ENTER_FAIL					10

#define dfSHIFT_RIGHT_SERVERNO				32

#define dfRESERVER_CLIENTKEYMAP_SIZE		8192

#include "LanServer.h"

#include <unordered_map>
#include <map>

using namespace std;

class CNetServerSerializationBuf;
class CSerializationBuf;
class CMasterToBattleServer;
class CMasterMonitoringClient;

class CMasterServer : CLanServer
{
	friend CMasterToBattleServer;

public :
	CMasterServer();
	~CMasterServer();

	bool MasterServerStart(const WCHAR *szMasterLanServerFileName, const WCHAR *szMasterToBattleServerFileName,
		const WCHAR *szMasterMonitoringClientFileName);
	void MasterServerStop();

	int GetNumOfClientInfo();
	int GetUsingLanServerSessionBufCount();
	int GetUsingLanSerializeChunkCount();
	int GetAllocLanSerializeBufCount();

	// MasterToBattle �� �ִ� �Լ� �� ��
	int CallGetRoomInfoMemoryPoolChunkAllocSize();
	int CallGetRoomInfoMemoryPoolChunkUseSize();
	int CallGetRoomInfoMemoryPoolBufUseSize();
	int CallGetBattleStanbyRoomSize();

public :
	UINT								NumOfMatchmakingSessionAll;
	UINT								NumOfMatchmakingLoginSession;
	UINT								*pNumOfBattleServerSessionAll;
	UINT								*pNumOfBattleServerLoginSession;

private :
	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 ClientID/* Client ���� / ClientID / ��Ÿ��� */);
	// Disconnect �� ȣ��
	virtual void OnClientLeave(UINT64 ClientID);
	// Accept ���� IP ���ܵ��� ���� �뵵
	virtual bool OnConnectionRequest();

	// ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf);
	// ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend(UINT64 ClientID, int sendsize);

	// ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadBegin();
	// ��Ŀ������ 1���� ���� ��
	virtual void OnWorkerThreadEnd();
	// ����� ���� ó�� �Լ�
	virtual void OnError(st_Error *OutError);

	// Master LanServer �ɼ� �Ľ� �Լ�
	bool MasterLanServerOptionParsing(const WCHAR *szOptionFileName);

private :
	struct st_MATCHMAKING_CLIENT
	{
		st_MATCHMAKING_CLIENT()
		{
			bIsAuthorized = false;
			iMatchmakingServerNo = -1;
		}

		bool				bIsAuthorized;
		int					iMatchmakingServerNo;
	};

	//struct st_CLIENT_INFO
	//{
	//	int					iBattleServerNo;
	//	int					iRoomNo;
	//	UINT64				uiClientKey;
	//	INT64				iAccountNo;
	//};

	// ���� ��ġ����ŷ ������ ������ �� ���������� Ȯ���ϴ� ��
	// �� �ȿ� ���� ���ӽ� ��ġ����ŷ ������ �ִ� ServerNo �� ���ԵǾ�����
	// Key : ClientID, Value : st_MATCHMAKING_CLIENT
	map<UINT64, st_MATCHMAKING_CLIENT>				m_AuthorizedSessionMap;
	// m_AuthorizedSessionMap ���� SRWLock
	SRWLOCK											m_AuthorizedSessionMapSRWLock;
	
	// ��Ʋ ���� Ȥ�� ��ġ����ŷ ������ �ִ� Ŭ���̾�Ʈ���� �����ϴ� ��
	// Key : ClientKey, Value : st_CLIENT_INFO
	//unordered_map<UINT64, st_CLIENT_INFO*>			m_ClientInfoMapFindClientKey;
	// key : ClientKey, Valued : ���� 4byte ServerNo / ���� 4byte RoomNo
	unordered_map<UINT64, UINT64>					m_ClientInfoMapFindClientKey;
	// st_CLIENT_INFO ���� TLS �޸� Ǯ
	//CTLSMemoryPool<st_CLIENT_INFO>					*m_pClientInfoMemoryPool;
	// m_ClientInfoMapFindClientKey ���� Critical Section
	CRITICAL_SECTION								m_csClientInfoMapFindClientKey;

	// ������ ������ ��Ʋ �������� ����� ���� ������
	CMasterToBattleServer							*m_pMasterToBattleServer;
	// ����͸� ������ �����ϱ� ���� ������
	CMasterMonitoringClient							*m_pMasterMonitoringClient;

	// ��Ʋ ������ ��ġ����ŷ ������ ������ ������ ���� ��
	// ������ ���� ���ؾ� �� ��ū
	char											m_MasterToken[32];

};