#pragma once
#include "LanClient.h"
#include "ChatServer.h"

#define dfSESSIONKEY_SIZE			64

struct st_SessionKey;

class CChattingLanClient : CLanClient
{
public:
	CChattingLanClient();
	virtual ~CChattingLanClient();

	bool ChattingLanClientStart(CChatServer* pChattingServer, const WCHAR *szOptionFileName, CTLSMemoryPool<st_SessionKey> *pSessionKeyMemoryPool, UINT64 SessionKeyMapAddr);
	void ChattingLanClientStop();

	CRITICAL_SECTION* GetSessionKeyMapCS() { return &m_SessionKeyMapCS; }

	int GetUsingLanServerBufCount();

	int NumOfRecvPacket = 0;

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

private:
	UINT64										m_iIdentificationNumber;
	CRITICAL_SECTION							m_SessionKeyMapCS;
	CTLSMemoryPool<st_SessionKey>				*m_pSessionKeyMemoryPool;
	std::unordered_map<UINT64, st_SessionKey*>	*m_pSessionKeyMap;

	CChatServer									*m_pChattingServer;
};