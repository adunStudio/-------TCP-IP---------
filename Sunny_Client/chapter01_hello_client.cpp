#include <iostream>
#include <cstdlib>
#include <winsock2.h>

void ErrorHandling(char* message);

int main()
{
	char* input_port = "7711";
	char* input_ip   = "127.0.0.1";

	WSADATA wsaData;

	SOCKET client_socket;

	SOCKADDR_IN server_addr;

	char message[30];
	int  message_length;

	/**
	윈속 프로그래밍을 할 때에는 반드시 WSAStartup 함수를 호출해서, 프로그램에서 요구하는 윈소우 소켓의 버전을 알리고,
	해당 버전을 지원하는 라이브러리의 초기화 작업을 진행해야 한다.
	**/
	// 성공 시 0, 실패 시 0이 아닌 에러코드 값 반환
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	// 소켓 생성
	// 성공 시 소켓 핸들, 실패 시 INVALID_SOCKET 반환
	// int af, int type, int protocol
	// af       : 소켓이 사용할 프로토콜 체계(Protocol Family) 정보
	// type     : 소켓의 데이터 전송방식에 대한 정보
	// protocol : 두 컴퓨터간 통신에 사용되는 프로토콜 정보
	client_socket = socket(PF_INET, SOCK_STREAM, 0);  // PF_INET: IPv4 인터넷 프로토콜 체계, SOCK_STREAM: 연결지향형 소켓의 데이터 송수신 방식
	if (client_socket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(input_ip);
	server_addr.sin_port        = htons(atoi(input_port));

	// 클라이언트 프로그앰에서 소켓을 기반으로 연결요청
	// 성공 시 0, 실패 시 SOCKET_ERROR 반환
	// SOCKET s, const struct sockaddr* name, int namelen
	if (connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		ErrorHandling("connect() error!");

	// Read
	// 성공 시 수신한 바이트 수(단 EOF 전송시 0), 실패 시 SOCKET_ERROR 반환
	// SOCKET s        : 데이터 수신 대상과의 연결을 의미하는 소켓의 핸들 값
	// const char* buf : 수신된 데이터를 저장할 버퍼의 주소 값
	// int len         : 수신할 수 있는 최대 바이트 수
	// int flags       : 데이터 수신 시 적용할 다양한 옵션 정보
	message_length = recv(client_socket, message, sizeof(message) -1, 0);
	if (message_length == -1)
		ErrorHandling("read() error");

	printf("Message from server: %s,\n", message);

	// 소켓 닫기
	// 성공 시 0, 실패 시 SOCKET_ERROR 반환
	closesocket(client_socket);

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