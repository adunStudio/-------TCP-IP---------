#include <iostream>
#include <cstdlib>
#include <winsock2.h>

#define BUF_SIZE 1024

void ErrorHandling(char* message);

int main()
{
	WSADATA wsaData;

	SOCKET server_socket, client_socket;

	SOCKADDR_IN server_adr, client_adr;

	TIMEVAL timeout;

	fd_set reads, cpyReads;

	char* input_port = "7711";

	int client_adr_size;
	int strLen, fd_num;

	char buf[BUF_SIZE];

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	server_socket = socket(PF_INET, SOCK_STREAM, 0);

	memset(&server_adr, 0, sizeof(server_adr));
	server_adr.sin_family = AF_INET;
	server_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_adr.sin_port = htons(atoi(input_port));

	if (bind(server_socket, (SOCKADDR*)&server_adr, sizeof(server_adr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if (listen(server_socket, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	FD_ZERO(&reads);  // 인자로 전달된 주소의 fd_set형 변수의 모든 비트를 0으로 초기화 한다.
	FD_SET(server_socket, &reads); // 매개변수 fdset으로 전달된 주소의 변수에 매개변수 fd로 전달된 파일 디스크럽터 정보를 등록한다.
	
	while (1)
	{
		// select 함수호출이 끝나면 변화가 생긴 소켓의 위치를 제외한 나머지 위치의 비트들은 0으로 초기화 된다.
		// 따라서 원본의 유지를 위해서는 아래와 같이 복사의 과정을 거쳐야 한다.
		cpyReads = reads;

		// select 함수호출후에는 구조체 TIMEVAL의 멤버가 tv_sec 와 tv_usec에 저장된 값이 타움아웃이 발생하기까지 남았던 시간으로 바뀐다.
		// 그러므로 select 함수를 호출하기 전에 매번  TIMEVAL 구조체 변수의 초기화를 반복해야한다.
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;


		if ((fd_num = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
			break;

		if (fd_num == 0) continue; // Time-Out

		for (int i = 0; i < reads.fd_count; ++i)
		{
			// 매개변수 fdset으로 전달된 주소의 변수에 매개변수 fd로 전달된 소켓의 정보가 있으면 양수를 반환한다.
			// select 함수의 호출결과를 확인하는 용도로 사용된다.
			if (FD_ISSET(reads.fd_array[i], &cpyReads))
			{
				// 상태변화가 확인이 되면 제일먼저 서버 소켓에서 변화가 잇었는지 확인한다.
				// 그리고 서버 소켓의 상태변화가 맞으면 이어서 연결요청에 대한 수락의 과정을 진행한다.
				if (reads.fd_array[i] == server_socket)  // Connection Request
				{
					client_adr_size = sizeof(client_adr);

					client_socket = accept(server_socket, (SOCKADDR*)&client_adr, &client_adr_size);
					
					// fd_set형 변수 reads에 클라이언트와 연결된 소켓의 정보를 등록함에 주목
					FD_SET(client_socket, &reads);

					printf("connected client: %d \n", client_socket);
				}
				// 상태 변화가 발생한 소켓이 서버 소켓이 아닌 경우에 실행된다.
				// 즉, 수신할 데이터가 있는 경우에 실행한다.
				else                                     // Read Message
				{
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);

					if (strLen == 0) // Close Socket
					{
						FD_CLR(reads.fd_array[i], &reads); // 매개변수 fdset으로 전달된 주소의 변수에서 매개변수 fd로 전달된 소켓의 정보를 삭제한다.
						closesocket(cpyReads.fd_array[i]);
						printf("closed client: %d \n", cpyReads.fd_array[i]);
					}
					else
					{
						send(reads.fd_array[i], buf, strLen, 0); // echo!
					}
				}
			}
		}
	}

	closesocket(server_socket);
	WSACleanup();

	return 1;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}