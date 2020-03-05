#include "PreCompile.h"
#include "MMOServer.h"
#include "ServerCommon.h"
#include "NetServerSerializeBuffer.h"
#include "Log.h"
#include "Queue.h"

#include "Profiler.h"

#pragma comment(lib, "ws2_32")

CMMOServer::CMMOServer()
{

}

CMMOServer::~CMMOServer()
{

}

bool CMMOServer::InitMMOServer(UINT NumOfMaxClient)
{
	m_uiNumOfMaxClient = NumOfMaxClient;
	m_ppSessionPointerArray = new CSession*[m_uiNumOfMaxClient];

	return true;
}

bool CMMOServer::Start(const WCHAR *szMMOServerOptionFileName)
{
	if (!OptionParsing(szMMOServerOptionFileName))
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::PARSING_ERR;
		return false;
	}
	m_uiNumOfUser = 0;

	int retval;
	WSADATA Wsa;
	if (WSAStartup(MAKEWORD(2, 2), &Wsa))
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::WSASTARTUP_ERR;
		OnError(&Error);
		return false;
	}

	m_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_ListenSock == INVALID_SOCKET)
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::LISTEN_SOCKET_ERR;
		OnError(&Error);
		return false;
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(m_wPort);
	retval = bind(m_ListenSock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::LISTEN_BIND_ERR;
		OnError(&Error);
		return false;
	}

	retval = listen(m_ListenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::LISTEN_LISTEN_ERR;
		OnError(&Error);
		return false;
	}

	//CSession::AcceptUserInfo *AcceptInfoPointerArray = new CSession::AcceptUserInfo [m_uiNumOfMaxClient];
	
	for (int i = m_uiNumOfMaxClient - 1; i >= 0; --i)
	{
		//AcceptInfoPointerArray[i].AcceptedSock = INVALID_SOCKET;
		//AcceptInfoPointerArray[i].wAcceptedPort = 0;
		//AcceptInfoPointerArray[i].wSessionIndex = i;

		//m_SessionInfoStack.Push(&AcceptInfoPointerArray[i]);
		m_SessionInfoStack.Push(i);

		m_ppSessionPointerArray[i]->m_uiIOCount = 0;
	}

	m_pUserInfoMemoryPool = new CTLSMemoryPool<CSession::AcceptUserInfo>(3, false);

	bool KeepAlive = true;
	retval = setsockopt(m_ListenSock, SOL_SOCKET, SO_KEEPALIVE, (char*)&KeepAlive, sizeof(KeepAlive));
	if (retval == SOCKET_ERROR)
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::SETSOCKOPT_ERR;
		return false;
	}

	retval = setsockopt(m_ListenSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&m_bIsNagleOn, sizeof(int));
	if (retval == SOCKET_ERROR)
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::SETSOCKOPT_ERR;
		return false;
	}

	m_hWorkerIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, m_byNumOfUsingWorkerThread);
	if (m_hWorkerIOCP == NULL)
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::WORKERIOCP_NULL_ERR;
		return false;
	}

	NumOfAuthPlayer = 0;
	NumOfGamePlayer = 0;
	NumOfTotalPlayer = 0;

	NumOfTotalAccept = 0;

	RecvTPS = 0;
	SendTPS = 0;
	AcceptTPS = 0;

	SendThreadLoop = 0;
	AuthThreadLoop = 0;
	GameThreadLoop = 0;

	m_hThreadExitEvent[en_EVENT_NUMBER::EVENT_SEND] = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadExitEvent[en_EVENT_NUMBER::EVENT_AUTH] = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadExitEvent[en_EVENT_NUMBER::EVENT_GAME] = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThreadExitEvent[en_EVENT_NUMBER::EVENT_RELEASE] = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_pWorkerThreadHandle = new HANDLE[m_byNumOfWorkerThread];
	// static �Լ����� NetServer ��ü�� �����ϱ� ���Ͽ� this �����͸� ���ڷ� �ѱ�
	for (int i = 0; i < m_byNumOfWorkerThread; i++)
		m_pWorkerThreadHandle[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadStartAddress, this, 0, NULL);
	m_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThreadStartAddress, this, 0, NULL);
	
	m_hSendThread[0] = (HANDLE)_beginthreadex(NULL, 0, SendThreadStartAddress, this, 0, NULL);
	m_hSendThread[1] = (HANDLE)_beginthreadex(NULL, 0, SendThreadStartAddress, this, 0, NULL);
	
	m_hAuthThread = (HANDLE)_beginthreadex(NULL, 0, AuthThreadStartAddress, this, 0, NULL);
	m_hGameThread = (HANDLE)_beginthreadex(NULL, 0, GameThreadStartAddress, this, 0, NULL);
	
	m_hReleaseThread = (HANDLE)_beginthreadex(NULL, 0, ReleaseThreadStartAddress, this, 0, NULL);

	if (m_hAcceptThread == 0 || m_pWorkerThreadHandle == 0 || m_hSendThread == 0
		|| m_hAuthThread == 0 || m_hGameThread == 0 || m_hReleaseThread == 0)
	{
		st_Error Error;
		Error.GetLastErr = WSAGetLastError();
		Error.ServerErr = SERVER_ERR::BEGINTHREAD_ERR;
		OnError(&Error);
		return false;
	}

	return true;
}

void CMMOServer::Stop()
{
	closesocket(m_ListenSock);
	WaitForSingleObject(m_hAcceptThread, INFINITE);
	for (UINT i = 0; i < m_byNumOfUsingWorkerThread; ++i)
	{
		if (m_ppSessionPointerArray[i]->m_byNowMode != CSession::en_SESSION_MODE::MODE_NONE)
		{
			shutdown(m_ppSessionPointerArray[i]->m_UserNetworkInfo.AcceptedSock, SD_BOTH);
		}
	}

	for (int i = 0; i < dfNUM_OF_THREAD; ++i)
		SetEvent(m_hThreadExitEvent[i]);

	WaitForMultipleObjects(dfNUM_OF_THREAD, m_hThreadExitEvent, TRUE, INFINITE);

	PostQueuedCompletionStatus(m_hWorkerIOCP, 0, 0, (LPOVERLAPPED)1);
	WaitForMultipleObjects(m_byNumOfWorkerThread, m_pWorkerThreadHandle, TRUE, INFINITE);

	//CSession::AcceptUserInfo *AcceptInfo;
	//for (UINT i = 0; i < m_uiNumOfMaxClient; ++i)
	//{
	//	m_SessionInfoStack.Pop(&AcceptInfo);
	//	delete AcceptInfo;
	//}
	delete[] m_ppSessionPointerArray;

	CloseHandle(m_hAcceptThread);
	for (int i = 0; i < dfNUM_OF_THREAD; ++i)
		CloseHandle(m_hThreadExitEvent[i]);

	for (int i = 0; i < m_byNumOfUsingWorkerThread; ++i)
	{
		CloseHandle(m_pWorkerThreadHandle[i]);
	}
	delete[] m_pWorkerThreadHandle;
}

void CMMOServer::LinkSession(CSession *PlayerPointer, UINT PlayerIndex)
{
	m_ppSessionPointerArray[PlayerIndex] = PlayerPointer;
}

UINT CMMOServer::AcceptThread()
{
	SOCKET clientsock;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);

	while (1)
	{
		clientsock = accept(m_ListenSock, (SOCKADDR*)&clientaddr, &addrlen);
		if (clientsock == INVALID_SOCKET)
		{
			int Err = WSAGetLastError();
			if (Err == WSAEINTR)
			{
				break;
			}
			else
			{
				st_Error Error;
				Error.GetLastErr = Err;
				Error.ServerErr = SERVER_ERR::ACCEPT_ERR;
				OnError(&Error);
			}
		}

		CSession::AcceptUserInfo *UserInfo = m_pUserInfoMemoryPool->Alloc();
		//m_SessionInfoStack.Pop(&UserInfo);
		
		InetNtop(AF_INET, (const void*)&clientaddr.sin_addr, UserInfo->AcceptedIP, 16);
		UserInfo->AcceptedSock = clientsock;
		UserInfo->wAcceptedPort = clientaddr.sin_port;

		m_AcceptUserInfoQueue.Enqueue(UserInfo);
		
		NumOfTotalAccept++;
		InterlockedIncrement(&AcceptTPS);
	}

	return 0;
}

UINT CMMOServer::WorkerThread()
{
	char cPostRetval;
	int retval;
	DWORD Transferred;
	CSession *pSession;
	LPOVERLAPPED lpOverlapped;
	HANDLE &WorkerIOCP = m_hWorkerIOCP;

	while (1)
	{
		cPostRetval = -1;
		Transferred = 0;
		pSession = NULL;
		lpOverlapped = NULL;

		Begin("GQCS");
		GetQueuedCompletionStatus(WorkerIOCP, &Transferred, (PULONG_PTR)&pSession, &lpOverlapped, INFINITE);
		End("GQCS");
		if (lpOverlapped != NULL)
		{
			// �ܺ� �����ڵ忡 ���� ����
			if (pSession == NULL)
			{
				PostQueuedCompletionStatus(WorkerIOCP, 0, 0, (LPOVERLAPPED)1);
				break;
			}
			// recv ��Ʈ
			CSession &IOCompleteSession = *pSession;
			if (lpOverlapped == &IOCompleteSession.m_RecvIOData.RecvOverlapped)
			{
				// Ŭ���̾�Ʈ�� ������
				if (Transferred == 0)
				{
					// ���� IOCount�� �ٿ��� �������� 1�ϰ�� �ش� ������ ������
					if (InterlockedDecrement(&IOCompleteSession.m_uiIOCount) == 0)
						IOCompleteSession.m_bLogOutFlag = true;

					continue;
				}

				Begin("Recv");
				IOCompleteSession.m_RecvIOData.RecvRingBuffer.MoveWritePos(Transferred);
				int RingBufferRestSize = IOCompleteSession.m_RecvIOData.RecvRingBuffer.GetUseSize();

				// �ش� ���ǿ� ������ ũ�Ⱑ NetServer ����ȭ ���� ��� ������� ũ��
				while (RingBufferRestSize > df_HEADER_SIZE)
				{
					// �� ����ȭ ���۸� �Ҵ� ����
					CNetServerSerializationBuf &RecvSerializeBuf = *CNetServerSerializationBuf::Alloc();
					// ��Ŷ�� ũ�Ⱑ ������ �� �� �����Ƿ� Peek �Լ��� �̿��Ͽ� �����͸� ������
					IOCompleteSession.m_RecvIOData.RecvRingBuffer.Peek((char*)RecvSerializeBuf.m_pSerializeBuffer, df_HEADER_SIZE);
					// ����ȭ ���� Read �� (����� ���Ͽ�)5 �̹Ƿ� ���Ƿ� 0���� ������
					RecvSerializeBuf.m_iRead = 0;

					BYTE Code;
					WORD PayloadLength;
					RecvSerializeBuf >> Code >> PayloadLength;

					if (Code != CNetServerSerializationBuf::m_byHeaderCode)
					{
						shutdown(IOCompleteSession.m_UserNetworkInfo.AcceptedSock, SD_BOTH);
						st_Error Err;
						Err.GetLastErr = 0;
						Err.ServerErr = HEADER_CODE_ERR;
						OnError(&Err);
						CNetServerSerializationBuf::Free(&RecvSerializeBuf);
						break;
					}
					if (RingBufferRestSize < PayloadLength + df_HEADER_SIZE)
					{
						if (PayloadLength > dfDEFAULTSIZE)
						{
							shutdown(IOCompleteSession.m_UserNetworkInfo.AcceptedSock, SD_BOTH);
							st_Error Err;
							Err.GetLastErr = 0;
							Err.ServerErr = PAYLOAD_SIZE_OVER_ERR;
							OnError(&Err);
						}
						CNetServerSerializationBuf::Free(&RecvSerializeBuf);
						break;
					}
					// �� ���ǹ��鿡�� �����۰� ������ �ִ� ����� ũ�ų� ������ �ǴܵǾ����Ƿ�
					// ��� ����� ����
					IOCompleteSession.m_RecvIOData.RecvRingBuffer.RemoveData(df_HEADER_SIZE);

					// �Ҵ���� ����ȭ ���ۿ� �����͸� �����԰� ���ÿ� ����
					retval = IOCompleteSession.m_RecvIOData.RecvRingBuffer.Dequeue(&RecvSerializeBuf.m_pSerializeBuffer[RecvSerializeBuf.m_iWrite], PayloadLength);
					RecvSerializeBuf.m_iWrite += retval;
					if (!RecvSerializeBuf.Decode())
					{
						shutdown(IOCompleteSession.m_UserNetworkInfo.AcceptedSock, SD_BOTH);
						st_Error Err;
						Err.GetLastErr = 0;
						Err.ServerErr = CHECKSUM_ERR;
						OnError(&Err);
						CNetServerSerializationBuf::Free(&RecvSerializeBuf);
						break;
					}

					RingBufferRestSize -= (retval + df_HEADER_SIZE);
					// Auth Ȥ�� Game �����尡 ó���� �ֱ⸦ ����ϰ�
					// �� ������ ���ϰ� �ִ� Recv�Ϸ�ť �� �־���
					IOCompleteSession.m_RecvCompleteQueue.Enqueue(&RecvSerializeBuf);
					InterlockedIncrement(&RecvTPS);
				}

				cPostRetval = RecvPost(&IOCompleteSession);
				End("Recv");
			}
			// send ��Ʈ
			else if (lpOverlapped == &IOCompleteSession.m_SendIOData.SendOverlapped)
			{
				Begin("Send");
				int BufferCount = IOCompleteSession.m_SendIOData.uiBufferCount;
				for (int i = 0; i < BufferCount; ++i)
					CNetServerSerializationBuf::Free(IOCompleteSession.pSeirializeBufStore[i]);

				InterlockedAdd((LONG*)&SendTPS, BufferCount);
				// ���������� ó���� �Ϸ�Ǿ��� ��� ���� ī���Ͱ� 0�� ��
				// ���� ī���ʹ� SendThread ������ �����ϱ� ������
				IOCompleteSession.m_SendIOData.uiBufferCount = 0;

				IOCompleteSession.m_bSendIOMode = NONSENDING;
				cPostRetval = POST_RETVAL_COMPLETE;
				End("Send");
			}
		}
		// overlapped �� NULL �̸� �������� ��Ȳ�̶�� �������� ����
		else
		{
			st_Error Error;
			Error.GetLastErr = WSAGetLastError();
			Error.ServerErr = SERVER_ERR::OVERLAPPED_NULL_ERR;
			OnError(&Error);

			g_Dump.Crash();
		}

		// �̹� ������ �Լ��ο��� �������ٸ� �ٽ� ���� �ö�
		if (cPostRetval == POST_RETVAL_ERR_SESSION_DELETED)
			continue;
		// ������ IOCount �� ��� 0 �̸� �ٸ� �����尡 �����ֱ� ����ϸ�
		// �÷��׸� true �� ������
		if (InterlockedDecrement(&pSession->m_uiIOCount) == 0)
			pSession->m_bLogOutFlag = true;
	}

	CNetServerSerializationBuf::ChunkFreeForcibly();
	return 0;
}

UINT CMMOServer::SendThread()
{
	UINT NumOfMaxClient = m_uiNumOfMaxClient;

	WORD SendThreadSleepTime = m_wSendThreadSleepTime;
	HANDLE &SendThreadEvent = m_hThreadExitEvent[en_EVENT_NUMBER::EVENT_SEND];

	WORD NumOfSendProgress = m_wNumOfSendProgress;
	
	CSession **SessionPointerArray = m_ppSessionPointerArray;

	UINT SendThreadLoopStart = InterlockedExchange(&m_uiSendThreadLoopStartValue, m_uiSendThreadLoopStartValue + 1);
	UINT i;

	while (1)
	{
		// Stop �Լ��� �̺�Ʈ�� Set ���� �ʴ� �̻� ����ؼ� SendThreadSleepTime �� �ֱ�� �ݺ���
		if (WaitForSingleObject(SendThreadEvent, SendThreadSleepTime) == WAIT_TIMEOUT)
		{
			InterlockedIncrement(&SendThreadLoop);
			i = SendThreadLoopStart;
			// Send �����带 2���� ������ ������ 2�� ����
			for (; i < NumOfMaxClient; i += 2)
			{
				CSession &SendSession = *SessionPointerArray[i];

				while (1)
				{
					if (SendSession.m_SendIOData.uiBufferCount > 0)
						break;

					if (SendSession.m_bSendIOMode == NONSENDING)
						SendSession.m_bSendIOMode = SENDING;
					else
						break;

					if (SendSession.m_bLogOutFlag)
					{
						SendSession.m_bSendIOMode = NONSENDING;
						break;
					}

					int SendQUseSize = SendSession.m_SendIOData.SendQ.GetRestSize();
					if (SendQUseSize == 0)
					{
						SendSession.m_bSendIOMode = NONSENDING;
						// ���� �ʿ� ���� �� ��
						//if (SendSession.m_SendIOData.SendQ.GetRestSize() > 0)
							//continue;
						break;
					}
					//else if (SendQUseSize < 0)
					//{
					//	if (InterlockedDecrement(&SendSession.m_uiIOCount) == 0)
					//		SendSession.m_bLogOutFlag = true;
					//	break;
					//}

					Begin("SendBufInit");
					WSABUF wsaSendBuf[dfONE_SEND_WSABUF_MAX];
					// ������ ������ ��ŭ�� ��������
					if (dfONE_SEND_WSABUF_MAX < SendQUseSize)
						SendQUseSize = dfONE_SEND_WSABUF_MAX;

					for (int SendCount = 0; SendCount < SendQUseSize; ++SendCount)
					{
						SendSession.m_SendIOData.SendQ.Dequeue(&SendSession.pSeirializeBufStore[SendCount]);
						wsaSendBuf[SendCount].buf = SendSession.pSeirializeBufStore[SendCount]->m_pSerializeBuffer;//GetBufferPtr();
						wsaSendBuf[SendCount].len = SendSession.pSeirializeBufStore[SendCount]->m_iWriteLast;
					}

					// ���Ե� ���� �迭 ���ο� �󸶳� ����ȭ ���۰� �����ִ����� �Ǵ��ϱ� ���� ����
					// ���Ե� ���� �Ϸ��������� 0���� �ٲ����ų�
					// Ȥ�� �����Ͽ��� ��� Game �����尡 �ȿ� �ִ� ������ ��Ʈ���� 0���� �ٲܰ���
					SendSession.m_SendIOData.uiBufferCount = SendQUseSize;

					InterlockedIncrement(&SendSession.m_uiIOCount);
					ZeroMemory(&SendSession.m_SendIOData.SendOverlapped, sizeof(OVERLAPPED));
					End("SendBufInit");

					Begin("WSASend");
					int retval = WSASend(SendSession.m_UserNetworkInfo.AcceptedSock, wsaSendBuf, SendQUseSize, NULL, 0, &SendSession.m_SendIOData.SendOverlapped, 0);
					if (retval == SOCKET_ERROR)
					{
						int err = WSAGetLastError();
						if (err != ERROR_IO_PENDING)
						{
							if (err == WSAENOBUFS)
								g_Dump.Crash();
							st_Error Error;
							Error.GetLastErr = err;
							Error.ServerErr = SERVER_ERR::WSASEND_ERR;
							OnError(&Error);

							if (InterlockedDecrement(&SendSession.m_uiIOCount) == 0)
								SendSession.m_bLogOutFlag = true;
							//else
							//	shutdown(SendSession.m_pUserNetworkInfo->AcceptedSock, SD_BOTH);

							// ������ ������ Send ���°� �ƴϿ��� �ǹǷ� ������ NONSENDING ���·� ������
							//InterlockedExchange(&SendSession.m_bSendIOMode, NONSENDING);
							SendSession.m_bSendIOMode = NONSENDING;
							End("WSASend");
							break;
						}
					}
					End("WSASend");
				}
			}
		}
		else
			break;
	}

	return 0;
}

UINT CMMOServer::AuthThread()
{
	UINT NumOfMaxClient = m_uiNumOfMaxClient;

	WORD NumOfAuthProgress = m_wNumOfAuthProgress;
	WORD LoopAuthHandlingPacket = m_wLoopAuthHandlingPacket;
	WORD AuthThreadSleepTime = m_wAuthThreadSleepTime;

	HANDLE &AuthThreadEvent = m_hThreadExitEvent[en_EVENT_NUMBER::EVENT_AUTH];
	
	CSession **SessionPointerArray = m_ppSessionPointerArray;

	HANDLE &WorkerIOCP = m_hWorkerIOCP;

	CLockFreeStack<WORD> &SessionIndexStack = m_SessionInfoStack;

	CTLSMemoryPool<CSession::AcceptUserInfo> &UserInfoMemoryPool = *m_pUserInfoMemoryPool;
	
	while (1)
	{
		// Stop �Լ��� �̺�Ʈ�� Set ���� �ʴ� �̻� ����ؼ� AuthThreadSleepTime �� �ֱ�� �ݺ���
		if (WaitForSingleObject(AuthThreadEvent, AuthThreadSleepTime) == WAIT_TIMEOUT)
		{
			InterlockedIncrement(&AuthThreadLoop);
			// ���� �󸶸�ŭ�� ������ ť���� ��������� Ȯ����
			int NumOfAcceptWaitingUser = m_AcceptUserInfoQueue.GetRestSize();

			// ������ �� ���� ó������ ������ ���ٸ� ������ �� ��ŭ�� ó���ϰ� ���� ������
			if (NumOfAuthProgress < NumOfAcceptWaitingUser)
				NumOfAcceptWaitingUser = NumOfAuthProgress;

			for (int i = 0; i < NumOfAcceptWaitingUser; ++i)
			{
				Begin("Login");
				CSession::AcceptUserInfo *pNewUserAcceptInfo;
				m_AcceptUserInfoQueue.Dequeue(&pNewUserAcceptInfo);
				//WORD SessionIndex = pNewUserAcceptInfo->wSessionIndex;
				WORD SessionIndex;
				SessionIndexStack.Pop(&SessionIndex);

				CSession &LoginSession = *SessionPointerArray[SessionIndex];
				LoginSession.m_wSessionIndex = SessionIndex;

				memcpy(LoginSession.m_UserNetworkInfo.AcceptedIP, pNewUserAcceptInfo->AcceptedIP, 16);
				LoginSession.m_UserNetworkInfo.AcceptedSock = pNewUserAcceptInfo->AcceptedSock;
				LoginSession.m_UserNetworkInfo.wAcceptedPort = pNewUserAcceptInfo->wAcceptedPort;
				UserInfoMemoryPool.Free(pNewUserAcceptInfo);

				//LoginSession.m_pUserNetworkInfo = pNewUserAcceptInfo;
				//LoginSession.m_Socket = pNewUserAcceptInfo->AcceptedSock;

				CreateIoCompletionPort((HANDLE)LoginSession.m_UserNetworkInfo.AcceptedSock, WorkerIOCP,
					(ULONG_PTR)SessionPointerArray[SessionIndex], NULL);

				LoginSession.OnAuth_ClientJoin();
				LoginSession.m_bLogOutFlag = false;

				RecvPost(&LoginSession);
				// ���� ��带 �ٲٸ� �� ��ü�� ���� ó���� ���۵ǹǷ�
				// ���� �������� �ٲ���
				LoginSession.m_byNowMode = CSession::en_SESSION_MODE::MODE_AUTH;

				InterlockedIncrement(&NumOfAuthPlayer);
				InterlockedIncrement(&NumOfTotalPlayer);
				End("Login");
			}

			for (UINT i = 0; i < NumOfMaxClient; ++i)
			{
				CSession &NowSession = *SessionPointerArray[i];

				if (NowSession.m_byNowMode != CSession::en_SESSION_MODE::MODE_AUTH)
					continue;
				
				CNetServerSerializationBuf *RecvPacket;
				// ������ �� ��ŭ�� �Ϸ������� �� ��Ŷ�� ó����
				for (int NumOfDequeue = 0; NumOfDequeue < LoopAuthHandlingPacket; ++NumOfDequeue)
				{
					if (NowSession.m_RecvCompleteQueue.Dequeue(&RecvPacket))
					{
						Begin("AuthRecvPacket");
						NowSession.OnAuth_Packet(RecvPacket);
						CNetServerSerializationBuf::Free(RecvPacket);
						End("AuthRecvPacket");
						// �ܺο��� Game ���� �����ߴٸ� 
						// ���̻� Auth ������ ó���ϸ� �ȵǹǷ� �ݺ����� Ż����
						if (NowSession.m_bAuthToGame == true)
							break;
					}
					// ���̻� ó���� ��Ŷ�� ������ �ݺ����� Ż����
					else
						break;
				}
			}

			OnAuth_Update();

			for (UINT i = 0; i < NumOfMaxClient; ++i)
			{
				CSession &NowSession = *SessionPointerArray[i];
				
				if (NowSession.m_bLogOutFlag)
				{
					// Auth ����̰� Send ���� �ƴ� ���Ǹ� �α׾ƿ� ��� ���·� ������ ������
					if (NowSession.m_byNowMode == CSession::en_SESSION_MODE::MODE_AUTH && NowSession.m_bSendIOMode == NONSENDING)
					{
						NowSession.m_byNowMode = CSession::en_SESSION_MODE::MODE_WAIT_LOGOUT;
						NowSession.OnAuth_ClientLeave(true);

						InterlockedDecrement(&NumOfAuthPlayer);
					}
				}
				// �������� �Ѿ �����̰� ���ÿ� ��尡 Auth ����
				else if (NowSession.m_byNowMode == CSession::en_SESSION_MODE::MODE_AUTH && NowSession.m_bAuthToGame )
				{
					// Game �����尡 �� ������ ������ �� �ֵ��� ���¸� ������
					NowSession.m_byNowMode = CSession::en_SESSION_MODE::MODE_AUTH_TO_GAME;
					NowSession.OnAuth_ClientLeave(false);

					InterlockedDecrement(&NumOfAuthPlayer);
				}
			}
		}
		else
			break;
	}

	return 0;
}

UINT CMMOServer::GameThread()
{
	UINT NumOfMaxClient = m_uiNumOfMaxClient;
	UINT ModeChangeStartPoint = 0;

	WORD NumOfGameProgress = m_wNumOfGameProgress;
	WORD LoopGameHandlingPacket = m_wLoopGameHandlingPacket;
	WORD GameThreadSleepTime = m_wGameThreadSleepTime;

	HANDLE &GameThreadEvent = m_hThreadExitEvent[en_EVENT_NUMBER::EVENT_GAME];

	CSession **SessionPointerArray = m_ppSessionPointerArray;
	
	WORD NumOfModeChangeCycle;

	while (1)
	{
		// Stop �Լ��� �̺�Ʈ�� Set ���� �ʴ� �̻� ����ؼ� GameThreadSleepTime �� �ֱ�� �ݺ���
		if (WaitForSingleObject(GameThreadEvent, GameThreadSleepTime) == WAIT_TIMEOUT)
		{
			InterlockedIncrement(&GameThreadLoop);
			NumOfModeChangeCycle = 0;

			for (UINT i = ModeChangeStartPoint; i < NumOfMaxClient; ++i)
			{
				CSession &NowSession = *SessionPointerArray[i];
				// Auth ���� Game ���� �Ѿ������ �����̶��
				if (NowSession.m_byNowMode == CSession::en_SESSION_MODE::MODE_AUTH_TO_GAME)
				{
					Begin("AuthToGame");
					// ��带 Game ���� ������
					NowSession.m_byNowMode = CSession::en_SESSION_MODE::MODE_GAME;
					NowSession.OnGame_ClientJoin();

					++NumOfModeChangeCycle;
					ModeChangeStartPoint = i;

					InterlockedIncrement(&NumOfGamePlayer);
					End("AuthToGame");

					// ������ ó�� Ƚ������ �ݺ������� ó���Ѱ� ũ�ų� ������ �ݺ����� Ż����
					if (NumOfModeChangeCycle >= NumOfGameProgress)
					{
						// �ݺ��� Ƚ���� Ŭ���̾�Ʈ �ִ�ġ��
						if (i == NumOfMaxClient - 1)
							// �ݺ��� ���۰��� 0���� ������
							ModeChangeStartPoint = 0;
						break;
					}
				}
			}

			// ������ ó��Ƚ���� ó�� Ƚ������ ũ�� ó�� Ƚ���� 0���� ������
			if (NumOfModeChangeCycle < NumOfGameProgress)
				ModeChangeStartPoint = 0;

			for (UINT i = 0; i < NumOfMaxClient; ++i)
			{
				CSession &NowSession = *SessionPointerArray[i];

				if (NowSession.m_byNowMode != CSession::en_SESSION_MODE::MODE_GAME)
					continue;

				CNetServerSerializationBuf *RecvPacket;
				// ������ ó�� Ƚ����ŭ�� ó����
				for (int NumOfDequeue = 0; NumOfDequeue < LoopGameHandlingPacket; ++NumOfDequeue)
				{
					if (NowSession.m_RecvCompleteQueue.Dequeue(&RecvPacket))
					{
						Begin("GameRecv");
						NowSession.OnGame_Packet(RecvPacket);
						CNetServerSerializationBuf::Free(RecvPacket);
						End("GameRecv");
					}
					// ���̻� �Ϸ�� ��Ŷ�� ���ٸ� �ݺ����� Ż����
					else
						break;
				}
			}

			OnGame_Update();

			//Begin("GameRelease");
			for (UINT i = 0; i < NumOfMaxClient; ++i)
			{
				CSession &NowSession = *SessionPointerArray[i];
				if (NowSession.m_bLogOutFlag)
				{
					// Game �����̰� Send ���� �ƴ϶�� 
					if (NowSession.m_byNowMode == CSession::en_SESSION_MODE::MODE_GAME && NowSession.m_bSendIOMode == NONSENDING)
					{
						// �α׾ƿ� ��� ���·� ������
						NowSession.m_byNowMode = CSession::en_SESSION_MODE::MODE_WAIT_LOGOUT;
						NowSession.OnAuth_ClientLeave(true);
						InterlockedDecrement(&NumOfGamePlayer);
					}
				}
			}

			/*
			//	if (NowSession.m_byNowMode == CSession::en_SESSION_MODE::MODE_WAIT_LOGOUT)
			//	{
			//		int SendBufferRestSize = NowSession.m_SendIOData.uiBufferCount;
			//		int Rest = NowSession.m_SendIOData.SendQ.GetRestSize();
			//		// Send ������ ���� �Ű������� ������ ���� ����ȭ ���۵��� ��ȯ��
			//		for (int SendRestCnt = 0; SendRestCnt < SendBufferRestSize; ++SendRestCnt)
			//			CNetServerSerializationBuf::Free(NowSession.pSeirializeBufStore[SendRestCnt]);

			//		NowSession.m_SendIOData.uiBufferCount = 0;

			//		// ť���� ���� �������� ���� ����ȭ ���۰� �ִٸ� �ش� ����ȭ ���۵��� ��ȯ��
			//		if (Rest > 0)
			//		{
			//			CNetServerSerializationBuf *DeleteBuf;
			//			for (int SendQRestSize = 0; SendQRestSize < Rest; ++SendQRestSize)
			//			{
			//				NowSession.m_SendIOData.SendQ.Dequeue(&DeleteBuf);
			//				CNetServerSerializationBuf::Free(DeleteBuf);
			//			}
			//		}

			//		// ���� �Ϸ�ť���� �������� ���� ����ȭ ���۰� �ִٸ� �ش� ����ȭ ���۵��� ��ȯ��
			//		CNetServerSerializationBuf *DeleteBuf;
			//		while (1)
			//		{
			//			if (!NowSession.m_RecvCompleteQueue.Dequeue(&DeleteBuf))
			//				break;
			//			CNetServerSerializationBuf::Free(DeleteBuf);
			//		}

			//		CSession::AcceptUserInfo *UserNetworkInfo = NowSession.m_pUserNetworkInfo;
			//		WORD SessionIndex = NowSession.m_wSessionIndex;
			//		closesocket(UserNetworkInfo->AcceptedSock);
			//		UserNetworkInfo->AcceptedSock = INVALID_SOCKET;

			//		NowSession.m_bAuthToGame = false;
			//		NowSession.m_byNowMode = CSession::en_SESSION_MODE::MODE_NONE;
			//		NowSession.m_pUserNetworkInfo = NULL;
			//		NowSession.m_Socket = INVALID_SOCKET;
			//		NowSession.m_RecvIOData.RecvRingBuffer.InitPointer();

			//		// �ش� UserNetworkInfo �� ���� �� �� �ֵ��� ���ÿ� ����
			//		m_SessionInfoStack.Push(SessionIndex);
			//		m_pUserInfoMemoryPool->Free(UserNetworkInfo);

			//		NowSession.OnGame_ClientRelease();

			//		InterlockedDecrement(&NumOfTotalPlayer);
			//	}
			//}
			//End("GameRelease");
			*/
		}
		else
			break;
	}

	return 0;
}

UINT CMMOServer::ReleaseThread()
{
	UINT NumOfMaxClient = m_uiNumOfMaxClient;

	WORD GameThreadSleepTime = m_wReleaseThreadSleepTime;

	HANDLE &ReleaseThreadEvent = m_hThreadExitEvent[en_EVENT_NUMBER::EVENT_RELEASE];

	CSession **SessionPointerArray = m_ppSessionPointerArray;

	CLockFreeStack<WORD> &SessionIndexStack = m_SessionInfoStack;

	while (1)
	{
		// Stop �Լ��� �̺�Ʈ�� Set ���� �ʴ� �̻� ����ؼ� ReleaseThreadSleepTime �� �ֱ�� �ݺ���
		if (WaitForSingleObject(ReleaseThreadEvent, GameThreadSleepTime) == WAIT_TIMEOUT)
		{
			for (UINT i = 0; i < NumOfMaxClient; ++i)
			{
				CSession &NowSession = *SessionPointerArray[i];

				if (NowSession.m_byNowMode == CSession::en_SESSION_MODE::MODE_WAIT_LOGOUT)
				{
					Begin("GameRelease");
					int SendBufferRestSize = NowSession.m_SendIOData.uiBufferCount;
					int Rest = NowSession.m_SendIOData.SendQ.GetRestSize();
					// Send ������ ���� �Ű������� ������ ���� ����ȭ ���۵��� ��ȯ��
					for (int SendRestCnt = 0; SendRestCnt < SendBufferRestSize; ++SendRestCnt)
						CNetServerSerializationBuf::Free(NowSession.pSeirializeBufStore[SendRestCnt]);

					NowSession.m_SendIOData.uiBufferCount = 0;

					// ť���� ���� �������� ���� ����ȭ ���۰� �ִٸ� �ش� ����ȭ ���۵��� ��ȯ��
					if (Rest > 0)
					{
						CNetServerSerializationBuf *DeleteBuf;
						for (int SendQRestSize = 0; SendQRestSize < Rest; ++SendQRestSize)
						{
							NowSession.m_SendIOData.SendQ.Dequeue(&DeleteBuf);
							CNetServerSerializationBuf::Free(DeleteBuf);
						}
					}

					// ���� �Ϸ�ť���� �������� ���� ����ȭ ���۰� �ִٸ� �ش� ����ȭ ���۵��� ��ȯ��
					CNetServerSerializationBuf *DeleteBuf;
					while (1)
					{
						if (!NowSession.m_RecvCompleteQueue.Dequeue(&DeleteBuf))
							break;
						CNetServerSerializationBuf::Free(DeleteBuf);
					}

					//CSession::AcceptUserInfo *UserNetworkInfo = NowSession.m_pUserNetworkInfo;
					closesocket(NowSession.m_UserNetworkInfo.AcceptedSock);
					//UserNetworkInfo->AcceptedSock = INVALID_SOCKET;

					NowSession.m_bAuthToGame = false;
					NowSession.m_byNowMode = CSession::en_SESSION_MODE::MODE_NONE;
					NowSession.m_UserNetworkInfo.AcceptedIP[0] = L'\0';
					NowSession.m_UserNetworkInfo.wAcceptedPort = NULL;
					NowSession.m_UserNetworkInfo.AcceptedSock = INVALID_SOCKET;
					NowSession.m_RecvIOData.RecvRingBuffer.InitPointer();

					// �ش� UserNetworkInfo �� ���� �� �� �ֵ��� ���ÿ� ����
					SessionIndexStack.Push(NowSession.m_wSessionIndex);
					//m_pUserInfoMemoryPool->Free(UserNetworkInfo);

					NowSession.OnGame_ClientRelease();

					InterlockedDecrement(&NumOfTotalPlayer);
					End("GameRelease");
				}
			}
		}
		else
			break;
	}

	return 0;
}

char CMMOServer::RecvPost(CSession *pSession)
{
	CSession &RecvSession = *pSession;
	// �����۰� �� á�ٸ� �������� �������� �������� ����
	if (RecvSession.m_RecvIOData.RecvRingBuffer.IsFull())
	{
		if (InterlockedDecrement(&RecvSession.m_uiIOCount) == 0)
		{
			RecvSession.m_bLogOutFlag = true;
			return POST_RETVAL_ERR_SESSION_DELETED;
		}
		return POST_RETVAL_ERR;
	}

	Begin("RecvPost");
	int BrokenSize = RecvSession.m_RecvIOData.RecvRingBuffer.GetNotBrokenPutSize();
	int RestSize = RecvSession.m_RecvIOData.RecvRingBuffer.GetFreeSize() - BrokenSize;
	int BufCount = 1;
	DWORD Flag = 0;

	WSABUF wsaBuf[2];
	wsaBuf[0].buf = RecvSession.m_RecvIOData.RecvRingBuffer.GetWriteBufferPtr();
	wsaBuf[0].len = BrokenSize;
	if (RestSize > 0)
	{
		wsaBuf[1].buf = RecvSession.m_RecvIOData.RecvRingBuffer.GetBufferPtr();
		wsaBuf[1].len = RestSize;
		BufCount++;
	}

	InterlockedIncrement(&RecvSession.m_uiIOCount);
	ZeroMemory(&RecvSession.m_RecvIOData.RecvOverlapped, sizeof(OVERLAPPED));
	int retval = WSARecv(RecvSession.m_UserNetworkInfo.AcceptedSock, wsaBuf, BufCount, NULL, &Flag, &RecvSession.m_RecvIOData.RecvOverlapped, 0);
	if (retval == SOCKET_ERROR)
	{
		int GetLastErr = WSAGetLastError();
		if (GetLastErr != ERROR_IO_PENDING)
		{
			if (InterlockedDecrement(&RecvSession.m_uiIOCount) == 0)
			{
				RecvSession.m_bLogOutFlag = true;
				End("RecvPost");
				return POST_RETVAL_ERR_SESSION_DELETED;
			}
			st_Error Error;
			Error.GetLastErr = GetLastErr;
			Error.ServerErr = SERVER_ERR::WSARECV_ERR;
			OnError(&Error);
			End("RecvPost");
			return POST_RETVAL_ERR;
		}
	}
	End("RecvPost");
	return POST_RETVAL_COMPLETE;
}

UINT CMMOServer::AcceptThreadStartAddress(LPVOID CMMOServerPointer)
{
	return ((CMMOServer*)CMMOServerPointer)->AcceptThread();
}

UINT CMMOServer::WorkerThreadStartAddress(LPVOID CMMOServerPointer)
{
	return ((CMMOServer*)CMMOServerPointer)->WorkerThread();
}

UINT CMMOServer::SendThreadStartAddress(LPVOID CMMOServerPointer)
{
	return ((CMMOServer*)CMMOServerPointer)->SendThread();
}

UINT CMMOServer::AuthThreadStartAddress(LPVOID CMMOServerPointer)
{
	return ((CMMOServer*)CMMOServerPointer)->AuthThread();
}

UINT CMMOServer::GameThreadStartAddress(LPVOID CMMOServerPointer)
{
	return ((CMMOServer*)CMMOServerPointer)->GameThread();
}

UINT CMMOServer::ReleaseThreadStartAddress(LPVOID CMMOServerPointer)
{
	return ((CMMOServer*)CMMOServerPointer)->ReleaseThread();
}

bool CMMOServer::OptionParsing(const WCHAR *szMMOServerOptionFileName)
{
	_wsetlocale(LC_ALL, L"Korean");

	CParser parser;
	WCHAR cBuffer[BUFFER_MAX];

	FILE *fp;
	_wfopen_s(&fp, szMMOServerOptionFileName, L"rt, ccs=UNICODE");

	int iJumpBOM = ftell(fp);
	fseek(fp, 0, SEEK_END);
	int iFileSize = ftell(fp);
	fseek(fp, iJumpBOM, SEEK_SET);
	int FileSize = (int)fread_s(cBuffer, BUFFER_MAX, sizeof(WCHAR), BUFFER_MAX / 2, fp);
	int iAmend = iFileSize - FileSize; // ���� ���ڿ� ���� ����� ���� ������
	fclose(fp);

	cBuffer[iFileSize - iAmend] = '\0';
	WCHAR *pBuff = cBuffer;

	BYTE HeaderCode, XORCode1, XORCode2, DebugLevel;

	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"SEND_SLEEPTIME", (short*)&m_wSendThreadSleepTime))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"AUTH_SLEEPTIME", (short*)&m_wAuthThreadSleepTime))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"GAME_SLEEPTIME", (short*)&m_wGameThreadSleepTime))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"RELEASE_SLEEPTIME", (short*)&m_wReleaseThreadSleepTime))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"SEND_NUMOFPROGRESS", (short*)&m_wNumOfSendProgress))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"AUTH_NUMOFPROGRESS", (short*)&m_wNumOfAuthProgress))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"GAME_NUMOFPROGRESS", (short*)&m_wNumOfGameProgress))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"AUTH_NUMOFPACKET", (short*)&m_wLoopAuthHandlingPacket))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"GAME_NUMOFPACKET", (short*)&m_wLoopGameHandlingPacket))
		return false;

	if (!parser.GetValue_String(pBuff, L"MMOSERVER", L"BIND_IP", m_IP))
		return false;
	if (!parser.GetValue_Short(pBuff, L"MMOSERVER", L"BIND_PORT", (short*)&m_wPort))
		return false;
	if (!parser.GetValue_Byte(pBuff, L"MMOSERVER", L"WORKER_THREAD", &m_byNumOfWorkerThread))
		return false;
	if (!parser.GetValue_Byte(pBuff, L"MMOSERVER", L"USE_IOCPWORKER", &m_byNumOfUsingWorkerThread))
		return false;
	if (!parser.GetValue_Byte(pBuff, L"MMOSERVER", L"NAGLE_ON", (BYTE*)&m_bIsNagleOn))
		return false;

	if (!parser.GetValue_Byte(pBuff, L"SERIALIZEBUF", L"PACKET_CODE", &HeaderCode))
		return false;
	if (!parser.GetValue_Byte(pBuff, L"SERIALIZEBUF", L"PACKET_KEY1", &XORCode1))
		return false;
	if (!parser.GetValue_Byte(pBuff, L"SERIALIZEBUF", L"PACKET_KEY2", &XORCode2))
		return false;

	if (!parser.GetValue_Byte(pBuff, L"OPTION", L"LOG_LEVEL", &DebugLevel))
		return false;

	CNetServerSerializationBuf::m_byHeaderCode = HeaderCode;
	CNetServerSerializationBuf::m_byXORCode = XORCode1 ^ XORCode2;
	SetLogLevel(DebugLevel);

	return true;
}

UINT CMMOServer::GetAcceptTPSAndReset()
{
	UINT NowAcceptTPS = AcceptTPS;
	InterlockedExchange(&AcceptTPS, 0);

	return NowAcceptTPS;
}

UINT CMMOServer::GetRecvTPSAndReset()
{
	UINT NowRecvTPS = RecvTPS;
	InterlockedExchange(&RecvTPS, 0);

	return NowRecvTPS;
}

UINT CMMOServer::GetSendTPSAndReset()
{
	UINT NowSendTPS = SendTPS;
	InterlockedExchange(&SendTPS, 0);

	return NowSendTPS;
}

UINT CMMOServer::GetSendThreadFPSAndReset()
{
	UINT NowSendFPS = SendThreadLoop;
	InterlockedExchange(&SendThreadLoop, 0);

	return NowSendFPS;
}

UINT CMMOServer::GetAuthThreadFPSAndReset()
{
	UINT NowAuthFPS = AuthThreadLoop;
	InterlockedExchange(&AuthThreadLoop, 0);

	return NowAuthFPS;
}

UINT CMMOServer::GetGameThreadFPSAndReset()
{
	UINT NowGameFPS = GameThreadLoop;
	InterlockedExchange(&GameThreadLoop, 0);

	return NowGameFPS;
}

int CMMOServer::GetRecvCompleteQueueTotalNodeCount()
{
	return CListBaseQueue<CNetServerSerializationBuf*>::GetMemoryPoolTotalNodeCount();
}