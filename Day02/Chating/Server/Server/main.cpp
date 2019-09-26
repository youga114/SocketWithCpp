#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <iostream>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT	4000

// ���� �� ���� �ڵ� ���
// ���� �� �̺�Ʈ ���, read, send, accept
//while (true)
//{
	//select()
	//TIMEVAL �ð� ��� ���缭, ���� �ϴ� ���Ͽ� �̺�Ʈ�� �߻��ߴ��� Ȯ��
	//�̺�Ʈ�� �߻��ϸ� ����ü�� �̺�Ʈ�� �߻��� �����̶�, �̺�Ʈ
	//�װ� ó�� �ϰ�
	//���� �ϴ� �� ó��
	//if a = select(), 1000��
	//if a.read
	//{
	//
	//}
	//else if a.send
	//{
	//
	//}
	// ���� ó��
	// �� �Ŀ� ���� ������ �۾�
	// OS ��� ������Ʈ (IOCP(Thread), Epoll)
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
	//	fd_set Exceptions;				//��ȭ�� ����� ������ �ٲ�

	fd_set CopyReads;				//�� ���¿� ���ϱ� ���� ����

	//WinSock �ʱ�ȭ
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Winsocket Error\n");
		exit(-1);
	}

	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);						//SOCK_STREAM: TCP
	if (hServerSocket == INVALID_SOCKET)
	{
		printf("socket() error %d", GetLastError());					//������ ���� �ڵ带 �����
		exit(-1);
	}

	memset(&ServerAddress, 0, sizeof(SOCKADDR));
	ServerAddress.sin_family = PF_INET;
	ServerAddress.sin_port = htons(PORT);				//htons: host to nextwork short (��Ʈ��ũ�� �򿣵�� ���, short�� ����)
	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);				//�ƹ� �����ǳ�

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

	FD_ZERO(&Reads);							//�ʱ�ȭ
	FD_SET(hServerSocket, &Reads);				//���� ���� �����ض�� ���

	char Buffer[1024 * 8];

	int ClientSocketAddrSize = sizeof(ClientAddress);
	while (true)
	{
		CopyReads = Reads;
		TimeOut.tv_sec = 0;					//s
		TimeOut.tv_usec = 5;				//ms		��ٸ��� �ð�

		int fdReadCount = select(0, &CopyReads, 0, 0, &TimeOut);

		if (fdReadCount == SOCKET_ERROR)
		{
			printf("select error\n");
			exit(-1);
		}

		if (fdReadCount == 0)					//������ 0�̸� �ƹ��ϵ� ����
		{
			//�������� �ϴ� �ٸ� �۾�
			//printf("TimeOut\n");
			continue;
		}

		//������ �� ������ ���� ó��
		for (int i = 0; i < (int)Reads.fd_count; ++i)
		{
			if (FD_ISSET(Reads.fd_array[i], &CopyReads))
			{
				//connect client
				if (Reads.fd_array[i] == hServerSocket)					//ù ��û (�������ֽʼ�)
				{
					// Ŭ���̾�Ʈ ������ ��ٸ�
					hClientSocket = accept(hServerSocket, (SOCKADDR*)&ClientAddress, &ClientSocketAddrSize);			//accept �ϸ� OS�� ���� ��Ʈ��ȣ�� 4000���� �ƴ� ��ȣ�� �ٲ�
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
							shutdown(hClientSocket, SD_SEND);						//"�����״� �޽��� ��" ����
							RecvCount = recv(hClientSocket, Buffer, sizeof(Buffer) - 1, 0);
							FD_CLR(Reads.fd_array[i], &Reads);
							closesocket(Reads.fd_array[i]);
						}
						else
						{
							for (int j = 1; j < (int)Reads.fd_count; ++j)				//0�� ����(������ ����)
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