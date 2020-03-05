#include "PreComfile.h"
#include "Parse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


Parser::Parser()
{

}

Parser::~Parser()
{
	
}

bool Parser::GetNextWord(const WCHAR *pKeyName, bool *bIsStop)
{
	bool bIsCorrectWord = false;
	if (**m_ppBufferPointer == '\0' ||
		**m_ppBufferPointer == ':')
	{
		KeyError(pKeyName);
		*bIsStop = true;
		return false;
	}
	bIsCorrectWord = GetStringWord(pKeyName);
	SkipToToken();

	return bIsCorrectWord;
}

void Parser::SkipToToken()
{
	// ã�� ���ڿ� �ٸ� ������ ��� ���� ��ū�� ���� �� ���� ���� ����
	// ã�� ���ڰ� ���Դٸ� �ٷ� ������ ����Ű�� ���ڰ� ��ū�̹Ƿ� �ٷ� ó����
	while (1) 
	{
		if (IsToken())
		{
			*m_ppBufferPointer = *m_ppBufferPointer + 1;
			break;
		}
		else if (**m_ppBufferPointer == '/')
		{
			JumpComment(); // ���� '/'�� ��� ���۸� �ϳ� �ڷ� �о���
			break;
		}
		*m_ppBufferPointer = *m_ppBufferPointer + 1;
	}
}

void Parser::JumpComment()
{
	*m_ppBufferPointer = *m_ppBufferPointer + 1;
	switch (**m_ppBufferPointer)
	{
	case '/':
		while (**m_ppBufferPointer != 0x0a) //  ���� �ǵ尡 ���� �� ���� ���۸� �ű�
		{
			*m_ppBufferPointer = *m_ppBufferPointer + 1;
		}
		break;
	case '*':
		while (**m_ppBufferPointer != '*' || //  ������ �� ������ �ȳ����� �� �� ���Ŷ�� �Ǵ�
			   *(*m_ppBufferPointer + 1) != '/')   //  */ �� ���� �� ���� ���۸� �ű�
		{
			*m_ppBufferPointer = *m_ppBufferPointer + 1;
		}
		*m_ppBufferPointer = *m_ppBufferPointer + 1; //  �� �κп��� / �� �����Ͱ� ����Ű�� ����
		break;
	default:
		break;
	}

	*m_ppBufferPointer = *m_ppBufferPointer + 1; //  ���� ����Ű�� �ִ� �����Ͱ� ��� �����̹Ƿ�, ������ ����Ű�� ��
}

bool Parser::JumpScope(bool *bIsStop)
{
	while (1)
	{
		if (**m_ppBufferPointer == ':')
		{
			*m_ppBufferPointer = *m_ppBufferPointer + 1;
			if (GetStringWord(m_pKeyScope))
			{
				return true;
			}
		}
		if (**m_ppBufferPointer == '\0')
		{
			ScopeError();
			*bIsStop = true;
			return false;
		}
		*m_ppBufferPointer = *m_ppBufferPointer + 1;
	}
}

bool Parser::GetStringWord(const WCHAR *pKeyName)
{
	while (1)
	{
		//////////////////////////////////
		//���� ���ڿ��� ª�� �� ���� ���ڿ��� �Ϻο� ������ �����Ѵٸ�?
		//ex) asdf, asdfzxcv
		//���� ���ڿ��� �ƴԿ��� ��������� �˻� �� �Ŀ�
		//��ŵĿ�ǵ�� ���� ��
		//���� �߻�
		// -> �ι��� ���� �� pBufferPointer�� ���� ���ڰ� ��ū���� Ȯ���ϴ� ������� ó����
		if (**m_ppBufferPointer == *pKeyName) //  �ΰ��� �����͸� �ű�鼭 �� ���� �����Ͱ� ����Ű�� ���� ������
		{
			*m_ppBufferPointer = *m_ppBufferPointer + 1;
			pKeyName++;
			if (*pKeyName == '\0')
			{
				if(IsToken())
					break;
				else
				{
					return false;
				}
			}
		}
		//���� �߻�
		// -> �ι��� ���� �� pBufferPointer�� ���� ���ڰ� ��ū���� Ȯ���ϴ� ������� ó����
		//////////////////////////////////
		else
		{
			return false;
		}
	}

	return true;
}

bool Parser::IsToken()
{
	if (**m_ppBufferPointer == ','  || **m_ppBufferPointer == '\"' ||
		**m_ppBufferPointer == 0x20 || **m_ppBufferPointer == 0x08 ||
		**m_ppBufferPointer == 0x09 || **m_ppBufferPointer == 0x0a ||
		**m_ppBufferPointer == 0x0d)
		return true;
	else
		return false;
}

bool Parser::FindWord(const WCHAR *pKeyName)
{
	bool bIsStop = false;
	while (!JumpScope(&bIsStop))// �ش� �������� ���� �� ���� �˻�
	{
		if (bIsStop)
			return false;
	}
	while (!GetNextWord(pKeyName, &bIsStop))// Ű ���Ӱ� ���� �̸� �˻�
	{
		if (bIsStop)
			return false;
	}
	while (!GetNextWord(L"=", &bIsStop)); // '=' �˻�

	return true;
}

void Parser::TokenError(int iCnt, const WCHAR *pKeyName)
{
	wprintf_s(L"�ش� �� �տ� �߰����� ��ū�� �����մϴ� : %ls\n", pKeyName);
}

void Parser::KeyError(const WCHAR *pKeyName)
{
	wprintf_s(L"�������� �����Ͽ�����, �ش� Ű���� �������� �ʽ��ϴ� : %ls\n", pKeyName);
}

void Parser::ScopeError()
{
	wprintf_s(L"�ش� �������� �������� �ʽ��ϴ� : %ls\n", m_pKeyScope);
}

bool Parser::GetValue_Int(WCHAR *pBufferPointer, const WCHAR *pKeyName, const WCHAR *pKeyScope, int *OutValue)
{
	m_ppBufferPointer = &pBufferPointer;
	m_pKeyScope = const_cast<WCHAR*>(pKeyScope);
	WCHAR TempBuffer[10];
	int iCnt = 0;
	int i = 0;
	memset(TempBuffer, 0, sizeof(TempBuffer));

	if (!FindWord(pKeyName))
		return false;
	while (!IsToken()) // ��ū�� ���� �� ���� �˻�
	{
		TempBuffer[iCnt] = **m_ppBufferPointer;
		iCnt++;
		*m_ppBufferPointer = *m_ppBufferPointer + 1;
	}
	if (iCnt <= 0)
	{
		TokenError(iCnt, pKeyName);
		return false;
	}

	TempBuffer[iCnt] = '\0';

	*OutValue = _wtoi(TempBuffer);
	return true;
}

bool Parser::GetValue_Double(WCHAR *pBufferPointer, const WCHAR *pKeyName, const WCHAR *pKeyScope, double *OutValue)
{
	m_ppBufferPointer = &pBufferPointer;
	m_pKeyScope = const_cast<WCHAR*>(pKeyScope);
	WCHAR TempBuffer[20];
	int iCnt = 0;
	int i = 0;
	memset(TempBuffer, 0, sizeof(TempBuffer));

	if (!FindWord(pKeyName))
		return false;
	while (!IsToken()) // ��ū�� ���� �� ���� �˻�
	{
		TempBuffer[iCnt] = **m_ppBufferPointer;
		iCnt++;
		*m_ppBufferPointer = *m_ppBufferPointer + 1;
	}
	if (iCnt <= 0)
	{
		TokenError(iCnt, pKeyName);
		return false;
	}

	TempBuffer[iCnt] = '\0';

	*OutValue = _wtof(TempBuffer);
	return true;
}

bool Parser::GetValue_Char(WCHAR *pBufferPointer, const WCHAR *pKeyName, const WCHAR *pKeyScope, WCHAR *OutValue)
{
	m_ppBufferPointer = &pBufferPointer;
	m_pKeyScope = const_cast<WCHAR*>(pKeyScope);
	WCHAR TempBuffer;
	int iCnt = 0;
	int i = 0;
	TempBuffer = '0';

	if (!FindWord(pKeyName))
		return false;
	if (!IsToken()) // ��ū�� ���� �� ���� �˻�
	{
		TempBuffer = **m_ppBufferPointer;
		*m_ppBufferPointer = *m_ppBufferPointer + 1;
	}
	else
	{
		TokenError(iCnt, pKeyName);
		return false;
	}

	*OutValue = TempBuffer;
	return true;
}

bool Parser::GetValue_String(WCHAR *pBufferPointer, const WCHAR *pKeyName, const WCHAR *pKeyScope, WCHAR *OutValue)
{
	m_ppBufferPointer = &pBufferPointer;
	m_pKeyScope = const_cast<WCHAR*>(pKeyScope);
	WCHAR TempBuffer[100];
	int iCnt = 0;
	int i = 0;
	memset(TempBuffer, 0, sizeof(TempBuffer));

	if (!FindWord(pKeyName))
		return false;
	if (**m_ppBufferPointer != '\"') // ������ '"'���� �˻�
	{
		printf_s("���ڿ� ���� : �ش� ���ڿ��� ã������, ������ ��ȣ�� ǥ�ð� �߸��Ǿ����ϴ�.\n");
		return false;
	}
	*m_ppBufferPointer = *m_ppBufferPointer + 1;
	while (**m_ppBufferPointer != '\"') // �Ŀ����� '"'�� ���� �� ���� �˻�
	{
		TempBuffer[iCnt] = **m_ppBufferPointer;
		iCnt++;
		*m_ppBufferPointer = *m_ppBufferPointer + 1;
		if (**m_ppBufferPointer == '\0')
		{
			printf_s("���ڿ� ���� : �ش� ���ڿ��� ã������, �Ŀ��� ��ȣ�� ǥ�ð� �߸��Ǿ����ϴ�.\n");
			return false;
		}
	}

	wcscpy_s(OutValue, sizeof(TempBuffer), TempBuffer);
	return true;
}
