#include <iostream>
#include <cstdlib>
#include <winsock2.h>

// 계산기 클라이언트
// 1. 클라이언트는 서버에 접속하자마자 피연산자의 개수정보를 1바이트 정수형태로 전달한다.
// 2. 클라이언트가 서버에 전달하는 정수 하나는 4바이트로 표현한다.
// 3. 정수를 전달한 다음에는 연산의 종류를 전달한다. 연산정보는 1바이트로 전달한다.
// 4. 문자 +, -, * 중 하나를 선택해서 전달한다.
// 5. 서버는 연산결과를 4바이트 정수의 형태로 클라이언트에게 전달한다.
// 6. 연산결과를 얻은 클라이언트는 서버와의 연결을 종료한다.

#define BUF_SIZE 1024
#define RLT_SIZE    4 // 피연산자의 바이트 수
#define OPSZ        4 // 연산결과의 바이트 수

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