#include <iostream>
#include <cstdlib>
#include <winsock2.h>

#define BUF_SIZE 1024
#define OPSZ        4

void ErrorHandling(char* message);

int calculate(int opnum, int opnds[], char oprator);

int main()
{
	WSADATA wsaData;

	SOCKET server_socket, client_socket;

	SOCKADDR_IN server_addr, client_addr;

	int client_addr_size;

	char* input_port = "7711";

	char opinfo[BUF_SIZE];

	int result, opndCount;
	
	int sendCount, sendLen;


	/**
	윈속 프로그래밍을 할 때에는 반드시 WSAStartup 함수를 호출해서, 프로그램에서 요구하는 윈소우 소켓의 버전을 알리고,
	해당 버전을 지원하는 라이브러리의 초기화 작업을 진행해야 한다.
	**/
	// 성공 시 0, 실패 시 0이 아닌 에러코드 값 반환
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// 1단계. 소켓 생성
	// 성공 시 소켓 핸들, 실패 시 INVALID_SOCKET 반환
	// int af, int type, int protocol
	server_socket = socket(PF_INET, SOCK_STREAM, 0);  // PF_INET: IPv4 인터넷 프로토콜 체계, SOCK_STREAM: 연결지향형 소켓의 데이터 송수신 방식
	if (server_socket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(input_port));

	// 2단계. IP주소와 PORT번호 할당
	// 성공시 0, 실패 시 SOCKET_ERROR 반환
	// SOCKET s, const struct sockaddr* name, int namelen
	if (bind(server_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	// 3단계. 연결요청 가능 상태로 변경
	// 성공시 0, 실패 시 SOCKET_ERROR 반환
	// SOCKET s, int backlog
	if (listen(server_socket, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	client_addr_size = sizeof(client_addr);

	for (int i = 0; i < 5; ++i)
	{
		opndCount = 0;

		client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &client_addr_size);
		std::cout << "---" << std::endl;

		recv(client_socket, (char*)&opndCount, 1, 0);

		std::cout << opndCount << std::endl;

		sendLen = 0;


		while ((opndCount * OPSZ + 1) > sendLen)
		{
			sendCount = recv(client_socket, &opinfo[sendLen], BUF_SIZE - 1, 0);
			sendLen += sendCount;
		}

		std::cout << opinfo[sendLen - 1] << std::endl;
		result = calculate(opndCount, (int*)opinfo, opinfo[sendLen - 1]);

		send(client_socket, (char*)&result, sizeof(result), 0);

		closesocket(client_socket);
	}

	std::cout << "sdfsdfsd" << std::endl;
	// 소켓 닫기
	// 성공 시 0, 실패 시 SOCKET_ERROR 반환
	closesocket(server_socket);

	// WSAStartup에서 초기화한 소켓 라이브러리 해제
	WSACleanup();

	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int calculate(int opnum, int opnds[], char oprator)
{
	int result = opnds[0];
	int i;

	switch (oprator)
	{
	case '+':
		for (i = 1; i < opnum; ++i) result += opnds[i];
		break;
	case '-':
		for (i = 1; i < opnum; ++i) result -= opnds[i];
		break;
	case '*':
		for (i = 1; i < opnum; ++i) result *= opnds[i];
		break;
	}

	return result;
}