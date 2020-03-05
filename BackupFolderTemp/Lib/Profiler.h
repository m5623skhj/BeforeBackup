#pragma once
#include <stdio.h>
#include <time.h>

// �� �����忡 ���Ͽ� �ִ�� �������ϸ� �� �� �ִ� ����
#define df_MAX_PROFILE				30

// �� �������ϸ� �׸� ���Ͽ� �ּ� �ִ밪�� ������ ����
// �� �ذ��� �ǹ� ���� ���� �� ���̹Ƿ� ������
#define df_MAX_EXCLUDE_VAULE		10

// �������ϸ� �̸� �ִ� ����
#define df_NAME_LENGTH				30
// ���� �̸� ����
#define df_CANCLE_RETURN			34

// �������ϸ� �Ŵ����� �ִ�� ���� �� �ִ� ������ ����
// ������ ������ �� ������ Ŭ ��� �������ϸ��� �������ų� 
// Ȥ�� ���ܰ� �Ͼ �� ����
#define df_MAX_THREAD				5

// �������ϸ� ���� ����
// �ش� ����� �ƹ��͵� ���� ������ ����� ��
#define NOT_PROFILING				 0
// �̱� ������ ���� �������ϸ� ����
// �̱� ������ �������� ���ư���
// �� ���·� ��Ƽ �����带 �������ϸ� �� 
// �������ϸ��� �������ų� ���ܰ� �Ͼ �� ����
#define SINGLE_THREAD_PROFILING		 1
// ��Ƽ ������ ���� �������ϸ� ����
#define MULTI_THREAD_PROFILING		 2

#define PROFILING NOT_PROFILING

#if PROFILING == SINGLE_THREAD_PROFILING

struct st_PROFILE_INFO
{
	char				m_pName[df_NAME_LENGTH];
	bool				m_bIsOpen;
	int					m_iNumOfCall;
	long long			m_llMin[df_MAX_EXCLUDE_VAULE];
	long long			m_llMax[df_MAX_EXCLUDE_VAULE];
	long long			m_llTimeSum;
	LARGE_INTEGER		m_liStartTime;
};

class CProfiler
{
private:
	LARGE_INTEGER		m_liStandard;
	st_PROFILE_INFO		m_ProfileInfo[30];
public:
	CProfiler();
	~CProfiler();

	void Begin(const char* ProfileName);
	void End(const char* ProfileName);
	void WritingProfile();
	void WritingProfile(FILE *fp);
	void InitializeProfiler();
};

extern CProfiler g_Profiler;

#define Begin(x) g_Profiler.Begin(x)
#define End(x) g_Profiler.End(x)
#define WritingProfile() g_Profiler.WritingProfile()
#define InitializeProfiler() g_Profiler.InitializeProfiler()

#elif PROFILING == MULTI_THREAD_PROFILING

struct st_PROFILE_INFO
{
	char				m_pName[df_NAME_LENGTH];
	bool				m_bIsOpen;
	int					m_iNumOfCall;
	long long			m_llMin[df_MAX_EXCLUDE_VAULE];
	long long			m_llMax[df_MAX_EXCLUDE_VAULE];
	long long			m_llTimeSum;
	LARGE_INTEGER		m_liStartTime;
};

class CProfiler
{
private:
	int					m_iThreadID;
	LARGE_INTEGER		m_liStandard;
	st_PROFILE_INFO		m_ProfileInfo[30];
public:
	CProfiler();
	~CProfiler();

	void Begin(const char* ProfileName);
	void End(const char* ProfileName);
	void WritingProfile();
	void WritingProfile(FILE *fp);
	void InitializeProfiler();

	void SetThreadID(int ThreadID) { m_iThreadID = ThreadID; }
};

class CProfilerManager
{
private:
	int						m_iThreadIndex;
	int						m_iTLSIndex;
	CProfiler				*m_pProfiler[df_MAX_THREAD];

public:
	CProfilerManager();
	~CProfilerManager();

	void Begin(const char *pProfileName);
	void End(const char *pProfileName);
	void WritingProfileAll();
	void ClearProfileAll();
};

extern CProfilerManager g_ProfilerManager;

#define Begin(x) g_ProfilerManager.Begin(x)
#define End(x) g_ProfilerManager.End(x)
#define WritingProfile() g_ProfilerManager.WritingProfileAll()
#define InitializeProfiler() g_ProfilerManager.ClearProfileAll()

#else

#define Begin(x)
#define End(x)
#define WritingProfile()
#define InitializeProfiler()

#endif



