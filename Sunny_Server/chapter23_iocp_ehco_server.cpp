// # Completion Port
/*
IOCP������ �Ϸ�� IO�� ������ Completion Port(���� CP ������Ʈ)��� Ŀ�� ������Ʈ�� ��ϵȴ�.
�׷��� �׳� ��ϵǴ� ���� �ƴ϶� ������ ���� ��û�� ������ ����Ǿ�� �Ѵ�.

"�� ������ ������� ����Ǵ� IO�� �Ϸ� ��Ȳ�� �� CP ������Ʈ�� ����� �ּ���."

�̸� ������ '���ϰ� CP ������Ʈ���� ���� ��û'�̶� �Ѵ�.
*/


////////////////////////////////////////////////////////////
// # IOCP ���� ���� ������ ���ؼ��� ���� �� ���� ���� �����ؾ� �Ѵ�.
// 1. Completion Port ������Ʈ�� ����
// 2. Completion Prot ������Ʈ�� ������ ����

// �̶� ������ �ݵ�� OVerlapped �Ӽ��� �ο��� �����̾�� �ϸ�,
// ���� �� ���� ���� ���� �ϳ��� �Լ��� ���� �̷�����.

// Handle CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
// FileHandle                : CP ������Ʈ �����ÿ��� INVALID_HANDLE_VALUE�� ����.
// ExistingCompletionPort    : CP ������Ʈ �����ÿ��� NULL ����.
// CompletionKey             : CP ������Ʈ �����ÿ��� 0 ����.
// NumberOfConcurrentThreads : CP ������Ʈ�� �Ҵ�Ǿ� �Ϸ�� IO�� ó���� �������� ���� ����,
//                             2 -> �������� �� 2���� ����
//                             0 -> �ý����� CPU ������ ���� ���� ������ �������� �ִ���� ����

// Ex) HANDLE hCPObject 
//     CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0, 2);



//////////////////////////////////////////////////////////////
// # Completion Port ������Ʈ�� ������ ����
// CP ������Ʈ�� �����Ǿ��ٸ�, �̸� ���ϰ� ������Ѿ� �Ѵ�. �׷��� �Ϸ�� ������ IO ������ CP ������Ʈ�� ��ϵȴ�.

// Handle CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads);
// FileHandle                : CP ������Ʈ�� ������ ������ �ڵ� ����.
// ExistingCompletionPort    : ���ϰ� ������ CP ������Ʈ�� �ڵ� ����.
// CompletionKey             : �Ϸ�� IO ���� ������ ������ ���� �Ű�����, (GetQueuedCompletionStatus �Լ��� �Բ� �����ؾ� �Ѵ�.)
// NumberOfConcurrentThreads : ��� ���� �����ϰ�, �� ���� �Ű������� NULL�� �ƴϸ� �׳� ����

// Ex) HANDLE hCPObject
//     SOCKET socket = ...
//     CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0, 2);
//     CreateIoCompletionPort((HANDLE)socket, hCPObject, (DWORD)ioInfo, 0);



//////////////////////////////////////////////////////////////
// # Completion Port�� �Ϸ�� IO Ȯ�ΰ� �������� IO ó��

// BOOL GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytes, PULONG_PTR lpCompletionKey, LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds);
// CompletionPort   : �Ϸ�� IO ������ ��ϵǾ� �մ� CP ������Ʈ�� �ڵ� ����.
// lpNumberOfBytes  : ����� �������� �ۼ��� �� �������� ũ�������� ������ ������ �ּ� �� ����.
// lpCompletionKey  : CreateIoCompletionPort �Լ��� �� ��° ���ڷ� ���޵� ���� ������ ���� ������ �ּ� �� ����.
// lpOverlapped     : WSASend, WSARecv �Լ�ȣ�� �� �����ϴ� OVERLAPPED ����ü ������ �ּ� ���� �����, ������ �ּ� �� ����
// dwMilliseconds   : Ÿ�Ӿƿ� ��������, ���⼭ ������ �ð��� �Ϸ�Ǹ� FALS�� ��ȯ�ϸ鼭 �Լ��� ����������,
//                    INFINITE�� �����ϸ� �Ϸ�� IO�� CP ������Ʈ�� ��ϵ� ������ ���ŷ ���¿� �հ� �ȴ�.

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <winsock2.h>
#include <thread>

#define BUF_SIZE 100
#define READ       3
#define WRITE      5

typedef struct                          // ���� ����
{
	SOCKET      socket;
	SOCKADDR_IN adr;
} HANDLE_DATA;

typedef struct                          // ���� ����
{
	OVERLAPPED overlapped;
	WSABUF     wsaBuf;
	char       buffer[BUF_SIZE];
	int        rwMode; // READ or WRITE
} IO_DATA;
// "����ü ������ �ּ� ���� ����ü ù ��° ����� �ּ� ���� ��ġ�Ѵ�."
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

	// CP ������Ʈ ����, ������ ���ڰ� 0�̴� �ھ�� ����ŭ �����尡 CP ������Ʈ�� �Ҵ�� �� �ִ�.
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

		// HANDLE_DATA ����ü ������ �����Ҵ��� ������, Ŭ���̾�Ʈ�� ����� ����, �׸��� Ŭ���̾�Ʈ �ּ������� ��´�.
		handleInfo = (HANDLE_DATA*)malloc(sizeof(HANDLE_DATA));
		handleInfo->socket = clientSocket;
		memcpy(&(handleInfo->adr), &clientAdr, adrLen);
		
		// CP ������Ʈ�� ������ ������ �����Ѵ�.
		CreateIoCompletionPort((HANDLE)clientSocket, completionPort, (ULONG_PTR)handleInfo, 0);
		// �� ��° ������ ���ڷ� ����ü ������ �ּ� ���� �����Ѵ�.


		// IO_DATA ����ü ������ ���� �Ҵ��Ѵ�.
		ioInfo = (IO_DATA*)malloc(sizeof(IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len = BUF_SIZE;
		ioInfo->wsaBuf.buf = ioInfo->buffer;
		ioInfo->rwMode = READ; // IOCP�� �⺻������ �Է��� �Ϸ�� ����� �ϷḦ ���� �������� �ʴ´�.
		                       // �ٸ� �Է��̰� ����̰� �Ϸ�Ǿ��ٴ� ��Ǹ� �νý����ش�.
		                       // ���� ���� ���������ش�.


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
		// CompletionPort   : �Ϸ�� IO ������ ��ϵǾ� �մ� CP ������Ʈ�� �ڵ� ����.
		// lpNumberOfBytes  : ����� �������� �ۼ��� �� �������� ũ�������� ������ ������ �ּ� �� ����.
		// lpCompletionKey  : CreateIoCompletionPort �Լ��� �� ��° ���ڷ� ���޵� ���� ������ ���� ������ �ּ� �� ����.
		// lpOverlapped     : WSASend, WSARecv �Լ�ȣ�� �� �����ϴ� OVERLAPPED ����ü ������ �ּ� ���� �����, ������ �ּ� �� ����
		// dwMilliseconds   : Ÿ�Ӿƿ� ��������, ���⼭ ������ �ð��� �Ϸ�Ǹ� FALS�� ��ȯ�ϸ鼭 �Լ��� ����������,
		//                    INFINITE�� �����ϸ� �Ϸ�� IO�� CP ������Ʈ�� ��ϵ� ������ ���ŷ ���¿� �հ� �ȴ�.
		GetQueuedCompletionStatus(completionPort, &bytesTrans, (PULONG_PTR)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
	
		socket = handleInfo->socket;

		if (ioInfo->rwMode == READ)
		{
			puts("message received!");
			if (bytesTrans == 0) // EOF ���� ��
			{
				closesocket(socket);
				free(handleInfo);
				free(ioInfo);
				continue;
			}

			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = bytesTrans;
			ioInfo->rwMode = WRITE;

			// ������ ������ �޽����� Ŭ���̾�Ʈ���� ������ �Ѵ�.
			WSASend(socket, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);

			// ������ ���� Ŭ���̾�Ʈ�� �����ϴ� �޽����� �����Ѵ�.
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


