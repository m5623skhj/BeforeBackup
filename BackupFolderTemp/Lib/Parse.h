#pragma once
#include <Windows.h>
#include <locale.h>
// 	_wsetlocale(LC_ALL, L"Korean");

#define BUFFER_MAX 1024 * 10 

class Parser
{
private :
	WCHAR **m_ppBufferPointer;
	WCHAR *m_pKeyScope;

	// ���� ���� + ���� ��ū������ �˻���
	//bool GetNextWord(char **pBufferPointer, const char *pKeyName);
	bool GetNextWord(const WCHAR *pKeyName, bool *bIsStop);

	// ��ū �� �ϳ��� ������ �Ǹ� ���߰�, 
	// ��ū�� �ƴϸ� ��� �����͸� �ű�
	void SkipToToken();

	// �ּ� ó���� �κ��� ���� �� ���� �����͸� �ű�
	void JumpComment();

	// ���ڷ� ���޵� �������� ã�� �� ���� �˻���
	bool JumpScope(bool *bIsStop);

	// �ش� ���ڸ� ã�� �ݺ���
	bool FindWord(const WCHAR *pKeyName);

	// ���ڿ��� �����ؼ� ������ true Ʋ���� false�� ��ȯ�� 
	bool GetStringWord(const WCHAR *pKeyName);

	// �ش� ���ڰ� ��ū������ Ȯ���Ͽ� ��ū�̸� true, ��ū�� �ƴϸ� false�� ��ȯ��
	bool IsToken();

	// ���ڿ��� ã������ �ش� ���ڿ� �տ� �߰����� ��ū�� ������
	void TokenError(int iCnt, const WCHAR *pKeyName);

	// �������� �ش� ���ڿ��� �������� ����
	void KeyError(const WCHAR *pKeyName);

	// �ش� �������� �������� ����
	void ScopeError();
public :
	Parser();
	~Parser();

	bool GetValue_Int(WCHAR *pBufferPointer, const WCHAR *KeyName, const WCHAR *KeyScope, int *OutValue);
	bool GetValue_Double(WCHAR *pBufferPointer, const WCHAR *KeyName, const WCHAR *KeyScope, double *OutValue);
	bool GetValue_Char(WCHAR *pBufferPointer, const WCHAR *KeyName, const WCHAR *KeyScope, WCHAR *OutValue);
	bool GetValue_String(WCHAR *pBufferPointer, const WCHAR *KeyName, const WCHAR *KeyScope, WCHAR *OutValue);
};