#include <iostream>
#include <cstdlib>
#include <winsock2.h>

#define BUF_SIZE 1024

void ErrorHandling(char* message);

int main()
{
	WSADATA wsaData;

	SOCKET server_socket, client_socket;

	SOCKADDR_IN server_addr, client_addr;

	char* input_port = "7711";

	char message[BUF_SIZE];

	int strLen;

	int client_addr_size;

	/**
	���� ���α׷����� �� ������ �ݵ�� WSAStartup �Լ��� ȣ���ؼ�, ���α׷����� �䱸�ϴ� ���ҿ� ������ ������ �˸���,
	�ش� ������ �����ϴ� ���̺귯���� �ʱ�ȭ �۾��� �����ؾ� �Ѵ�.
	**/
	// ���� �� 0, ���� �� 0�� �ƴ� �����ڵ� �� ��ȯ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// 1�ܰ�. ���� ����
	// ���� �� ���� �ڵ�, ���� �� INVALID_SOCKET ��ȯ
	// int af, int type, int protocol
	server_socket = socket(PF_INET, SOCK_STREAM, 0);  // PF_INET: IPv4 ���ͳ� �������� ü��, SOCK_STREAM: ���������� ������ ������ �ۼ��� ���
	if (server_socket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(input_port));

	// 2�ܰ�. IP�ּҿ� PORT��ȣ �Ҵ�
	// ������ 0, ���� �� SOCKET_ERROR ��ȯ
	// SOCKET s, const struct sockaddr* name, int namelen
	if (bind(server_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	// 3�ܰ�. �����û ���� ���·� ����
	// ������ 0, ���� �� SOCKET_ERROR ��ȯ
	// SOCKET s, int backlog
	if (listen(server_socket, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	client_addr_size = sizeof(client_addr);

	for (int i = 0; i < 5; ++i)
	{
		// 4�ܰ�. �����û�� ���� ����)
		// ���� �� ���� �ڵ�, ���� �� INVALID_SOCKET ��ȯ
		// SOCKET s, struct sockaddr* addr, int* addrlen
		client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &client_addr_size);
		
		if (client_socket == INVALID_SOCKET)
			ErrorHandling("accept() error");
		else
			std::cout << "Connected client " << i + 1 << std::endl;

		// Read
		// ���� �� ������ ����Ʈ ��(�� EOF ���۽� 0), ���� �� SOCKET_ERROR ��ȯ
		// SOCKET s        : ������ ���� ������ ������ �ǹ��ϴ� ������ �ڵ� ��
		// const char* buf : ���ŵ� �����͸� ������ ������ �ּ� ��
		// int len         : ������ �� �ִ� �ִ� ����Ʈ ��
		// int flags       : ������ ���� �� ������ �پ��� �ɼ� ����
		while ((strLen = recv(client_socket, message, BUF_SIZE, 0)) != 0)
		{
			// Write
			// ���� �� ���۵� ����Ʈ ��, ���� �� SOCKET_ERROR ��ȯ
			// SOCKET s        : ������ ���� ������ ������ �ǹ��ϴ� ������ �ڵ� ��
			// const char* buf : ������ �����͸� �����ϰ� �ִ� ������ �ּ� ��
			// int len         : ������ ����Ʈ ��
			// inet flags      : ������ ���� �� ������ �پ��� �ɼ� ����
			//std::cout << message << std::endl;
			
			send(client_socket, message, strLen, 0);
		}

		closesocket(client_socket);
	}

	std::cout << "sdfsdfsd" << std::endl;
	// ���� �ݱ�
	// ���� �� 0, ���� �� SOCKET_ERROR ��ȯ
	closesocket(server_socket);

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