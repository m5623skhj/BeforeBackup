#include "PreCompile.h"
#include "MatchmakingServer.h"

int main()
{
	timeBeginPeriod(1);
	system("mode con cols=84 lines=32");

	CMatchmakingServer MatchmakingServer;

	if (!MatchmakingServer.MatchmakingServerStart(L"MatchmakingNetServer.ini",
		L"MatchmakingServerLanClient.ini", L"MatchmakingServerMonitoringClient.ini"))
	{
		wprintf_s(L"Matchmaking Server Start Error\n");
		return 0;
	}

	while (1)
	{
		Sleep(1000);
		wprintf(L"\n\nExit Program : ESC\n");

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			WritingProfile();
			MatchmakingServer.MatchmakingServerStop();
			break;
		}
		
		wprintf(L"-------------------------------------------------------------------------\n\n\n");
		wprintf(L"Session Count				:	%15d\n", MatchmakingServer.m_uiNumOfConnectUser);
		wprintf(L"Login Player Count		:	%15d\n", MatchmakingServer.m_uiNumOfLoginCompleteUser);
		wprintf(L"Total Accept				:	%15d\n\n", MatchmakingServer.GetNumOfTotalAccept());

		//wprintf(L"Recv TPS				:	%15d\n", MatchmakingServer.GetRecvTPSAndReset());
		//wprintf(L"Send TPS				:	%15d\n", MatchmakingServer.GetSendTPSAndReset());
		wprintf(L"Accept TPS				:	%15d\n\n", MatchmakingServer.GetAcceptTPSAndReset());
		wprintf(L"Login Success TPS			:	%15d\n\n", MatchmakingServer.GetLoginSuccessTPSAndReset());

		wprintf(L"User Memory Pool Chunk Alloc Count	:	%15d\n", MatchmakingServer.GetUserInfoAlloc());
		wprintf(L"User Memory Pool Chunk Use Count	:	%15d\n", MatchmakingServer.GetUserInfoUse());
		wprintf(L"LanSerializeBuf Alloc Count		:	%15d\n", MatchmakingServer.GetNumOfAllocLanSerializeChunk());
		wprintf(L"LanSerializeBuf Use Count		:	%15d\n", MatchmakingServer.GetNumOfUsingLanSerializeBuf());
		wprintf(L"LanSerializeBuf Chunk Use Count		:	%15d\n", MatchmakingServer.GetNumOfUsingLanSerializeChunk());
		wprintf(L"NetSerializeBuf Alloc Count		:	%15d\n", MatchmakingServer.GetNumOfAllocSerializeChunk());
		wprintf(L"NetSerializeBuf Use Count			:	%15d\n", MatchmakingServer.GetNumOfUsingSerializeBuf());
		wprintf(L"NetSerializeBuf Chunk Use Count		:	%15d\n\n", MatchmakingServer.GetNumOfUsingSerializeChunk());
		wprintf(L"-------------------------------------------------------------------------\n");
	}
}
