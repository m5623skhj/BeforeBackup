#include "PreCompile.h"
#include <Windows.h>
#include "CPUUsageObserver.h"

CCPUUsageObserver::CCPUUsageObserver(HANDLE hProcess)
{
	if (hProcess == INVALID_HANDLE_VALUE)
		m_hProcess = GetCurrentProcess();

	SYSTEM_INFO SystemInfo;

	GetSystemInfo(&SystemInfo);

	m_iNumOfProcessors = SystemInfo.dwNumberOfProcessors;

	m_fProcessorTotal = 0;
	m_fProcessorKernel = 0;
	m_fProcessorUser = 0;

	m_fProcessTotal = 0;
	m_fProcessKernel = 0;
	m_fProcessUser = 0;

	m_uiProcessor_LastKernel.QuadPart = 0;
	m_uiProcessor_LastUser.QuadPart = 0;
	m_uiProcessor_LastIdle.QuadPart = 0;

	m_uiProcess_LastKernel.QuadPart = 0;
	m_uiProcess_LastUser.QuadPart = 0;
	m_uiProcess_LastTime.QuadPart = 0;

	UpdateCPUTime();
}

CCPUUsageObserver::~CCPUUsageObserver()
{

}

void CCPUUsageObserver::UpdateCPUTime()
{
	////////////////////////////////////////////
	// ���μ��� ������ ������
	////////////////////////////////////////////
	ULARGE_INTEGER IdleTime;
	ULARGE_INTEGER KernelTime;
	ULARGE_INTEGER UserTime;

	////////////////////////////////////////////
	// �ý��� ��� �ð��� ����
	////////////////////////////////////////////
	if (GetSystemTimes((PFILETIME)&IdleTime, (PFILETIME)&KernelTime, (PFILETIME)&UserTime) == false)
		return;

	ULONGLONG IdleTimeDif = IdleTime.QuadPart - m_uiProcessor_LastIdle.QuadPart;
	// KernelTime ���� IdleTime �� ���ԵǾ� ����
	ULONGLONG KernelTimeDif = KernelTime.QuadPart - m_uiProcessor_LastKernel.QuadPart;
	ULONGLONG UserTimeDif = UserTime.QuadPart - m_uiProcessor_LastUser.QuadPart;

	ULONGLONG TotalTime = KernelTimeDif + UserTimeDif;
	ULONGLONG TimeDif;

	if (TotalTime == 0)
	{
		m_fProcessorTotal = 0;
		m_fProcessorKernel = 0;
		m_fProcessorUser = 0;
	}
	else
	{
		// KernelTime �� IdleTime �� ���ԵǾ� �����Ƿ�
		// IdleTime �� ���� �����
		m_fProcessorTotal = (float)((double)(TotalTime - IdleTimeDif) / TotalTime * 100.0f);
		m_fProcessorKernel = (float)((double)UserTimeDif / TotalTime * 100.0f);
		m_fProcessorUser = (float)((double)(KernelTimeDif - IdleTimeDif) / TotalTime * 100.0f);
	}

	ULARGE_INTEGER m_uiProcessor_LastKernel = KernelTime;
	ULARGE_INTEGER m_uiProcessor_LastUser = UserTime;
	ULARGE_INTEGER m_uiProcessor_LastIdle = IdleTime;

	// ������ ���μ��� ������ ������
	ULARGE_INTEGER None;
	ULARGE_INTEGER NowTime;

	////////////////////////////////////////////
	// ������ 100 ���뼼���� ���� �ð��� ���� (UTC �ð�)
	// ���μ��� ���� �Ǵ��� ����
	// a = ���ð����� �ý��� �ð� (���� ������ �ð�)
	// b = ���μ����� CPU ��� �ð�
	// a : 100 = b : ����
	////////////////////////////////////////////

	// �󸶳� �ð��� �������� 100 ���뼼���� �ð��� ����
	GetSystemTimeAsFileTime((LPFILETIME)&NowTime);

	// �ش� ���μ����� ����� �ð��� ����
	// �ι�°, ����°�� ���� / ���� �ð��̹Ƿ� ������� ����
	GetProcessTimes(m_hProcess, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&KernelTime, (LPFILETIME)&UserTime);

	// ������ ����� ���μ��� �ð����� ���� ���Ͽ�
	// �ð��� �󸶳� ���������� Ȯ����
	TimeDif = NowTime.QuadPart - m_uiProcess_LastTime.QuadPart;
	UserTimeDif = UserTime.QuadPart - m_uiProcess_LastUser.QuadPart;
	KernelTimeDif = KernelTime.QuadPart - m_uiProcess_LastKernel.QuadPart;

	TotalTime = KernelTimeDif + UserTimeDif;

	m_fProcessTotal = (float)(TotalTime / (double)m_iNumOfProcessors / (double)TimeDif * 100.0f);
	m_fProcessKernel = (float)(KernelTimeDif / (double)m_iNumOfProcessors / (double)TimeDif * 100.0f);
	m_fProcessUser = (float)(UserTimeDif / (double)m_iNumOfProcessors / (double)TimeDif * 100.0f);

	m_uiProcess_LastTime = NowTime;
	m_uiProcess_LastKernel = KernelTime;
	m_uiProcess_LastUser = UserTime;
}

