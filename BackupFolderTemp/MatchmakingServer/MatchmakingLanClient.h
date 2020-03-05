#pragma once

#include "LanClient.h"

class CSerializationBuf;
class CMatchmakingServer;

class CMatchmakingLanClient : CLanClient
{
	friend CMatchmakingServer;

public :
	CMatchmakingLanClient();
	virtual ~CMatchmakingLanClient();

	bool MatchmakingLanClientStart(CMatchmakingServer *pMatchmakingNetServer,int ServerNo, const WCHAR *szMatchmakingLanServerOptionFile);
	void MatchmakingLanClientStop();

private :
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

	// ������ �������� �� ������ �䱸�ϱ� ���� ȣ���ϴ� �Լ�
	// MatchmakingNetServer ������ ȣ���� �����ϵ��� private �Լ��� ����
	void SendToMaster_GameRoom(UINT64 ClientKey, UINT64 AccountNo);
	// ������ �������� ������ �濡 ������ �˷��ֱ� ���� ȣ���ϴ� �Լ�
	// MatchmakingNetServer ������ ȣ���� �����ϵ��� private �Լ��� ����
	void SendToMaster_GameRoomEnterSuccess(WORD BattleServerNo, int RoomNo, UINT64 ClientKey);
	// ������ �������� ������ �濡 ������ ���� �Ͽ��ٴ� ���� �˷��ֱ� ���� ȣ���ϴ� �Լ�
	// MatchmakingNetServer ������ ȣ���� �����ϵ��� private �Լ��� ����
	void SendToMaster_GameRoomEnterFail(UINT64 ClientKey);

	// Matchmaking LanClient �ɼ� �Ľ� �Լ�
	bool MatchmakingLanClientOptionParsing(const WCHAR *szOptionFileName);

private :
	// �ش� ������ ������ ������ ���� �Ǿ������� ���� ��
	// MatchmakingNetServer ������ �� ���� ���Ͽ� ����ڵ��� ��� ó������ �Ǵ��Ѵ�
	bool											m_bConnectMasterServer;

	// �ش� ������ ���� ��ȣ
	// �� ���� Matchmaking NetServer ���� �Ѱ��ִ� ���� �����
	int												m_iServerNo;

	// ������ ������ �����ϱ� ���� ��ū
	// ��ȣ ���� �Ͽ� �ۼ��� ���̸� ������ �����ϱ� ���� ������������ ����
	char											m_MasterToken[32];

	// Matchmaking NetServer ������
	// �� ���� MatchmakingLanClientStart �� ���ڷ� ����
	// private ��� �Լ��� �Ǿ��ִ� 
	// SendToClient_GameRoom �� SendToClient_GameRoomEnter �� �����Ѵ�
	CMatchmakingServer								*m_pMatchmakingNetServer;
};
