#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <Windows.h>
#include "Player.h"
#include "PacketMaker.h"
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector<Player*> UserList;

CRITICAL_SECTION UserListSync;

int main()
{
	WSAData wsaData;
	SOCKET ServerSocket;
	SOCKADDR_IN ServerAddress;

	srand(time(nullptr));

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

	TIMEVAL TimeOut; //select ��� �ð�
	fd_set Reads;

	fd_set CopyReads;

	FD_ZERO(&Reads);
	FD_SET(ServerSocket, &Reads);

	while (true)
	{
		CopyReads = Reads;

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
			//���� ��ȭ�� ����
			continue;
		}

		//���� �ϴ� ���� ���� ��ŭ Ȯ��
		for (u_int i = 0; i < Reads.fd_count; ++i)
		{
			//i��° ���� ������ ��ȭ�� ����, ���Ͽ� ������ ����
			if (FD_ISSET(Reads.fd_array[i], &CopyReads))
			{
				//���� ��û
				if (Reads.fd_array[i] == ServerSocket)
				{
					SOCKET ConnectClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);

					//�ű� ���� ���� ����
					Player* NewPlayer = new Player;
					NewPlayer->UserID = (uint16)ConnectClientSocket;
					NewPlayer->MySocket = ConnectClientSocket;
					NewPlayer->X = rand() % 20;
					NewPlayer->Y = rand() % 20;

					//���� ����Ʈ ����
					EnterCriticalSection(&UserListSync);
					UserList.push_back(NewPlayer);
					LeaveCriticalSection(&UserListSync);

					//���� ���� ����Ʈ, Ŭ���̾�Ʈ ���� �߰�
					FD_SET(ConnectClientSocket, &Reads);

					printf("Client Connect\n");

					//�ű� ���� �������� ������.
					//�ű� Ŭ���̾�Ʈ�� ��������Ʈ ù��°�� �ڱⲨ
					PacketMaker PM;
					PM.MakeEnterPacket(*NewPlayer);
					send(NewPlayer->MySocket, PM.Packet, PM.PacketSize, 0);

					//�ű� ������ ����� ��� �������� ��� �������� ����
					EnterCriticalSection(&UserListSync);
					for (auto User : UserList)
					{
						for (auto UserData : UserList)
						{
							PacketMaker PM;
							PM.MakeEnterPacket(*UserData);
							send(User->MySocket, PM.Packet, PM.PacketSize, 0);
							printf("Enter Packet Send. %d, %d\n", User->UserID, UserData->UserID);
						}
					}
					LeaveCriticalSection(&UserListSync);
				}
				else //���� ����, �����ϴ� Ŭ���̾�Ʈ�� ��ȭ�� ����
				{
					//��Ŷ �����ŭ ���������� ��� �޴°� ������Ʈ
					char Buffer[1024] = { 0, };
					int recvlen = recv(Reads.fd_array[i], Buffer, sizeof(Buffer)-1, 0);

					if (recvlen <= 0)
					{
						//error ����ų� ������ ����ų�
						SOCKET DisconnectSocket = Reads.fd_array[i];
						// ���� ���� ����Ʈ ����
						FD_CLR(DisconnectSocket, &Reads);

						//���� ���� Ȯ��
						Player* DisconnectUser = nullptr;
						for (auto User : UserList)
						{
							if (User->MySocket == DisconnectSocket)
							{
								DisconnectUser = User;
							}
						}

						//��������Ʈ ����
						EnterCriticalSection(&UserListSync);
						UserList.erase(find(UserList.begin(), UserList.end(), DisconnectUser));
						LeaveCriticalSection(&UserListSync);

						//��� �������� ������ ���� ���� ����
						for (auto User : UserList)
						{
							PacketMaker PM;
							PM.MakeExitPacket(*DisconnectUser);
							send(User->MySocket, PM.Packet, PM.PacketSize, 0);
						}

						printf("Disconnect client.\n");

						closesocket(DisconnectUser->MySocket);
						delete DisconnectUser;
					}
					else //Ŭ���̾�Ʈ���׼� ������ ��
					{
						uint16 Temp;
						byte PacketLength;

						//Temp = Buffer[1] << 8 + Buffer[0];
						//PacketLength = (byte)Buffer[2];
						memcpy(&Temp, Buffer, 2);
						memcpy(&PacketLength, &Buffer[2], 1);

						MSGCode Code = (MSGCode)Temp;

						switch (Code)
						{
							case MSGCode::Move:
							{
								//��Ŷ ����
								PacketMaker PM;
								Player MovePlayer;
								PM.ExtractPacket(Code, &Buffer[3], &MovePlayer);

								//���� ����Ʈ �ݿ�
								for (auto User : UserList)
								{
									if (User->UserID == MovePlayer.UserID)
									{
										EnterCriticalSection(&UserListSync);
										User->X = MovePlayer.X;
										User->Y = MovePlayer.Y;
										LeaveCriticalSection(&UserListSync);
										break;
									}
								}

								//�ٲ� ���� ����
								for (auto User : UserList)
								{
									PacketMaker PM;
									PM.MakeMovePacket(MovePlayer);
									send(User->MySocket, PM.Packet, PM.PacketSize, 0);
								}

								//�浹 üũ
								for (auto User : UserList)
								{
									if (User->X == MovePlayer.X  &&
										User->Y == MovePlayer.Y &&
										User->UserID != MovePlayer.UserID)
									{
										PacketMaker PM;
										PM.MakeByePacket(*User);
										send(User->MySocket, PM.Packet, PM.PacketSize, 0);
									}
								}
							}

							break;
						}

					}
				}
			}
		}
	}

	DeleteCriticalSection(&UserListSync);
}