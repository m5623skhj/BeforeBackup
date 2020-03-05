#pragma once
#include "LockFreeMemoryPool.h"

#define dfDEFAULTSIZE		1024
#define BUFFFER_MAX			10000

// Header �� ũ�⸦ ������
// ���α׷����� ��������
// ��� ũ�⿡ ���� ������ ��
// ���� ���ο��� �ݵ�� 
// WritePtrSetHeader() �� WritePtrSetLast() �� ��������ν�
// ��� ���� �κа� ���̷ε� ������ �κ����� �����͸� �Ű���� ��
// Ȥ�� GetLastWrite() �� ����Ͽ� ����ڰ� �� ������ ���� ������� �Ѱ���
#define df_HEADER_SIZE		5

#define NUM_OF_CHUNK		10

//#define df_HEADER_CODE		(BYTE)0x11
// ����� �� ��ȣȭ�� XOR ���� �ڵ�
// ���� �ڵ� 2���� ���Ͽ� XOR �� �ڵ���
//#define df_XOR_CODE			(BYTE)0x41

#define df_RAND_CODE_LOCATION		 3
#define df_CHECKSUM_CODE_LOCATION	 4

class CNetServer;
class CLanClient;

class CNetServerSerializationBuf
{
private:
	BYTE		m_byError;
	BOOL		m_bIsEncoded;
	int			m_iWrite;
	int			m_iRead;
	int			m_iSize;
	// WritePtrSetHeader �Լ��� ���� �������� �� ������ �����
	int			m_iWriteLast;
	int			m_iUserWriteBefore;
	UINT		m_iRefCount;
	char		*m_pSerializeBuffer;
	//static CLockFreeMemoryPool<CSerializationBuf>	*pMemoryPool;

	static BYTE												m_byHeaderCode;
	static BYTE												m_byXORCode;

	static CTLSMemoryPool<CNetServerSerializationBuf>		*pMemoryPool;

	friend CNetServer;
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
	
	// ���ϴ� ���̸�ŭ �б� ��ġ���� ����
	void RemoveData(int Size);

	void Encode();
	bool Decode();

	// ���� ���� �� ���� ��� �� ��
	static void ChunkFreeForcibly();

	int GetAllUseSize();
public:
	CNetServerSerializationBuf();
	~CNetServerSerializationBuf();

	void Init();

	void WriteBuffer(char *pData, int Size);
	void ReadBuffer(char *pDest, int Size);
	void PeekBuffer(char *pDest, int Size);

	char *GetReadBufferPtr();
	char *GetWriteBufferPtr();

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


	// --------------- ��ȯ�� ---------------
	// 0 : ����ó�� �Ǿ���
	// 1 : ���۸� �������� �Ͽ�����, �д� ������ ���� ũ�⺸�� ũ�ų� ���� �������� ���� ������
	// 2 : ���۸� ������ �Ͽ�����, ���� ������ ���� ũ�⺸�� ŭ
	// --------------- ��ȯ�� ---------------
	BYTE GetBufferError();

	int GetUseSize();
	int GetFreeSize();

	static CNetServerSerializationBuf*	Alloc();
	static void							AddRefCount(CNetServerSerializationBuf* AddRefBuf);
	static void							Free(CNetServerSerializationBuf* DeleteBuf);


	CNetServerSerializationBuf& operator<<(int Input);
	CNetServerSerializationBuf& operator<<(WORD Input);
	CNetServerSerializationBuf& operator<<(DWORD Input);
	CNetServerSerializationBuf& operator<<(char Input);
	CNetServerSerializationBuf& operator<<(BYTE Input);
	CNetServerSerializationBuf& operator<<(float Input);
	CNetServerSerializationBuf& operator<<(UINT Input);
	CNetServerSerializationBuf& operator<<(UINT64 Input);
	CNetServerSerializationBuf& operator<<(__int64 Input);

	CNetServerSerializationBuf& operator>>(int &Input);
	CNetServerSerializationBuf& operator>>(WORD &Input);
	CNetServerSerializationBuf& operator>>(DWORD &Input);
	CNetServerSerializationBuf& operator>>(char &Input);
	CNetServerSerializationBuf& operator>>(BYTE &Input);
	CNetServerSerializationBuf& operator>>(float &Input);
	CNetServerSerializationBuf& operator>>(UINT &Input);
	CNetServerSerializationBuf& operator>>(UINT64 &Input);
	CNetServerSerializationBuf& operator>>(__int64 &Input);
};

