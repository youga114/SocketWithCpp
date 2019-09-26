#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	WSADATA wasData;
	SOCKET hServerSocket;
	SOCKADDR_IN ServerAddress;

	char Buffer[8192] = { 0, };
	char Message[8192] = { 0, };
	int MessageLength;

	if (WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
	{
		printf("Winsocket error %d\n", GetLastError());
		exit(-1);
	}

	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
	{
		printf("socket error %d\n", GetLastError());
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(SOCKADDR_IN));
	ServerAddress.sin_family = PF_INET;							//IP V4
	ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");		//IP
	ServerAddress.sin_port = htons(4000);						//PORT

	if (connect(hServerSocket, (SOCKADDR*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		printf("connect error %d\n", GetLastError());
		exit(-1);
	}

	while (true)
	{
		cout << "Message : ";
		cin >> Message;

		MessageLength = send(hServerSocket, Message, strlen(Message), 0);
		if(MessageLength == -1)
		{
			printf("send error %d\n", GetLastError());
			exit(-1);
		}

		if (strcmp(Message, "end") == 0)
		{
			// 우아한 종료용
			send(hServerSocket, Message, strlen(Message), 0);
			closesocket(hServerSocket);
			break;
		}
		else
		{
			memset(Buffer, 0, sizeof(Buffer));
			int recvLength = recv(hServerSocket, Buffer, MessageLength, 0);
			if (recvLength == -1)
			{
				printf("recv error %d\n", GetLastError());
				exit(-1);
			}


			cout << "Message from server : " << Buffer << endl;
		}
	}

	WSACleanup();
	return 0;
}