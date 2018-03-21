#include <iostream>
#include <cstdlib>
#include <winsock2.h>

#define BUF_SIZE 1024

void ErrorHandling(char* message);

int main()
{
	WSADATA wsaData;

	SOCKET client_socket;

	SOCKADDR_IN server_addr;

	char* input_port = "7711";
	char* input_ip = "127.0.0.1";

	char message[BUF_SIZE];

	int strLen;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(input_ip);
	server_addr.sin_port = htons(atoi(input_port));

	if (connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");
	else
		std::cout << "Connected.........." << std::endl;

	while (true)
	{
		std::cout << "Input message(Q to quit): " << std::endl;
		std::cin >> message;

		if (!strcmp(message, "q") || !strcmp(message, "Q"))
			break;

		send(client_socket, message, strlen(message), 0);

		strLen = recv(client_socket, message, BUF_SIZE - 1, 0);
	
		message[strLen] = 0;

		std::cout << "Message from server: " << message << std::endl;
	}

	closesocket(client_socket);

	WSACleanup();

	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}