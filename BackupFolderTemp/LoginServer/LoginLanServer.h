#pragma once
#include "LanServer.h"

#define dfSESSIONKEY_SIZE			64

//#define dfACQUIRE_RESPONSE_SHIFT	60
#define dfACQUIRE_RESPONSE_SHIFT	60
#define dfIDENTIFIER_ERR_SHIFT		56
//#define dfAND_PARAMETER_PART		0x0fffffffffffffff
#define dfAND_PARAMETER_PART		0x00ffffffffffffff

#define dfNUM_OF_SERVER				1

class CLoginServer;

class CLoginLanServer : CLanServer
{
public:
	CLoginLanServer();
	virtual ~CLoginLanServer();

	bool LoginLanServerStart(CLoginServer* pLoginServer, const WCHAR *szOptionFileName);
	void Stop();

	void SendLoginPacketToLanClientAll(INT64 AccountNo, char *SessionKey);

	bool ChattingServerJoined() { return m_bChattingServerClientJoined; }

	int GetUsingSessionNodeCount() { return m_pSessionInfoTLS->GetUseNodeCount(); }
	int GetUsingLanSerializeBufCount();

	int NumOfSendLanClient;
	int NumOfRecvLanClient;

private :
	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 OutClientID/* Client ���� / ClientID / ��Ÿ��� */);
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

private :
	struct st_ServerInfo
	{
		BYTE	ServerType;			// dfSERVER_TYPE_GAME / dfSERVER_TYPE_CHAT
		UINT64  ClientSessionID;
		WCHAR	ServerName[32];		// �ش� ������ �̸�. 
	};

	struct st_SessionInfo
	{
		UINT64 SessionKeyIdentifier;
		BYTE   SessionKey[64];
	};
	
	BYTE										m_byServerIndex;
	bool										m_bChattingServerClientJoined = false;

	UINT64										m_iIdentificationNumber;
	CTLSMemoryPool<st_SessionInfo>				*m_pSessionInfoTLS;
	std::unordered_map<INT64, st_SessionInfo*>	m_AccountMap;
	CRITICAL_SECTION							m_AccountMapCriticalSection;

	CLoginServer								*m_pLoginServer;
	st_ServerInfo								m_ChattingServer;
};
