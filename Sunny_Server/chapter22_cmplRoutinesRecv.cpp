// Overlapped IO���� ������� �Ϸ� �� ����� Ȯ���ϴ� ������� �� ������ �ִ�.
// 1. WSASend, WSARecv �Լ��� ���� ��° �Ű����� Ȱ�� ���, Event ������Ʈ ���
// 2. WSASend, WSARecv �Լ��� �ϰ� ��° �Ű����� Ȱ�� ���, Completion Routine ���

// Completion Routine ����ϱ�
// "Pending�� IO�� �Ϸ�Ǹ�, �� �Լ��� ȣ���� �޶�!"
// "IO�� ��û�� �����尡 alertable wai ���¿� �������� ���� Completion Routin�� ȣ���Ұ�!"

// ���� �Լ��� ȣ��� ��Ȳ���� ������� alertable wait ���°� �ȴ�.
// - WaitForSingleObjectEx
// - WaitForMultipleObjectsEx
// - WSAWaitForMultipleEvents
// - SleepEx

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_SIZE 1024

void CALLBACK CompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char* message);

WSABUF dataBuf;
char buf[BUF_SIZE];
unsigned long recvBytes = 0;


int main()
{
	char* input_port = "7711";

	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAdr, recvAdr;
	int recvAdrSz;

	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	int idx = 0;
	unsigned long flags = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family = AF_INET;
	lisnAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAdr.sin_port = htons(atoi(input_port));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAdr, sizeof(lisnAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAdrSz = sizeof(recvAdr);
	hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);

	evObj = WSACreateEvent();
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj;
	dataBuf.len = BUF_SIZE;
	dataBuf.buf = buf;

	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, CompRoutine) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			puts("Background data receive");
			;
		}
		else
			ErrorHandling("WSARecv() error");
	}

	// main �����带 alertable wait ���·� �α� ���� �Լ� ȣ���̴�.
	// �׷��� �� �Լ��� ȣ���� ���ؼ� ���ʿ��� Event ������Ʈ�� �ϳ� �����Ͽ���. (���� ������Ʈ)
	// ���� SleepEx �Լ��� Ȱ���Ѵٸ� �̷��� ���� ������Ʈ�� ������ ���� �� �ֵ�.
	idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE); 

	if (idx == WAIT_IO_COMPLETION)
		puts("Overlapped I/O Completed");
	else
		ErrorHandling("WSARecv() error");

	WSACloseEvent(evObj);
	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();

	return 1;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void CALLBACK CompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	if (dwError != 0)
		ErrorHandling("CompRoutine error");
	

	recvBytes = szRecvBytes;
	printf("Received message: %s \n", buf);
}