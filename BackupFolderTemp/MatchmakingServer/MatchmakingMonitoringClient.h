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

class CMatchmakingMonitoringClient : CLanClient
{
public:
	CMatchmakingMonitoringClient();
	virtual ~CMatchmakingMonitoringClient();

	bool MatchmakingMonitoringClientStart(const WCHAR *szMatchmakingMonitoringClientOptionFile, UINT *SessionAll, UINT *LoginPlayer);
	void MatchmakingMonitoringClientStop();

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
	bool MatchmakingMonitoringClientOptionParsing(const WCHAR *szOptionFileName);

	// MonitoringInfoSender �� ȣ���Ű�� ���� static �Լ�
	static UINT __stdcall	MonitoringInfoSendThread(LPVOID MonitoringClient);
	// 1 �� ���� �Ͼ�� ����͸� ������ �۽��ϴ� ������
	UINT MonitoringInfoSender();

	// ---------------------------------------------------------------------------
	// ��ġ����ŷ ���� ����͸��� �Լ�
	void SendMatchmakingServerOn(int TimeStamp);
	void SendMatchmakingCPU(int TimeStamp);
	void SendMatchmakingMemoryCommit(int TimeStamp);
	void SendMatchmakingPacketPool(int TimeStamp);
	void SendMatchmakingSessionAll(int TimeStamp);
	void SendMatchmakingLoginPlayer(int TimeStamp);
	void SendMatchmakingEnterRoomSuccessTPS(int TimeStamp);
	// ---------------------------------------------------------------------------

	// ---------------------------------------------------------------------------
	// ���� ������ ����͸��� �Լ�
	void SendCPUTotal(int TimeStamp);
	void SendAvailableMemroy(int TimeStamp);
	void SendNetworkRecv(int TimeStamp);
	void SendNetworkSend(int TimeStamp);
	void SendNonpagedMemory(int TimeStamp);
	// ---------------------------------------------------------------------------

private :
	// �� ����͸� ������ ���� ��ȣ
	int								m_iServerNo;

	// ���� �� ��ġ����ŷ ������ �����ִ� �� �ο� ���� ���� ������
	// Matchmaking �������� �����͸� �Ѱ���
	UINT							*m_pNumOfSessionAll;
	// ���� �� ��ġ����ŷ �������� �α��ο� ������ �� �ο� ���� ���� ������
	// Matchmaking �������� �����͸� �Ѱ��� 
	UINT							*m_pNumOfLoginCompleteUser;

	// ����͸� �����带 ������Ű�� ���� �ڵ�
	HANDLE							m_hMonitoringThreadExitHandle;
	// ����͸� ������ �ڵ�
	HANDLE							m_hMonitoringThreadHandle;
	// ��ġ����ŷ ������ CPU ���� �� ���� ������ �� CPU ������ ����ϱ� ���� ��ü
	CCPUUsageObserver				m_CPUObserver;

	// ---------------------------------------------------------------------------
	// �� ��ġ����ŷ ������ ����͸� ������ �Ѱ��ֱ� ���Ͽ� �ʿ��� ��ü��
	PDH_HQUERY						m_PdhQuery;
	// ��ġ����ŷ ����
	PDH_HCOUNTER					m_PrivateBytes;
	// �� ���� ��ü ��� ����
	PDH_HCOUNTER					m_ProcessorUsage;
	PDH_HCOUNTER					m_AvailableMemory;
	PDH_HCOUNTER					m_NetworkRecv;
	PDH_HCOUNTER					m_NetworkSend;
	PDH_HCOUNTER					m_NonPagedMemory;
	// ---------------------------------------------------------------------------
};
