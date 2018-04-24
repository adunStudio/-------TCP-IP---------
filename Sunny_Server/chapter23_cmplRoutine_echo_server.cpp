// �����쿡���� ������ �Լ�ȣ���� ���ؼ� ��-���ŷ ���� ������ �Ӽ��� �����Ѵ�.
/*
SOCKET hLisnSOck,
int mode = 1;
....
hLishSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
ioctlsocket(hLisnSock, FIONBIO, &mode); // for non-blocking socket
....
*/

// "�ڵ� hLisnSock�� �����ϴ� ������ ����� ���(FIONBIO)�� ���� mode�� ����� ���� ���·� �����Ѵ�."

// ��, FIONBIO�� ������ ����� ��带 �����ϴ� �ɼ��̸�, �� �Լ��� �� ��° ���ڷ� ���޵� �ּ� ���� ������ 0�� ����Ǿ� ������ ���ŷ ����, 0�� �ƴ� ���� ����Ǿ� ������ ��-���ŷ ���� ������ ����� �Ӽ��� �����Ѵ�.
// - Ŭ���̾�Ʈ�� �����û�� �������� �ʴ� ���¿��� accept �Լ��� ȣ��Ǹ� INVALID_SOCKET�� ��ٷ� ��ȯ�ȴ�. �׸��� �̾ WSAGetLastError �Լ��� ȣ���ϸ� WSAEWOULDBLOCK�� ��ȯ�ȴ�.
// - accept �Լ�ȣ���� ���ؼ� ���� �����Ǵ� ���� ���� ��-���ŷ �Ӽ��� ���Ѵ�.

// ���� ��-���ŷ ����� ������ ������� accept �Լ��� ȣ���ؼ� INVALID_SOCKET�� ��ȯ�Ǹ�, WSAGetLastError �Լ��� ȣ���� ���ؼ� INVALID_SOCKET�� ��ȯ�� ������ Ȯ���ϰ�, �׿� ������ ó���� �ؾ߸� �Ѵ�.

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

// "Overlapped IO�� ������ ���� ���� �����ϱ�"

#define BUF_SIZE 1024

void CALLBACK ReadCompRoutine (DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

void ErrorHandling(char* message);

typedef struct
{
	SOCKET hClntSock;         // ��Ĺ�� �ڵ�
	char buf[BUF_SIZE];       // ����
	WSABUF wsaBuf;            // ���۰��� ������ ��� WSABUF�� ���� (len, buf) 
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
	ioctlsocket(hLisnSock, FIONBIO, &mode); // ������ ��-���ŷ ���� �����Ѵ�.

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
		// Completion Routine�� ������ ���ؼ�..
		SleepEx(100, TRUE);  // for alertable wait state

		// while �ݺ��� �ȿ��� accept �Լ��� ȣ���ϰ� �ִ�. Ư�� ȣ���� ����� �Ǵ� ������ ��-���ŷ ����̹Ƿ�, INVALID_SOCKET�� ��ȯ�� ���� ó�������� �ָ�����.
		hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);

		if(hRecvSock == INVALID_SOCKET)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			else
				ErrorHandling("accept() error");
		}
		puts("Client connected.....");

		
		/* Overlapped IO�� �ʿ��� ����ü ������ �Ҵ��ϰ�, �̸� �ʱ�ȭ�Ѵ�. */

		// �ݺ��� �ȿ��� WSAOVERLAPPED ����ü�� �Ҵ��ϴ� ������ Ŭ���̾�Ʈ �ϳ��� WSAOVERLAPPED ����ü ������ �ϳ��� �Ҵ��ؾ� �ϱ� �����̴�.
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));

		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		hbInfo->hClntSock = (DWORD)hRecvSock;
		(hbInfo->wsaBuf).buf = hbInfo->buf;
		(hbInfo->wsaBuf).len = BUF_SIZE;

		// WSDAOVERLAPPED ����ü�� ������ ��� hEvent�� �Ҵ��� ������ �ּ� ���� �����Ѵ�.
		// ��Completion Routine ����� Overlapped IO������ Event������Ʈ�� ���ʿ��ϱ� ������ hEvent�� �ʿ��� �ٸ� ������ ä���� �ȴ�.
		lpOvLp->hEvent = (HANDLE)hbInfo;


		// ���⼭ ���� ��° ���ڷ� ������ WSAOVERLAPPED ����ü ������ �ּ� ���� Completion Routine�� �� ��° �Ű������� ���޵ȴ�.
		// ������ CompletionRoutine �Լ� �������� ����� �Ϸ�� ������ �ڵ�� ���ۿ� ������ �� �ִ�.
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