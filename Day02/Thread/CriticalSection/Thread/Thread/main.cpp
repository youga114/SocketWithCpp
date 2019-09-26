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

CRITICAL_SECTION CriticalSenction;

#define ThreadNumber 10


//CriticalSection	- UserMode, 젤 가볍다, 기능 별로 없다.
//Mutex				- KernelMode, 중간치 성능, 기능 추가
//Semaphore			- KernelMode, 무거운 성능, 기능 많음
int main()
{
	time_t start;
	time_t end;
	HANDLE* ThreadHandle;
	//ThreadHandle = (HANDLE*) malloc(sizeof(ThreadHandle) * TreadNumber);			//C
	ThreadHandle = new HANDLE[ThreadNumber];										//C++

	InitializeCriticalSection(&CriticalSenction);			//초기화
	
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
	//WaitForSingleObject
	cout << "end : " << num << endl;
	cout << "time : " << end - start << endl;

	DeleteCriticalSection(&CriticalSenction);				//해제

	delete[] ThreadHandle;

	return 0;
}

DWORD WINAPI ThreadRun1(LPVOID Arg)
{
	EnterCriticalSection(&CriticalSenction);				//0초
	for (int i = 0; i < 9000000; ++i)
	{
		//EnterCriticalSection(&CriticalSenction);				//25초
		num++;
		//LeaveCriticalSection(&CriticalSenction);				//25초
	}
	LeaveCriticalSection(&CriticalSenction);				//0초

	return 0;
}

DWORD WINAPI ThreadRun2(LPVOID Arg)
{
	EnterCriticalSection(&CriticalSenction);				//0초
	for (int i = 0; i < 9000000; ++i)
	{
		//EnterCriticalSection(&CriticalSenction);				//25초
		num--;
		//LeaveCriticalSection(&CriticalSenction);				//25초
	}
	LeaveCriticalSection(&CriticalSenction);				//0초

	return 0;
}