#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandlling(char* message);

int main()
{
	char* input_port = "7711";
	char* input_ip = "127.0.0.1";

	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN servAdr;
	
	char message[BUF_SIZE];
	int strLen, readLen;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandlling("WSAStartup() error!");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);

	if (hSocket == INVALID_SOCKET)
		ErrorHandlling("socket() error");

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family      = AF_INET;
	servAdr.sin_addr.s_addr = inet_addr(input_ip);
	servAdr.sin_port        = htons(atoi(input_port));

	if (connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandlling("connect() error!");
	else
		puts("Connected.......");

	while (true)
	{
		fputs("Input message(Q to quit): ", stdout);
		fgets(message, BUF_SIZE, stdin);
		if (!strcmp(message, "\qn") || !strcmp(message, ("Q\n"))) break;

		strLen = strlen(message);

		send(hSocket, message, strLen, 0);

		readLen = 0;

		while (true)
		{
			readLen += recv(hSocket, &message[readLen], BUF_SIZE - 1, 0);
		
			if (readLen >= strLen) break;
		}
		message[strLen] = 0;
		printf("Message from server: %s", message);
	}

	closesocket(hSocket);
	WSACleanup();

	return 1;
}

void ErrorHandlling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}