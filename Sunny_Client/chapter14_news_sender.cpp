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

	SOCKET send_socket;

	SOCKADDR_IN multi_adr;

	int timeLive = TTL;

	FILE* fp;

	char buf[BUF_SIZE];

	char* input_port = "7711";
	char* input_ip = "127.0.0.1";

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	send_socket = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&multi_adr, 0, sizeof(multi_adr));
	multi_adr.sin_family = AF_INET;
	multi_adr.sin_addr.s_addr = inet_addr(input_ip);
	multi_adr.sin_port = htons(atoi(input_port));

	setsockopt(send_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&timeLive, sizeof(timeLive));

	if ((fp = fopen("news.txt", "r")) == NULL)
		ErrorHandling("fopen() error");

	while (!feof(fp))
	{
		fgets(buf, BUF_SIZE, fp);
		sendto(send_socket, buf, strlen(buf), 0, (SOCKADDR*)&multi_adr, sizeof(multi_adr));
		Sleep(2000);
	}
	
	fclose(fp);

	closesocket(send_socket);

	WSACleanup();

	return 1;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}