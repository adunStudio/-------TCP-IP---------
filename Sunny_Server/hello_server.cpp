#include <iostream>
#include <cstdlib>
#include <winsock2.h>

void ErrorHandling(char* message);

int main()
{
	char* input_port = "7711";
	char* input_ip   = "127.0.0.1";

	WSADATA wsaData;

	SOCKET server_socket, client_socket;
	
	SOCKADDR_IN server_addr, client_addr;

	
	/***
	struct SOCKADDR_IN
	{
		ADDRESS_FAMILY sin_family; // 주소 체계(Address Family)
		USHORT sin_port;           // 16비트 TCP/UDP PORT 번호
		IN_ADDR sin_addr;          // 32비트 IP주소
		CHAR sin_zero[8];          // 사용되지 않음, 단순히 구조체 SOCKADDR와 일치시키기 위해 삽입된 멤버
	};
	***/
	

	int client_addr_size;

	char message[] = "Hello World!";

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
	// int tcp_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	// int udp_socket = socket(PF_INET, SOCK_DGRAM,  IPPROTO_UDP);
	if (server_socket == INVALID_SOCKET)
		ErrorHandling("socket() error");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;                        // AF_INET  : IPv4 인터넷 프로토콜에 적용하는 주소체계
	                                                         // AF_INET6 : IPv6 인터넷 프로토콜에 적용하는 주소체계
															 // AF_LOCAL : 로컬 통신을 위한 유닉스 프로토콜의 주소체계

	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(input_port));
	// 바이트 순서를 빅 에디안으로 바꿔주는 함수, h는 호스트(host) n은 네트워크(newtwork)
	// htonl : l은  long 4바이트를 의미하므로 IP주소의 변환에 사용
	// htons : s는 short 2바이트를 의미하므로 PORT번호의 변환에 사용

	// 2단계. IP주소와 PORT번호 할당
	// 성공시 0, 실패 시 SOCKET_ERROR 반환
	// SOCKET s, const struct sockaddr* name, int namelen
	if (bind(server_socket, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");


	// 3단계. 연결요청 가능 상태로 변경
	// 성공시 0, 실패 시 SOCKET_ERROR 반환
	// SOCKET s, int backlog
	if (listen(server_socket, 5) == SOCKET_ERROR)
		ErrorHandling("socket() error");

	// 4단계. 연결요청에 대한 수락)
	// 성공 시 소켓 핸들, 실패 시 INVALID_SOCKET 반환
	// SOCKET s, struct sockaddr* addr, int* addrlen
	client_addr_size = sizeof(client_addr);
	client_socket = accept(server_socket, (SOCKADDR*)&client_addr, &client_addr_size);
	if (client_socket == INVALID_SOCKET)
		ErrorHandling("accept() error");

	
	// Write
	// 성공 시 전송된 바이트 수, 실패 시 SOCKET_ERROR 반환
	// SOCKET s        : 데이터 전송 대상과의 연결을 의미하는 소켓의 핸들 값
	// const char* buf : 전송할 데이터를 저장하고 있는 버퍼의 주소 값
	// int len         : 전송할 바이트 수
	// inet flags      : 데이터 전송 시 적용할 다양한 옵션 정보
	send(client_socket, message, sizeof(message), 0);


	// 소켓 닫기
	// 성공 시 0, 실패 시 SOCKET_ERROR 반환
	closesocket(client_socket);
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