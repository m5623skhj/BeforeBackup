#pragma once

#include "NetServer.h"
#include <unordered_map>

#define dfUSER_INFO_SIZE						80
#define dfWCHAR_IP_SIZE							32


class CNetServerSerializationBuf;
class CDBConnector;
class CLoginLanServer;

class CLoginServer : CNetServer
{
public :
	CLoginServer();
	virtual ~CLoginServer();

	bool LoginServerStart(const WCHAR *szLoginServerFileName, const WCHAR *szLoginLanServerFileName, const WCHAR *szLogInServerOptionFile);
	void LoginServerStop();
	void LoginComplete(UINT64 AccountNo, BYTE Status);

	bool ChattingServerJoined();

	int NumOfLoginWaitingUser;
	int NumOfLoginCompleteUser;

	int GetLanServerSend();
	int GetLanServerRecv();
	int GetNetServerAcceptTotal() { return GetAcceptTotal(); }

	int GetUsingNetSerializeBufCount();
	int GetUsingNetUserInfoNodeCount();

	int GetUsingLanServerSessionNodeCount();
	int GetUsingLanSerializeBufCount();

private:
	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 OutClientID/* Client ���� / ClientID / ��Ÿ��� */);
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

	bool LoginServerOptionParsing(const WCHAR *szLogInServerOptionFile);

	struct st_LoginUserInfo
	{
		UINT64		uiSessionID;
		WCHAR		szID[20];
		WCHAR		szNick[20];
	};

	CRITICAL_SECTION								m_UserSessionIDMapCriticalSection;
	std::unordered_map<INT64, st_LoginUserInfo*>	m_LoginUserInfoMap;
	CTLSMemoryPool<st_LoginUserInfo>				*m_pUserInfoMemeoryPool;

	//CDBConnector									*m_pDBConnector;
	// ä�ü������� ���� ����� ���� ��ü
	CLoginLanServer									*m_pLoginLanServer;

	USHORT											m_usGameServerPort;
	USHORT											m_usChatServerPort;
	WCHAR											m_GameServerIP[16];
	WCHAR											m_ChatServerIP[16];
};