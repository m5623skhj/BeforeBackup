#pragma once
#include "MMOServer.h"
#include "EchoPlayer.h"

class CMMOMonitoringLanClient;

class CEchoServer : public CMMOServer
{
public :
	CEchoServer();
	~CEchoServer();

	bool EchoServerStart(const WCHAR *szMMOServerOptionFileName, const WCHAR *szMonitoringClientFileName);
	void EchoServerStop();

	UINT GetNumOfUsingSerializeBuf();

private :
	// ���� �������� �� �����Ӹ��� ȣ���
	virtual void OnAuth_Update();
	// ������Ʈ �������� �� �����Ӹ��� ȣ���
	virtual void OnGame_Update();
	// ���� ��Ȳ �߻��� ȣ�� ��
	virtual void OnError(st_Error *pError);

	bool ParsingEchoOption();

private :
	bool							m_bIsServerRunning;

	UINT							m_uiNumOfPlayer;
	CEchoPlayer						*m_pEchoPlayerArray;
	CMMOMonitoringLanClient			*m_pMonitoringClient;
};