// Overlapped IO���� ������� �Ϸ� �� ����� Ȯ���ϴ� ������� �� ������ �ִ�.
// 1. WSASend, WSARecv �Լ��� ���� ��° �Ű����� Ȱ�� ���, Event ������Ʈ ���
// 2. WSASend, WSARecv �Լ��� �ϰ� ��° �Ű����� Ȱ�� ���, Completion Routine ���

// Event ������Ʈ ����ϱ�
// - IO�� �Ϸ�Ǹ� WSAOVERLAPPED ����ü ������ �����ϴ� Event ������Ʈ�� signaled ���°� �ȴ�.
// - IO�� �Ϸ� �� ����� Ȯ���Ϸ��� WSAGetOverlappedResult �Լ��� ����Ѵ�.

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

void ErrorHandling(char* msg);

int main()
{
	char* input_port = "7711";
	char* input_ip = "127.0.0.1";

	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN sendAdr;

	WSABUF dataBuf;
	char msg[] = "Network is Computer!";
	unsigned long sendBytes = 0;

	WSAEVENT evObj;
	WSAOVERLAPPED overlapped;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");
	
	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	memset(&sendAdr, 0, sizeof(sendAdr));
	sendAdr.sin_family = AF_INET;
	sendAdr.sin_addr.s_addr = inet_addr(input_ip);
	sendAdr.sin_port = htons(atoi(input_port));

	if (connect(hSocket, (SOCKADDR*)&sendAdr, sizeof(sendAdr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	// Event ������Ʈ�� ���� �� ������ �������� �������� �ʱ�ȭ ����
	evObj = WSACreateEvent();
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj;
	dataBuf.len = strlen(msg) + 1;
	dataBuf.buf = msg;

	if (WSASend(hSocket, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			puts("Background data send");
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);
			WSAGetOverlappedResult(hSocket, &overlapped, &sendBytes, FALSE, NULL);
		}
		else
			ErrorHandling("WSASend() error");
	}
	


	printf("Send data size: %d \n", sendBytes);
	WSACloseEvent(evObj);
	closesocket(hSocket);
	WSACleanup();

	return 1;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}