#pragma once

#define HTTP_PORT				80
#define HTTP_COMPLETECODE		200
#define HTTP_PATH_MAX			200
#define HTTP_SIZE_MAX			2048

//------------------------------------------
// �ش� Ŭ������ ������ ���� ������ ���ȴ�.
// ���� �� �ʱ�ȭ								->
// HTTP �������� ������ ����					->
// �������� Connect								->
// ������ ������� �����͸� send				->
// send �� �ٷ� recv							->
// ������ ���� �� HTTP ������� �Ϸ��ڵ� ���	->
// HTTP ������� Content-Length ���			->
// ��� �� ã��									->
// �Ϸ��ڵ�� BODY ������ ������				->
// �Ϸ��ڵ尡 200�� �ƴ϶�� ����				
//------------------------------------------
class CCToHTTP
{
private :
	int m_iSockTimeout;
	WORD m_wRecvSize;
	SOCKET m_HTTPSock;
	WCHAR m_IP[16];
public :
	// �� �����̹Ƿ� TimeOut�� ����
	// TimeOut ���� �̸� ������ ������ �Է�
	// 5~10 �� ����
	// RecvSize ���� ����� �ٵ� ���ļ� ���� �ִ� ũ��� ���� (Default 1024)
	CCToHTTP(int SockTimeout, const WCHAR* ConnectIP, WORD RecvSize = 1024);
	~CCToHTTP();

	// BodyDest�� HTTPBody�� �־���
	// ��ȯ������ HTTP �Ϸ��ڵ尡 ��ȯ��
	// �Լ� �߰��� ������ �߻��� -1�� ��ȯ��
	short GetHTTPBody(char* BodyDest, const char* InputHTTP, WORD InputHTTPSize);
	short GetHTTPBody(WCHAR* BodyDest, const char* InputHTTP, WORD InputHTTPSize);

	// ���ڷ� ���� IP�� ����Ͽ� ���� �ʱ�ȭ
	// Connect�� �ش� �Լ����� �������� ����
	bool InitSocket();
	// �ش� IP�� Connect
	bool ConnectToHTTP();

	// HTTP ���������� �����͸� ����
	// IP�� �����ڿ��� ��������� ���� ���� ���
	// Path : HTTP ȣ�� ���
	// ContentLength : �ش� Body�� ������
	// Body ���� �ȿ� ����ǥ ���� �������÷� ������� ��
	// ��ȯ������ HTTP�� ũ�Ⱑ ��ȯ��
	USHORT CreateHTTPData(char* Dest, const WCHAR* PathWithIP, const char* Body);
};