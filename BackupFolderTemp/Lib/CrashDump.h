#pragma once

#pragma comment(lib, "DbgHelp.lib")

#include <Windows.h>
#include <signal.h>
#include <crtdbg.h>
#include <Psapi.h>
#include <DbgHelp.h>
#include <stdio.h>

class CCrashDump
{
public:
	static long _DumpCount;

	CCrashDump()
	{
		_DumpCount = 0;

		_invalid_parameter_handler oldHandler, newHandler;
		newHandler = myInvalidParameterHandler;

		oldHandler = _set_invalid_parameter_handler(newHandler); // crt �Լ��� null ������ ���� �־��� ��
		_CrtSetReportMode(_CRT_WARN, 0);						 // crt ���� �޽��� ǥ�� �ߴ� �� ������ �ٷ� ����
		_CrtSetReportMode(_CRT_ASSERT, 0);						 // crt ���� �޽��� ǥ�� �ߴ� �� ������ �ٷ� ����
		_CrtSetReportMode(_CRT_ERROR, 0);						 // crt ���� �޽��� ǥ�� �ߴ� �� ������ �ٷ� ����

																 // pure virtual function called ���� �ڵ鷯�� ����� ���� �Լ��� ��ȸ��Ų��
		_set_purecall_handler(myPurecallHandler);

		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
		signal(SIGABRT, signalHandler);
		signal(SIGINT, signalHandler);
		signal(SIGILL, signalHandler);
		signal(SIGFPE, signalHandler);
		signal(SIGSEGV, signalHandler);
		signal(SIGTERM, signalHandler);

		SetHandlerDump();
	}
	static void Crash()
	{
		int *p = nullptr;
		*p = 0;
	}
	static long __stdcall MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
	{
		int iWorkingMemory = 0;
		SYSTEMTIME stNowTime;

		long DumpCount = InterlockedIncrement(&_DumpCount);
		// ���� ���μ����� �޸� ��뷮�� ���´�
		HANDLE hProcess = 0;
		PROCESS_MEMORY_COUNTERS pmc;

		hProcess = GetCurrentProcess();

		if (NULL == hProcess)
			return 0;

		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		{
			iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);
		}
		CloseHandle(hProcess);

		// ���� ��¥�� �ð��� �˾ƿ´�
		WCHAR filename[MAX_PATH];

		GetLocalTime(&stNowTime);
		wsprintf(filename, L"Dump_%d02%d02%d_%02d.%02d.%02d_%d_%dMB.dmp", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay,
			stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount, iWorkingMemory);
		wprintf(filename, L"\n\n\n ----- Crash Error ----- %d.%d.%d / %d:%d:%d \n\n\n", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay,
			stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
		wprintf(L"Now Save dump file ....\n\n\n");

		HANDLE hDumpFile = ::CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			_MINIDUMP_EXCEPTION_INFORMATION Minidump;
			Minidump.ThreadId = ::GetCurrentThreadId();
			Minidump.ExceptionPointers = pExceptionPointer;
			Minidump.ClientPointers = TRUE;

			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile,
				MiniDumpWithFullMemory, &Minidump, NULL, NULL);

			CloseHandle(hDumpFile);
			wprintf(L"CrashDump Save Finish\n\n\n");
		}

		return EXCEPTION_EXECUTE_HANDLER;
	}
	static void SetHandlerDump()
	{
		SetUnhandledExceptionFilter(MyExceptionFilter);
	}
	static long __stdcall RedirectedSetUnHandledExceptionFilter(EXCEPTION_POINTERS *exceptionInfo)
	{
		MyExceptionFilter(exceptionInfo);
		return EXCEPTION_EXECUTE_HANDLER;
	}
	static void myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* filfile, unsigned int line, uintptr_t pReserved)
	{
		Crash();
	}
	static void myPurecallHandler()
	{
		Crash();
	}
	static void signalHandler(int Error)
	{
		Crash();
	}
};

long CCrashDump::_DumpCount = 0;
