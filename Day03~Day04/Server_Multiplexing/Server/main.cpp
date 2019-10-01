#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <Windows.h>
#include "Player.h"
#include "PacketMaker.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

vector<Player*> UserList;

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

	TIMEVAL TimeOut;	// select 대기 시간
	fd_set Reads;

	fd_set CopyReads;

	FD_ZERO(&Reads);
	FD_SET(ServerSocket, &Reads);

	while (true)
	{
		CopyReads = Reads;			//바뀐 정보 확인하기 위해 복사해둠

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
			// 소켓 변화가 없음
			continue;
		}

		// 감시 하는 소켓 갯수 만큼 확인
		for (u_int i = 0; i < Reads.fd_count; ++i)
		{
			//i번째 감시 소켓의 변화가 있음, 소켓에 정보가 들어옴
			if (FD_ISSET(Reads.fd_array[i], &CopyReads))
			{
				// 접속 요청
				if (Reads.fd_array[i] == ServerSocket)
				{
					SOCKET ConnectClientSocket = accept(ServerSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength);

					//신규 접속 유저 생성
					Player* NewPlayer = new Player;
					NewPlayer->UserID = (uint16)ConnectClientSocket;
					NewPlayer->MySocket = ConnectClientSocket;

					//유저리스트 관리
					EnterCriticalSection(&UserListSync);
					UserList.push_back(NewPlayer);
					LeaveCriticalSection(&UserListSync);

					//감시 소켓 리스트, 클라이언트 소켓 추가
					FD_SET(ConnectClientSocket, &Reads);

					//신규 유저 정보부터 보낸다.
					//신규 클라이언트의 유저리스트 첫번째는 자기꺼
					PacketMaker PM;
					PM.MakeEnterPacket(*NewPlayer);
					send(NewPlayer->MySocket, PM.Packet, PM.PacketSize, 0);

					//신규 유저가 생기면 모든 유저한테 모든 유저정보 전송
					EnterCriticalSection(&UserListSync);
					for (auto User : UserList)
					{
						for (auto UserData : UserList)
						{
							PacketMaker PM;
							PM.MakeEnterPacket(*UserData);
							send(User->MySocket, PM.Packet, PM.PacketSize, 0);

							printf("Enter Packet send. to %d, data: %d\n", User->UserID, UserData->UserID);
						}
					}
					LeaveCriticalSection(&UserListSync);


					printf("Client Connect\n");
				}
				else // 접속 종료, 관리하는 클라이언트의 변화가 생김
				{
					char Buffer[1024] = { 0, };
					int recvlen = recv(Reads.fd_array[i], Buffer, sizeof(Buffer) - 1, 0);

					if (recvlen <= 0)
					{
						//error 종료거나 연결이 끊겼거나
						SOCKET DisconnectSocket = Reads.fd_array[i];
						// 소켓 감시 리스트 제거
						FD_CLR(DisconnectSocket, &Reads);

						//삭제 유저 확인
						Player* DisconnectUser = nullptr;
						for (auto User : UserList)
						{
							if (User->MySocket == DisconnectSocket)
							{
								DisconnectUser = User;
							}
						}

						//유저리스트 삭제
						EnterCriticalSection(&UserListSync);
						UserList.erase(find(UserList.begin(), UserList.end(), DisconnectUser));
						LeaveCriticalSection(&UserListSync);

						//모든 유저한테 삭제될 유저 정보 전송
						for (auto User : UserList)
						{
							PacketMaker PM;
							PM.MakeExitPacket(*DisconnectUser);
							printf("PM.PacketSize: %d\n", PM.PacketSize);
							send(User->MySocket, PM.Packet, PM.PacketSize, 0);
						}

						printf("Disconnect client\n");

						closesocket(DisconnectUser->MySocket);
						delete DisconnectUser;
					}
					else	//클라이언트한테서 정보가 옴
					{
						uint16 Temp;
						byte PacketLength;

						//Temp = Buffer[0] << 8 + Buffer[1];
						//PacketLength = (byte)Buffer[2];
						memcpy(&Temp, Buffer, 2);
						memcpy(&PacketLength, Buffer + 2, 1);

						MSGCode Code = (MSGCode)Temp;

						switch (Code)
						{
							case MSGCode::Move:
							{
								//패킷 분해
								PacketMaker PM;
								Player MovePlayer;
								PM.ExtractPacket(Code, &Buffer[3], &MovePlayer);

								//유저 리스트 반영
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
								
								//바뀐 정보 전송
								for (auto User : UserList)
								{
									PacketMaker PM;
									PM.MakeMovePacket(MovePlayer);
									send(User->MySocket, PM.Packet, PM.PacketSize, 0);
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

	return 0;
}