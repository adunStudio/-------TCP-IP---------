#include <stdio.h>
#include <string.h>
#include <winsock2.h>

// �񵿱� ������̶� ����� �Լ��� ��ȯ������ ������ �ۼ����� �Ϸ������ ��ġ���� �ʴ� ��츦 ���Ѵ�.
// ����ȭ�� ������� ����: "������� ����Ǵ� ���� ȣ��� �Լ��� ��ȯ�� ���� ������ �ٸ����� �� �� ����."
// �񵿱�: �������� �ۼ��� �Ϸῡ �������, ȣ��� �Լ��� ��ȯ�� �ϱ⶧���� �ٸ� ���� ������ ���� �ֵ�.

// �񵿱� Notification ����� �𵨿� ���� ����
// Asynchronous Notification IO Model
// �� "�Է¹��ۿ� �����Ͱ� ���ŵǾ �������� ������ �ʿ��ϰų�, ��¹��۰� �� �������� ������ ������ ��Ȳ�� �˸�"

// WSAEventSelect �Լ��� Notification
// IO�� ���� ��ȭ
//  ������ ���� ��ȭ          ���Ͽ� ���� IO�� ���º�ȭ
//  ������ �̺�Ʈ �߻�        ���Ͽ� ���� IO���� �̺�Ʈ �߻�

// int WSAEventSelect(SOCKET s, WSAEVENT hEventObject, long InetworkEvents);
// ������ 0, ���� �� SOCKET_ERROR ��ȯ
// s              : ��������� ������ �ڵ� ����.
// hEventObject   : �̺�Ʈ �߻������� Ȯ���� ���� Event ������Ʈ�� �ڵ� ����.
// INetworkEvents : �����ϰ��� �ϴ� �̺�Ʈ�� ���� ���� ����. 

// �̺�Ʈ ���� (��Ʈ OR �����ڸ� ���ؼ� �� �̻��� ������ ���ÿ� ������ �� �ִ�.)
// FD_READ   : ������ �����Ͱ� �����ϴ°�?
// FD_WRITE  : ���ŷ ���� ������ ������ �����Ѱ�?
// FD_OOB    : Out-of-band �����Ͱ� ���ŵǾ��°�?
// FD_ACCEPT : �����û�� �־��°�?
// FD_CLOSE  : ������ ���ᰡ ��û�Ǿ��°�?

// ����: ��? select �Լ��� ���� ������ ������� ȣ���� �����ѵ�, WSAEventSelect �Լ��� �� �ϳ��� ������ ������θ� ȣ���� �����ϳ׿�?!
//   ��: WSAEventSelect �Լ�ȣ���� ���ؼ� ���޵� ������ ������ �ü���� ����� �ǰ�, �̷��� ��ϵ� ���Ͽ� ���ؼ��� WSAEventSelect �Լ��� ��ȣ���� ���ʿ��ϴ�.

// - WSAEventSelect �Լ��� �� ��¥ ���������� ���� �̺�Ʈ ������Ʈ�� ���� ���
// WSAEVENT WSACreateEvent(void);
// ������ Event ������Ʈ �ڵ�, ���� �� WSA_INVALID_EVENT ��ȯ

// ���� �Լ��� ���ؼ� ������ Event ������Ʈ�� ���Ḧ ���� �Լ��� ������ ���� ������ ���õǾ� �ִ�.
// BOOL WSACloseEvent(WSAEVENT hEvent);
// ������ TRUE, ���� �� FALSE ��ȯ

// �̺�Ʈ �߻������� Ȯ���� ���ؼ��� Event ������Ʈ�� Ȯ���ؾ� �Ѵ�. �� �� ����ϴ� �Լ��� ������ ����.
// DWORD WSAWaitForMultipleEvents(DWORD cEvents, const WSAEVENT* lphEvents, BOOL fWaitAll, DWORD dwTimeout, BOOL fAlertable);
// ���� �� �̺�Ʈ �߻� ������Ʈ ��������, ���� �� WSA_INVALID_EVENT ��ȯ.
// cEvents   : signaled ���·��� ���̿��θ� Ȯ���� Event ������Ʈ�� ���� ���� ����.
// lphEvents : Event ������Ʈ�� �ڵ��� �����ϰ� �ִ� �迭�� �ּ� �� ����.
// fWaitAll  : TRUE ���� �� ��� Event ������Ʈ�� signaled ������ �� ��ȯ, FALS ���޽� �ϳ��� signaled ���°� �Ǿ ��ȯ.
// dwTimeout : 1/1000�� ������ Ÿ�Ӿƿ� ����, WSA_INFINITE ���� �� signaled ���°� �ɶ����� ��ȯ���� �ʴ´�.
// fAlterable: TRUE ���� ��, alterable wait ���·��� ����
// ��ȯ ��   : ��ȯ�� ���� ������ ��� �� WSA_WAIT_EVENT_0��  ����, �� ��° �Ű������� ���޵� �迭�� ��������, signaled ���°� ��
//             Event ������Ʈ�� �ڵ��� ����� �ε����� ���ȴ�. (�� �̻��� ��� ���� �ε��� ��)
//             �׸��� Ÿ�Ӿƿ��� �߻��ϸ� WAIT_TIMEOUT�� ��ȯ�ȴ�.

// �̺�Ʈ ������ ����
// ���������� �ش� ������Ʈ�� signaled ���°� �� ������ Ȯ���ؾ� �Ѵ�.
// int WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents);
// ���� �� 0, ���� �� SOCKET_ERROR ��ȯ
// s               : �̺�Ʈ�� �߻��� ������ �ڵ� ����.
// hEventObject    : ���ϰ� �����(WSAEventSelect �Լ�ȣ�⿡ ����), signaled ������ Event ������Ʈ�� �ڵ� ����.
// lpNetworkEvents : �߻��� �̺�Ʈ�� ���������� ���������� ä���� WSANETWORKEVENTS ����ü ������ �ּ� �� ����. 

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

	// WSAEventSelect �Լ�ȣ���� ���ؼ� ����Ǵ� ���ϰ� Event ������Ʈ�� �ڵ������� ���� �迭�� �����ϴ� �ڵ�
	// �׷��� �� ���� ����� ������ �Ǿ�� �Ѵ�. �׷��� �迭�� ������ �� ���� ��ġ�� ��ǽ�Ų��.
	hSockArr[numOfClntSock]  = hServSock;   // hSockArr[n] �� ����� ���ϰ� ����� Event ������Ʈ�� hEventArr[n] �� ����Ǿ��ִ�.
	hEventArr[numOfClntSock] = newEvent;    // hEventArr[n] �� ����� Event ������Ʈ�� ����� ������ hSockArr[n] �� ����Ǿ� �ִ�.
	numOfClntSock++;

	while (true)
	{
		// �̺�Ʈ �߻������� Ȯ���� ���ؼ��� Event ������Ʈ�� Ȯ���ؾ� �Ѵ�.
		posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);

		startIdx = posInfo - WSA_WAIT_EVENT_0;

		for (int i = startIdx; i < numOfClntSock; ++i)
		{
			int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);

			if (sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT) continue;
			
			sigEventIdx = i;

			// �ش� ������Ʈ�� signaled ���°� �� ������ Ȯ���ؾ� �Ѵ�.
			WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);

			// ���� ��û��
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

			// ������ ���� ��
			if (netEvents.lNetworkEvents & FD_READ)
			{
				if (netEvents.iErrorCode[FD_READ_BIT] != 0) { puts("Read Error"); break; }

				strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);

				send(hSockArr[sigEventIdx], msg, strLen, 0);
			}

			// ���� ��û ��
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