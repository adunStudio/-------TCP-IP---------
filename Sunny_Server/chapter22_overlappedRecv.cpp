// Chapter 21에서 비동기로 처리되었던 것은 IO가 아닌 'Notifaction(알림)' 이었다.
// 여기서는 IO를 비동기로 처리하는 방법을 공부한다.


// # IO(입출력)의 중첩이란?
// 하나의 쓰레드 내에서 동시에 둘 이상의 영역으로 데이터를 전송(또는 수신)함으로 인해서,
// 입출력의 중첩되는 상황을 가리켜 'IO의 중첩'이라 한다.
// 이러한 일이 가능 하려면 호출된 입출력 함수가 바로 반환을 해야한다. 그래야 두 번째, 세 번째 데이터 전송을 시도할 수 있기 때문이다.
// 이러한 모델로 데이터를 송수신 하는데 있어서 핵심이 되는 사항은 '비동기 IO'다.
// 비동기 IO가 가능하려면 호출되는 입출력 함수는 넌-블로킹 모드로 동작해야한다.


// # Overlapped IO 소켓의 생성
// SOCKET WSASocket(int af, int type, int protocol, LPWSAPROTOCOL_INFO ipProtocolInfo, GROUP g, DWOR dwFlags);
// af             : 프로토콜 체계 정보.
// type           : 소켓의 데이터 전송방식에 대한 정보 전달.
// protocol       : 두 소켓 사이에 사용되는 프로토콜 정보 전달.
// ---- 위 세번째 매개변수까지는 socket 함수의 파라미터와 같다.
// ipProtocolInfo : 생성되는 소켓의 특성 정보를 담고 있는 WSAPROTOCOL_INFO 구조체 변수의 주소 값 전달, 필요 없는 경우 NULL
// g              : 함수의 확장을 위해서 예약되어 있는 매개변수, 따라서 0 전달
// dwFlags        : 소켓의 속성정보 전달. (WSA_FLAG_OVERLAPPED를 전달해서, 생성되는 소켓에 Overlapped IO가 가능한 속성을 부여하자.)


// Overlapped IO 속성이 부여된 소켓의 생성 이후에 진행되는 두 소켓간의(서버, 클라이언트) 연결 과정은 일반 소켓의 연결과정가 차이가 없다.
// 그러나 데이터 입출력에 사용되는 함수는 달리해야 한다.


// # Overlapped IO를 진행하는 WSASend 함수
// int WSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD, lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
// s                   : 소켓의 핸들 전달, Overlapped IO 속성이 부여된 소켓의 핸들 전달시 Overlapped IO 모델로 출력 진행.
// lpBuffers           : 전송할 데이터 정보를 지니는 WSABUF 구조체 변수들로 이뤄진 배열의 주소 값 전달.
// dwBufferCount       : 두 번째 인자로 전달된 배열의 길이정보 전달.
// lpNumberOfBytesSent : 전송된 바이트 수가 저장될 변수의 주소 값 전달
// dwFlags             : 함수의 데이터 전송특성을 변경하는 경우에 사용 (Ex: OOB)
// lpOverlapped        : WSAOVERAPPED 구조체 변수의 주소 값 전달, Event 오브젝트를 사용해서 데이터 전송의 완료를 확인하는 경우에 사용되는 매개변수
// lpCompletionRoutine : Completion Routine이라는 함수의 주소 값 전달, 이를 통해서도 데이터 전송의 완료를 확인할 수 있다.

// ## 두 번째 인자인 WSABUF : 이 구조체에는 전송할 데이터를 담고 있는 버퍼의 주소 값과 크기정보를 저장할 수 있도록 정의되어 있다.
/*
typedef struct __WSABUF
{
	u_long len;    // 전송할 데이터의 크기
	char FAR* buf; // 버퍼의 주소 값
} WSABUF, *LPWSABUF;
*/

// ## 여섯 번째 인자인 WSAOVERLAPPED 구조체
/*
typedef struct _WSAOVERLAPPED
{
	DWORD Internal;     // 
	DWORD InternalHigh; // ㄴ Overlapped IO가 진행되는 과정에서 운영체제 내부적으로 사용되는 멤버
	DWORD Offset;       // 
	DWORD OffsetHigh;   // ㄴ 역시 사용이 예약되어 있는 멤버 -> 그러므로 실제로 관심을 둘 멤버는 hEvent가 전부이다.
	WSAEVENT hEvent;    
} WSAOVERLAPPED *LPWSAOVERLAPPED:
*/
// 만약 lpOverlapped에 NULL이 전달되면, WSASend 함수의 첫 번째 인자로 전달된 핸들의 소켓은 블로킹 모드로 동작한다.
// ### 주의 : "WSASend 함수호출을 통해서 동시에 둘 이상의 영역으로 데이터를 전송하는 경우에는 인자로전달되는 WSAOVERLAPPED 구조체 변수를 각각 별도로 구성해야 한다."


// # "WSASend 함수가 호출되자마자 반환하는데, 어떻게 전송된 데이터의 크기가 저장되나요?"
// -> 출력버퍼가 비어잇고, 전송하는 데이터의 크기가 크지 않다면, 함수호출과 동시에 데이터의 전송이 완료될 수 있다. 
// (ㄴ이러한 경우에는 0을 반환하고, 매개변수 lpNumberOfBytesSent로 전달된 주소의 변수에는 실제 전송된 데이터의 크기정보가 저장된다.)
// -> WSASend 함수가 반환을 한 다음에도 계속해서 데이터의 전송이 이뤄지는 상황이면 SOCKT_ERROR을 반환한다.
// (ㄴ이러한 경우에는 WSAGetLastError 함수호출을 통해서 확인 가능한 오류코드로는 WSA_IO_PENDING이 등록된다.)
// ㄴ 그리고 이 경우에는 다음 함수호출을 통해서 실제 전송된 데이터의 크기를 확인해야 한다.

// # BOOL WSAGetOverlappedResult(SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpchbTransfer, BOOL fWait, LPDWORD lpdwFlags); 
// 성공 시 TRUE, 실패시 FALSE 반환
// s : Overlapped IO가 진행된 소켓의 핸들
// lpOverlapped : Overlapped 진행 시 전달한 WSAOVERLAPPED 구조체 변수의 주소값 전달
// lpcbTransfer : 실제 송수신된 바이트 크기를 저장할 변수의 주소 값 전달
// fWait : 여전히 IO가 진행중인 상황의 경우, TRUE 전달 시 IO가 완료될 댸까지 대기를 하고, FALSE 전달 시 FALSE를 반환하면서 함수를 빠져나온다.
// lpdwFlags : WSARecv함수가 호출된 경우, 부수적인 정보(수신된 메시지가 OOB 메시지 인지와 같은)를 얻기 위해 사용된다. 불필요하면 NULL을 전달한다.

// 위 함수는 데이터의 전송결과 분만 아니라, 데이터의 수신 결과의 확인에도 사용되는 함수이다.


// # Overlapped IO를 진행하는 WSARecv 함수
// int WSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
// 성공 시 0, 실패 시 SOCKET_ERROR 반환
// s                    : Overlapped IO 속성이 부여된 소켓의 핸들 전달.
// lpBuffers            : 수신된 데이터 정보가 저장될 버퍼의 정보를 지니는 WSABUF 구조체 배열의 주소 값 전달
// dwBufferCount        : 두 번째 인자로 전달된 배열의 길이정보 전달.
// lpNumberOfBytesRecvd : 수신된 데이터의 크기정보가 저장될 변수의 주소 값 전달
// lpFlags              : 전송특성과 관련된 정보를 지정하거나 수신하는 경우에 사용된다.
// lpOverlapped         : WSAOVERLAPPED 구조체 변수의 주소 값 전달
// lpCOmpletionRoutine  : Completion Routine이라는 함수의 주소 값 전달



// Overlapped IO에서 입출력의 완료 및 결과를 확인하는 방법에는 두 가지가 있다.
// 1. WSASend, WSARecv 함수의 여섯 번째 매개변수 활용 방법, Event 오브젝트 기반
// 2. WSASend, WSARecv 함수의 일곱 번째 매개변수 활용 방법, Completion Routine 기반

// Event 오브젝트 사용하기
// - IO가 완료되면 WSAOVERLAPPED 구조체 변수가 참조하는 Event 오브젝트가 signaled 상태가 된다.
// - IO의 완료 및 결과를 확인하려면 WSAGetOverlappedResult 함수를 사용한다.

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024

void ErrorHandling(char* message);

int main()
{
	char* input_port = "7711";

	WSADATA wsaData;
	SOCKET hLisnSock, hRecvSock;
	SOCKADDR_IN lisnAdr, recvAdr;
	int recvAdrSz;

	WSABUF dataBuf;
	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	char buf[BUF_SIZE];
	unsigned long recvBytes = 0;
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

	if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			puts("Background data receive");
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);
			WSAGetOverlappedResult(hRecvSock, &overlapped, &recvBytes, FALSE, NULL);
		}
		else
			ErrorHandling("WSARecv() error");
	}


	printf("Received message: %s \n", buf);
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



