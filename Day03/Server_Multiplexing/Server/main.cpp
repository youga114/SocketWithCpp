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

	TIMEVAL TimeOut;	// select ��� �ð�
	fd_set Reads;

	fd_set CopyReads;

	FD_ZERO(&Reads);
	FD_SET(ServerSocket, &Reads);

	while (true)
	{
		CopyReads = Reads;			//�ٲ� ���� Ȯ���ϱ� ���� �����ص�

		TimeOut.tv_sec = 0;
		TimeOut.tv_usec = 5;

		int fdReadCount = select(0, &CopyReads, 0, 0, &TimeOut);
		if (fdReadCount == SOCKET_ERROR)
		{
			printf("select error\n");
			exit(-1);
		}
		else if (fdReadCount == 0)
		{
			// ���� ��ȭ�� ����
			continue;
		}

		// ���� �ϴ� ���� ���� ��ŭ Ȯ��
		for (u_int i = 0; i < Reads.fd_count; ++i)
		{
			//i��° ���� ������ ��ȭ�� ����, ���Ͽ� ������ ����
			if (FD_ISSET(Reads.fd_array[i], &CopyReads))
			{
				// ���� ��û
				if (Reads.fd_array[i] == ServerSocket)
				{
					SOCKET ConnectClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);

					EnterCriticalSection(&UserListSync);
					UserList.push_back(ConnectClientSocket);
					LeaveCriticalSection(&UserListSync);

					//���� ���� ����Ʈ, Ŭ���̾�Ʈ ���� �߰�
					FD_SET(ConnectClientSocket, &Reads);

					printf("Client Connect\n");
				}
				else // ���� ����, �����ϴ� Ŭ���̾�Ʈ�� ��ȭ�� ����
				{
					char Buffer[1024] = { 0, };
					int recvlen = recv(Reads.fd_array[i], Buffer, sizeof(Buffer), 0);

					if (recvlen <= 0)
					{
						//error ����ų� ������ ����ų�
						SOCKET DisconnectSocket = Reads.fd_array[i];
						// ���� ���� ����Ʈ ����
						FD_CLR(DisconnectSocket, &Reads);
						closesocket(DisconnectSocket);
						printf("Disconnect client\n");

						//��������Ʈ ����
						EnterCriticalSection(&UserListSync);
						UserList.erase(find(UserList.begin(), UserList.end(), DisconnectSocket));
						LeaveCriticalSection(&UserListSync);
					}
					else	//Ŭ���̾�Ʈ���׼� ������ ��
					{
						//���� �޽��� �������� ����
						EnterCriticalSection(&UserListSync);
						for (auto User : UserList)
						{
							send(User, Buffer, recvlen, 0);
						}
						LeaveCriticalSection(&UserListSync);
					}
				}
			}
		}
	}

	DeleteCriticalSection(&UserListSync);

	return 0;
}