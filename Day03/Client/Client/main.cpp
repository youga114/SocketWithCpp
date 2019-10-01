#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

DWORD WINAPI ReceiveThread(LPVOID Arg);

int main()
{
	WSAData wsaData;
	SOCKET ServerSocket;
	SOCKADDR_IN ServerAddress;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Winsock2 error\n");
		exit(-1);
	}

	ServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (ServerSocket == INVALID_SOCKET)
	{
		printf("socket error\n");
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = PF_INET;
	//ServerAddress.sin_addr.s_addr = inet_addr("192.168.0.200");
	ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerAddress.sin_port = htons(4000);

	if (connect(ServerSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("connect error\n");
		exit(-1);
	}

	DWORD ThreadID;
	HANDLE ReceiveThreadHandle = CreateThread(nullptr, 0, ReceiveThread, (LPVOID)&ServerSocket, 0, &ThreadID);

	char Message[1024] = { 0, };
	while (true)
	{
		memset(Message, 0, sizeof(Message));
		//cout << "Message : ";
		//cin >> Message;

		//send(ServerSocket, "Message", strlen(Message), 0);

		while (true)
			send(ServerSocket, "Message", 7, 0);
	}
}

DWORD WINAPI ReceiveThread(LPVOID Arg)
{
	SOCKET ServerSocket = *(SOCKET*)Arg;

	char Buffer[1024] = { 0, };

	while (true)
	{
		memset(Buffer, 0, sizeof(Buffer));
		int recvLen = recv(ServerSocket, Buffer, sizeof(Buffer) - 1, 0);
		if (recvLen == 0)
		{
			printf("Disconnect\n");
			exit(-1);
		}
		else if (recvLen == -1)
		{
			printf("Error Disconnect\n");
			exit(-1);
		}
		else
		{
			printf("Receive Message : %s\n", Buffer);
		}
	}

	return 0;
}