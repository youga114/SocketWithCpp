#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT	4000

int main()
{
	WSADATA wsaData;
	SOCKET ClientSocketHandle;
	SOCKADDR_IN ServerAddress;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("winsock error\n");
		exit(-1);
	}

	ClientSocketHandle = socket(PF_INET, SOCK_DGRAM, 0);	//UDP
	if (ClientSocketHandle == INVALID_SOCKET)
	{
		printf("socket error\n");
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	ServerAddress.sin_port = htons(PORT);

	char Message[8192] = { 0, };
	char Buffer[8192] = { 0, };
	int MessageLength = 0;


	while (true)
	{
		memset(Message, 0, sizeof(Message));
		cout << "Message : ";
		cin >> Message;

		MessageLength = strlen(Message);

		int SendtoLength = sendto(ClientSocketHandle, Message, MessageLength, 0, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress));
		if (SendtoLength == -1)
		{
			printf("sendto error\n");
			exit(-1);
		}

		int ServerAddressSize = sizeof(ServerAddress);
		memset(Buffer, 0, sizeof(Buffer));
		int RecvLength = recvfrom(ClientSocketHandle, Buffer, sizeof(Buffer) - 1, 0, (SOCKADDR*)&ServerAddress, &ServerAddressSize);
		if (RecvLength == -1)
		{
			printf("recvfrom error\n");
			exit(-1);
		}
		printf("Server from : %s\n", Buffer);

		if (strcmp(Buffer, "end") == 0)
		{
			break;
		}
	}

	closesocket(ClientSocketHandle);
	WSACleanup();

	return 0;
}