#pragma once
#include "Session.h"

class CEchoPlayer : public CSession
{
public :
	CEchoPlayer();
	~CEchoPlayer();

private :
	// ���� �����忡 ������ ������
	virtual void OnAuth_ClientJoin();
	// ���� �����忡�� ������ ����
	virtual void OnAuth_ClientLeave(bool IsClientLeaveServer);
	// ���� �����忡 �ִ� ������ ��Ŷ�� ����
	virtual void OnAuth_Packet(CNetServerSerializationBuf *pRecvPacket);
	// ���� �����忡 ������ ������
	virtual void OnGame_ClientJoin();
	// ���� �����忡 ������ ����
	virtual void OnGame_ClientLeave();
	// ���� �����忡 �ִ� ������ ��Ŷ�� ����
	virtual void OnGame_Packet(CNetServerSerializationBuf *pRecvPacket);
	// ���������� ������ ������ ��
	virtual void OnGame_ClientRelease();

private :
	bool							m_bIsLoginPlayer;
	UINT64							m_uiAccountNo;

};