// LoginServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "PreCompile.h"
#include "LoginServer.h"

int main()
{
	CLoginServer LoginServer;
	timeBeginPeriod(1);
	system("mode con cols=84 lines=19");

	for (int i = 0; i < 40; ++i)
	{
		wprintf(L"\n");
	}

	bool bIsServerRunning = false;
	bool LockServer = true;

	while (1)
	{
		Sleep(1000);
		if (LockServer)
		{
			wprintf(L"\n\nLogin Server Start : F1 / Login Server Stop : Z / Exit Program : ESC / UnLock : U\n");
			if(GetAsyncKeyState(0x55) & 0x8000)
				LockServer = false;
		}
		else
		{
			wprintf(L"\nLogin Server Start : F1 / Login Server Stop : Z / Exit Program : ESC / Lock : L\n");
			if (GetAsyncKeyState(0x4C) & 0x8000)
				LockServer = true;
		}

		if (!LockServer && GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			if (bIsServerRunning)
				LoginServer.LoginServerStop();
			break;
		}
		if (bIsServerRunning)
		{
			if (!LockServer && GetAsyncKeyState(0x5A) & 0x8000)
			{
				bIsServerRunning = false;
				LoginServer.LoginServerStop();
			}

			wprintf(L"-------------------------------------------------------------------------\n\n");
			wprintf(L"Chatting Server Logined		       : %15d\n\n", LoginServer.ChattingServerJoined());
			wprintf(L"NetServer / Accept Total Count	       : %15d\n", LoginServer.GetNetServerAcceptTotal());
			wprintf(L"NetServer / Login Waiting User	       : %15d\n", LoginServer.NumOfLoginWaitingUser);
			wprintf(L"NetServer / Login Complete User	       : %15d\n\n", LoginServer.NumOfLoginCompleteUser);
			wprintf(L"LanServer / Send To Client	       : %15d\n", LoginServer.GetLanServerSend());
			wprintf(L"LanServer / Recv To Client	       : %15d\n\n", LoginServer.GetLanServerRecv());
			wprintf(L"NetServer / Using SerializeBuf Count   : %15d\n", LoginServer.GetUsingNetSerializeBufCount());
			wprintf(L"LanServer / Using Session Node Count   : %15d\n", LoginServer.GetUsingLanServerSessionNodeCount());
			wprintf(L"LanServer / Using User Info Node Count : %15d\n", LoginServer.GetUsingNetUserInfoNodeCount());
			wprintf(L"LanServer / Using SerializeBuf Count   : %15d\n", LoginServer.GetUsingLanSerializeBufCount());
			wprintf(L"-------------------------------------------------------------------------\n");
		}
		else if (!LockServer && GetAsyncKeyState(VK_F1) & 0x8000)
		{
			if (!LoginServer.LoginServerStart(L"LoginNetServerOption.txt", L"LoginLanServerOption.txt", L"LoginServerOption.txt"))
				break;
			bIsServerRunning = true;
		}
	}
}
