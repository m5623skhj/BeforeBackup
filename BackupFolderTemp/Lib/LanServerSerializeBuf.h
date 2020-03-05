#pragma once
//#include "LanServer.h"
#include "LockFreeMemoryPool.h"
#include <Windows.h>

#define dfDEFAULTSIZE		1024
#define BUFFFER_MAX			10000

// Header �� ũ�⸦ ������
// ���ο��� �ݵ�� 
// WritePtrSetHeader() �� WritePtrSetLast() �� ��������ν�
// ��� ���� �κа� ���̷ε� ������ �κ����� �����͸� �Ű���� ��
// Ȥ�� GetLastWrite() �� ����Ͽ� ����ڰ� �� ������ ���� ������� �Ѱ���
#define HEADER_SIZE			2

#define NUM_OF_CHUNK		5

class CLanServer;
class CLanClient;

class CSerializationBuf
{
private:
	BYTE		m_byDelCnt = 0;

	BYTE		m_byError;
	BOOL		m_bHeaderInputted;
	int			m_iWrite;
	int			m_iRead;
	int			m_iSize;
	// WritePtrSetHeader �Լ��� ���� �������� �� ������ �����
	int			m_iWriteLast;
	int			m_iUserWriteBefore;
	UINT		m_iRefCount;
	char		*m_pSerializeBuffer;
	//static CLockFreeMemoryPool<CSerializationBuf>	*pMemoryPool;
	static CTLSMemoryPool<CSerializationBuf>		*pMemoryPool;

	friend CLanServer;
	friend CLanClient;
private:
	void Initialize(int BufferSize);

	void SetWriteZero();
	// ���� �������� ������ ������ ������ ����ϰ�
	// ���� �����͸� ����ȭ ���� ���� ó������ �ű�
	void WritePtrSetHeader();
	// ���� �����͸� ������ ����ߴ� ������ ������ �κ����� �̵���Ŵ
	void WritePtrSetLast();

	// ������ ó������ ����ڰ�
	// ���������� �� ���������� ���� ����
	int GetLastWrite();

	char *GetBufferPtr();
	char *GetReadBufferPtr();
	char *GetWriteBufferPtr();

	void PeekBuffer(char *pDest, int Size);

	// ���ϴ� ���̸�ŭ �б� ��ġ���� ����
	void RemoveData(int Size); 

	// ���� ���� �� ���� ��� �� ��
	static void ChunkFreeForcibly();

	int GetAllUseSize();
public:
	CSerializationBuf();
	~CSerializationBuf();

	void Init();

	// ���� ����ũ�⸦ �Ҵ��Ͽ� ���� �����͸� �ű�
	void Resize(int Size);

	// ���ϴ� ���̸�ŭ �б� ���� ��ġ �̵�
	void MoveWritePos(int Size);
	// ThisPos�� ���� �����͸� �̵���Ŵ
	void MoveWritePosThisPos(int ThisPos);
	// MoveWritePosThisPos �� �̵��ϱ� ���� ���� �����ͷ� �̵���Ŵ
	// �� �Լ��� ������� �ʰ� �� �Լ��� ȣ���� ���
	// ���� �����͸� ���� ó������ �̵���Ŵ
	void MoveWritePosBeforeCallThisPos();

	void WriteBuffer(char *pData, int Size);
	void ReadBuffer(char *pDest, int Size);

	// --------------- ��ȯ�� ---------------
	// 0 : ����ó�� �Ǿ���
	// 1 : ���۸� �������� �Ͽ�����, �д� ������ ���� ũ�⺸�� ũ�ų� ���� �������� ���� ������
	// 2 : ���۸� ������ �Ͽ�����, ���� ������ ���� ũ�⺸�� ŭ
	// --------------- ��ȯ�� ---------------
	BYTE GetBufferError();

	int GetUseSize();
	int GetFreeSize();

	static CSerializationBuf* Alloc();
	static void				  AddRefCount(CSerializationBuf* AddRefBuf);
	static void				  Free(CSerializationBuf* DeleteBuf);


	CSerializationBuf& operator<<(int Input);
	CSerializationBuf& operator<<(WORD Input);
	CSerializationBuf& operator<<(DWORD Input);
	CSerializationBuf& operator<<(char Input);
	CSerializationBuf& operator<<(BYTE Input);
	CSerializationBuf& operator<<(float Input);
	CSerializationBuf& operator<<(UINT Input);
	CSerializationBuf& operator<<(UINT64 Input);
	CSerializationBuf& operator<<(__int64 Input);

	CSerializationBuf& operator>>(int &Input);
	CSerializationBuf& operator>>(WORD &Input);
	CSerializationBuf& operator>>(DWORD &Input);
	CSerializationBuf& operator>>(char &Input);
	CSerializationBuf& operator>>(BYTE &Input);
	CSerializationBuf& operator>>(float &Input);
	CSerializationBuf& operator>>(UINT &Input);
	CSerializationBuf& operator>>(UINT64 &Input);
	CSerializationBuf& operator>>(__int64 &Input);
};

