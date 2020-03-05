#pragma once
#include <Windows.h>

#define DEFAULT_BUFFERMAX	8192

class CRingbuffer
{
private:
	int m_iFront;
	int m_iRear;
	int m_iSize;
	char *m_pBuffer;
	SRWLOCK m_SRWLock;
	//CRITICAL_SECTION m_CriticalSection;

private:
	void Initialize(int  BufferSize);

public:
	CRingbuffer();
	~CRingbuffer();

	// �ٱ����� rear �� front �� 0���� ������
	void InitPointer();

	// ���� �����͸� �����ϰ� ���� ����ũ�⸦ �Ҵ���
	void Resize(int Size);
	// ���� ������� �뷮
	int GetUseSize();
	// ���� �����ִ� �뷮
	int GetFreeSize();

	// ���� �����ͷ� �ܺο��� �� ���� ���� �� �ִ� ����
	int GetNotBrokenGetSize();
	// ���� �����ͷ� �ܺο��� �� ���� �� �� �ִ� ����
	int GetNotBrokenPutSize();

	int Enqueue(char *pData, int Size);
	int Dequeue(char *pDest, int Size);
	int Peek(char *pDest, int Size);

	// ���ϴ� ���̸�ŭ �б� ��ġ���� ����
	void RemoveData(int Size);
	// ���ϴ� ���̸�ŭ �б� ���� ��ġ �̵�
	void MoveWritePos(int Size);

	void ClearBuffer();

	char *GetBufferPtr();
	char *GetReadBufferPtr();
	char *GetWriteBufferPtr();

	bool IsEmpty();
	bool IsFull();

	//---------------- SRWLock ���� �Լ� ----------------//

	// SRWLockExclusive �� Enqueue �۾��� ������
	// �۾��� ������ Lock�� ������
	int LockEnqueue(char *pData, int Size);
	int LockDequeue(char *pDest, int Size);

	// SRWLockExclusive �� MoveWritePos �۾��� ������
	// �۾��� ������ Lock�� ������
	void LockMoveWritePos(int Size);
	//void LockRemoveData(int Size);

	// �ܺο��� �ش� ť�� bool ���� �̿��Ͽ� Shared / Exclusive ���� Lock ��
	// Lock ���� �Լ��鿡�Ը� ������ ����
	// �׷��� �̰� ȿ�뼺�� �������� �𸣰���
	void QueueLock(bool IsShared);
	// �ܺο��� �ش� ť�� bool ���� �̿��Ͽ� Shared / Exclusive ���� Lock�� ������
	// Lock ���� �Լ��鿡�Ը� ������ ����
	// �׷��� �̰� ȿ�뼺�� �������� �𸣰���
	void QueueUnlock(bool IsShared);

	//---------------- SRWLock ���� �Լ� ----------------//
};