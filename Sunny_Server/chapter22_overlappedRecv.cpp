// Chapter 21���� �񵿱�� ó���Ǿ��� ���� IO�� �ƴ� 'Notifaction(�˸�)' �̾���.
// ���⼭�� IO�� �񵿱�� ó���ϴ� ����� �����Ѵ�.


// # IO(�����)�� ��ø�̶�?
// �ϳ��� ������ ������ ���ÿ� �� �̻��� �������� �����͸� ����(�Ǵ� ����)������ ���ؼ�,
// ������� ��ø�Ǵ� ��Ȳ�� ������ 'IO�� ��ø'�̶� �Ѵ�.
// �̷��� ���� ���� �Ϸ��� ȣ��� ����� �Լ��� �ٷ� ��ȯ�� �ؾ��Ѵ�. �׷��� �� ��°, �� ��° ������ ������ �õ��� �� �ֱ� �����̴�.
// �̷��� �𵨷� �����͸� �ۼ��� �ϴµ� �־ �ٽ��� �Ǵ� ������ '�񵿱� IO'��.
// �񵿱� IO�� �����Ϸ��� ȣ��Ǵ� ����� �Լ��� ��-���ŷ ���� �����ؾ��Ѵ�.


// # Overlapped IO ������ ����
// SOCKET WSASocket(int af, int type, int protocol, LPWSAPROTOCOL_INFO ipProtocolInfo, GROUP g, DWOR dwFlags);
// af             : �������� ü�� ����.
// type           : ������ ������ ���۹�Ŀ� ���� ���� ����.
// protocol       : �� ���� ���̿� ���Ǵ� �������� ���� ����.
// ---- �� ����° �Ű����������� socket �Լ��� �Ķ���Ϳ� ����.
// ipProtocolInfo : �����Ǵ� ������ Ư�� ������ ��� �ִ� WSAPROTOCOL_INFO ����ü ������ �ּ� �� ����, �ʿ� ���� ��� NULL
// g              : �Լ��� Ȯ���� ���ؼ� ����Ǿ� �ִ� �Ű�����, ���� 0 ����
// dwFlags        : ������ �Ӽ����� ����. (WSA_FLAG_OVERLAPPED�� �����ؼ�, �����Ǵ� ���Ͽ� Overlapped IO�� ������ �Ӽ��� �ο�����.)


// Overlapped IO �Ӽ��� �ο��� ������ ���� ���Ŀ� ����Ǵ� �� ���ϰ���(����, Ŭ���̾�Ʈ) ���� ������ �Ϲ� ������ ��������� ���̰� ����.
// �׷��� ������ ����¿� ���Ǵ� �Լ��� �޸��ؾ� �Ѵ�.


// # Overlapped IO�� �����ϴ� WSASend �Լ�
// int WSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD, lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
// s                   : ������ �ڵ� ����, Overlapped IO �Ӽ��� �ο��� ������ �ڵ� ���޽� Overlapped IO �𵨷� ��� ����.
// lpBuffers           : ������ ������ ������ ���ϴ� WSABUF ����ü ������� �̷��� �迭�� �ּ� �� ����.
// dwBufferCount       : �� ��° ���ڷ� ���޵� �迭�� �������� ����.
// lpNumberOfBytesSent : ���۵� ����Ʈ ���� ����� ������ �ּ� �� ����
// dwFlags             : �Լ��� ������ ����Ư���� �����ϴ� ��쿡 ��� (Ex: OOB)
// lpOverlapped        : WSAOVERAPPED ����ü ������ �ּ� �� ����, Event ������Ʈ�� ����ؼ� ������ ������ �ϷḦ Ȯ���ϴ� ��쿡 ���Ǵ� �Ű�����
// lpCompletionRoutine : Completion Routine�̶�� �Լ��� �ּ� �� ����, �̸� ���ؼ��� ������ ������ �ϷḦ Ȯ���� �� �ִ�.

// ## �� ��° ������ WSABUF : �� ����ü���� ������ �����͸� ��� �ִ� ������ �ּ� ���� ũ�������� ������ �� �ֵ��� ���ǵǾ� �ִ�.
/*
typedef struct __WSABUF
{
	u_long len;    // ������ �������� ũ��
	char FAR* buf; // ������ �ּ� ��
} WSABUF, *LPWSABUF;
*/

// ## ���� ��° ������ WSAOVERLAPPED ����ü
/*
typedef struct _WSAOVERLAPPED
{
	DWORD Internal;     // 
	DWORD InternalHigh; // �� Overlapped IO�� ����Ǵ� �������� �ü�� ���������� ���Ǵ� ���
	DWORD Offset;       // 
	DWORD OffsetHigh;   // �� ���� ����� ����Ǿ� �ִ� ��� -> �׷��Ƿ� ������ ������ �� ����� hEvent�� �����̴�.
	WSAEVENT hEvent;    
} WSAOVERLAPPED *LPWSAOVERLAPPED:
*/
// ���� lpOverlapped�� NULL�� ���޵Ǹ�, WSASend �Լ��� ù ��° ���ڷ� ���޵� �ڵ��� ������ ���ŷ ���� �����Ѵ�.
// ### ���� : "WSASend �Լ�ȣ���� ���ؼ� ���ÿ� �� �̻��� �������� �����͸� �����ϴ� ��쿡�� ���ڷ����޵Ǵ� WSAOVERLAPPED ����ü ������ ���� ������ �����ؾ� �Ѵ�."


// # "WSASend �Լ��� ȣ����ڸ��� ��ȯ�ϴµ�, ��� ���۵� �������� ũ�Ⱑ ����ǳ���?"
// -> ��¹��۰� ����հ�, �����ϴ� �������� ũ�Ⱑ ũ�� �ʴٸ�, �Լ�ȣ��� ���ÿ� �������� ������ �Ϸ�� �� �ִ�. 
// (���̷��� ��쿡�� 0�� ��ȯ�ϰ�, �Ű����� lpNumberOfBytesSent�� ���޵� �ּ��� �������� ���� ���۵� �������� ũ�������� ����ȴ�.)
// -> WSASend �Լ��� ��ȯ�� �� �������� ����ؼ� �������� ������ �̷����� ��Ȳ�̸� SOCKT_ERROR�� ��ȯ�Ѵ�.
// (���̷��� ��쿡�� WSAGetLastError �Լ�ȣ���� ���ؼ� Ȯ�� ������ �����ڵ�δ� WSA_IO_PENDING�� ��ϵȴ�.)
// �� �׸��� �� ��쿡�� ���� �Լ�ȣ���� ���ؼ� ���� ���۵� �������� ũ�⸦ Ȯ���ؾ� �Ѵ�.

// # BOOL WSAGetOverlappedResult(SOCKET s, LPWSAOVERLAPPED lpOverlapped, LPDWORD lpchbTransfer, BOOL fWait, LPDWORD lpdwFlags); 
// ���� �� TRUE, ���н� FALSE ��ȯ
// s : Overlapped IO�� ����� ������ �ڵ�
// lpOverlapped : Overlapped ���� �� ������ WSAOVERLAPPED ����ü ������ �ּҰ� ����
// lpcbTransfer : ���� �ۼ��ŵ� ����Ʈ ũ�⸦ ������ ������ �ּ� �� ����
// fWait : ������ IO�� �������� ��Ȳ�� ���, TRUE ���� �� IO�� �Ϸ�� �۱��� ��⸦ �ϰ�, FALSE ���� �� FALSE�� ��ȯ�ϸ鼭 �Լ��� �������´�.
// lpdwFlags : WSARecv�Լ��� ȣ��� ���, �μ����� ����(���ŵ� �޽����� OOB �޽��� ������ ����)�� ��� ���� ���ȴ�. ���ʿ��ϸ� NULL�� �����Ѵ�.

// �� �Լ��� �������� ���۰�� �и� �ƴ϶�, �������� ���� ����� Ȯ�ο��� ���Ǵ� �Լ��̴�.


// # Overlapped IO�� �����ϴ� WSARecv �Լ�
// int WSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
// ���� �� 0, ���� �� SOCKET_ERROR ��ȯ
// s                    : Overlapped IO �Ӽ��� �ο��� ������ �ڵ� ����.
// lpBuffers            : ���ŵ� ������ ������ ����� ������ ������ ���ϴ� WSABUF ����ü �迭�� �ּ� �� ����
// dwBufferCount        : �� ��° ���ڷ� ���޵� �迭�� �������� ����.
// lpNumberOfBytesRecvd : ���ŵ� �������� ũ�������� ����� ������ �ּ� �� ����
// lpFlags              : ����Ư���� ���õ� ������ �����ϰų� �����ϴ� ��쿡 ���ȴ�.
// lpOverlapped         : WSAOVERLAPPED ����ü ������ �ּ� �� ����
// lpCOmpletionRoutine  : Completion Routine�̶�� �Լ��� �ּ� �� ����



// Overlapped IO���� ������� �Ϸ� �� ����� Ȯ���ϴ� ������� �� ������ �ִ�.
// 1. WSASend, WSARecv �Լ��� ���� ��° �Ű����� Ȱ�� ���, Event ������Ʈ ���
// 2. WSASend, WSARecv �Լ��� �ϰ� ��° �Ű����� Ȱ�� ���, Completion Routine ���

// Event ������Ʈ ����ϱ�
// - IO�� �Ϸ�Ǹ� WSAOVERLAPPED ����ü ������ �����ϴ� Event ������Ʈ�� signaled ���°� �ȴ�.
// - IO�� �Ϸ� �� ����� Ȯ���Ϸ��� WSAGetOverlappedResult �Լ��� ����Ѵ�.

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



