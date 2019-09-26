#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>
#include <time.h>
#include <process.h>

using namespace std;

DWORD WINAPI ThreadRun1(LPVOID Arg);
DWORD WINAPI ThreadRun2(LPVOID Arg);

long long num = 0;

HANDLE hMutex;

#define ThreadNumber 2

int main()
{
	time_t start;
	time_t end;
	HANDLE* ThreadHandle;
	//ThreadHandle = (HANDLE*) malloc(sizeof(ThreadHandle) * TreadNumber);			//C
	ThreadHandle = new HANDLE[ThreadNumber];										//C++

	hMutex = CreateMutex(NULL, FALSE, NULL);

	start = time(NULL);
	cout << "first : " << num << endl;

	for (int i = 0; i < ThreadNumber; ++i)
	{
		if (i % 2 == 0)
		{
			//_beginthread()		//������ �ٸ� ���� �Լ�(�������� ���)
			ThreadHandle[i] = CreateThread(NULL, 0, ThreadRun1, NULL, 0, NULL);
		}
		else
		{
			ThreadHandle[i] = CreateThread(NULL, 0, ThreadRun2, NULL, 0, NULL);
		}
	}

	WaitForMultipleObjects(ThreadNumber, ThreadHandle, TRUE, INFINITE);			//���� �Լ����� �����带 ��ٸ���� ����ϴ� �Լ� (True�� ���)			//�ռ��ڴ� ������ ����
	end = time(NULL);
	cout << "end : " << num << endl;
	cout << "time : " << end - start << endl;

	CloseHandle(hMutex);

	for (int i = 0; i < ThreadNumber; ++i)
	{
		CloseHandle(ThreadHandle[i]);
	}
	delete[] ThreadHandle;

	return 0;
}

DWORD WINAPI ThreadRun1(LPVOID Arg)
{
	//WaitForSingleObject(hMutex, INFINITE);		//0��
	for (int i = 0; i < 9000000; ++i)
	{
		WaitForSingleObject(hMutex, INFINITE);
		num++;
		ReleaseMutex(hMutex);
	}
	//ReleaseMutex(hMutex);							//0��

	return 0;
}

DWORD WINAPI ThreadRun2(LPVOID Arg)
{
	//WaitForSingleObject(hMutex, INFINITE);		//0��
	for (int i = 0; i < 9000000; ++i)
	{
		WaitForSingleObject(hMutex, INFINITE);		//2���ε� 58��
		num--;
		ReleaseMutex(hMutex);
	}
	//ReleaseMutex(hMutex);							//0��

	return 0;
}