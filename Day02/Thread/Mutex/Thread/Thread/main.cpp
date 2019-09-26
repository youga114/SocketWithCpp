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
			//_beginthread()		//스레드 다른 생성 함수(리눅스와 비슷)
			ThreadHandle[i] = CreateThread(NULL, 0, ThreadRun1, NULL, 0, NULL);
		}
		else
		{
			ThreadHandle[i] = CreateThread(NULL, 0, ThreadRun2, NULL, 0, NULL);
		}
	}

	WaitForMultipleObjects(ThreadNumber, ThreadHandle, TRUE, INFINITE);			//메인 함수에서 스레드를 기다리라고 명령하는 함수 (True면 기달)			//앞숫자는 스레드 개수
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
	//WaitForSingleObject(hMutex, INFINITE);		//0초
	for (int i = 0; i < 9000000; ++i)
	{
		WaitForSingleObject(hMutex, INFINITE);
		num++;
		ReleaseMutex(hMutex);
	}
	//ReleaseMutex(hMutex);							//0초

	return 0;
}

DWORD WINAPI ThreadRun2(LPVOID Arg)
{
	//WaitForSingleObject(hMutex, INFINITE);		//0초
	for (int i = 0; i < 9000000; ++i)
	{
		WaitForSingleObject(hMutex, INFINITE);		//2개인데 58초
		num--;
		ReleaseMutex(hMutex);
	}
	//ReleaseMutex(hMutex);							//0초

	return 0;
}