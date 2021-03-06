#include "PreCompile.h"
#include "MasterServer.h"

int main()
{
	timeBeginPeriod(1);
	system("mode con cols=84 lines=32");

	CMasterServer MasterServer;

	if (!MasterServer.MasterServerStart(L"", L"", L""))
	{
		wprintf_s(L"Master Server Start Error\n");
		return 0;
	}

	while (1)
	{
		Sleep(1000);
		wprintf(L"\n\nExit Program : ESC\n");

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			WritingProfile();
			MasterServer.MasterServerStop();
			break;
		}

		wprintf(L"-------------------------------------------------------------------------\n\n\n");
		wprintf(L"Matchmaking Server Session Count			:	%15d\n", MasterServer.NumOfMatchmakingSessionAll);
		wprintf(L"Matchmaking Server Login Session Count	:	%15d\n", MasterServer.NumOfMatchmakingLoginSession);
		wprintf(L"Battle Server Session Count	:	%15d\n", *MasterServer.pNumOfBattleServerSessionAll);
		wprintf(L"Battle Server Session Count	:	%15d\n", *MasterServer.pNumOfBattleServerLoginSession);

		wprintf(L"ClientInfo Count					:	%15d\n", MasterServer.GetNumOfClientInfo());
		wprintf(L"Room Info Memory Pool Chunk Alloc Count	:	%15d\n", MasterServer.CallGetRoomInfoMemoryPoolChunkAllocSize());
		wprintf(L"Room Info Memory Pool Chunk Use Count	:	%15d\n", MasterServer.CallGetRoomInfoMemoryPoolChunkUseSize());
		wprintf(L"Room Info Memory Pool Use Count	:	%15d\n\n", MasterServer.CallGetRoomInfoMemoryPoolBufUseSize());

		wprintf(L"LanSerializeBuf Alloc Count		:	%15d\n", MasterServer.GetAllocLanSerializeBufCount());
		wprintf(L"LanSerializeBuf Use Count		:	%15d\n", MasterServer.GetUsingLanServerSessionBufCount());
		wprintf(L"LanSerializeBuf Chunk Use Count		:	%15d\n", MasterServer.GetUsingLanSerializeChunkCount());
		wprintf(L"-------------------------------------------------------------------------\n");
	}

	wprintf(L"Master Server Close\n");
	Sleep(5000);


	return 0;
}
