#include "PreComfile.h"
#include "CToHTTP.h"


CCToHTTP::CCToHTTP(int SockTimeout, const WCHAR* ConnectIP, WORD RecvSize)
{
	m_iSockTimeout = SockTimeout;
	m_wRecvSize = RecvSize;
	wcscpy_s(m_IP, ConnectIP);
}

CCToHTTP::~CCToHTTP()
{
	closesocket(m_HTTPSock);
}

bool CCToHTTP::InitSocket()
{
	m_HTTPSock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_HTTPSock == INVALID_SOCKET)
	{
		printf_s("Socket Err %d\n", WSAGetLastError());
		return false;
	}

	int retval = setsockopt(m_HTTPSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&m_iSockTimeout, sizeof(m_iSockTimeout));
	if (retval == SOCKET_ERROR)
	{
		printf_s("Recv SockOpr Err %d\n", WSAGetLastError());
		return false;
	}
	retval = setsockopt(m_HTTPSock, SOL_SOCKET, SO_SNDTIMEO, (char*)&m_iSockTimeout, sizeof(m_iSockTimeout));
	if (retval == SOCKET_ERROR)
	{
		printf_s("Send SockOpr Err %d\n", WSAGetLastError());
		return false;
	}


	return true;
}

USHORT CCToHTTP::CreateHTTPData(char* Dest, const WCHAR* PathWithIP, const char* Body)
{
	/*
		POST http:// m_IP / Path~~~ HTTP/1.1\r\n
		User-Agent: ~~~~\r\n
		Host: m_IP\r\n
		Connection: Close\r\n
		Content-Length: ContentLegnth\r\n
		\r\n
		1\r\n
		1\r\n
		0\r\n
		{ Body }
	*/
	
	const char* HTTPComponent[3] = 
	{ " HTTP/1.1\r\nHost: ",
	  "\r\nConnection: Close\r\nContent-Length: ",
	  "\r\n\r\n1\r\n1\r\n0\r\n"
	};
	
	char Path[HTTP_PATH_MAX];
	int PathSize = lstrlenW(PathWithIP);
	WideCharToMultiByte(CP_UTF8, 0, PathWithIP, PathSize, Path, PathSize, NULL, NULL);
	Path[PathSize] = '\0';
	char IP[16];
	WideCharToMultiByte(CP_UTF8, 0, m_IP, 16, IP, 16, NULL, NULL);
	char ContentLengthArr[5];
	WORD ContentLength = (WORD)strlen(Body);

	char HTTPData[HTTP_SIZE_MAX] = "POST ";
	strcat_s(HTTPData, Path);
	strcat_s(HTTPData, HTTPComponent[0]);
	strcat_s(HTTPData, IP);
	strcat_s(HTTPData, HTTPComponent[1]);
	sprintf_s(ContentLengthArr, "%d", ContentLength);
	strcat_s(HTTPData, ContentLengthArr);
	strcat_s(HTTPData, HTTPComponent[2]);
	// Json �߰�ȣ
	// �ٵ𿡸� �����
	// ���Ŀ��� rapidJson�� �̿��ؾ� ������ ��
	strcat_s(HTTPData, "{");
	strcat_s(HTTPData, Body);
	strcat_s(HTTPData, "}");

	USHORT HTTPSize = (USHORT)strlen(HTTPData);

	strcpy_s(Dest, HTTP_SIZE_MAX, HTTPData);
	return HTTPSize;
}

bool CCToHTTP::ConnectToHTTP()
{
	// ����Ʈ�� ��� ����ϰ� ������ �ȵǹǷ�
	// HTTP ������ �ͺ�� �������� ����
	u_long on = 1;
	int retval = ioctlsocket(m_HTTPSock, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
	{
		printf_s("On ioctlsocket Err %d\n", WSAGetLastError());
		return false;
	}

	SOCKADDR_IN serveraddr;
	IN_ADDR Addr;
	InetPton(AF_INET, m_IP, &Addr);

	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr = Addr;
	serveraddr.sin_port = htons(HTTP_PORT);

	retval = connect(m_HTTPSock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		printf_s("Connect Err %d\n", WSAGetLastError());
		return false;
	}

	FD_SET wset;
	// ����Ʈ Ÿ�Ӿƿ� ����
	timeval t;
	t.tv_sec = 0;
	t.tv_usec = 100;
	FD_ZERO(&wset);
	FD_SET(m_HTTPSock, &wset);

	// select �Լ��� ���� �����ð� ��ٸ���
	// �ð��� �ʰ��ϸ� Ÿ�Ӿƿ���Ŵ
	retval = select(0, NULL, &wset, NULL, &t);
	if (retval == SOCKET_ERROR)
	{
		printf_s("Select Err%d\n", WSAGetLastError());
		return false;
	}
	else if (!FD_ISSET(m_HTTPSock, &wset))
	{
		printf_s("Connect timeout");
		return false;
	}

	// �ٽ� HTTP ������ ��� �������� ����
	on = 0;
	retval = ioctlsocket(m_HTTPSock, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
	{
		printf_s("Off ioctlsocket Err %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

short CCToHTTP::GetHTTPBody(char* BodyDest, const char* InputHTTP, WORD InputHTTPSize)
{
	int retval = send(m_HTTPSock, (char*)InputHTTP, InputHTTPSize, 0);
	if (retval == 0)
	{
		printf_s("Send Err %d\n", WSAGetLastError());
		return -1;
	}

	// ó�� ' ' ~ ���� 0x20 ������ �Ϸ��ڵ�
	char recvData[HTTP_SIZE_MAX];
	retval = recv(m_HTTPSock, (char*)recvData, m_wRecvSize, 0);
	char* CompleteCodePtr = strchr(recvData, ' ');
	// ���� ���ڸ� �پ�ѱ� ���� ������ �̵�
	CompleteCodePtr += 1;
	int CompleteCode = atoi(CompleteCodePtr);

	// Content-Length ~ ���� 0x0d ������ Body�� ������
	char* ContentLengthPtr = strstr((char*)recvData, "Content-Length");
	// Content-Length �� �پ�ѱ� ���� ������ �̵�
	ContentLengthPtr += 16;
	int ContentLength = atoi(ContentLengthPtr);

	// \r\n\r\n ���ĺ��� Body
	char* EndOfHeader = strstr((char*)recvData, "\r\n\r\n");
	// \r\n\r\n �� �پ�ѱ� ���� ������ �̵�
	EndOfHeader += 4;
	strcpy_s(BodyDest, HTTP_SIZE_MAX, EndOfHeader);
	// ContentLength ��ŭ�� ����ϱ� ���Ͽ� �ι��ڸ� ����
	BodyDest[ContentLength] = '\0';

	return CompleteCode;
}

short CCToHTTP::GetHTTPBody(WCHAR* BodyDest, const char* InputHTTP, WORD InputHTTPSize)
{
	int retval = send(m_HTTPSock, (char*)InputHTTP, InputHTTPSize, 0);
	if (retval == 0)
	{
		printf_s("Send Err %d\n", WSAGetLastError());
		return -1;
	}

	// ó�� ' ' ~ ���� 0x20 ������ �Ϸ��ڵ�
	char recvData[HTTP_SIZE_MAX];
	retval = recv(m_HTTPSock, (char*)recvData, m_wRecvSize, 0);
	char* CompleteCodePtr = strchr(recvData, ' ');
	// ���� ���ڸ� �پ�ѱ� ���� ������ �̵�
	CompleteCodePtr += 1;
	int CompleteCode = atoi(CompleteCodePtr);

	// Content-Length ~ ���� 0x0d ������ Body�� ������
	char* ContentLengthPtr = strstr((char*)recvData, "Content-Length");
	// Content-Length �� �پ�ѱ� ���� ������ �̵�
	ContentLengthPtr += 16;
	int ContentLength = atoi(ContentLengthPtr);

	// \r\n\r\n ���ĺ��� Body
	char* EndOfHeader = strstr((char*)recvData, "\r\n\r\n");
	// \r\n\r\n �� �پ�ѱ� ���� ������ �̵�
	EndOfHeader += 4;
	char cBody[HTTP_SIZE_MAX];
	strcpy_s(cBody, HTTP_SIZE_MAX, EndOfHeader);
	int cBodySize = strlen(cBody);
	MultiByteToWideChar(CP_ACP, 0, cBody, cBodySize, BodyDest, cBodySize * sizeof(WCHAR));
	// ContentLength ��ŭ�� ����ϱ� ���Ͽ� �ι��ڸ� ����
	BodyDest[cBodySize] = '\0';

	return CompleteCode;
}
