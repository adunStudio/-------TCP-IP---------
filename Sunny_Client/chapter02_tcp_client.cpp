#include <iostream>
#include <cstdlib>
#include <winsock2.h>

void ErrorHandling(char* message);

int main()
{
	char* input_port = "7711";
	char* input_ip = "127.0.0.1";

	WSADATA wsaData;

	SOCKET client_socket;

	SOCKADDR_IN server_addr;

	char message[30];
	int  message_length = 0;
	int  idx = 0;
	int  read_length = 0;

	/**
	���� ���α׷����� �� ������ �ݵ�� WSAStartup �Լ��� ȣ���ؼ�, ���α׷����� �䱸�ϴ� ���ҿ� ������ ������ �˸���,
	�ش� ������ �����ϴ� ���̺귯���� �ʱ�ȭ �۾��� �����ؾ� �Ѵ�.
	**/
	// ���� �� 0, ���� �� 0�� �ƴ� �����ڵ� �� ��ȯ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// ���� ����
	// ���� �� ���� �ڵ�, ���� �� INVALID_SOCKET ��ȯ
	// int af, int type, int protocol
	// af       : ������ ����� �������� ü��(Protocol Family) ����
	// type     : ������ ������ ���۹�Ŀ� ���� ����
	// protocol : �� ��ǻ�Ͱ� ��ſ� ���Ǵ� �������� ����
	client_socket = socket(PF_INET, SOCK_STREAM, 0);  // PF_INET: IPv4 ���ͳ� �������� ü��, SOCK_STREAM: ���������� ������ ������ �ۼ��� ���
	// int tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	// int udp_socket = socket(PF_INET, SOCK_DGRAM,  IPPROTO_UDP);
	if (client_socket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(input_ip);
	server_addr.sin_port = htons(atoi(input_port));

	// inet_addr : ���ڿ� ������ ��Ʈ��ũ ����Ʈ ������ ������ ��ȯ
	// ���� �� �� ��������� ��ȯ�� 32��Ʈ ���� ��, ���� �� INADDR_NONE ��ȯ

	// Ŭ���̾�Ʈ ���α׾ڿ��� ������ ������� �����û
	// ���� �� 0, ���� �� SOCKET_ERROR ��ȯ
	// SOCKET s, const struct sockaddr* name, int namelen
	if (connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	// Read
	// ���� �� ������ ����Ʈ ��(�� EOF ���۽� 0), ���� �� SOCKET_ERROR ��ȯ
	// SOCKET s        : ������ ���� ������ ������ �ǹ��ϴ� ������ �ڵ� ��
	// const char* buf : ���ŵ� �����͸� ������ ������ �ּ� ��
	// int len         : ������ �� �ִ� �ִ� ����Ʈ ��
	// int flags       : ������ ���� �� ������ �پ��� �ɼ� ����
	while (read_length = recv(client_socket, &message[idx++], 1, 0))
	{
		if(read_length == -1)
			ErrorHandling("read() error");

		message_length += read_length;
	}

	printf("Message from server: %s,\n", message);
	printf("Function read call count: %d \n", message_length);

	// ���� �ݱ�
	// ���� �� 0, ���� �� SOCKET_ERROR ��ȯ
	closesocket(client_socket);

	// WSAStartup���� �ʱ�ȭ�� ���� ���̺귯�� ����
	WSACleanup();

	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}