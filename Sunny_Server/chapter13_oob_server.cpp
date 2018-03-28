#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 30

void ErrorHandling(char* message);

int main()
{
	WSADATA wsaData;

	SOCKET server_socket, client_socket;

	SOCKADDR_IN server_adr, client_adr;

	char* input_port = "7711";

	int client_adr_size, strLen;

	char buf[BUF_SIZE];

	int result;

	fd_set read, except, readCopy, exceptCopy;

	struct timeval timeout;

	if (WSAStartup((2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	server_socket = socket(PF_INET, SOCK_STREAM, 0);

	memset(&server_adr, 0, sizeof(server_adr));
	server_adr.sin_family = AF_INET;
	server_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_adr.sin_port = htons(atoi(input_port));

	if (bind(server_socket, (SOCKADDR*)&server_adr, sizeof(server_adr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(server_socket, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	client_adr_size = sizeof(client_adr);

	client_socket = accept(server_socket, (SOCKADDR*)&client_adr, &client_adr_size);

	FD_ZERO(&read);
	FD_ZERO(&except);

	FD_SET(client_socket, &read);
	FD_SET(client_socket, &except);

	while (1)
	{
		readCopy = read;
		exceptCopy = except;

		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		result = select(0, &readCopy, 0, &exceptCopy, &timeout);

		if (result > 0)
		{
			if (FD_ISSET(client_socket, &exceptCopy))
			{
				strLen = recv(client_socket, buf, BUF_SIZE - 1, MSG_OOB);
				buf[strLen] = 0;
				printf("Urgent message: %s \n", buf);
			}

			if (FD_ISSET(client_socket, &readCopy))
			{
				strLen = recv(client_socket, buf, BUF_SIZE - 1, 0);

				if (strLen == 0)
				{
					break;
					closesocket(client_socket);
				}
				else
				{
					buf[strLen] = 0;
					puts(buf);
				}
			}
		}
		
	}

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