#include "PreComfile.h"
#include "Profiler.h"
//#include <Windows.h>
#include <processthreadsapi.h>

#undef Begin
#undef End
#undef WritingProfile
#undef InitializeProfiler

#if PROFILING == SINGLE_THREAD_PROFILING
CProfiler g_Profiler;
#elif PROFILING == MULTI_THREAD_PROFILING
CProfilerManager g_ProfilerManager;
#endif

#if PROFILING == SINGLE_THREAD_PROFILING || PROFILING == MULTI_THREAD_PROFILING
CProfiler::CProfiler()
{
	QueryPerformanceFrequency(&m_liStandard);
	InitializeProfiler();
}

CProfiler::~CProfiler()
{
	//WritingProfile();
}

void CProfiler::Begin(const char* ProfileName)
{
	int ProfileNumber = 0;
	// Profiler �� ������ ���� ����ϰ� �־����� Ȥ��
	// ���� �� �ڸ��� ����� �������� �Ǻ���
	for (int i = 0; i < df_MAX_PROFILE; i++)
	{
		// ���� �ڸ��� ����ִٸ� ���ڷ� ���� ���ڿ��� ������
		if (m_ProfileInfo[i].m_pName[0] == 0)
		{
			strcpy_s(m_ProfileInfo[i].m_pName, ProfileName);
			ProfileNumber = i;
			break;
		}
		// ���� �ڸ��� ������� �ʴٸ�
		// ���� ������ ���ڷ� ���� ���ڿ��� ���� ���ڿ����� Ȯ����
		else if (strcmp(ProfileName, m_ProfileInfo[i].m_pName) == 0)
		{
			ProfileNumber = i;
			break;
		}
	}

	// ���� �ڸ��� �������Ϸ��� ������ ��� �� ���� �ʾҴٸ� (Begin �� �ι� ���� ȣ���ߴٸ�)
	// ������ �ν��ϰ� ����ڰ� ó���ϵ��� ������Ŵ
	if (m_ProfileInfo[ProfileNumber].m_bIsOpen == true)
		throw;

	m_ProfileInfo[ProfileNumber].m_bIsOpen = true;
	// ���� �ð��� Ŭ�� �߻� ������ �����ϰ� ������
	QueryPerformanceCounter(&m_ProfileInfo[ProfileNumber].m_liStartTime);
}

void CProfiler::End(const char* ProfileName)
{
	LARGE_INTEGER EndTime;
	QueryPerformanceCounter(&EndTime);

	int ProfileNumber = 0;
	// ���ڷ� �޾ƿ� ���ڿ��� ������ Begin ���� ����� ���ڿ��� ���Ͽ�
	// ��� ���������� ���¸� ������ �Ǵ���
	// ������ ������ ������ ���� �� ���ܸ� ���� ����ڰ� ó���ϵ��� ������Ŵ
	for (int i = 0; i < 30; i++)
	{
		if (strcmp(ProfileName, m_ProfileInfo[i].m_pName) == 0 &&
			m_ProfileInfo[i].m_bIsOpen == true)
		{
			ProfileNumber = i;
			break;
		}
		if (m_ProfileInfo[i].m_pName[0] == 0)
		{
			throw;
		}
	}
	
	// ��� ���� EndTime ���� ������ Begin ���� ���� StartTime �� ���� ���Ͽ�
	// Ŭ�� ������ ������ �ð��� ����
	long long Result = EndTime.QuadPart - m_ProfileInfo[ProfileNumber].m_liStartTime.QuadPart;
	for (int insertNumber = 0; insertNumber < df_MAX_EXCLUDE_VAULE; insertNumber++)
	{
		// �̹��� ���� ����� ������ ��ϵ� insertNumber �� °�� �� ���� ���� ���
		// �̹��� ���� ������� insertNumber �� �ڸ��� ��ü�� 
		if (m_ProfileInfo[ProfileNumber].m_llMin[insertNumber] > Result)
		{
			// insertNumber �� 0�� ��� ������ �ڸ��� �ִ� ���� ����
			if (insertNumber != 0)
			{
				m_ProfileInfo[ProfileNumber].m_llMin[insertNumber - 1] = m_ProfileInfo[ProfileNumber].m_llMin[insertNumber];
			}

			m_ProfileInfo[ProfileNumber].m_llMin[insertNumber] = Result;
		}
		else if (m_ProfileInfo[ProfileNumber].m_llMax[insertNumber] < Result)
		{
			// insertNumber �� 0�� ��� ������ �ڸ��� �ִ� ���� ����
			if (insertNumber != 0)
			{
				m_ProfileInfo[ProfileNumber].m_llMax[insertNumber - 1] = m_ProfileInfo[ProfileNumber].m_llMax[insertNumber];
			}

			m_ProfileInfo[ProfileNumber].m_llMax[insertNumber] = Result;
		}
	}

	m_ProfileInfo[ProfileNumber].m_llTimeSum += Result;
	++m_ProfileInfo[ProfileNumber].m_iNumOfCall;
	m_ProfileInfo[ProfileNumber].m_bIsOpen = false;
}

void CProfiler::WritingProfile()
{
	char FileName[40] = "Profiling ";
	char TimeBuffer[30];
	__time64_t Time;
	tm newTime;
	_time64(&Time);
	localtime_s(&newTime, &Time);
	asctime_s(TimeBuffer, (size_t)sizeof(TimeBuffer), &(const tm)newTime);
	strcat_s(FileName, TimeBuffer);
	FileName[df_CANCLE_RETURN] = '\0';
	strcat_s(FileName, ".txt");

	for (int iCnt = 0; iCnt < 30; iCnt++)
	{
		if (FileName[iCnt] == ':')
			FileName[iCnt] = '_';
	}

	FILE *fp;
	fopen_s(&fp, FileName, "w");

	fprintf_s(fp, "                       Name  |             Average  |                 Min  |                 Max  |    Call  \n");
	fprintf_s(fp, "-----------------------------|----------------------|----------------------|----------------------|----------\n");

	for (int i = 0; i < 30; i++)
	{
		if (m_ProfileInfo[i].m_pName[0] == NULL)
			break;
		fprintf_s(fp, "%29s|%20f��|%20f��|%20f��|%10d\n", m_ProfileInfo[i].m_pName, (long double)m_ProfileInfo[i].m_llTimeSum / (long double)m_ProfileInfo[i].m_iNumOfCall * 1000000 / (long double)m_liStandard.QuadPart
			, (long double)m_ProfileInfo[i].m_llMin[0] * 1000000 / (long double)m_liStandard.QuadPart, (long double)m_ProfileInfo[i].m_llMax[0] * 1000000 / (long double)m_liStandard.QuadPart
			, m_ProfileInfo[i].m_iNumOfCall);
	}

	fprintf_s(fp, "-----------------------------|----------------------|----------------------|----------------------|----------\n");
	fclose(fp);
}

void CProfiler::WritingProfile(FILE *fp)
{
	fprintf_s(fp, " Thread : %d\n", m_iThreadID);
	fprintf_s(fp, "                       Name  |             Average  |                 Min  |                 Max  |    Call  \n");
	fprintf_s(fp, "-----------------------------|----------------------|----------------------|----------------------|----------\n");

	for (int i = 0; i < 30; i++)
	{
		if (m_ProfileInfo[i].m_pName[0] == NULL)
			break;
		fprintf_s(fp, "%29s|%20f��|%20f��|%20f��|%10d\n", m_ProfileInfo[i].m_pName, (long double)m_ProfileInfo[i].m_llTimeSum / (long double)m_ProfileInfo[i].m_iNumOfCall * 1000000 / (long double)m_liStandard.QuadPart
			, (long double)m_ProfileInfo[i].m_llMin[0] * 1000000 / (long double)m_liStandard.QuadPart, (long double)m_ProfileInfo[i].m_llMax[0] * 1000000 / (long double)m_liStandard.QuadPart
			, m_ProfileInfo[i].m_iNumOfCall);
	}

	fprintf_s(fp, "-----------------------------|----------------------|----------------------|----------------------|----------\n\n\n\n\n\n");
}

void CProfiler::InitializeProfiler()
{
	for (int ProfileIdx = 0; ProfileIdx < 30; ++ProfileIdx)
	{
		m_ProfileInfo[ProfileIdx].m_pName[0] = 0;
		m_ProfileInfo[ProfileIdx].m_iNumOfCall = 0;
		for (int LeaveValue = 0; LeaveValue < df_MAX_EXCLUDE_VAULE; ++LeaveValue)
		{
			m_ProfileInfo[ProfileIdx].m_llMin[LeaveValue] = 90000000;
			m_ProfileInfo[ProfileIdx].m_llMax[LeaveValue] = 0;
		}
		m_ProfileInfo[ProfileIdx].m_llTimeSum = 0;
		m_ProfileInfo[ProfileIdx].m_bIsOpen = false;
	}
}



///////////////////////////////////////////
// CProfileManager Method
///////////////////////////////////////////

CProfilerManager::CProfilerManager()
{
	m_iThreadIndex = 0;
	for (int ProfileIdx = 0; ProfileIdx < df_MAX_THREAD; ++ProfileIdx)
	{
		m_pProfiler[ProfileIdx] = new CProfiler();
	}

	m_iTLSIndex = TlsAlloc();
}

CProfilerManager::~CProfilerManager()
{
	for (int ProfileIdx = 0; ProfileIdx < df_MAX_THREAD; ++ProfileIdx)
	{
		delete m_pProfiler[ProfileIdx];
	}
}

void CProfilerManager::Begin(const char *pProfileName)
{
	CProfiler *pProfiler = (CProfiler*)TlsGetValue(m_iTLSIndex);
	if (pProfiler == NULL)
	{
		UINT NowIndex = InterlockedIncrement((UINT*)&m_iThreadIndex) - 1;
		pProfiler = m_pProfiler[NowIndex];
		pProfiler->SetThreadID(GetCurrentThreadId());
		TlsSetValue(m_iTLSIndex, pProfiler);
	}

	pProfiler->Begin(pProfileName);
}

void CProfilerManager::End(const char *pProfileName)
{
	((CProfiler*)(TlsGetValue(m_iTLSIndex)))->End(pProfileName);
}

void CProfilerManager::WritingProfileAll()
{
	char FileName[40] = "Profiling ";
	char TimeBuffer[30];
	__time64_t Time;
	tm newTime;
	_time64(&Time);
	localtime_s(&newTime, &Time);
	asctime_s(TimeBuffer, (size_t)sizeof(TimeBuffer), &(const tm)newTime);
	strcat_s(FileName, TimeBuffer);
	FileName[df_CANCLE_RETURN] = '\0';
	strcat_s(FileName, ".txt");

	for (int iCnt = 0; iCnt < 30; iCnt++)
	{
		if (FileName[iCnt] == ':')
			FileName[iCnt] = '_';
	}

	FILE *fp;
	fopen_s(&fp, FileName, "w");

	for (int i = 0; i < df_MAX_THREAD; ++i)
		m_pProfiler[i]->WritingProfile(fp);

	fclose(fp);
	wprintf(L"Profiling Complete\n");
}

void CProfilerManager::ClearProfileAll()
{
	for (int i = 0; i < df_MAX_THREAD; ++i)
		m_pProfiler[i]->InitializeProfiler();

	wprintf(L"Profiling Sample Initialize Complete\n");
}


#endif