#include <iostream>
#include <cstdlib>
#include <winsock2.h>

#define BUF_SIZE 1024
#define OPSZ        4

void ErrorHandling(char* message);

int calculate(int opnum, int opnds[], char oprator);

int main()
{
	WSADATA wsaData;

	SOCKET server_socket, client_socket;

	SOCKADDR_IN server_addr, client_addr;

	int client_addr_size;

	char* input_port = "7711";

	char opinfo[BUF_SIZE];

	int result, opndCount;
	
	int sendCount, sendLen;


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
		opndCount = 0;

		client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &client_addr_size);
		std::cout << "---" << std::endl;

		recv(client_socket, (char*)&opndCount, 1, 0);

		std::cout << opndCount << std::endl;

		sendLen = 0;


		while ((opndCount * OPSZ + 1) > sendLen)
		{
			sendCount = recv(client_socket, &opinfo[sendLen], BUF_SIZE - 1, 0);
			sendLen += sendCount;
		}

		std::cout << opinfo[sendLen - 1] << std::endl;
		result = calculate(opndCount, (int*)opinfo, opinfo[sendLen - 1]);

		send(client_socket, (char*)&result, sizeof(result), 0);

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

int calculate(int opnum, int opnds[], char oprator)
{
	int result = opnds[0];
	int i;

	switch (oprator)
	{
	case '+':
		for (i = 1; i < opnum; ++i) result += opnds[i];
		break;
	case '-':
		for (i = 1; i < opnum; ++i) result -= opnds[i];
		break;
	case '*':
		for (i = 1; i < opnum; ++i) result *= opnds[i];
		break;
	}

	return result;
}