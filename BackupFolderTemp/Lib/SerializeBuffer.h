#pragma once
#include <Windows.h>

#define dfDEFAULTSIZE		2000
#define BUFFFER_MAX			10000

// Header �� ũ�⸦ ������
// ���α׷����� ��������
// ��� ũ�⿡ ���� ������ ��
// ���� ���ο��� �ݵ�� 
// WritePtrSetHeader() �� WritePtrSetLast() �� ��������ν�
// ��� ���� �κа� ���̷ε� ������ �κ����� �����͸� �Ű���� ��
// Ȥ�� GetLastWrite() �� ����Ͽ� ����ڰ� �� ������ ���� ������� �Ѱ���
#define HEADER_SIZE			2

class CSerializationBuffer
{
private:
	BYTE m_byError;
	int m_iWrite;
	int m_iRead;
	int m_iSize;
	// WritePtrSetHeader �Լ��� ���� �������� �� ������ �����
	int m_iWriteLast;
	char *m_pSerializeBuffer;
private:
	void Initialize(int BufferSize);
public:
	CSerializationBuffer();
	~CSerializationBuffer();

	// ���� ����ũ�⸦ �Ҵ��Ͽ� ���� �����͸� �ű�
	void Resize(int Size);

	void WriteBuffer(char *pData, int Size);
	void ReadBuffer(char *pDest, int Size);
	void PeekBuffer(char *pDest, int Size);

	// ���ϴ� ���̸�ŭ �б� ��ġ���� ����
	void RemoveData(int Size);
	// ���ϴ� ���̸�ŭ �б� ���� ��ġ �̵�
	void MoveWritePos(int Size);
	// ���ϴ� ������ �б�� ���� �����͸� �̵�
	void MoveWriteAndReadPos(int Pos);
	// ThisPos�� ���� �����͸� �̵���Ŵ
	void MoveWritePosThisPos(int ThisPos);

	// ���̷ε� ���� ����� ���� ��� �����ŭ ���� �����͸� �̵���Ŵ
	void WritePtrSetPayload(int HeaderSize);
	// ���� �������� ������ ������ ������ ����ϰ�
	// ���� �����͸� ����ȭ ���� ���� ó������ �ű�
	void WritePtrSetHeader();
	// ���� �����͸� ������ ����ߴ� ������ ������ �κ����� �̵���Ŵ
	void WritePtrSetLast();


	// --------------- ��ȯ�� ---------------
	// 0 : ����ó�� �Ǿ���
	// 1 : ���۸� �������� �Ͽ�����, �д� ������ ���� ũ�⺸�� ũ�ų� ���� �������� ���� ������
	// 2 : ���۸� ������ �Ͽ�����, ���� ������ ���� ũ�⺸�� ŭ
	// --------------- ��ȯ�� ---------------
	BYTE GetBufferError();

	char *GetBufferPtr();
	char *GetReadBufferPtr();
	char *GetWriteBufferPtr();
	int GetUseSize();
	// ������ ó������ ����ڰ� ���������� �� ���������� 
	// ���� ����
	int GetLastWrite();
	int GetFreeSize();

	CSerializationBuffer& operator<<(int Input);
	CSerializationBuffer& operator<<(WORD Input);
	CSerializationBuffer& operator<<(DWORD Input);
	CSerializationBuffer& operator<<(char Input);
	CSerializationBuffer& operator<<(BYTE Input);
	CSerializationBuffer& operator<<(float Input);
	CSerializationBuffer& operator<<(UINT Input);
	CSerializationBuffer& operator<<(UINT64 Input);
	CSerializationBuffer& operator<<(__int64 Input);

	CSerializationBuffer& operator>>(int &Input);
	CSerializationBuffer& operator>>(WORD &Input);
	CSerializationBuffer& operator>>(DWORD &Input);
	CSerializationBuffer& operator>>(char &Input);
	CSerializationBuffer& operator>>(BYTE &Input);
	CSerializationBuffer& operator>>(float &Input);
	CSerializationBuffer& operator>>(UINT &Input);
	CSerializationBuffer& operator>>(UINT64 &Input);
	CSerializationBuffer& operator>>(__int64 &Input);
};