#pragma once

#include "LanServer.h"

#include <list>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

using namespace std;

#define dfSIZE_BATTLE_ON				132
#define dfSIZE_BATTLE_REISSUE_TOKEN		36
#define dfSIZE_CREATE_ROOM				48
#define dfSIZE_CLOSE_ROOM				8
#define dfSIZE_LEFT_USER				16

#define dfSIZE_CONNECT_TOKEN			32
#define dfPRIORITY_QUEUE_DEFAULT_SIZE	5000

struct st_CLIENT_INFO;

class CSerializationBuf;
class CMasterServer;

class CMasterToBattleServer : CLanServer
{
	friend CMasterServer;

public:
	CMasterToBattleServer();
	~CMasterToBattleServer();

	bool MasterToBattleServerStart(const WCHAR *szMasterNetServerFileName, CMasterServer *pCMasterServer);
	void MasterToBattleServerStop();

	int GetRoomInfoMemoryPoolChunkAllocSize();
	int GetRoomInfoMemoryPoolChunkUseSize();
	int GetRoomInfoMemoryPoolBufUseSize();
	int GetBattleStanbyRoomSize();

public:
	UINT								NumOfBattleServerSessionAll;
	UINT								NumOfBattleServerLoginSession;

private:
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

	// Master NetServer �ɼ� �Ľ� �Լ�
	bool MasterToBattleServerOptionParsing(const WCHAR *szOptionFileName);

	// ----------------------------------------------------------------------------------------
	// MastertToBattle ���ν��� �Լ�
	void MasterToBattleServerOnProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf);
	void MasterToBattleConnectTokenProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf);
	void MasterToBattleLeftUserProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf);
	void MasterToBattleCreatedRoomProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf);
	void MasterToBattleClosedRoomProc(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf);
	// ----------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------
	// MatchToMaster ���ν��� �Լ�
	// �Ʒ� �Լ����� ȣ���� ��� ������ �������� �ݵ�� ������ ��ġ�� �� ��
	void MatchToMasterRequireRoom(CSerializationBuf *OutSendBuf);
	void MatchToMasterEnterRoomSuccess(UINT64 ClientKey, int BattleServerNo, int BattleRoomNo);
	void MatchToMasterEnterRoomFail(UINT64 ClientKey, UINT64 BattleRoomInfo);
	// ----------------------------------------------------------------------------------------

private:
	// �����Ϳ��� ���� ��Ʋ�������� ����
	struct st_BATTLE_SERVER_INFO
	{
		st_BATTLE_SERVER_INFO()
		{
			InitializeSRWLock(&ConnectTokenSRWLock);
		}

		WCHAR			ServerIP[16];
		WORD			wPort;
		char			ConnectTokenNow[32];
		char			ConnectTokenBefore[32];
		WCHAR			ChatServerIP[16];
		WORD			wChatServerPort;
		int				iServerNo;

		SRWLOCK			ConnectTokenSRWLock;
	};

	// �� ����
	struct st_ROOM_INFO
	{
		int						iBattleServerNo;
		int						iRoomNo;
		//int						iRemainingUser;
		int						iMaxUser;
		char					EnterToken[32];
		// UserClientKey ���� ���� ��
		//CRITICAL_SECTION		UserSetLock;
		// ������ �ش� �濡 �ִٴ� ���� Ȯ���ϱ� ���� ��
		set<UINT64>				UserClientKeySet;
	};

	// m_RoomInfoPriorityQueue �� ���� ���
	//struct st_ROOM_INFO_COMPARE
	//{
	//	bool operator() (st_ROOM_INFO *lhs, st_ROOM_INFO *rhs)
	//	{
	//		if (lhs->iRemainingUser > rhs->iRemainingUser)
	//			return true;
	//		else
	//			return false;
	//	}
	//};
	
	// ���� ��Ʋ ������ ������ �� ���������� Ȯ���ϴ� ��
	// Key : ClientID, Value : st_MATCHMAKING_CLIENT
	map<UINT64, st_BATTLE_SERVER_INFO*>											m_BattleServerInfoMap;
	// m_BattleServerInfoMap ���� SRWLock
	SRWLOCK																		m_BattleServerInfoMapSRWLock;
	// m_BattleServerInfoMap ���� Critical Section
	//CRITICAL_SECTION															m_csBattleServerInfoLock;
	// BATTLE_CLIENT ���� TLS �޸� Ǯ
	CTLSMemoryPool<st_BATTLE_SERVER_INFO*>										*m_pBattleServerInfoMemoryPool;
	
	// �����ϰ� �ִ� st_ROOM_INFO ����ü�� �߿���
	// ������ ���� ������ ������ ���� �� �� �ֵ��� �ϱ� ���Ͽ�
	// MaxUser �� ���� �Ǿ��ִ� Min Heap �� �����
	//priority_queue<st_ROOM_INFO*, vector<st_ROOM_INFO*>, st_ROOM_INFO_COMPARE>	m_RoomInfoPriorityQueue;
	//vector<st_ROOM_INFO*>														m_RoomInfoPriorityQueue;

	// �� ���� �濡 �ο��� �� ���� �ʾҴ� ��鿡 ���� ����Ʈ
	// �� ����Ʈ�� ���� ���� ��Ŷ�� �޾��� �� �ش� ���� push_back �ϰ�
	// ���� ������ ���� ��Ŷ�� �޾��� ���� ��ȸ�� �ϸ�
	// �濡 �ο��� �� ���ٸ� m_PlayStandbyRoom ���� �����ϴ� �����͸� �Ѱ��ش�
	list<st_ROOM_INFO*>															m_RoomInfoList;
	// m_RoomInfoList ���� Critical Section
	CRITICAL_SECTION															m_csRoomInfoLock;
	// ROOM_INFO ���� TLS �޸� Ǯ
	CTLSMemoryPool<st_ROOM_INFO>												*m_pRoomInfoMemoryPool;
	// ������ ������ �Ǵ����� �� �濡 �� �� �� �ִ� �ο��� ���ٸ�
	// m_PlayStandbyRoom ���� ���� m_PlayStandbyRoom �� ����־�
	// ���̻� �濡 �ο��� �߰����� ���ϵ��� ��
	// ���� ������ ��Ʋ�������� �ش� �濡 ���� ���� ���� ��Ŷ�� ����
	// �� m_PlayStandbyRoomMap ������ �ǰ�ü�� ��ȯ�Ѵ�
	// �̷��� ������ ��ġ�� ������ ��Ŷ�� ������ �ٷ� ���� �ı��ϰ�
	// ���� ��Ʋ���� ��Ŷ�� �����ϱ� ������ �ο��� ���������ٸ�
	// �����Ϳ����� �ı��Ǿ����� ��Ʋ������ 1 ���� �� ä�� ������ ���� ������ ���ϰ� ��
	// �� �ڷᱸ���� �����Ͱ� �Ѿ�Դٸ� �ο��� ���������� m_PlayStandbyRoom �� ���ư��� ������
	// ������ �ο��� ���� �� �� �� ���� ��ȸ�Ͽ� �켱������ �ο��� ä���
	// ���� �ӵ��� �����ٰ� �Ǵܵȴٸ� �ο��� �����ִ� �� ������ RoomNo �� �����ϴ�
	// list �� �ξ �� �տ� �ִ� �͸� �̾Ƴ��� ���� �� ä��� ���� �� ���°͵� ����� ��
	// Key : RoomNo, Value : st_ROOM_INFO*
	map<UINT64, st_ROOM_INFO*>													m_PlayStandbyRoomMap;
	// m_PlayStandbyRoomMap ���� Critical Section
	CRITICAL_SECTION															m_csPlayStandbyRoomLock;

	// ��Ʋ ���� Ȥ�� ��ġ����ŷ ������ �ִ� Ŭ���̾�Ʈ���� �����ϴ� ��
	// Key : AccountNo, Value : st_CLIENT_INFO
	//unordered_map<UINT64, st_CLIENT_INFO*>										m_ClientInfoMapFindAccountNo;
	// m_ClientInfoMapFindClientKey ���� Critical Section
	//CRITICAL_SECTION															m_csClientInfoMapFindAccountNo;

	// ������ �������� �����ִ� ��Ʋ ������ ��ȣ
	UINT																		m_iBattleServerNo;

	// ������ ������ ����ϱ� ���� ������
	CMasterServer																*m_pMasterServer;

};