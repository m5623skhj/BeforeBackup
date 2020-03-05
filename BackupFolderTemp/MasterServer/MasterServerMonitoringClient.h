#pragma once

#include "LanClient.h"
#include "CPUUsageObserver.h"
#include <pdh.h>
#include <pdhmsg.h>

#pragma comment(lib, "pdh.lib")

#define dfSESSIONKEY_SIZE			64

#define dfDIVIDE_KILOBYTE			1024
#define dfDIVIDE_MEGABYTE			1048576

class CSerializationBuf;
class CMasterServer;

class CMasterMonitoringClient : CLanClient
{
public:
	CMasterMonitoringClient();
	~CMasterMonitoringClient();

	bool MasterMonitoringClientStart(const WCHAR *szMasterMonitoringClientOptionFile, CMasterServer *pMasterServer);
	void MasterMonitoringClientStop();

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

	// Matchmaking Monitoring LanClient �ɼ� �Ľ� �Լ�
	bool MasterMonitoringClientOptionParsing(const WCHAR *szOptionFileName);

	// MonitoringInfoSender �� ȣ���Ű�� ���� static �Լ�
	static UINT __stdcall	MonitoringInfoSendThread(LPVOID MonitoringClient);
	// 1 �� ���� �Ͼ�� ����͸� ������ �۽��ϴ� ������
	UINT MonitoringInfoSender();

	// ---------------------------------------------------------------------------
	// ������ ���� ����͸��� �Լ�
	void SendMasterServerOn(int TimeStamp);
	void SendMasterServerCPU(int TimeStamp);
	void SendMasterServerMemoryCommit(int TimeStamp);
	void SendMasterServerPacketPool(int TimeStamp);
	void SendMasterServerMatchmakingSessionAll(int TimeStamp);
	void SendMasterServerMatchmakingLoginSession(int TimeStamp);
	void SendMasterServerStayPlayer(int TimeStamp);
	void SendMasterServerBattleSessionAll(int TimeStamp);
	void SendMasterServerBattleLoginSession(int TimeStamp);
	void SendMasterServerStanbyRoom(int TimeStamp);
	
	void SendCPUTotal(int TimeStamp);
	// ---------------------------------------------------------------------------

private:
	// �� ����͸� ������ ���� ��ȣ
	int								m_iServerNo;

	// ���� �� ������ ������ ���� �ִ� �� ��Ʋ ���� ���� ���� ������
	// MasterToBattle ���� �����͸� �Ѱ���
	int								*m_pNumOfMasterSessionAll;
	// ���� �� ������ �������� �α��ο� ������ �� ��Ʋ ���� ���� ���� ������
	// MasterToBattle ���� �����͸� �Ѱ���
	int								*m_pNumOfLoginCompleteMasterSession;

	// ������ ���� ����͸� �Լ� �� �� ������
	CMasterServer					*m_pMasterServer;

	// ����͸� �����带 ������Ű�� ���� �ڵ�
	HANDLE							m_hMonitoringThreadExitHandle;
	// ����͸� ������ �ڵ�
	HANDLE							m_hMonitoringThreadHandle;
	// ��ġ����ŷ ������ CPU ���� �� ���� ������ �� CPU ������ ����ϱ� ���� ��ü
	CCPUUsageObserver				m_CPUObserver;

	// ---------------------------------------------------------------------------
	// �� ������ ������ ����͸� ������ �Ѱ��ֱ� ���Ͽ� �ʿ��� ��ü��
	PDH_HQUERY						m_PdhQuery;
	PDH_HCOUNTER					m_PrivateBytes;

	PDH_HCOUNTER					m_ProcessorUsage;
};
