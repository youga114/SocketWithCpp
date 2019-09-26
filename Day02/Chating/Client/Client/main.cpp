#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

DWORD WINAPI ThreadRun1(LPVOID Arg);
DWORD WINAPI ThreadRun2(LPVOID Arg);

CRITICAL_SECTION CriticalSenction;

SOCKET hServerSocket;

char Buffer[8192] = { 0, };
char Message[8192] = { 0, };
int MessageLength;

int main()
{
	WSADATA wasData;
	SOCKADDR_IN ServerAddress;

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


	HANDLE* ThreadHandle;
	ThreadHandle = new HANDLE[2];

	InitializeCriticalSection(&CriticalSenction);			//초기화

	ThreadHandle[0] = CreateThread(NULL, 0, ThreadRun1, NULL, 0, NULL);
	ThreadHandle[1] = CreateThread(NULL, 0, ThreadRun2, NULL, 0, NULL);

	WaitForMultipleObjects(2, ThreadHandle, TRUE, INFINITE);			//메인 함수에서 스레드를 기다리라고 명령하는 함수 (True면 기달)			//앞숫자는 스레드 개수

	DeleteCriticalSection(&CriticalSenction);				//해제

	delete[] ThreadHandle;

	WSACleanup();
	return 0;
}


DWORD WINAPI ThreadRun1(LPVOID Arg)
{
	while (true)
	{
		memset(Buffer, 0, sizeof(Buffer));
		int recvLength = recv(hServerSocket, Buffer, sizeof(Buffer) - 1, 0);		//블로킹
		if (recvLength == -1)
		{
			printf("recv error %d\n", GetLastError());
			exit(-1);
		}

		if (strcmp(Buffer, "end") == 0)
		{
			// 우아한 종료용
			send(hServerSocket, Message, strlen(Message), 0);
			closesocket(hServerSocket);
			break;
		}

		cout << "Message from server : " << Buffer << endl;
	}
}

DWORD WINAPI ThreadRun2(LPVOID Arg)
{
	while (true)
	{
		cin >> Message;							//블락킹

		MessageLength = send(hServerSocket, Message, strlen(Message), 0);
		if (MessageLength == -1)
		{
			printf("send error %d\n", GetLastError());
			exit(-1);
		}

		if (strcmp(Message, "end") == 0)
		{
			break;
		}
	}
}