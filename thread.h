#include <windows.h>

#ifndef __THREAD_H__
#define __THREAD_H__

typedef struct tagTHREADDATA{
	HANDLE	hMailslot;
	HANDLE	hSendFile;
	HANDLE	hOwnEvent;
	HANDLE	hSendEvent;
	DWORD   dwOwnNum;
	DWORD	dwSendNum;
	DWORD	dwThreadNum;
	LPSTR   szOwnName;
	LPSTR	szSendName;
	LPSTR	szCreate;
	LPSTR	szOwnNum;
//	LPSTR*	
} THREADDATA,*LPTHREADDATA;

//DWORD _stdcall ThreadProc(LPVOID);
BOOL		Init();
BOOL		Done();
BOOL WINAPI CreateMessage(LPTHREADDATA lpData);
BOOL WINAPI ReadMessage(LPTHREADDATA lpData,LPSTR *szBuffer,LPDWORD);
BOOL WINAPI WriteMessage(LPTHREADDATA lpData,LPSTR szBuffer);
void WINAPI MakeName(LPTHREADDATA lpData);
BOOL WINAPI CreateSlots(LPTHREADDATA lpData,DWORD dwNum);
void WINAPI CloseSlots(LPTHREADDATA lpData,DWORD dwNum);
DWORD WINAPI ThreadProc(LPVOID lpSomeData);

#define NOTINIT 0
#define IDLE	1
#define CREATE	2
#define READ	3
#define WRITE	4

#endif //ifndef