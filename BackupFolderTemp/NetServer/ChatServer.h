#pragma once
#include "NetServer.h"
#include <unordered_map>
#include <string>

#include "CommonSource/LockFreeQueueA.h"
using namespace Olbbemi;

#define dfMAX_SECTOR_X									51
#define dfMAX_SECTOR_Y									51

#define dfID_NICKNAME_LENGTH							40

#define dfWAIT_UPDATETHREAD_TIMEOUT						10

#define dfSTOP_SERVER									0
#define dfRECVED_PACKET									1

#define dfINIT_SECTOR_VALUE								65535
#define dfACCOUNT_INIT_VALUE							0xffffffffffffffff

#define dfPACKET_TYPE_JOIN								90
#define dfPACKET_TYPE_LEAVE								91
#define dfPACKET_TYPE_RECV_FROM_LOGIN_SERVER			92

#define dfPACKETSIZE_SECTOR_MOVE						12
#define dfPACKETSIZE_CHAT_MESSAGE						10
#define dfPACKETSIZE_LOGIN								152

#define dfSESSIONKEY_SIZE								64

#define dfHEARTBEAT_TIMEOVER							30

// �ܺ� ����ڰ� ������ ����ϰ� �ϰ� ������
// Ư�� �� ���� ���Ŀ��� ��� �����ϰ� �Ѵٴ� ������ �ʿ���
// ���� ���� �� ��
enum CHATSERVER_ERR
{
	NOT_IN_USER_MAP_ERR = 1001,
	NOT_IN_USER_SECTOR_MAP_ERR,
	NOT_IN_USER_ACCOUNTMAP_ERR,
	INCORRECT_SESSION_KEY_ERR,
	NOT_LOGIN_USER_ERR,
	SERIALIZEBUF_SIZE_ERR,
	SECTOR_RANGE_ERR,
	INCORRECT_ACCOUNTNO,
	INCORREC_PACKET_ERR,
	SAME_ACCOUNTNO_ERR,
};

class Log;
class CNetServerSerializationBuf;
class CChattingLanClient;

struct st_SessionKey
{
	char SessionElement[64];
};

class CChatServer : public CNetServer
{
private:
	struct st_MESSAGE
	{
		WORD										wType;
		UINT64										uiSessionID;
		CNetServerSerializationBuf					*Packet;
	};

	struct st_USER
	{
		BOOL										bIsLoginUser;
		BYTE										byNumOfOverWriteSessionKey;
		WORD										SectorX;
		WORD										SectorY; 
		UINT64										uiBeforeRecvTime;
		UINT64										uiSessionID;
		UINT64										uiAccountNO;
		WCHAR										szID[20];
		WCHAR										szNickName[20];
		char										SessionKey[64];
	};

	int												m_iNumOfRecvJoinPacket;
	int												m_iNumOfSessionKeyMiss;
	int												m_iNumOfSessionKeyNotFound;

	UINT											m_uiUpdateTPS;

	UINT64											m_uiHeartBeatTime;
	
	UINT64											m_uiAccountNo;
	CTLSMemoryPool<st_SessionKey>					*m_pSessionKeyMemoryPool;

	CTLSMemoryPool<st_USER>							*m_pUserMemoryPool;
	CRITICAL_SECTION								m_SessionKeyMapCS;
	std::unordered_map<UINT64, st_SessionKey*>		m_UserSessionKeyMap;
	std::unordered_map<UINT64, st_USER*>			m_UserSessionMap;
	std::unordered_map<UINT64, st_USER*>			m_UserSectorMap[dfMAX_SECTOR_Y][dfMAX_SECTOR_X];

	CTLSMemoryPool<st_MESSAGE>						*m_pMessageMemoryPool;
	C_LFQueue<st_MESSAGE*>							*m_pMessageQueue;

	CChattingLanClient								*m_pChattingLanClient;

	HANDLE											m_hUpdateThreadHandle;
	HANDLE											m_hUpdateEvent;
	HANDLE											m_hExitEvent;
	
	void BroadcastSectorAroundAll(WORD CharacterSectorX, WORD CharacterSectorY, CNetServerSerializationBuf *SendBuf);

	static UINT __stdcall UpdateThread(LPVOID pChatServet);
	UINT Updater();
	static UINT __stdcall HeartbeatCheckThread(LPVOID pChatServet);
	UINT HeartbeatChecker();
	
	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 JoinClientID/* Client ���� / ClientID / ��Ÿ��� */);
	// Disconnect �� ȣ��
	virtual void OnClientLeave(UINT64 LeaveClientID);
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

	// LanServer ���� ���� �߻��� ȣ��
	// ���ϰ� �ֿܼ� �������� GetLastError() ��ȯ���� ���� ������ Error �� ����ϸ�
	// 1000 �� ���ķ� ��ӹ��� Ŭ�������� ����� �����Ͽ� ��� �� �� ����
	// ��, GetLastError() �� ��ȯ���� 10054 ���� ���� �̸� �������� ����
	virtual void OnError(st_Error *OutError);

	bool PacketProc_PlayerJoin(UINT64 SessionID);
	bool PacketProc_PlayerLeave(UINT64 SessionID);
	bool PacketProc_PlayerLoginFromLoginServer(CNetServerSerializationBuf *pRecvPacket);
	void PacketProc_HeartBeat(CNetServerSerializationBuf *pRecvPacket, CNetServerSerializationBuf *pOutSendPacket);
	bool PacketProc_SectorMove(CNetServerSerializationBuf *pRecvPacket, CNetServerSerializationBuf *pOutSendPacket, UINT64 SessionID);
	bool PacketProc_ChatMessage(CNetServerSerializationBuf *pRecvPacket, CNetServerSerializationBuf *pOutSendPacket, UINT64 SessionID);
	bool PacketProc_Login(CNetServerSerializationBuf *pRecvPacket, CNetServerSerializationBuf *pOutSendPacket, UINT64 SessionID);

public:

	CChatServer();
	virtual ~CChatServer();

	bool ChattingServerStart(const WCHAR *szChatServerOptionFileName, const WCHAR *szChatServerLanClientFileName);
	void ChattingServerStop();

	bool LoginPacketRecvedFromLoginServer(UINT64 AccountNo, st_SessionKey *SessionKey);

	int GetNumOfPlayer();
	int GetNumOfAllocPlayer();
	int GetNumOfAllocPlayerChunk();

	int GetNumOfMessageInPool();
	int GetNumOfMessageInPoolChunk();
	int GetRestMessageInQueue();
	UINT GetUpdateTPSAndReset();

	int GetNumOfClientRecvPacket();
	int GetNumOfRecvJoinPacket();
	int GetNumOfSessionKeyMiss();
	int GetNumOfSessionKeyNotFound();

	int GetUsingNetServerBufCount();
	int GetUsingLanServerBufCount();
};