#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>
#include "Player.h"
#include <vector>
#include "Common.h"
#include "PacketMaker.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector<Player*> UserList;

CRITICAL_SECTION UserListSync;

DWORD WINAPI ReceiveThread(LPVOID Arg);

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
		cin >> Message;
	}

	DeleteCriticalSection(&UserListSync);
}

DWORD WINAPI ReceiveThread(LPVOID Arg)
{
	SOCKET ServerSocket = *(SOCKET*)Arg;

	char Buffer[1024] = { 0, };

	while (true)
	{
		memset(Buffer, 0, sizeof(Buffer));

		//��Ŷ �ڵ�
		int recvLen = recv(ServerSocket, Buffer, 3, 0);
		printf("recvLen: %d\n", recvLen);
		if (recvLen == 0)
		{
			printf("Disconnect\n");
			exit(-1);
		}
		else if (recvLen == SOCKET_ERROR)
		{
			printf("Error Disconnect\n");
			exit(-1);
		}
		else
		{
			uint16 Temp;
			byte PacketLength;

			memcpy(&Temp, Buffer, 2);
			memcpy(&PacketLength, Buffer + 2, 1);
			printf("PacketLength: %d\n", PacketLength);

			MSGCode Code = (MSGCode)Temp;

			int BodyLength = 0;
			int HeaderLength = 3; 
			PacketLength -= HeaderLength;			//Code, PacketLength, 3����Ʈ ����
			while (BodyLength < PacketLength)		//���� ��Ŷ�� �� �о�� ������
			{
				int recvLen = recv(ServerSocket, &Buffer[BodyLength], PacketLength, 0);
				if (recvLen <= 0)
				{
					printf("Error or Disconnect\n");
					exit(-1);
				}
				else
				{
					BodyLength += recvLen;
				}
			}

			switch (Code)
			{
				case MSGCode::Enter:
				{
					Player* CanAddPlayer = new Player();
					PacketMaker PM;
					PM.ExtractPacket(Code, Buffer, CanAddPlayer);

					EnterCriticalSection(&UserListSync);

					//������ ���� ���� �˻�
					bool bInsert = true;
					for (auto User : UserList)
					{
						//�̹� �߰� �� ����
						if (User->UserID == CanAddPlayer->UserID)
						{
							bInsert = false;
							break;
						}
					}
					if (bInsert)
					{
						//�ű� ���� �߰�
						UserList.push_back(CanAddPlayer);
					}
					else
					{
						//�̹� �߰� ����
						delete CanAddPlayer;
					}

					LeaveCriticalSection(&UserListSync);
				}
				break;

				case MSGCode::Exit:
				{
					EnterCriticalSection(&UserListSync);

					//���� ����Ʈ���� ������ ����
					Player* DeleteUser = nullptr;
					//�������� ���� ������ ���� ����
					Player* ExitUser = new Player();
					PacketMaker PM;
					PM.ExtractPacket(Code, Buffer, ExitUser);
					//������ ���� ���� �˻�
					for (auto User : UserList)
					{
						if (User->UserID == ExitUser->UserID)
						{
							DeleteUser = User;
						}
					}
					UserList.erase(find(UserList.begin(), UserList.end(), DeleteUser));
					delete DeleteUser;
					delete ExitUser;
					LeaveCriticalSection(&UserListSync);
				}
				break;

				case MSGCode::Move:
				{

				}
				break;
			}

			//����
			//system("cls");
			//EnterCriticalSection(&UserListSync);
			//for (auto User : UserList)
			//{
			//	printf("UserID %d\n", User->UserID);
			//}
			//LeaveCriticalSection(&UserListSync);

		}
	}

	return 0;
}