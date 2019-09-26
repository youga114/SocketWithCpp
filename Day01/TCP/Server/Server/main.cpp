#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT	4000

int main()
{
	WSADATA wsaData;
	SOCKET hServerSocket;
	SOCKET hClientSocket;
	struct sockaddr_in ServerAddress;				//struct sockaddr_in==SOCKADDR
	SOCKADDR ClientAddress;

	//WinSock �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Winsocket Error\n");
		exit(-1);
	}

	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);						//SOCK_STREAM: TCP
	if (hServerSocket == INVALID_SOCKET)
	{
		printf("socket() error %d", GetLastError());					//������ ���� �ڵ带 �����
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(SOCKADDR));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_port = htons(PORT);				//htons: host to nextwork short (��Ʈ��ũ�� �򿣵�� ���, short�� ����)
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);				//�ƹ� �����ǳ�

	if (bind(hServerSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("bind() error %d\n", GetLastError());
		exit(-1);
	}

	if (listen(hServerSocket, 5) == SOCKET_ERROR)
	{
		printf("listen() error %d\n", GetLastError());
		exit(-1);
	}

	char Buffer[1024*8];

	int ClientSocketAddrSize = sizeof(ClientAddress);
	while (true)
	{
		// Ŭ���̾�Ʈ ������ ��ٸ�
		hClientSocket = accept(hServerSocket, (SOCKADDR*)&ClientAddress, &ClientSocketAddrSize);			//��ٷ�, ���ŷ blocking socket					accept �ϸ� OS�� ���� ��Ʈ��ȣ�� 4000���� �ƴ� ��ȣ�� �ٲ�
		if (hClientSocket == INVALID_SOCKET)
		{
			printf("accept() error %d\n", GetLastError());
			exit(-1);
		}

		while (true)	//������ �����ϸ� ��� ���
		{
			memset(Buffer, 0, sizeof(Buffer));
			int RecvCount = recv(hClientSocket, Buffer, sizeof(Buffer) - 1, 0);				//���ŷ
			if (RecvCount == -1)
			{
				printf("Client Disconnect\n");
				break;
			}
			else
			{
				if (strcmp(Buffer, "end") == 0)
				{
					shutdown(hClientSocket, SD_SEND);
					RecvCount = recv(hClientSocket, Buffer, sizeof(Buffer) - 1, 0);				//���ŷ
					closesocket(hClientSocket);
				}
				else
				{
					printf("Client Send %s\n", Buffer);
					send(hClientSocket, Buffer, RecvCount, 0);
				}
			}
		}
	}

	closesocket(hServerSocket);
	WSACleanup();

	return 0;
}