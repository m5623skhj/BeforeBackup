#pragma once
#include <strsafe.h>
#include <Windows.h>

#define LOG_BUF_MAX				2048
#define LOG_LONGTEXT_CUT		1024
#define LOG_FILEPATH_MAX		512
#define LOG_LONGPATH_CUT		256
#define LOG_HEX_MAX				128

extern int g_iLogLevel;
enum LOG_LEVEL {LOG_DEBUG = 0, LOG_WARNING, LOG_ERROR, LOG_SYSTEM, LOGLEVEL_MAX};

// �α׸� ���� ��� ����
void SetLogDirectoryPath(const WCHAR* DirectoryPath);
// ���� �α׵��� �ּ� ���� ����
void SetLogLevel(int LogLevel);
void Log(const WCHAR *LogType, const WCHAR *szLog, int LogLevel);
void LogHex(const WCHAR *LogType, WCHAR *szLog, int LogLevel, BYTE *HexPtr, int HexLen);

// ������ ����� ������ ���ų� ���� ������ �α׸� �߻���Ŵ
// ������ �ִ�ũ��(LOG_BUF_MAX) ���� Ŭ ��� 
// LOG_LONGTEXT_CUT �� ��ŭ �߶� �ش� �αװ� �ʹ� ��ٴ� �α׸� ����
#define _LOG(LogLevel, LogType, ...)															\
do{																								\
	WCHAR szLogBuf[LOG_BUF_MAX];																\
	if(g_iLogLevel <= LogLevel)																	\
	{																							\
		if(FAILED(StringCchPrintfW(szLogBuf, LOG_BUF_MAX, ##__VA_ARGS__)))						\
		{																						\
			StringCchPrintfW(szLogBuf, LOG_LONGTEXT_CUT, ##__VA_ARGS__);						\
			szLogBuf[LOG_LONGTEXT_CUT - 1] = '\0';												\
			Log(L"[Too Long Log]", szLogBuf, LOG_LEVEL::LOG_SYSTEM);							\
		}																						\
		else																					\
		{																						\
			Log(LogType, szLogBuf, LogLevel);													\
		}																						\
	}																							\
}while(0)							

// ������ ����� ������ ���ų� ���� ������ �α׸� �߻���Ŵ
// HexPtr ���� ��� �����͸� ���� ���� ���� �����͸�
// HexLen ���� �ش� ���̸� �־� �����
// ������ �ִ�ũ��(LOG_BUF_MAX) ���� Ŭ ��� 
// LOG_LONGTEXT_CUT �� ��ŭ �߶� �ش� �αװ� �ʹ� ��ٴ� �α׸� ����
#define _LOGHEX(LogLevel, LogType, HexPtr, HexLen, ...)											\
do{																								\
	WCHAR szLogBuf[LOG_BUF_MAX];																\
	if(g_iLogLevel <= LogLevel)																	\
	{																							\
		if(FAILED(StringCchPrintfW(szLogBuf, LOG_BUF_MAX, ##__VA_ARGS__)))						\
		{																						\
			StringCchPrintfW(szLogBuf, LOG_LONGTEXT_CUT, ##__VA_ARGS__);						\
			szLogBuf[LOG_LONGTEXT_CUT - 1] = '\0';												\
			Log(L"[Too Long Log]", szLogBuf, LOG_LEVEL::LOG_SYSTEM);							\
		}																						\
		else																					\
		{																						\
			LogHex(LogType, szLogBuf, LogLevel, HexPtr, HexLen);								\
		}																						\
	}																							\
}while(0)		