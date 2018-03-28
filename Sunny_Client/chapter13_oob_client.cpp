#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 30

void ErrorHandling(char* message);

int main()
{
	WSADATA wsaData;

	SOCKET server_socket;

	SOCKADDR_IN server_adr;

	char* input_port = "7711";
	char* input_ip = "127.0.0.1";

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	server_socket = socket(PF_INET, SOCK_STREAM, 0);

	memset(&server_adr, 0, sizeof(server_adr));
	server_adr.sin_family = AF_INET;
	server_adr.sin_addr.s_addr = inet_addr(input_ip);
	server_adr.sin_port = htons(atoi(input_port));

	if (connect(server_socket, (SOCKADDR*)&server_adr, sizeof(server_adr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	send(server_socket, "123", 3, 0);
	send(server_socket,   "4", 1, MSG_OOB);
	send(server_socket, "567", 3, 0);
	send(server_socket, "890", 3, MSG_OOB);

	closesocket(server_socket);

	WSACleanup();

	return 1;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}