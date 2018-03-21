#include <iostream>
#include <cstdlib>
#include <winsock2.h>

// ���� Ŭ���̾�Ʈ
// 1. Ŭ���̾�Ʈ�� ������ �������ڸ��� �ǿ������� ���������� 1����Ʈ �������·� �����Ѵ�.
// 2. Ŭ���̾�Ʈ�� ������ �����ϴ� ���� �ϳ��� 4����Ʈ�� ǥ���Ѵ�.
// 3. ������ ������ �������� ������ ������ �����Ѵ�. ���������� 1����Ʈ�� �����Ѵ�.
// 4. ���� +, -, * �� �ϳ��� �����ؼ� �����Ѵ�.
// 5. ������ �������� 4����Ʈ ������ ���·� Ŭ���̾�Ʈ���� �����Ѵ�.
// 6. �������� ���� Ŭ���̾�Ʈ�� �������� ������ �����Ѵ�.

#define BUF_SIZE 1024
#define RLT_SIZE    4 // �ǿ������� ����Ʈ ��
#define OPSZ        4 // �������� ����Ʈ ��

void ErrorHandling(char* message);

int main()
{
	WSADATA wsaData;

	SOCKET client_socket;

	SOCKADDR_IN server_addr;

	char* input_port = "7711";
	char* input_ip = "127.0.0.1";

	char opmsg[BUF_SIZE];

	int result, opndCount;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	client_socket = socket(PF_INET, SOCK_STREAM, 0);

	if (client_socket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(input_ip);
	server_addr.sin_port = htons(atoi(input_port));

	if (connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");
	else
		std::cout << "Connected.........." << std::endl;

	std::cout << "Operand count: ";
	std::cin >> opndCount;

	opmsg[0] = (char)opndCount;

	for (int i = 0; i < opndCount; ++i)
	{
		std::cout << "Operand " << i + 1 << ": ";
		scanf("%d", (int*)&opmsg[i * OPSZ + 1]);
	}

	fgetc(stdin);
	std::cout << "Operator: ";
	scanf("%c", &opmsg[opndCount * OPSZ + 1]);

	send(client_socket, opmsg, opndCount * OPSZ + 2, 0);
	recv(client_socket, (char*)&result, RLT_SIZE, 0);

	std::cout << "Operation result: " << result << std::endl;

	closesocket(client_socket);

	WSACleanup();

	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}