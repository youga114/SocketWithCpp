#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT	4000

int main()
{
	WSADATA wsaData;
	SOCKET ServerSocketHandle;
	SOCKADDR_IN ServerAddress;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("winsock error\n");
		exit(-1);
	}

	ServerSocketHandle = socket(PF_INET, SOCK_DGRAM, 0);	//UDP
	if (ServerSocketHandle == INVALID_SOCKET)
	{
		printf("socket error\n");
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddress.sin_port = htons(PORT);

	if (bind(ServerSocketHandle, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("bind error\n");
		exit(-1);
	}

	SOCKADDR_IN ClientAddress;
	int ClientAddressSize = sizeof(ClientAddress);
	char Buffer[8192] = { 0, };

	while (true)
	{
		memset(Buffer, 0, sizeof(Buffer));
		int RecvLength = recvfrom(ServerSocketHandle, Buffer, sizeof(Buffer) - 1, 0, (SOCKADDR*)&ClientAddress, &ClientAddressSize);
		if (RecvLength == -1)
		{
			printf("recvfrom error\n");
			exit(-1);
		}
		printf("Client from : %s\n", Buffer);
		int SendtoLength = sendto(ServerSocketHandle, Buffer, RecvLength, 0, (SOCKADDR*)&ClientAddress, ClientAddressSize);

		if (SendtoLength == -1)
		{
			printf("sendto error\n");
			exit(-1);
		}
	}

	closesocket(ServerSocketHandle);
	WSACleanup();

	return 0;
}

