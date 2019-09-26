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

	//WinSock 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Winsocket Error\n");
		exit(-1);
	}

	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);						//SOCK_STREAM: TCP
	if (hServerSocket == INVALID_SOCKET)
	{
		printf("socket() error %d", GetLastError());					//마지막 에러 코드를 찍어줌
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(SOCKADDR));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_port = htons(PORT);				//htons: host to nextwork short (네트워크는 빅엔디안 방식, short로 통일)
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);				//아무 아이피나

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
		// 클라이언트 접속을 기다림
		hClientSocket = accept(hServerSocket, (SOCKADDR*)&ClientAddress, &ClientSocketAddrSize);			//기다려, 블락킹 blocking socket					accept 하면 OS에 의해 포트번호가 4000번이 아닌 번호로 바뀜
		if (hClientSocket == INVALID_SOCKET)
		{
			printf("accept() error %d\n", GetLastError());
			exit(-1);
		}

		while (true)	//유저가 접속하면 계속 통신
		{
			memset(Buffer, 0, sizeof(Buffer));
			int RecvCount = recv(hClientSocket, Buffer, sizeof(Buffer) - 1, 0);				//블락킹
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
					RecvCount = recv(hClientSocket, Buffer, sizeof(Buffer) - 1, 0);				//블락킹
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