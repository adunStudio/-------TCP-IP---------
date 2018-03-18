#include <iostream>
#include <cstdlib>
#include <winsock2.h>

void ErrorHandling(char* message);

int main()
{
	char* input_port = "7711";
	char* input_ip   = "127.0.0.1";

	WSADATA wsaData;

	SOCKET server_socket, client_socket;
	
	SOCKADDR_IN server_addr, client_addr;

	int client_addr_size;

	char message[] = "Hello World!";

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
		ErrorHandling("socket() error");

	// 4�ܰ�. �����û�� ���� ����)
	// ���� �� ���� �ڵ�, ���� �� INVALID_SOCKET ��ȯ
	// SOCKET s, struct sockaddr* addr, int* addrlen
	client_addr_size = sizeof(client_addr);
	client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &client_addr_size);
	if (client_socket == INVALID_SOCKET)
		ErrorHandling("accept() error");

	
	// Write
	// ���� �� ���۵� ����Ʈ ��, ���� �� SOCKET_ERROR ��ȯ
	// SOCKET s        : ������ ���� ������ ������ �ǹ��ϴ� ������ �ڵ� ��
	// const char* buf : ������ �����͸� �����ϰ� �ִ� ������ �ּ� ��
	// int len         : ������ ����Ʈ ��
	// inet flags      : ������ ���� �� ������ �پ��� �ɼ� ����
	send(client_socket, message, sizeof(message), 0);


	// ���� �ݱ�
	// ���� �� 0, ���� �� SOCKET_ERROR ��ȯ
	closesocket(client_socket);
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