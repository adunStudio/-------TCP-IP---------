#include <stdio.h>
#include <string.h>
#include <winsock2.h>

// 비동기 입출력이란 입출력 함수의 반환시점과 데이터 송수신의 완료시점이 일치하지 않는 경우를 뜻한다.
// 동기화된 입출력의 단점: "입출력이 진행되는 동안 호출된 함수가 반환을 하지 않으니 다른일을 할 수 없다."
// 비동기: 데이터의 송수신 완료에 상관없이, 호출된 함수가 반환을 하기때문에 다른 일을 진행하 ㄹ수 있따.

// 비동기 Notification 입출력 모델에 대한 이해
// Asynchronous Notification IO Model
// ㄴ "입력버퍼에 데이터가 수신되어서 데이터의 수신이 필요하거나, 출력버퍼가 비어서 데이터의 전송이 가능한 상황의 알림"

// WSAEventSelect 함수와 Notification
// IO의 상태 변화
//  소켓의 상태 변화          소켓에 대한 IO의 상태변화
//  소켓의 이벤트 발생        소켓에 대한 IO관련 이벤트 발생

// int WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long InetworkEvents);
// 성공시 0, 실패 시 SOCKET_ERROR 반환
// s              : 관찰대상인 소켓의 핸들 전달.
// hEventObject   : 이벤트 발생유무의 확인을 위한 Event 오브젝트의 핸들 전달.
// INetworkEvents : 감시하고자 하는 이벤트의 유형 정보 전달. 

// 이벤트 종류 (비트 OR 연산자를 통해서 둘 이상의 정보를 동시에 전달할 수 있다.)
// FD_READ   : 수신할 데이터가 존재하는가?
// FD_WRITE  : 블로킹 없이 데이터 전송이 가능한가?
// FD_OOB    : Out-of-band 데이터가 수신되었는가?
// FD_ACCEPT : 연결요청이 있었는가?
// FD_CLOSE  : 연결의 종료가 요청되었는가?

// 질문: 어? select 함수는 여러 소켓을 대상으로 호출이 가능한데, WSAEventSelect 함수는 단 하나의 소켓을 대상으로만 호출이 가능하네요?!
//   답: WSAEventSelect 함수호출을 통해서 전달된 소켓의 정보는 운영체제에 등록이 되고, 이렇게 등록된 소켓에 대해서는 WSAEventSelect 함수의 재호출이 불필요하다.

// - WSAEventSelect 함수의 두 번짜 인자전달을 위한 이벤트 오브젝트의 생성 방법
// WSAEVENT WSACreateEvent(void);
// 성공시 Event 오브젝트 핸들, 실패 시 WSA_INVALID_EVENT 반환

// 위의 함수를 통해서 생성된 Event 오브젝트의 종료를 위한 함수는 다음과 같이 별도로 마련되어 있다.
// BOOL WSACloseEvent(WSAEVENT hEvent);
// 성공시 TRUE, 실패 시 FALSE 반환

// 이벤트 발생유무의 확인을 위해서는 Event 오브젝트를 확인해야 한다. 이 때 사용하는 함수는 다음과 같다.
// DWORD WSAWaitForMultipleEvents(DWORD cEvents, const WSAEVENT* lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable);
// 성공 시 이벤트 발생 오브젝트 관련정보, 실패 시 WSA_INVALID_EVENT 반환.
// cEvents   : signaled 상태로의 전이여부를 확인할 Event 오브젝트의 개수 정보 전달.
// lphEvents : Event 오브젝트의 핸들을 저장하고 있는 배열의 주소 값 전달.
// fWaitAll  : TRUE 전달 시 모든 Event 오브젝트가 signaled 상태일 때 반환, FALS 전달시 하나만 signaled 상태가 되어도 반환.
// dwTimeout : 1/1000초 단위로 타임아웃 지정, WSA_INFINITE 전달 시 signaled 상태가 될때까지 반환하지 않는다.
// fAlterable: TRUE 전달 시, alterable wait 상태로의 진입
// 반환 값   : 반환된 정수 값에서 상수 값 WSA_WAIT_EVENT_0를  빼면, 두 번째 매개변수로 전달된 배열을 기준으로, signaled 상태가 된
//             Event 오브젝트의 핸들이 저장된 인덱스가 계산된다. (둘 이상일 경우 작은 인덱스 값)
//             그리고 타임아웃이 발생하면 WAIT_TIMEOUT이 반환된다.

// 이벤트 종류의 구분
// 마지막으로 해당 오브젝트가 signaled 상태가 된 원인을 확인해야 한다.
// int WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
// 성공 시 0, 실패 시 SOCKET_ERROR 반환
// s               : 이벤트가 발생한 소켓의 핸들 전달.
// hEventObject    : 소켓과 연결된(WSAEventSelect 함수호출에 의해), signaled 상태인 Event 오브젝트의 핸들 전달.
// lpNetworkEvents : 발생한 이벤트의 유형정보와 오류정보로 채워질 WSANETWORKEVENTS 구조체 변수의 주소 값 전달. 

#define BUF_SIZE 100

void ErrorHandling(char* message);
void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);

int main()
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;

	SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT newEvent;
	WSANETWORKEVENTS netEvents;

	int numOfClntSock = 0;
	int strLen;
	int posInfo, startIdx;
	int clntAdrLen;

	char msg[BUF_SIZE];

	char* input_port = "7711";

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(atoi(input_port));

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	newEvent = WSACreateEvent();

	if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
		ErrorHandling("WSAEventSelect() error");

	// WSAEventSelect 함수호출을 통해서 연결되는 소켓과 Event 오브젝트의 핸들정보를 각각 배열에 저장하는 코드
	// 그런데 이 둘의 관계는 유지가 되어야 한다. 그래서 배열에 저장할 때 저장 위치를 통실시킨다.
	hSockArr[numOfClntSock]  = hServSock;   // hSockArr[n] 에 저장된 소켓과 연결된 Event 오브젝트는 hEventArr[n] 에 저장되어있다.
	hEventArr[numOfClntSock] = newEvent;    // hEventArr[n] 에 저장된 Event 오브젝트와 연결된 소켓은 hSockArr[n] 에 저장되어 있다.
	numOfClntSock++;

	while (true)
	{
		// 이벤트 발생유무의 확인을 위해서는 Event 오브젝트를 확인해야 한다.
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);

		startIdx = posInfo - WSA_WAIT_EVENT_0;

		for (int i = startIdx; i < numOfClntSock; ++i)
		{
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);

			if (sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT) continue;
			
			sigEventIdx = i;

			// 해당 오브젝트가 signaled 상태가 된 원인을 확인해야 한다.
			WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);

			// 연결 요청시
			if (netEvents.lNetworkEvents & FD_ACCEPT)
			{
				if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) { puts("Aceept Error"); break; }

				clntAdrLen = sizeof(clntAdr);
				hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &clntAdrLen);

				newEvent = WSACreateEvent();
				WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

				hEventArr[numOfClntSock] = newEvent;
				hSockArr[numOfClntSock]  = hClntSock;
				numOfClntSock++;

				puts("connected new client...");
			}

			// 데이터 수신 시
			if (netEvents.lNetworkEvents & FD_READ)
			{
				if (netEvents.iErrorCode[FD_READ_BIT] != 0) { puts("Read Error"); break; }

				strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);

				send(hSockArr[sigEventIdx], msg, strLen, 0);
			}

			// 종료 요청 시
			if (netEvents.lNetworkEvents & FD_CLOSE)
			{
				if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0) { puts("Close Error"); break; }

				WSACloseEvent(hEventArr[sigEventIdx]);
				closesocket(hSockArr[sigEventIdx]);

				numOfClntSock--;

				CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
				CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
			}
		}

	}

	WSACleanup();

	return 1;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; ++i)
		hSockArr[i] = hSockArr[i + 1];
}

void CompressEvents(WSAEVENT hEventArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; ++i)
		hEventArr[i] = hEventArr[i + 1];
}