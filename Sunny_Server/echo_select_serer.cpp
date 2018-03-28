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

	FD_ZERO(&reads);  // ���ڷ� ���޵� �ּ��� fd_set�� ������ ��� ��Ʈ�� 0���� �ʱ�ȭ �Ѵ�.
	FD_SET(server_socket, &reads); // �Ű����� fdset���� ���޵� �ּ��� ������ �Ű����� fd�� ���޵� ���� ��ũ���� ������ ����Ѵ�.
	
	while (1)
	{
		// select �Լ�ȣ���� ������ ��ȭ�� ���� ������ ��ġ�� ������ ������ ��ġ�� ��Ʈ���� 0���� �ʱ�ȭ �ȴ�.
		// ���� ������ ������ ���ؼ��� �Ʒ��� ���� ������ ������ ���ľ� �Ѵ�.
		cpyReads = reads;

		// select �Լ�ȣ���Ŀ��� ����ü TIMEVAL�� ����� tv_sec �� tv_usec�� ����� ���� Ÿ��ƿ��� �߻��ϱ���� ���Ҵ� �ð����� �ٲ��.
		// �׷��Ƿ� select �Լ��� ȣ���ϱ� ���� �Ź�  TIMEVAL ����ü ������ �ʱ�ȭ�� �ݺ��ؾ��Ѵ�.
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;


		if ((fd_num = select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
			break;

		if (fd_num == 0) continue; // Time-Out

		for (int i = 0; i < reads.fd_count; ++i)
		{
			// �Ű����� fdset���� ���޵� �ּ��� ������ �Ű����� fd�� ���޵� ������ ������ ������ ����� ��ȯ�Ѵ�.
			// select �Լ��� ȣ������ Ȯ���ϴ� �뵵�� ���ȴ�.
			if (FD_ISSET(reads.fd_array[i], &cpyReads))
			{
				// ���º�ȭ�� Ȯ���� �Ǹ� ���ϸ��� ���� ���Ͽ��� ��ȭ�� �վ����� Ȯ���Ѵ�.
				// �׸��� ���� ������ ���º�ȭ�� ������ �̾ �����û�� ���� ������ ������ �����Ѵ�.
				if (reads.fd_array[i] == server_socket)  // Connection Request
				{
					client_adr_size = sizeof(client_adr);

					client_socket = accept(server_socket, (SOCKADDR*)&client_adr, &client_adr_size);
					
					// fd_set�� ���� reads�� Ŭ���̾�Ʈ�� ����� ������ ������ ����Կ� �ָ�
					FD_SET(client_socket, &reads);

					printf("connected client: %d \n", client_socket);
				}
				// ���� ��ȭ�� �߻��� ������ ���� ������ �ƴ� ��쿡 ����ȴ�.
				// ��, ������ �����Ͱ� �ִ� ��쿡 �����Ѵ�.
				else                                     // Read Message
				{
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);

					if (strLen == 0) // Close Socket
					{
						FD_CLR(reads.fd_array[i], &reads); // �Ű����� fdset���� ���޵� �ּ��� �������� �Ű����� fd�� ���޵� ������ ������ �����Ѵ�.
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