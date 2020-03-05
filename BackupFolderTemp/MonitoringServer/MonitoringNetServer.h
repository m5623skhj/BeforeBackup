#pragma once
#include "NetServer.h"
#include "MonitoringServerCommon.h"

#define dfACCOUNT_MAX			30

class CMonitoringNetServer : public CNetServer
{
public :
	CMonitoringNetServer();
	virtual ~CMonitoringNetServer();

	bool MonitoringNetServerStart(const WCHAR *szNetServerOptionFileName, const WCHAR *szMonitoringServerOption);
	void MonitoringNetServerStop();

	bool MonitoringServerOptionParsing(const WCHAR *szOptionFileName);

	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 OutClientID);
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

	void SendMonitoringInfoToNetClient(st_MonitorToolUpdateData &SendToolData);
private :
	//WORD									m_wNumOfMaxMonitoringClient;
	//WORD									m_wNowMonitoringClientIndex;
	//st_ServerLoginInfo						*m_pNetServerLoginInfo;
	char									m_LoginSessionKey[32];

	int										m_NumOfLoginUser;
	UINT64									m_uiAccountNoArray[dfACCOUNT_MAX];
	CRITICAL_SECTION						m_AccountNoArrayCS;
	//std::map<UINT64, st_ServerLoginInfo>	m_LoginInfoMap;
	//st_ServerLoginInfo						m_NetServerLoginInfo;
};