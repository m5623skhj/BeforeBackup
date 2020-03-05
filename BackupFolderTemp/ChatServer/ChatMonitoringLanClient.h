#pragma once
#include "LanClient.h"
#include "ChatServer.h"
#include "CPUUsageObserver.h"
#include <pdh.h>
#include <pdhmsg.h>
#include <strsafe.h>

#pragma comment(lib, "pdh.lib")

#define dfSESSIONKEY_SIZE			64

#define dfDIVIDE_KILOBYTE			1024
#define dfDIVIDE_MEGABYTE			1048576

class CChatMonitoringLanClient : CLanClient
{
public:
	CChatMonitoringLanClient();
	virtual ~CChatMonitoringLanClient();

	bool MonitoringLanClientStart(const WCHAR *szOptionFileName, UINT *pNumOfSessionAll, UINT *pNumOfLoginCompleteUser);
	void MonitoringLanClientStop();

private:
	// ������ Connect �� �Ϸ� �� ��
	virtual void OnConnectionComplete();

	// ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(CSerializationBuf *OutReadBuf);
	// ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend();

	// ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadBegin();
	// ��Ŀ������ 1���� ���� ��
	virtual void OnWorkerThreadEnd();
	// ����� ���� ó�� �Լ�
	virtual void OnError(st_Error *OutError);

	static UINT __stdcall	MonitoringInfoSendThread(LPVOID MonitoringClient);
	UINT __stdcall			MonitoringInfoSender();

	// ����͸� ���� �۽ſ�
	void SendChatServerOn(int TimeStamp);
	void SendChatCPU(int TimeStamp);
	void SendChatMemoryCommit(int TimeStamp);
	void SendChatPacketPool(int TimeStamp);
	void SendChatSession(int TimeStamp);
	void SendChatPlayer(int TimeStamp);
	void SendChatRoom(int TimeStamp);

	// ��ġ ����ŷ ������ �Ѿ�� ����͸� ���� �۽ſ� �Լ�
	// ��ġ ����ŷ ���� ���� ��ġ ����ŷ ������ �ش� �Լ��� �ű��
	void SendServerCPUTotal(int TimeStamp);
	void SendServerAvailableMemroy(int TimeStamp);
	void SendServerNetworkRecv(int TimeStamp);
	void SendServerNetworkSend(int TimeStamp);
	void SendServerNonpagedMemory(int TimeStamp);


private:
	UINT							*m_pNumOfSessionAll;
	UINT							*m_pNumOfLoginCompleteUser;

	HANDLE							m_hMonitoringThreadHandle;
	CCPUUsageObserver				m_CPUObserver;
	
	PDH_HQUERY						m_PdhQuery;
	PDH_HCOUNTER					m_WorkingSet;
	
	// ��ġ ����ŷ ������ �ӽ� ī���� ����
	PDH_HCOUNTER					m_ProcessorUsage;
	PDH_HCOUNTER					m_AvailableMemory;
	PDH_HCOUNTER					m_NetworkRecv;
	PDH_HCOUNTER					m_NetworkSend;
	PDH_HCOUNTER					m_NonPagedMemory;
};