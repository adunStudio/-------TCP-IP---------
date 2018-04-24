// 위도우에서는 다음의 함수호출을 통해서 넌-블로킹 모드로 소켓의 속성을 변경한다.
/*
SOCKET hLisnSOck,
int mode = 1;
....
hLishSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
ioctlsocket(hLisnSock, FIONBIO, &mode); // for non-blocking socket
....
*/

// "핸들 hLisnSock이 참조하는 소켓의 입출력 모드(FIONBIO)를 변수 mode에 저장된 값의 형태로 변경한다."

// 즉, FIONBIO는 소켓의 입출력 모드를 변경하는 옵션이며, 이 함수의 세 번째 인자로 전달된 주소 값의 변수에 0이 저장되어 잇으면 블로킹 모드로, 0이 아닌 값이 저장되어 있으면 넌-블로킹 모드로 소켓의 입출력 속성을 변경한다.
// - 클라이언트의 연결요청이 존재하지 않는 상태에서 accept 함수가 호출되면 INVALID_SOCKET이 곧바로 반환된다. 그리고 이어서 WSAGetLastError 함수를 호출하면 WSAEWOULDBLOCK가 반환된다.
// - accept 함수호출을 통해서 새로 생성되는 소켓 역시 넌-블로킹 속성을 지닌다.

// 따라서 넌-블로킹 입출력 소켓을 대상으로 accept 함수를 호출해서 INVALID_SOCKET이 반환되면, WSAGetLastError 함수의 호출을 통해서 INVALID_SOCKET이 반환된 이유를 확인하고, 그에 적절한 처리를 해야만 한다.

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

// "Overlapped IO만 가지고 에코 서버 구현하기"

#define BUF_SIZE 1024

void CALLBACK ReadCompRoutine (DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

void ErrorHandling(char* message);

typedef struct
{
	SOCKET hClntSock;         // 소캣의 핸들
	char buf[BUF_SIZE];       // 버퍼
	WSABUF wsaBuf;            // 버퍼관련 정보를 담는 WSABUF형 변수 (len, buf) 
} PER_IO_DATA, * LPPER_IO_DATA;

int main()
{
	char* input_port = "7711";

	WSADATA wsaData;

	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAdr, recvAdr;

	LPWSAOVERLAPPED lpOvLp;

	DWORD recvBytes;

	LPPER_IO_DATA hbInfo;

	unsigned long mode = 1;

	int recvAdrSz;

	unsigned long flagInfo = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	ioctlsocket(hLisnSock, FIONBIO, &mode); // 소켓을 넌-블로킹 모드로 변경한다.

	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family      = AF_INET;
	lisnAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	lisnAdr.sin_port        = htons(atoi(input_port));

	if (bind(hLisnSock, (SOCKADDR*)&lisnAdr, sizeof(lisnAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");


	recvAdrSz = sizeof(recvAdr);

	while (true)
	{
		// Completion Routine의 실행을 위해서..
		SleepEx(100, TRUE);  // for alertable wait state

		// while 반복문 안에서 accept 함수를 호출하고 있다. 특히 호출의 대상이 되는 소켓이 넌-블록킹 모드이므로, INVALID_SOCKET이 반환될 때의 처리과정에 주목하자.
		hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);

		if(hRecvSock == INVALID_SOCKET)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			else
				ErrorHandling("accept() error");
		}
		puts("Client connected.....");

		
		/* Overlapped IO에 필요한 구조체 변수를 할당하고, 이를 초기화한다. */

		// 반복문 안에서 WSAOVERLAPPED 구조체를 할당하는 이유는 클라이언트 하나당 WSAOVERLAPPED 구조체 변수를 하나씩 할당해야 하기 때문이다.
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		hbInfo->hClntSock = (DWORD)hRecvSock;
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// WSDAOVERLAPPED 구조체의 변수의 멤버 hEvent에 할당한 변수의 주소 값을 저장한다.
		// ㄴCompletion Routine 기반의 Overlapped IO에서는 Event오브젝트가 불필요하기 때문에 hEvent에 필요한 다른 정보를 채워도 된다.
		lpOvLp->hEvent = (HANDLE)hbInfo;


		// 여기서 여섯 번째 인자로 전달한 WSAOVERLAPPED 구조체 변수의 주소 값은 Completion Routine의 세 번째 매개변수에 전달된다.
		// 때문에 CompletionRoutine 함수 내에서는 입출력 완료된 소켓의 핸들과 버퍼에 접근할 수 있다.
		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOvLp, ReadCompRoutine);
	}

	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();

	return 1;
}

void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD sentBytes;

	if (szRecvBytes == 0)
	{
		closesocket(hSock);
		free(lpOverlapped->hEvent);
		free(lpOverlapped);

		puts("Client disconnected....");
	}
	else     // Echo
	{
		bufInfo->len = szRecvBytes;
		WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
	}
}

void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock = hbInfo->hClntSock;
	LPWSABUF bufInfo = &(hbInfo->wsaBuf);
	DWORD recvBytes;
	unsigned long flagInfo = 0;

	WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}