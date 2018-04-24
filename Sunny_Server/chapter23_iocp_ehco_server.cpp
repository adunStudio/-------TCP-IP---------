// # Completion Port
/*
IOCP에서는 완료된 IO의 정보가 Completion Port(이하 CP 오브젝트)라는 커널 오브젝트에 등록된다.
그런데 그냥 등록되는 것이 아니라 다음과 같은 요청의 과정이 선행되어야 한다.

"이 소켓을 기반으로 진행되는 IO의 완료 상황은 저 CP 오브젝트에 등록해 주세요."

이를 가리켜 '소켓과 CP 오브젝트와의 연결 요청'이라 한다.
*/


////////////////////////////////////////////////////////////
// # IOCP 모델의 서버 구현을 위해서는 다음 두 가지 일을 진행해야 한다.
// 1. Completion Port 오브젝트의 생성
// 2. Completion Prot 오브젝트와 소켓의 연결

// 이때 소켓은 반드시 OVerlapped 속성이 부여된 소켓이어야 하며,
// 위의 두 가지 일은 다음 하나의 함수를 통해 이뤄진다.

// Handle CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
// FileHandle                : CP 오브젝트 생성시에는 INVALID_HANDLE_VALUE를 전달.
// ExistingCompletionPort    : CP 오브젝트 생성시에는 NULL 전달.
// CompletionKey             : CP 오브젝트 생성시에는 0 전달.
// NumberOfConcurrentThreads : CP 오브젝트에 할당되어 완료된 IO를 처리할 쓰레드의 수를 전달,
//                             2 -> 쓰레드의 수 2개로 제한
//                             0 -> 시스템의 CPU 개수가 동시 실행 가능한 쓰레드의 최대수로 지정

// Ex) HANDLE hCPObject 
//     CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0, 2);



//////////////////////////////////////////////////////////////
// # Completion Port 오브젝트와 소켓의 연결
// CP 오브젝트가 생성되었다면, 이를 소켓과 연결시켜야 한다. 그래야 완료된 소켓의 IO 정보가 CP 오브젝트에 등록된다.

// Handle CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
// FileHandle                : CP 오브젝트에 연결할 소켓의 핸들 전달.
// ExistingCompletionPort    : 소켓과 연결할 CP 오브젝트의 핸들 전달.
// CompletionKey             : 완료된 IO 관련 정보의 전달을 위한 매개변수, (GetQueuedCompletionStatus 함수와 함께 이해해야 한다.)
// NumberOfConcurrentThreads : 어떠한 값을 전달하건, 두 번재 매개변수가 NULL이 아니면 그냥 무시

// Ex) HANDLE hCPObject
//     SOCKET socket = ...
//     CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0, 2);
//     CreateIoCompletionPort((HANDLE)socket, hCPObject, (DWORD)ioInfo, 0);



//////////////////////////////////////////////////////////////
// # Completion Port의 완료된 IO 확인과 쓰레드의 IO 처리

// BOOL GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytes, PULONG_PTR lpCompletionKey, LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds);
// CompletionPort   : 완료된 IO 정보가 등록되어 잇는 CP 오브젝트의 핸들 전달.
// lpNumberOfBytes  : 입출력 과정에서 송수신 된 데이터의 크기정보를 저장할 변수의 주소 값 전달.
// lpCompletionKey  : CreateIoCompletionPort 함수의 세 번째 인자로 전달된 값의 저장을 위한 변수의 주소 값 전달.
// lpOverlapped     : WSASend, WSARecv 함수호출 시 전달하는 OVERLAPPED 구조체 변수의 주소 값이 저장될, 변수의 주소 값 전달
// dwMilliseconds   : 타임아웃 정보전달, 여기서 지정한 시간이 완료되면 FALS를 반환하면서 함수를 빠져나가며,
//                    INFINITE를 전달하면 완료된 IO가 CP 오브젝트에 등록될 때가지 블로킹 상태에 잇게 된다.

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <winsock2.h>
#include <thread>

#define BUF_SIZE 100
#define READ       3
#define WRITE      5

typedef struct                          // 소켓 정보
{
	SOCKET      socket;
	SOCKADDR_IN adr;
} HANDLE_DATA;

typedef struct                          // 버퍼 정보
{
	OVERLAPPED overlapped;
	WSABUF     wsaBuf;
	char       buffer[BUF_SIZE];
	int        rwMode; // READ or WRITE
} IO_DATA;
// "구조체 변수의 주소 값은 구조체 첫 번째 멤버의 주소 값과 일치한다."
// &ioData == &(ioData.overlapped)

void ErrorHandling(char* message);
void EchoThreadMain(LPVOID CompletionPortIO);

int main()
{
	char* input_port = "7711";

	WSADATA      wsaData;

	HANDLE       completionPort;
		
	HANDLE_DATA* handleInfo;
	
	IO_DATA*     ioInfo;
	
	
	SOCKET      serverSocket;
	SOCKADDR_IN serverAdr;
	
	unsigned long recvBytes, i;
	unsigned long flags = 0;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error");

	// CP 오브젝트 생성, 마지막 인자가 0이니 코어싀 수만큼 쓰레드가 CP 오브젝트에 할당될 수 있다.
	completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	std::thread a = std::thread(EchoThreadMain, completionPort);

	serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&serverAdr, 0, sizeof(serverAdr));
	serverAdr.sin_family = AF_INET;
	serverAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAdr.sin_port = htons(atoi(input_port));

	bind(serverSocket, (SOCKADDR*)&serverAdr, sizeof(serverAdr));
	listen(serverSocket, 5);

	while (true)
	{
		SOCKET      clientSocket;
		SOCKADDR_IN clientAdr;
		int adrLen = sizeof(clientAdr);

		clientSocket = accept(serverSocket, (SOCKADDR*)&clientAdr, &adrLen);

		// HANDLE_DATA 구조체 변수를 동적할당한 다음에, 클라이언트와 연결된 소켓, 그리고 클라이언트 주소정보를 담는다.
		handleInfo = (HANDLE_DATA*)malloc(sizeof(HANDLE_DATA));
		handleInfo->socket = clientSocket;
		memcpy(&(handleInfo->adr), &clientAdr, adrLen);
		
		// CP 오브젝트와 생성한 소켓을 연결한다.
		CreateIoCompletionPort((HANDLE)clientSocket, completionPort, (ULONG_PTR)handleInfo, 0);
		// 세 번째 변수로 인자로 구조체 변수의 주소 값을 전달한다.


		// IO_DATA 구조체 변수를 동적 할당한다.
		ioInfo = (IO_DATA*)malloc(sizeof(IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ; // IOCP는 기본적으로 입력의 완료와 출력의 완료를 구분 지어주지 않는다.
		                       // 다만 입력이건 출력이건 완료되었다는 사실만 인시시켜준다.
		                       // 따라서 따로 구분지어준다.


		WSARecv(handleInfo->socket, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
	}
	a.join();

	return 1;

}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void EchoThreadMain(LPVOID CompletionPortIO)
{
	HANDLE completionPort = (HANDLE)CompletionPortIO;
	SOCKET socket;
	DWORD bytesTrans;
	
	HANDLE_DATA* handleInfo;
	IO_DATA*     ioInfo;

	DWORD flags = 0;

	while (1)
	{
		// BOOL GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytes, PULONG_PTR lpCompletionKey, LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds);
		// CompletionPort   : 완료된 IO 정보가 등록되어 잇는 CP 오브젝트의 핸들 전달.
		// lpNumberOfBytes  : 입출력 과정에서 송수신 된 데이터의 크기정보를 저장할 변수의 주소 값 전달.
		// lpCompletionKey  : CreateIoCompletionPort 함수의 세 번째 인자로 전달된 값의 저장을 위한 변수의 주소 값 전달.
		// lpOverlapped     : WSASend, WSARecv 함수호출 시 전달하는 OVERLAPPED 구조체 변수의 주소 값이 저장될, 변수의 주소 값 전달
		// dwMilliseconds   : 타임아웃 정보전달, 여기서 지정한 시간이 완료되면 FALS를 반환하면서 함수를 빠져나가며,
		//                    INFINITE를 전달하면 완료된 IO가 CP 오브젝트에 등록될 때가지 블로킹 상태에 잇게 된다.
		GetQueuedCompletionStatus(completionPort, &bytesTrans, (PULONG_PTR)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
	
		socket = handleInfo->socket;

		if (ioInfo->rwMode == READ)
		{
			puts("message received!");
			if (bytesTrans == 0) // EOF 전송 시
			{
				closesocket(socket);
				free(handleInfo);
				free(ioInfo);
				continue;
			}

			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->rwMode = WRITE;

			// 서버가 수신한 메시지를 클라이언트에게 재전송 한다.
			WSASend(socket, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			// 재전송 이후 클라이언트가 전송하는 메시지를 수신한다.
			ioInfo = (IO_DATA*)malloc(sizeof(IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			
			WSARecv(handleInfo->socket, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);

		}
		else
		{
			puts("message sent!");
			free(ioInfo);
		}
	}
}


