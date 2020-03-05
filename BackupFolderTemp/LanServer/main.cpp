// LanServer.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

//#include "stdafx.h"
#include "PreComfile.h"
#include "Profiler.h"
#include "echo.h"

int main()
{
	CEcho Echo(L"0.0.0.0", 6000, 4, true, 200);

	while (1)
	{
		Sleep(1000);
		if (GetAsyncKeyState('1'))
		{
			break;
		}
		//printf("Session Count	 :	%15d\nStack Rest Size  :	%15d\nNew		 :	%15d\nDel		 :	%15d\n\n", Echo.GetNumOfUser(), Echo.GetStackRestSize(), Echo.g_ULLConuntOfNew, Echo.g_ULLConuntOfDel);
		printf("Session Count	 :	%15d\n\n", Echo.GetNumOfUser());
		//printf("Session Count : %d / Session ReadBufferSize : %d %d / New : %lld / Delete : %lld\n", 
		//	Echo.GetNumOfUser(), Echo.GetReadSendBufferSize(0), Echo.GetReadSendBufferSize(1), Echo.m_ULLCountOfNew, Echo.m_ULLCountOfDelete);
	}

    return 0;
}

