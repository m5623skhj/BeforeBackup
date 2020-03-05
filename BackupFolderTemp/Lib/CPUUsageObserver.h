#pragma once


class CCPUUsageObserver
{
public :
	
	/////////////////////////////////////////////////////////
	// ������, Ȯ�δ�� ���μ��� �ڵ�
	// ���Է½� �ڱ� �ڽ��� ����Ʈ�� ��
	/////////////////////////////////////////////////////////
	CCPUUsageObserver(HANDLE hProcess = INVALID_HANDLE_VALUE);
	~CCPUUsageObserver();

	void UpdateCPUTime();
	
	float ProcessorTotal() { return m_fProcessorTotal; }
	float ProcessorUser() { return m_fProcessorUser; }
	float ProcessorKernel() { return m_fProcessorKernel; }

	float ProcessTotal() { return m_fProcessTotal; }
	float ProcessUser() { return m_fProcessUser; }
	float ProcessKernel() { return m_fProcessKernel; }

private :

	HANDLE m_hProcess;
	// ������ ���������� ���Ե� ���μ��� ����
	// ��� �ð��� �ش� ������ ����
	int	m_iNumOfProcessors;

	float m_fProcessorTotal;
	float m_fProcessorUser;
	float m_fProcessorKernel;

	float m_fProcessTotal;
	float m_fProcessUser;
	float m_fProcessKernel;

	ULARGE_INTEGER m_uiProcessor_LastKernel;
	ULARGE_INTEGER m_uiProcessor_LastUser;
	ULARGE_INTEGER m_uiProcessor_LastIdle;

	ULARGE_INTEGER m_uiProcess_LastKernel;
	ULARGE_INTEGER m_uiProcess_LastUser;
	ULARGE_INTEGER m_uiProcess_LastTime;
};