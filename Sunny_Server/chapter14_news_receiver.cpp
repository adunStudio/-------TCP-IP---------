#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#define TTL 64
#define BUF_SIZE 30

void ErrorHandling(char* message);

int main()
{
	WSAData wsaData;

	SOCKET receive_socket;

	SOCKADDR_IN adr;

	ip_mreq joinAdr;

	char* input_port = "7711";
	char* input_ip = "127.0.0.1";

	char buf[BUF_SIZE];

	int strLen;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	receive_socket = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&adr, 0, sizeof(adr));
	adr.sin_family = AF_INET;
	adr.sin_addr.s_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(atoi(input_port));

	if (bind(receive_socket, (SOCKADDR*)&adr, sizeof(adr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	joinAdr.imr_multiaddr.s_addr = inet_addr(input_ip);
	joinAdr.imr_interface.s_addr = htonl(INADDR_ANY);

	/*if (*/setsockopt(receive_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&joinAdr, sizeof(joinAdr));/* == SOCKET_ERROR)*/
		//ErrorHandling("setsock() error");

	while (1)
	{
		strLen = recvfrom(receive_socket, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (strLen < 0)
			break;
		buf[strLen] = 0;
		fputs(buf, stdout);
	}

	closesocket(receive_socket);
	WSACleanup();

	return 0;


	return 1;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}