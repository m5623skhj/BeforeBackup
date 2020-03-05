#pragma once
#include <Windows.h>

#define DEFAULT_BUFFERMAX	8192

template <typename T>
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
	void Initialize(int  BufferSize)
	{
		m_pBuffer = new char[BufferSize];
	}

public:
	CRingbuffer() : m_iFront(0), m_iRear(0), m_iSize(DEFAULT_BUFFERMAX)
	{
		Initialize(DEFAULT_BUFFERMAX);
		InitializeSRWLock(&m_SRWLock);
	}
	~CRingbuffer()
	{
		delete[] m_pBuffer;
	}

	// �ٱ����� rear �� front �� 0���� ������
	void Initialize()
	{
		m_iFront = 0;
		m_iRear = 0;
	}

	// ���� �����͸� �����ϰ� ���� ����ũ�⸦ �Ҵ���
	void Resize(int Size)
	{
		m_iFront = 0;
		m_iRear = 0;
		m_iSize = Size;

		delete[] m_pBuffer;
		Initialize(Size);
	}
	// ���� ������� �뷮
	int GetUseSize()
	{
		int f = m_iFront;
		int r = m_iRear;

		if (f < r)
		{
			return f + m_iSize - r;
		}
		else
		{
			return f - r;
		}
	}
	// ���� �����ִ� �뷮
	int GetFreeSize()
	{
		int f = m_iFront;
		int r = m_iRear;

		if (r == 0)
			return m_iSize - f - 1;
		else
			return m_iSize - f;
	}

	// ���� �����ͷ� �ܺο��� �� ���� ���� �� �ִ� ����
	int GetNotBrokenGetSize()
	{
		// �����忡�� ���� �ǵ帮�� ���� �� �� �����Ƿ�
		// ���������� ���� �־���� �����
		int r = m_iRear;
		int f = m_iFront;

		if (f >= r)
		{
			return f - r;
		}
		else
		{
			return m_iSize - r;
		}
	}
	// ���� �����ͷ� �ܺο��� �� ���� �� �� �ִ� ����
	int GetNotBrokenPutSize()
	{
		// �����忡�� ���� �ǵ帮�� ���� �� �� �����Ƿ�
		// ���������� ���� �־���� �����
		int r = m_iRear;
		int f = m_iFront;

		if (f < r)
		{
			return r - f - 1;
		}
		else
		{
			if (r == 0)
				return m_iSize - f - 1;
			else
				return m_iSize - f;
		}
	}

	int Enqueue(T *pData, int Size);
	int Dequeue(T *pDest, int Size);
	int Peek(T *pDest, int Size);

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
	int LockEnqueue(T *pData, int Size);
	//int LockDequeue(char *pDest, int Size);

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