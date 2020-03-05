#pragma once
#include <new.h>
#include <stdlib.h>

#define NODE_CODE	((int)0x3f08cc9d)

////////////////////////////////////////////////////////////////
// CLockFreeMemoryPool Class
////////////////////////////////////////////////////////////////

template <typename Data>
class CLockFreeMemoryPool
{
private:
	struct st_BLOCK_NODE
	{
		st_BLOCK_NODE()
		{
			stpNextBlock = NULL;
			iCode = NODE_CODE;
		}

		int									iCode;
		Data								NodeData;
		st_BLOCK_NODE						*stpNextBlock;
	};

	struct st_BLOCK_INFO
	{
		st_BLOCK_NODE						*pBlockPtr;
		ULONGLONG							ullBlockID;
	};

	bool									m_bIsPlacementNew;
	int										m_iUseCount;
	int										m_iAllocCount;
	__declspec(align(16)) st_BLOCK_INFO		m_Top;

public:
	CLockFreeMemoryPool(UINT MakeInInit, bool bIsPlacementNew);
	~CLockFreeMemoryPool();

	Data *Pop();
	void Push(Data *pData);
	int GetUseCount() { return m_iUseCount; }
	int GetAllocCount() { return m_iAllocCount; }
};


template <typename Data>
CLockFreeMemoryPool<Data>::CLockFreeMemoryPool(UINT MakeInInit, bool bIsPlacementNew)
	: m_bIsPlacementNew(bIsPlacementNew), m_iUseCount(0), m_iAllocCount(MakeInInit)
{
	m_Top.ullBlockID = 0;
	m_Top.pBlockPtr = nullptr;
	for (UINT i = 0; i < MakeInInit; i++)
	{
		++m_Top.ullBlockID;
		st_BLOCK_NODE *NewNode = new st_BLOCK_NODE();

		NewNode->stpNextBlock = m_Top.pBlockPtr;
		m_Top.pBlockPtr = NewNode;
	}
}

template <typename Data>
CLockFreeMemoryPool<Data>::~CLockFreeMemoryPool()
{
	st_BLOCK_NODE *next;
	if (!m_bIsPlacementNew)
	{
		while (m_Top.pBlockPtr)
		{
			next = m_Top.pBlockPtr->stpNextBlock;
			delete m_Top.pBlockPtr;
			m_Top.pBlockPtr = next;
		}
	}
	else
	{
		while (m_Top.pBlockPtr)
		{
			next = m_Top.pBlockPtr->stpNextBlock;
			free(m_Top.pBlockPtr);
			m_Top.pBlockPtr = next;
		}
	}
}

template <typename Data>
Data *CLockFreeMemoryPool<Data>::Pop()
{
	__declspec(align(16)) st_BLOCK_INFO CurTop, NewTop;
	// ������ �ڵ�
	// ���� ���� ������ ��
	//InterlockedIncrement((UINT*)&m_iUseCount);

	if (!m_bIsPlacementNew)
	{
		if (m_Top.pBlockPtr != nullptr)
		{
			do {
				CurTop.ullBlockID = m_Top.ullBlockID;
				CurTop.pBlockPtr = m_Top.pBlockPtr;

				//CurTop = m_Top;
				//CurTop.pBlockPtr = (st_BLOCK_NODE*)1;
				//CurTop.ullBlockID = -1;
				//InterlockedCompareExchange128((LONG64*)&m_Top, 1, -1, (LONG64*)&CurTop);
				if (CurTop.pBlockPtr == nullptr)
				{
					// ������ �ڵ�
					// ���� ���� ������ ��
					//InterlockedIncrement((UINT*)&m_iAllocCount);
					CurTop.pBlockPtr = new st_BLOCK_NODE();
					break;
				}

				NewTop.ullBlockID = CurTop.ullBlockID + 1;
				NewTop.pBlockPtr = CurTop.pBlockPtr->stpNextBlock;
			} while (!InterlockedCompareExchange128(
				(LONG64*)&m_Top, (LONG64)NewTop.ullBlockID, (LONG64)NewTop.pBlockPtr, (LONG64*)&CurTop));

			CurTop.pBlockPtr->stpNextBlock = nullptr;
		}
		else
		{
			// ������ �ڵ�
			// ���� ���� ������ ��
			//InterlockedIncrement((UINT*)&m_iAllocCount);
			CurTop.pBlockPtr = new st_BLOCK_NODE();
		}
	}
	else
	{
		if (m_Top.pBlockPtr != nullptr)
		{
			do {
				CurTop.ullBlockID = m_Top.ullBlockID;
				CurTop.pBlockPtr = m_Top.pBlockPtr;

				//CurTop = m_Top;
				//CurTop.pBlockPtr = (st_BLOCK_NODE*)1;
				//CurTop.ullBlockID = -1;
				//InterlockedCompareExchange128((LONG64*)&m_Top, 1, -1, (LONG64*)&CurTop);
				if (CurTop.pBlockPtr == nullptr)
				{
					// ������ �ڵ�
					// ���� ���� ������ ��
					//InterlockedIncrement((UINT*)&m_iAllocCount);
					CurTop.pBlockPtr = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
					break;
				}

				NewTop.ullBlockID = CurTop.ullBlockID + 1;
				NewTop.pBlockPtr = CurTop.pBlockPtr->stpNextBlock;
			} while (!InterlockedCompareExchange128(
				(LONG64*)&m_Top, (LONG64)NewTop.ullBlockID, (LONG64)NewTop.pBlockPtr, (LONG64*)&CurTop));
		}
		else
		{
			// ������ �ڵ�
			// ���� ���� ������ ��
			//InterlockedIncrement((UINT*)&m_iAllocCount);
			CurTop.pBlockPtr = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
		}
		new (CurTop.pBlockPtr) st_BLOCK_NODE();
	}

	return &(CurTop.pBlockPtr->NodeData);
}

template <typename Data>
void CLockFreeMemoryPool<Data>::Push(Data *pData)
{
	st_BLOCK_NODE *PushPtr = (st_BLOCK_NODE*)((char*)pData - 8);
	if (PushPtr->iCode != NODE_CODE)
		return;

	st_BLOCK_NODE *pCurTop = nullptr;
	//st_BLOCK_INFO CurTop, NewTop;
	//NewTop.pBlockPtr = PushPtr;
	//st_BLOCK_NODE *pCurTop;
	do {
		pCurTop = m_Top.pBlockPtr;
		PushPtr->stpNextBlock = pCurTop;
		//CurTop.pBlockPtr = m_Top.pBlockPtr;
		//NewTop.pBlockPtr->stpNextBlock = CurTop.pBlockPtr;

		//pCurTop = m_Top.pBlockPtr;
		//PushPtr->stpNextBlock = pCurTop;
		//PushPtr->stpNextBlock = m_Top.pBlockPtr;
	} 
	/*while (!InterlockedCompareExchange128(
		(LONG64*)&m_Top, (LONG64)NewTop.ullBlockID, (LONG64)NewTop.pBlockPtr, (LONG64*)&CurTop));*/
	while (InterlockedCompareExchange64(
		//(volatile LONG64*)&m_Top.pBlockPtr, (LONG64)PushPtr, (LONG64)pCurTop) != (LONG64)pCurTop);
		(LONG64*)&m_Top.pBlockPtr, (LONG64)PushPtr, (LONG64)pCurTop) != (LONG64)pCurTop);

	// ������ �ڵ�
	// ���� ���� ������ ��
	//InterlockedDecrement((UINT*)&m_iUseCount);

	if (m_bIsPlacementNew)
		PushPtr->NodeData.~Data();
}

//#include "CrashDump.h"
//CCrashDump dump;

///////////////////////////////
// CTLSMemoryPool Class
///////////////////////////////

#define df_CHUNK_ELEMENT_SIZE						1000

template <typename Data>
class CTLSMemoryPool
{
private:
	///////////////////////////////
	// CChunk Class
	///////////////////////////////
	template <typename T>
	class CChunk
	{
	private:
		struct st_CHUNK_NODE
		{
			T										Data;
			CChunk<T>								*pAssginedChunk;
		};
		unsigned int								m_uiChunkIndex;
		unsigned int								m_uiNodeFreeCount;

		// �����͸� �� ����
		// �ٸ� ûũ�� �˰� �־�� �ϱ� ������ ������ ������ 
		// �ڽ��� �Ҵ����� ûũ�� ����ϰ� �־�� ��
		// ûũ ����ü�� ûũ�� �����͸� �־���
		st_CHUNK_NODE								m_ChunkData[df_CHUNK_ELEMENT_SIZE];
		friend class CTLSMemoryPool;

	public:
		CChunk();
		~CChunk();

		// �Ʒ��� CTLSMemroyPool �� friend �� ó���Ͽ� ���� ���� ��Ű�°����� ������
		// ������ ��尡 �Ҵ� �޾Ҵٸ� OutNode �� false �� �־�
		// Chunk �� ����ٴ� ���� �˸�
		//T* NodeAlloc(bool *pOutNodeCanAllocMore);
		//bool NodeFree();
	};
	bool									m_bIsPlacementNew;
	
	int										m_iUseChunkCount;
	int										m_iAllocChunkCount;

	unsigned int							m_uiUseNodeCount;

	int										m_iTLSIndex;
	CLockFreeMemoryPool<CChunk<Data>>		*m_pChunkMemoryPool;

public:
	CTLSMemoryPool(UINT MakeChunkInInit, bool bIsPlacementNew);
	~CTLSMemoryPool();

	Data *Alloc();
	void Free(Data *pData);
	// ���� TLS �� �ö���ִ� ûũ�� �����͸� 
	// ������ m_pChunkMemroyPool �� �ȾƳ���
	// ���� ���� �� ���� ��� �� ��
	void ChunkFreeForcibly();

	int GetUseChunkCount()   { return m_pChunkMemoryPool->GetUseCount(); }
	int GetAllocChunkCount() { return m_pChunkMemoryPool->GetAllocCount(); }

	int GetUseNodeCount() { return m_uiUseNodeCount; }
};

///////////////////////////////
// CTLSMemoryPool Method
///////////////////////////////
template <typename Data>
CTLSMemoryPool<Data>::CTLSMemoryPool(UINT MakeChunkInInit, bool bIsPlacementNew) :
	m_bIsPlacementNew(bIsPlacementNew), m_iUseChunkCount(0), m_uiUseNodeCount(0), m_iAllocChunkCount(MakeChunkInInit)
{
	m_iTLSIndex = TlsAlloc();
	if (m_iTLSIndex == TLS_OUT_OF_INDEXES)
	{
		printf("TLS_OUT_OF_INDEXES ERROR\n\n");
		throw;
		//dump.Crash();
	}
	m_pChunkMemoryPool = new CLockFreeMemoryPool<CChunk<Data> >(MakeChunkInInit, bIsPlacementNew);
}

template <typename Data>
CTLSMemoryPool<Data>::~CTLSMemoryPool()
{
	TlsFree(m_iTLSIndex);
	delete m_pChunkMemoryPool;
}

template <typename Data>
Data* CTLSMemoryPool<Data>::Alloc()
{
	CChunk<Data> *TLSChunkPtr = (CChunk<Data>*)TlsGetValue(m_iTLSIndex);
	
	//if (GetLastError() != NO_ERROR)
	//	return NULL;
		//dump.Crash();

	// TLS �� Chunk �� ��� �Ǿ����� �������
	if (TLSChunkPtr == NULL)
	{
		// Chunk MemoryPool ���� �� ûũ�� �Ҵ� �ް�
		// �ش� Chunk�� �ʱ�ȭ ��
		TLSChunkPtr = m_pChunkMemoryPool->Pop();
		TLSChunkPtr->m_uiChunkIndex = 0;
		TLSChunkPtr->m_uiNodeFreeCount = 0;

		// �ش� Chunk �� TLS �� ��Ͻ�Ŵ
		TlsSetValue(m_iTLSIndex, TLSChunkPtr);
	}
	UINT &chunkIndex = TLSChunkPtr->m_uiChunkIndex;

	Data *pData = &TLSChunkPtr->m_ChunkData[chunkIndex].Data;
	++chunkIndex;
	if (chunkIndex >= df_CHUNK_ELEMENT_SIZE)
		TlsSetValue(m_iTLSIndex, NULL);

	// ������ �ڵ�
	// ���� ���� ������ ��
	//InterlockedIncrement(&m_uiUseNodeCount);
	
	return pData;
}

template <typename Data>
void CTLSMemoryPool<Data>::Free(Data *pData)
{
	// ���ø� ũ�⸦ �� �� �����Ƿ�
	// ������ ĳ���� �� �� ũ�Ⱑ Ȯ�������� ���ϹǷ�
	// �⺻ �ڷ������� ĳ���� �� �ٽ� ĳ�����Ͽ� �����ͷ� ��ȯ��Ŵ
	//CChunk<Data> *pChunk = ((CChunk<Data>*)*((LONG64*)(pData + 1)));
	CChunk<Data> *pChunk = ((typename CChunk<Data>::st_CHUNK_NODE*)pData)->pAssginedChunk;
	//UINT retval = InterlockedIncrement(&pChunk->m_uiNodeFreeCount);
	if (InterlockedIncrement(&pChunk->m_uiNodeFreeCount) >= df_CHUNK_ELEMENT_SIZE)
	{
		m_pChunkMemoryPool->Push(pChunk);
	}

	// ������ �ڵ�
	// ���� ���� ������ ��
	//InterlockedDecrement(&m_uiUseNodeCount);
}

template <typename Data>
void CTLSMemoryPool<Data>::ChunkFreeForcibly()
{
	CChunk<Data> *TLSChunkPtr = (CChunk<Data>*)TlsGetValue(m_iTLSIndex);
	if (TLSChunkPtr == NULL)
		return;

	m_pChunkMemoryPool->Push(TLSChunkPtr);
}

///////////////////////////////
// CChunk Method
///////////////////////////////
template <typename Data>
template <typename T>
CTLSMemoryPool<Data>::CChunk<T>::CChunk()
{
	for (int i = 0; i < df_CHUNK_ELEMENT_SIZE; ++i)
		m_ChunkData[i].pAssginedChunk = this;
}

template <typename Data>
template <typename T>
CTLSMemoryPool<Data>::CChunk<T>::~CChunk()
{

}

//template <typename Data>
//template <typename T>
//T* CTLSMemoryPool<Data>::CChunk<T>::NodeAlloc(bool *pOutNodeCanAllocMore)
//{
//	++m_uiChunkIndex;
//	if (m_uiChunkIndex >= df_CHUNK_ELEMENT_SIZE)
//		*pOutNodeCanAllocMore = false;
//	else
//		*pOutNodeCanAllocMore = true;
//
//	return &m_ChunkData[m_uiChunkIndex - 1].Data;
//}
//template <typename Data>
//template <typename T>
//bool CTLSMemoryPool<Data>::CChunk<T>::NodeFree()
//{
//	++m_uiNodeFreeCount;
//	if (m_uiNodeFreeCount >= df_CHUNK_ELEMENT_SIZE)
//		return false;
//	return true;
//}
