// Overlapped IO에서 입출력의 완료 및 결과를 확인하는 방법에는 두 가지가 있다.
// 1. WSASend, WSARecv 함수의 여섯 번째 매개변수 활용 방법, Event 오브젝트 기반
// 2. WSASend, WSARecv 함수의 일곱 번째 매개변수 활용 방법, Completion Routine 기반

// Completion Routine 사용하기
// "Pending된 IO가 완료되면, 이 함수를 호출해 달라!"
// "IO를 요청한 스레드가 alertable wai 상태에 놓여잇을 때만 Completion Routin을 호출할게!"

// 다음 함수가 호출된 상황에서 쓰레드는 alertable wait 상태가 된다.
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

	// main 쓰레드를 alertable wait 상태로 두기 위한 함수 호출이다.
	// 그런데 이 함수의 호출을 위해서 불필요한 Event 오브젝트를 하나 생성하였다. (더미 오브젝트)
	// 만약 SleepEx 함수를 활용한다면 이러한 더미 오브젝트의 생성을 피할 수 있따.
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