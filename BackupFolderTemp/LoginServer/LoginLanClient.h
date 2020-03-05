#pragma once
#include "LanClient.h"

class CLoginLanClient : CLanClient
{
public:
	CLoginLanClient();
	virtual ~CLoginLanClient();

	// Accept ���� IP ���ܵ��� ���� �뵵
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
private:

};