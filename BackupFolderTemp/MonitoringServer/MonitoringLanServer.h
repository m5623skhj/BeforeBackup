#pragma once
#include "LanServer.h"
#include "MonitoringServerCommon.h"

#define dfLOGIN_SERVER								0
#define dfGAME_SERVER								1
#define dfCHAT_SERVER								2

#define dfNUM_OF_SERVER								5
#define dfNUM_OF_DATATYPE							42

class CMonitoringNetServer;

class CMonitoringLanServer : public CLanServer
{
public :
	CMonitoringLanServer();
	virtual ~CMonitoringLanServer();

	bool MonitoringLanServerStart(const WCHAR *szMonitoringLanServerOptionFileName, const WCHAR *szNetServerOptionFileName, const WCHAR *szMonitoringServerOption);
	void MonitoringLanServerStop();

	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 OutClientID);
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
	static UINT __stdcall DBSendThread(LPVOID pMonitoringServer);
	UINT DBSender();

private :
	struct st_UnitItemInfo
	{
		int iLast = 0;
		int iMax = 0;
		int iMin = 99999;
		int iTotal = 0;
		int iNumOfRecv = 0;
	};

	CMonitoringNetServer			*m_pMonitoringNetServer;
	
	HANDLE							m_hDBSendThreadHandle;
	HANDLE							m_hSendEventHandle;

	CRITICAL_SECTION				m_csUnitData[dfNUM_OF_DATATYPE];
	st_UnitItemInfo					m_UnitValue[dfNUM_OF_DATATYPE];

	st_ServerLoginInfo				m_ServerLoginInfo[dfNUM_OF_SERVER];
};