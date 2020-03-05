#pragma once
#include "LanClient.h"
#include "CPUUsageObserver.h"
#include <pdh.h>
#include <pdhmsg.h>
#include <strsafe.h>

#pragma comment(lib, "pdh.lib")

#define dfSESSIONKEY_SIZE			64

#define dfDIVIDE_KILOBYTE			1024
#define dfDIVIDE_MEGABYTE			1048576

class CMMOMonitoringLanClient : CLanClient
{
public:
	CMMOMonitoringLanClient();
	virtual ~CMMOMonitoringLanClient();

	bool MonitoringLanClientStart(const WCHAR *szOptionFileName, UINT *pNumOfSessionAll,
		UINT *NumOfSessionAuth, UINT *NumOfSessionGame, UINT *AuthFPS, UINT *GameFPS,
		UINT *NumOfRoomWait, UINT *NumOfRoomPlay);
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
	void SendGameServerOn(int TimeStamp);
	void SendGameCPU(int TimeStamp);
	void SendGameMemoryCommit(int TimeStamp);
	void SendGamePacketPool(int TimeStamp);
	void SendGameAuthFPS(int TimeStamp);
	void SendGameGameFPS(int TimeStamp);
	void SendGameSessionAll(int TimeStamp);
	void SendGameAuthSession(int TimeStamp);
	void SendGameGameSession(int TimeStamp);
	void SendGameRoomWait(int TimeStamp);
	void SendGameRoomPlay(int TimeStamp);

private:
	UINT							*m_pAuthFPS;
	UINT							*m_pGameFPS;
	UINT							*m_pNumOfSessionAll;
	UINT							*m_pNumOfSessionAuth;
	UINT							*m_pNumOfSessionGame;
	UINT							*m_pNumOfRoomWait;
	UINT							*m_pNumOfRoomGame;

	HANDLE							m_hMonitoringThreadHandle;
	CCPUUsageObserver				m_CPUObserver;

	PDH_HQUERY						m_PdhQuery;
	PDH_HCOUNTER					m_PrivateByte;
};