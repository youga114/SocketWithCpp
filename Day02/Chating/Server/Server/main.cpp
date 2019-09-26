#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT	4000

// 감시 할 소켓 핸들 등록
// 감시 할 이벤트 등록, read, send, accept
//while (true)
//{
	//select()
	//TIMEVAL 시간 잠시 멈춰서, 감시 하는 소켓에 이벤트가 발생했는지 확인
	//이벤트가 발생하면 구조체에 이벤트가 발생한 소켓이랑, 이벤트
	//그걸 처리 하고
	//서버 하는 일 처리
	//if a = select(), 1000개
	//if a.read
	//{
	//
	//}
	//else if a.send
	//{
	//
	//}
	// 소켓 처리
	// 그 후에 서버 나머지 작업
	// OS 기능 업데이트 (IOCP(Thread), Epoll)
//}

int main()
{
	WSADATA wsaData;
	SOCKET hServerSocket;
	SOCKET hClientSocket;
	struct sockaddr_in ServerAddress;				//struct sockaddr_in==SOCKADDR
	SOCKADDR ClientAddress;

	TIMEVAL TimeOut;

	fd_set Reads;
	//	fd_set Writes;
	//	fd_set Exceptions;				//변화가 생기면 값들이 바뀜

	fd_set CopyReads;				//전 상태와 비교하기 위해 있음

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

	FD_ZERO(&Reads);							//초기화
	FD_SET(hServerSocket, &Reads);				//서버 소켓 감시해라고 명령

	char Buffer[1024 * 8];

	int ClientSocketAddrSize = sizeof(ClientAddress);
	while (true)
	{
		CopyReads = Reads;
		TimeOut.tv_sec = 0;					//s
		TimeOut.tv_usec = 5;				//ms		기다리는 시간

		int fdReadCount = select(0, &CopyReads, 0, 0, &TimeOut);

		if (fdReadCount == SOCKET_ERROR)
		{
			printf("select error\n");
			exit(-1);
		}

		if (fdReadCount == 0)					//개수가 0이면 아무일도 없음
		{
			//서버에서 하는 다른 작업
			//printf("TimeOut\n");
			continue;
		}

		//서버에 온 내용을 감시 처리
		for (int i = 0; i < (int)Reads.fd_count; ++i)
		{
			if (FD_ISSET(Reads.fd_array[i], &CopyReads))
			{
				//connect client
				if (Reads.fd_array[i] == hServerSocket)					//첫 요청 (연결해주십쇼)
				{
					// 클라이언트 접속을 기다림
					hClientSocket = accept(hServerSocket, (SOCKADDR*)&ClientAddress, &ClientSocketAddrSize);			//accept 하면 OS에 의해 포트번호가 4000번이 아닌 번호로 바뀜
					if (hClientSocket == INVALID_SOCKET)
					{
						printf("accept() error %d\n", GetLastError());
						exit(-1);
					}
					FD_SET(hClientSocket, &Reads);
				}
				else
				{
					//client send, process
					memset(Buffer, 0, sizeof(Buffer));
					int RecvCount = recv(Reads.fd_array[i], Buffer, sizeof(Buffer) - 1, 0);
					if (RecvCount <= 0)	//Disconnect, Error
					{
						printf("Client Disconnect\n");
						FD_CLR(Reads.fd_array[i], &Reads);
						closesocket(Reads.fd_array[i]);
					}
					else
					{
						if (strcmp(Buffer, "end") == 0)
						{
							send(Reads.fd_array[i], Buffer, RecvCount, 0);
							shutdown(hClientSocket, SD_SEND);						//"끊을테니 메시지 줘" 시작
							RecvCount = recv(hClientSocket, Buffer, sizeof(Buffer) - 1, 0);
							FD_CLR(Reads.fd_array[i], &Reads);
							closesocket(Reads.fd_array[i]);
						}
						else
						{
							for (int j = 1; j < (int)Reads.fd_count; ++j)				//0은 제외(서버는 제외)
							{
								printf("Client Send %s\n", Buffer);
								send(Reads.fd_array[j], Buffer, RecvCount, 0);
							}
						}
					}
				}
			}
		}

	}

	closesocket(hServerSocket);
	WSACleanup();

	return 0;
}