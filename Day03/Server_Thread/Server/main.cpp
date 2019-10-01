#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector<SOCKET> UserList;

DWORD WINAPI ReceiveThread(LPVOID Arg);

CRITICAL_SECTION UserListSync;

int main()
{
	WSAData wsaData;
	SOCKET ServerSocket;
	SOCKADDR_IN ServerAddress;

	InitializeCriticalSection(&UserListSync);

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
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddress.sin_port = htons(4000);

	if (bind(ServerSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("bind error\n");
		exit(-1);
	}

	if (listen(ServerSocket, 5) == SOCKET_ERROR)
	{
		printf("listen error\n");
		exit(-1);
	}

	SOCKADDR_IN ClientAddress;
	int ClientAddressLength = sizeof(ClientAddress);

	while (true)
	{
		SOCKET ClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);

		EnterCriticalSection(&UserListSync);
		UserList.push_back(ClientSocket);
		LeaveCriticalSection(&UserListSync);

		printf("Client Connect\n");

		DWORD ThreadID;
		HANDLE ReceiveThreadHandle = CreateThread(nullptr, 0, ReceiveThread, (LPVOID)&ClientSocket, 0, &ThreadID);
	}

	DeleteCriticalSection(&UserListSync);

	return 0;
}

DWORD WINAPI ReceiveThread(LPVOID Arg)
{
	SOCKET ClientSocket = *(SOCKET*)Arg;

	char Buffer[1024] = { 0, };

	while (true)
	{
		int recvlen = recv(ClientSocket, Buffer, sizeof(Buffer) - 1, 0);

		if (recvlen <= 0)
		{
			EnterCriticalSection(&UserListSync);
			// 에러나면 연결 끊기
			UserList.erase(find(UserList.begin(), UserList.end(), ClientSocket));
			LeaveCriticalSection(&UserListSync);
			printf("Client Disconnect\n");
			break;
		}
		else
		{
			EnterCriticalSection(&UserListSync);
			// 모두 보내기
			for (auto User : UserList)
			{
				send(User, Buffer, recvlen, 0);
			}
			LeaveCriticalSection(&UserListSync);
		}
	}

	return 0;
}