#include "PreComfile.h"
#include "Log.h"
#include <time.h>

// ��� ���� ����� �α� ����
int g_iLogLevel = LOG_LEVEL::LOG_DEBUG;
// �α׸� ���� �͵鿡 ���� ī����
// ������ �ŷ��� �� �ִ� ���� �ƴϸ�
// �뷫���� ������ �ľ��ϴ� �Ϳ��� ������ �����ϴ�
static int g_iLogCounter = 0;
// �α��� ���
static WCHAR g_szFilePath[256];
// ������ �� �� ���ÿ� �������� ���ϰ� �ϱ� ���ؼ�
// �ش� �κп� ���� �ɾ��
static SRWLOCK g_SRWLock;

void SetLogLevel(int LogLevel)
{
	// �α� ���ؿ� ������ ������ ����� ������ �α׸� ����
	if (LogLevel >= LOG_LEVEL::LOGLEVEL_MAX || LogLevel < LOG_LEVEL::LOG_DEBUG)
	{
		wprintf_s(L"Incorsrect Log Level\nSet Default Log Level : LOG_DEBUG\n\n");
		g_iLogLevel = LOG_LEVEL::LOG_DEBUG;
	}
	else
		g_iLogLevel = LogLevel;
	InitializeSRWLock(&g_SRWLock);
}

void SetLogDirectoryPath(const WCHAR* DirectoryPath)
{
	wcscpy_s(g_szFilePath, DirectoryPath);
	wcscat_s(g_szFilePath, L"\\");
}

void Log(const WCHAR *LogType, const WCHAR *szLog, int LogLevel)
{
	WCHAR szLogBuf[LOG_BUF_MAX];
	WCHAR FilePath[LOG_FILEPATH_MAX];

	time_t nowTime = 0;
	tm NowTime;
	nowTime = time(NULL);
	localtime_s(&NowTime, &nowTime);
	NowTime.tm_year -= 100;
	NowTime.tm_mon += 1;
	// �޾ƿ� �α��� Ÿ���� �ʹ� �� ��� Too Long Type ������ ��ü�Ͽ� �α׸� ����
	if (FAILED(StringCchPrintfW(FilePath, LOG_FILEPATH_MAX, L"%s%02d%02d_%s.txt", g_szFilePath, NowTime.tm_year, NowTime.tm_mon, LogType)))
	{
		memcpy(FilePath, FilePath, LOG_LONGPATH_CUT);
		FilePath[LOG_LONGPATH_CUT - 1] = '\0';
		Log(L"[Too Long Type]", FilePath, LOG_LEVEL::LOG_SYSTEM);
		return;
	}

	// �޾ƿ� �αװ� �ʹ� �� ��� Too Long Type ������ ��ü�Ͽ� �α׸� ����
	if (FAILED(StringCchPrintfW(szLogBuf, LOG_BUF_MAX, L"[%s] [%02d-%02d-%02d %02d:%02d:%02d] LEVEL : %d / [%09d] %s\n",
		LogType, NowTime.tm_year, NowTime.tm_mon, NowTime.tm_mday, NowTime.tm_hour, NowTime.tm_min, NowTime.tm_sec,
		LogLevel, InterlockedIncrement((volatile ULONGLONG*)&g_iLogCounter), szLog)))
	{
		memcpy(szLogBuf, szLogBuf, LOG_LONGTEXT_CUT);
		FilePath[LOG_LONGTEXT_CUT - 1] = '\0';
		Log(L"[Too Long Log]", szLogBuf, LOG_LEVEL::LOG_SYSTEM);
		return;
	}
	wprintf(L"Level : %d\n%s\n", LogLevel, szLog);

	AcquireSRWLockExclusive(&g_SRWLock);
	
	FILE *fp;
	// �׳� a ���� ������ �� ������ ���� ANSI �� ������ �����ϱ� ������
	// ������ ���ٸ� w ���� ������ �����ϰ� 
	// ������ �����Ѵٸ� r+ ���� ���� ������ �κк��� ��
	_wfopen_s(&fp, FilePath, L"r+, ccs=UNICODE");
	if (fp == NULL)
	{
		// ������ ���ٸ� ������ �����ϰ� �ű⿡ ��
		_wfopen_s(&fp, FilePath, L"wt, ccs=UNICODE");
		fwrite(szLogBuf, sizeof(WCHAR), wcslen(szLogBuf), fp);
		fclose(fp);
	}
	else
	{
		// ������ �ִٸ� ���� ������ �κ����� �Ѿ�� �ű⿡ ��
		fseek(fp, 0, SEEK_END);
		fwrite(szLogBuf, sizeof(WCHAR), wcslen(szLogBuf), fp);
		fclose(fp);
	}
	ReleaseSRWLockExclusive(&g_SRWLock);
}

void LogHex(const WCHAR *LogType, WCHAR *szLog, int LogLevel, BYTE *HexPtr, int HexLen)
{
	WCHAR HexBuf[LOG_HEX_MAX];

	BYTE *pHexPtr = (BYTE*)HexBuf;
	const char *hex = "0123456789ABCDEF";
	for (int i = 0; i < HexLen; ++i)
	{
		HexBuf[i*3]   = hex[(*HexPtr >> 4) & 0xF];
		HexBuf[i*3+1] = hex[(*HexPtr) & 0xF];
		HexBuf[i*3+2] = ' ';
		HexPtr++;
	}

	HexBuf[HexLen * 3 - 1] = '\0';
	StringCchCatW(szLog, LOG_BUF_MAX, HexBuf);
	Log(LogType, szLog, LogLevel);
}