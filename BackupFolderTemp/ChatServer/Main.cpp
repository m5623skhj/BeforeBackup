// NetServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "PreCompile.h"
#include "ChatServer.h"
#include <iostream>


int main()
{
	timeBeginPeriod(1);
	system("mode con cols=84 lines=31");

	CChatServer ChattingServer;
	for (int i = 0; i < 40; ++i)
	{
		wprintf(L"\n");
	}

	if (!ChattingServer.ChattingServerStart(L"ChatServerOption.txt", L"ChatServerLanClientOption.txt", L"ChatServerMonitoringClient.txt"))
		return 0;

	while (1)
	{
		Sleep(1000);
		wprintf(L"\n\nExit Program : ESC\n");

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			WritingProfile();
			ChattingServer.ChattingServerStop();
			break;
		}

		wprintf(L"-------------------------------------------------------------------------\n\n\n");
		wprintf(L"Session Count				:	%15d\n", ChattingServer.GetNumOfUser());
		wprintf(L"Session PacketPool Alloc Count		:	%15d\n", ChattingServer.GetNumOfAllocPlayer());
		wprintf(L"Session PacketPool Chunk Count		:	%15d\n", ChattingServer.GetNumOfAllocPlayerChunk());
		wprintf(L"Using NetServer SerializeBuf Count	:	%15d\n", ChattingServer.GetUsingNetServerBufCount());
		wprintf(L"Using LanServer SerializeBuf Count	:	%15d\n", ChattingServer.GetUsingLanServerBufCount());
		wprintf(L"Update Message Pool Node Count		:	%15d\n", ChattingServer.GetNumOfMessageInPool());
		wprintf(L"Update Message Pool Chunk Count		:	%15d\n", ChattingServer.GetNumOfMessageInPoolChunk());
		wprintf(L"Update Message Queue Node Count		:	%15d\n\n", ChattingServer.GetRestMessageInQueue());
		wprintf(L"Using SessionKey Chunk Count  		:	%15d\n", ChattingServer.GetUsingSessionKeyChunkCount());
		wprintf(L"Using SessionKey Node Count   		:	%15d\n", ChattingServer.GetUsingSessionKeyNodeCount());
		wprintf(L"Player Count				:	%15d\n\n", ChattingServer.GetNumOfPlayer());
		wprintf(L"Accept Total Count			:	%15d\n", ChattingServer.GetAcceptTotal());
		wprintf(L"Accept TPS				: 	%15d\n", ChattingServer.GetAcceptTPSAndReset());
		wprintf(L"Update TPS				:	%15d\n\n", ChattingServer.GetUpdateTPSAndReset());
		wprintf(L"LanClient Recved : %d\n", ChattingServer.GetNumOfClientRecvPacket());
		wprintf(L"Recv Join Packet : %d\n\n", ChattingServer.GetNumOfRecvJoinPacket());
		wprintf(L"------------------------------- ErrorPrint ------------------------------\n");
		wprintf(L"checksum %d / payload %d / HeaderCode %d\n", ChattingServer.checksum, ChattingServer.payloadOver, ChattingServer.HeaderCode);
		wprintf(L"SessionKey Miss %d / SessionKey Not Found %d\n", ChattingServer.GetNumOfSessionKeyMiss(), ChattingServer.GetNumOfSessionKeyNotFound());
		wprintf(L"-------------------------------------------------------------------------\n");
	}
}