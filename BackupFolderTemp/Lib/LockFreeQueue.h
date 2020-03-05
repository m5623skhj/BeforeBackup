#pragma once

#include "LockFreeMemoryPool.h"

#define DEFAULT_Q_SIZE 1024

enum {
	ALLOC = 0,
	FREE,
	ENQ_TRY,
	ENQ_COMPLETE,
	DEQ_TRY,
	DEQ_COMPLETE,
	CRASH_EXCEPTION,
	ENQ_MOVE_TAIL_TRY,
	ENQ_MOVE_TAIL_COMPLETE,
};

template <typename Data>
class CLockFreeQueue
{
private:
	///////////////////////////
	struct st_NODECHASER
	{
		char   who;
		char   Behavior;
		LONG64 CurHead;
		LONG64 CurTail;
	};
	///////////////////////////

	struct st_NODE
	{
		st_NODE									*pNext;
		Data									data;

		///////////////////////////
		//int										CallCount = 0;
		//st_NODECHASER							NodeChaser[1000];
		///////////////////////////
	};

	// ullBlockID �� ���� �ø���
	struct st_Q_NODE_BLOCK
	{
		st_NODE									*pNode;
		ULONGLONG								ullBlockID;
	};

	UINT										m_uiRestSize;
	__declspec(align(16)) st_Q_NODE_BLOCK		m_Head;
	__declspec(align(16)) st_Q_NODE_BLOCK		m_Tail;
	CLockFreeMemoryPool<__declspec(align(16)) st_NODE>				*m_pLockFreeMemeoryPool;

public:
	CLockFreeQueue();
	CLockFreeQueue(int MakeNumOfQNode);
	~CLockFreeQueue();

	void InitQueue();
	/////////////////////////////
	//void TestMake(int MakeNumOfQNode);
	//void TestDestroy(int *OutUseSize, int *OutAllocSize);
	/////////////////////////////

	void Enqueue(Data InputData);
	bool Dequeue(Data *pOutData);

	int GetRestSize();
	int GetMemoryPoolUseSize();
	int GetMemoryPoolAllocSize();
};

template <typename Data>
CLockFreeQueue<Data>::CLockFreeQueue() :
	m_uiRestSize(0)
{
	m_pLockFreeMemeoryPool = new CLockFreeMemoryPool<st_NODE>(DEFAULT_Q_SIZE, false);

	m_Head.pNode = m_pLockFreeMemeoryPool->Pop();
	m_Head.ullBlockID = 0;
	m_Head.pNode->pNext = nullptr;
	m_Tail.pNode = m_Head.pNode;
	m_Tail.pNode->pNext = nullptr;
	m_Tail.ullBlockID = 0;
}

template <typename Data>
CLockFreeQueue<Data>::CLockFreeQueue(int MakeNumOfQNode) :
	m_uiRestSize(MakeNumOfQNode)
{
	m_pLockFreeMemeoryPool = new CLockFreeMemoryPool<st_NODE>(MakeNumOfQNode, false);

	m_Head.pNode = m_pLockFreeMemeoryPool->Pop();
	m_Head.ullBlockID = 0;
	m_Head.pNode->pNext = nullptr;
	m_Tail.pNode = m_Head.pNode;
	m_Tail.pNode->pNext = nullptr;
	m_Tail.ullBlockID = 0;
}

template <typename Data>
CLockFreeQueue<Data>::~CLockFreeQueue()
{
	while (m_Head.pNode != nullptr)
	{
		st_NODE* curNode = m_Head.pNode;
		m_Head.pNode = m_Head.pNode->pNext;
		m_pLockFreeMemeoryPool->Push(curNode);
	}

	delete m_pLockFreeMemeoryPool;
}

template <typename Data>
void CLockFreeQueue<Data>::InitQueue()
{
	m_Head.ullBlockID = 0;
	m_Head.pNode->pNext = nullptr;
	m_Tail.pNode = m_Head.pNode;
	m_Tail.pNode->pNext = nullptr;
	m_Tail.ullBlockID = 0;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
//template <typename Data>
//void CLockFreeQueue<Data>::TestMake(int MakeNumOfQNode)
//{
//	m_pLockFreeMemeoryPool = new CLockFreeMemoryPool<st_NODE>(MakeNumOfQNode, false);
//
//	m_Head.pNode = m_pLockFreeMemeoryPool->Pop();
//	m_Tail.pNode = m_Head.pNode;
//	m_Tail.pNode->pNext = nullptr;
//	m_uiRestSize = MakeNumOfQNode;
//}
//
//
//template <typename Data>
//void CLockFreeQueue<Data>::TestDestroy(int *OutUseSize, int *OutAllocSize)
//{
//	while (m_Head.pNode != nullptr)
//	{
//		st_NODE* curNode = m_Head.pNode;
//		m_Head.pNode = m_Head.pNode->pNext;
//		m_pLockFreeMemeoryPool->Push(curNode);
//		--m_uiRestSize;
//	}
//
//	*OutUseSize = m_pLockFreeMemeoryPool->GetUseCount();
//	*OutAllocSize = m_pLockFreeMemeoryPool->GetAllocCount();
//
//	delete m_pLockFreeMemeoryPool;
//}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

template <typename Data>
void CLockFreeQueue<Data>::Enqueue(Data InputData)
{
	__declspec(align(16)) st_Q_NODE_BLOCK NewTail, CurTail;
	NewTail.pNode = m_pLockFreeMemeoryPool->Pop();
	NewTail.pNode->data = InputData;
	NewTail.pNode->pNext = nullptr;

	/////////////////////////////////////////////////////
	//if (NewTail.pNode->CallCount == 1000)
	//	NewTail.pNode->CallCount = 0;
	//else
	//	NewTail.pNode->CallCount = NewTail.pNode->CallCount + 1;
	//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].who = Caller;
	//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].Behavior = ALLOC;
	//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurHead = (LONG64)m_Head.pNode;
	//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurTail = (LONG64)m_Tail.pNode;
	/////////////////////////////////////////////////////

	while (1)
	{
		CurTail.ullBlockID = m_Tail.ullBlockID;
		CurTail.pNode = m_Tail.pNode;
		NewTail.ullBlockID = CurTail.ullBlockID + 1;

		/////////////////////////////////////////////////////
		//if (NewTail.pNode->CallCount == 1000)
		//	NewTail.pNode->CallCount = 0;
		//else
		//	NewTail.pNode->CallCount = NewTail.pNode->CallCount + 1;
		//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].who = Caller;
		//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].Behavior = ENQ_TRY;
		//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurHead = (LONG64)m_Head.pNode;
		//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurTail = (LONG64)m_Tail.pNode;
		/////////////////////////////////////////////////////

		// CurTail.pNode->pNext �� nullptr �̾�߸� ���� ������ ��带 �߰��� �� ����
		if (CurTail.pNode->pNext == nullptr)
		{
			if (InterlockedCompareExchange64(
				(LONG64*)&m_Tail.pNode->pNext, (LONG64)NewTail.pNode, (LONG64)nullptr) == (LONG64)nullptr)
			{
				if (NewTail.pNode == NewTail.pNode->pNext)
					printf("A");
				InterlockedCompareExchange128(
					(LONG64*)&m_Tail, (LONG64)NewTail.ullBlockID, (LONG64)NewTail.pNode, (LONG64*)&CurTail);
				/////////////////////////////////////////////////////
				//if (NewTail.pNode->CallCount == 1000)
				//	NewTail.pNode->CallCount = 0;
				//else
				//	NewTail.pNode->CallCount = NewTail.pNode->CallCount + 1;
				//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].who = Caller;
				//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].Behavior = ENQ_COMPLETE;
				//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurHead = (LONG64)m_Head.pNode;
				//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurTail = (LONG64)m_Tail.pNode;
				/////////////////////////////////////////////////////
				break;
			}
		}
		else
		{
			//////////////////////////////////////////////////
			//if (NewTail.pNode->CallCount == 1000)
			//	NewTail.pNode->CallCount = 0;
			//else
			//	NewTail.pNode->CallCount = NewTail.pNode->CallCount + 1;
			//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].who = Caller;
			//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].Behavior = ENQ_MOVE_TAIL_TRY;
			//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurHead = (LONG64)m_Head.pNode;
			//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurTail = (LONG64)m_Tail.pNode;
			//////////////////////////////////////////////////
			
			// ������ �̴°��� ��������
			// ���� m_Tail �� �� ĭ �о���
			// �����ߴٸ�, �������� �̹� �о� �� ���̹Ƿ� ������� ����
			InterlockedCompareExchange128((LONG64*)&m_Tail, (LONG64)CurTail.ullBlockID, (LONG64)CurTail.pNode->pNext, (LONG64*)&CurTail);
			//if (InterlockedCompareExchange128((LONG64*)&m_Tail, (LONG64)CurTail.ullBlockID, (LONG64)CurTail.pNode->pNext, (LONG64*)&CurTail))
			//{
				//InterlockedIncrement(&m_uiRestSize);
				//////////////////////////////////////////////////
				//if (NewTail.pNode->CallCount == 1000)
				//	NewTail.pNode->CallCount = 0;
				//else
				//	NewTail.pNode->CallCount = NewTail.pNode->CallCount + 1;
				//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].who = Caller;
				//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].Behavior = ENQ_MOVE_TAIL_COMPLETE;
				//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurHead = (LONG64)m_Head.pNode;
				//NewTail.pNode->NodeChaser[NewTail.pNode->CallCount].CurTail = (LONG64)m_Tail.pNode;
				//////////////////////////////////////////////////
			//}
		}
	}


	InterlockedIncrement(&m_uiRestSize);
}

template <typename Data>
bool CLockFreeQueue<Data>::Dequeue(Data *pOutData)
{
	//if (m_uiRestSize == 0)
	//	return false;
	__declspec(align(16)) st_Q_NODE_BLOCK CurHead, CurTail, NewHead;

	while (1)
	{
		CurHead.ullBlockID = m_Head.ullBlockID;
		CurHead.pNode = m_Head.pNode;
		CurTail.ullBlockID = m_Tail.ullBlockID;
		CurTail.pNode = m_Tail.pNode;

		// Head �� Tail �� ���ٸ� DeQueue �� ���� �� �� �����Ƿ� ���� ó����
		if (CurHead.pNode == CurTail.pNode)
		{
			// Head �� Tail �� �������� Tail->Next �� nullptr�̶��
			// ���� ���� �ƹ��͵� ������� ������ ��Ÿ��
			if (CurTail.pNode->pNext == nullptr)
				continue;
				//dump.Crash();
				//return false;

			// Head �� Tail �� ������ Tail->Next �� nullptr�� �ƴ϶��
			// ��𿡼��� Tail�� �ű�⸦ �����Ͽ��ų�
			// Ȥ�� ����� ���Ḹ ������ ���ؽ�Ʈ ����Ī���� ���Ͽ�
			// Tail�� �̵����� ���Ͽ��� �����
			// ���� Tail �ű�⸦ �õ��ϰ� ������ �������
			// ���⿡�� �ű�� ���� �����Ͽ��ٸ� �̹� �ٸ� �����忡�� �ű���̹Ƿ�
			// �ű��� �ʰ� �ö󰡵� ��� ����
			InterlockedCompareExchange128(
				(LONG64*)&m_Tail, (LONG64)CurTail.ullBlockID + 1, (LONG64)CurTail.pNode->pNext, (LONG64*)&CurTail);
			continue;
		}
		NewHead.ullBlockID = CurHead.ullBlockID + 1;
		NewHead.pNode = CurHead.pNode->pNext;

		if (NewHead.pNode == nullptr)
			continue;
			//dump.Crash();
		//return false;
		/////////////////////////////////////////////////////
		//if (NewHead.pNode->CallCount == 1000)
		//	NewHead.pNode->CallCount = 0;
		//else
		//	NewHead.pNode->CallCount = NewHead.pNode->CallCount + 1;
		//NewHead.pNode->NodeChaser[NewHead.pNode->CallCount].who = Caller;
		//NewHead.pNode->NodeChaser[NewHead.pNode->CallCount].Behavior = DEQ_TRY;
		//NewHead.pNode->NodeChaser[NewHead.pNode->CallCount].CurHead = (LONG64)m_Head.pNode;
		//NewHead.pNode->NodeChaser[NewHead.pNode->CallCount].CurTail = (LONG64)m_Tail.pNode;
		/////////////////////////////////////////////////////

		if (InterlockedCompareExchange128(
			(LONG64*)&m_Head, (LONG64)NewHead.ullBlockID, (LONG64)NewHead.pNode, (LONG64*)&CurHead))
		{
			*pOutData = NewHead.pNode->data;//CurHead.pNode->pNext->data;//
			/////////////////////////////////////////////////////
			//if (NewHead.pNode->CallCount == 1000)
			//	NewHead.pNode->CallCount = 0;
			//else
			//	NewHead.pNode->CallCount = NewHead.pNode->CallCount + 1;
			//NewHead.pNode->NodeChaser[NewHead.pNode->CallCount].who = Caller;
			//NewHead.pNode->NodeChaser[NewHead.pNode->CallCount].Behavior = DEQ_COMPLETE;
			//NewHead.pNode->NodeChaser[NewHead.pNode->CallCount].CurHead = (LONG64)m_Head.pNode;
			//NewHead.pNode->NodeChaser[NewHead.pNode->CallCount].CurTail = (LONG64)m_Tail.pNode;
			/////////////////////////////////////////////////////
			m_pLockFreeMemeoryPool->Push(CurHead.pNode);
			break;
		}
	}

	InterlockedDecrement(&m_uiRestSize);
	return true;
}

template <typename Data>
int CLockFreeQueue<Data>::GetRestSize()
{
	return m_uiRestSize;
}

template <typename Data>
int CLockFreeQueue<Data>::GetMemoryPoolUseSize()
{
	return m_pLockFreeMemeoryPool->GetUseCount();
}

template <typename Data>
int CLockFreeQueue<Data>::GetMemoryPoolAllocSize()
{
	return m_pLockFreeMemeoryPool->GetAllocCount();
}