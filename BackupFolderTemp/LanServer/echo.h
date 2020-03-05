#pragma once
#include "LanServer.h"
#include <unordered_map>

class Log;
class CSerializationBuf;

class CEcho : public CLanServer
{
private :
	struct User
	{
		int SomeData;
	};
	std::unordered_map<UINT64, User*> m_UserMap;
public :
	// Accept �� ����ó�� �Ϸ� �� ȣ��
	virtual void OnClientJoin(UINT64 OutClientID/* Client ���� / ClientID / ��Ÿ��� */);
	// Disconnect �� ȣ��
	virtual void OnClientLeave(UINT64 LeaveClientID);
	// Accept ���� IP ���ܵ��� ���� �뵵
	virtual bool OnConnectionRequest();

	// ��Ŷ ���� �Ϸ� ��
	virtual void OnRecv(UINT64 ReceivedSessionID, CSerializationBuf *OutReadBuf);
	// ��Ŷ �۽� �Ϸ� ��
	virtual void OnSend();

	// ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
	virtual void OnWorkerThreadBegin();
	// ��Ŀ������ 1���� ���� ��
	virtual void OnWorkerThreadEnd();

	// LanServer ���� ���� �߻��� ȣ��
	// ���ϰ� �ֿܼ� �������� GetLastError() ��ȯ���� ���� ������ Error �� ����ϸ�
	// 1000 �� ���ķ� ��ӹ��� Ŭ�������� ����� �����Ͽ� ��� �� �� ����
	// ��, GetLastError() �� ��ȯ���� 10054 ���� ���� �̸� �������� ����
	virtual void OnError(st_Error *OutError);

	CEcho(const WCHAR *IP, UINT PORT, BYTE NumOfWorkerThread, bool IsNagle, UINT MaxClient);
	virtual ~CEcho();

	//////////////////////////////////////////////////////////////
	//ULONGLONG m_ULLCountOfNew;
	//////////////////////////////////////////////////////////////
};