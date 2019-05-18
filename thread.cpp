// mail1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "thread.h"
#include <windows.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////

DWORD g_dwThreadNum;
LPTHREADDATA g_ThreadData;
HANDLE* g_hThread;
DWORD*	g_State;
LPSTR	g_lpszMessage;
HANDLE	g_hDraw;
HANDLE	g_hBuffer;
LPSTR**	g_ThreadBuffer;
extern	RECT g_rcDraw; 
extern	HWND g_hDlg;

//////////////////////////////////////////////////////////////////////////

void WaitTillDrawState()
{
	ResetEvent(g_hDraw);
	InvalidateRect(g_hDlg,&g_rcDraw,TRUE);
	UpdateWindow(g_hDlg);
	WaitForSingleObject(g_hDraw,INFINITE);
}

void WINAPI MakeName(LPTHREADDATA Data)
{
	LPCSTR szMail = "\\\\.\\mailslot\\thread";
	LPCSTR szCreateMes = "Message from ";
	LPSTR  szBuf = new TCHAR[0xF];
	ltoa(Data->dwOwnNum,Data->szOwnNum,10);
	lstrcpy(Data->szOwnName,szMail);
	lstrcpy(Data->szSendName,szMail);
	lstrcat(Data->szOwnName,Data->szOwnNum);
	memset(Data->szCreate,_T(' '),Data->dwThreadNum+1);
	ZeroMemory(Data->szCreate+sizeof(TCHAR)*(Data->dwThreadNum+1),1);
	lstrcat(Data->szCreate,szCreateMes);
	lstrcat(Data->szCreate,Data->szOwnNum);
	Data->szCreate[Data->dwOwnNum] = Data->szOwnNum[0]; 
	ltoa(Data->dwSendNum,szBuf,10);
	strcat(Data->szSendName,szBuf);
	delete szBuf;
}

//////////////////////////////////////////////////////////////////////

BOOL WINAPI CreateSlots(LPTHREADDATA Data,DWORD dwNum)
{
	for(DWORD i = 0;i < dwNum;i++)
	{
		Data[i].hMailslot = CreateMailslot(Data[i].szOwnName,0,MAILSLOT_WAIT_FOREVER,NULL);
		Data[i].hOwnEvent = CreateEvent(NULL,FALSE,TRUE,NULL);
		if((Data[i].hMailslot == INVALID_HANDLE_VALUE) ||
		  (Data[i].hOwnEvent == INVALID_HANDLE_VALUE)) return FALSE;
	}
	for(DWORD i = 0;i < dwNum;i++)
	{
		Data[i].hSendEvent = Data[(i+1)%dwNum].hOwnEvent ;
		Data[i].hSendFile = CreateFile(Data[i].szSendName, 
						 GENERIC_WRITE, 
						 FILE_SHARE_READ,				 // required to write to a mailslot 
						 (LPSECURITY_ATTRIBUTES) NULL, 
						 OPEN_EXISTING, 
						 FILE_ATTRIBUTE_NORMAL/*|FILE_FLAG_OVERLAPPED*/, 
						 (HANDLE) NULL  ); 
		if (Data[i].hSendFile == INVALID_HANDLE_VALUE) 
		{ 
			return FALSE; 
		} 
	}
	
	return TRUE;
}

void WINAPI CloseSlots(LPTHREADDATA Data,DWORD dwNum)
{
	for(DWORD i = 0;i < dwNum;i++)	
	{
		CloseHandle(Data[i].hSendFile);
		CloseHandle(Data[i].hMailslot);
		CloseHandle(Data[i].hOwnEvent);
	}
}

//////////////////////////////////////////////////////////////////////

DWORD WINAPI ThreadProc(LPVOID lpSomeData)
{
	LPTHREADDATA lpData = (LPTHREADDATA)lpSomeData;	
	DWORD dwStrNum = 0;
	DWORD i,j;
	BOOL bResult;
	g_State[lpData->dwOwnNum] = IDLE;
	InvalidateRect(g_hDlg,NULL,TRUE);
	Sleep(500);
	while(TRUE)
	{
		CreateMessage(lpData);
		bResult = ReadMessage(lpData,g_ThreadBuffer[lpData->dwOwnNum],&dwStrNum);
		if(bResult)
		{
			for(j=0;j < dwStrNum;j++)	
				if(g_ThreadBuffer[lpData->dwOwnNum][j][lpData->dwOwnNum] == lpData->szOwnNum[0])
				{
					WaitForSingleObject(g_hBuffer,INFINITE);
					memcpy(g_lpszMessage,g_ThreadBuffer[lpData->dwOwnNum][j],
						lstrlen(g_ThreadBuffer[lpData->dwOwnNum][j])+1);
					ReleaseMutex(g_hBuffer);
					WaitTillDrawState();
			
				}
				else
				{
					g_ThreadBuffer[lpData->dwOwnNum][j][lpData->dwOwnNum] = lpData->szOwnNum[0];
					WriteMessage(lpData,g_ThreadBuffer[lpData->dwOwnNum][j]);
				}
		}
		for(i = 0;i < lpData->dwThreadNum*5;i++)
			ZeroMemory((LPVOID)g_ThreadBuffer[lpData->dwOwnNum][i],MAX_PATH);
		g_State[lpData->dwOwnNum] = IDLE;
		Sleep(rand()%50*25);
	}
/*	for(i = 0;i < lpData->dwThreadNum;i++) 
		delete szBuffer[i];
	delete szBuf;
	delete szBuffer;*/
	return 0;
}

//////////////////////////////////////////////////////////////////////

BOOL WINAPI CreateMessage(LPTHREADDATA lpData)
{
	DWORD dwWritten;
	BOOL bResult;
	g_State[lpData->dwOwnNum] = IDLE;
	WaitForSingleObject(lpData->hSendEvent,INFINITE);
	bResult = WriteFile(lpData->hSendFile, 
						lpData->szCreate, 
						(DWORD) lstrlen(lpData->szCreate) + 1,  // include terminating null 
						 &dwWritten, 
						 (LPOVERLAPPED)NULL);
	if (bResult) 
	{ 
		g_State[lpData->dwOwnNum] = CREATE;
		WaitTillDrawState();
	}
	SetEvent(lpData->hSendEvent);
	Sleep(rand()%50*20);
	return TRUE;
}


BOOL WINAPI ReadMessage(LPTHREADDATA lpData,LPSTR* szBuffer,LPDWORD lpStrNum)
{
	BOOL bResult;
	DWORD dwSize=0,dwCount=0,dwRead; 
	g_State[lpData->dwOwnNum] = IDLE;
	WaitForSingleObject(lpData->hOwnEvent,INFINITE);
	bResult = GetMailslotInfo(lpData->hMailslot, // mailslot handle 
							  (LPDWORD) NULL,    // no maximum message size 
							  &dwSize,           // size of next message 
							  &dwCount,          // number of messages 
							  (LPDWORD)NULL);    // no read time-out 
	SetEvent(lpData->hOwnEvent);
	*lpStrNum = dwCount;		
	if (!bResult) 
	{ 
		return FALSE; 
	} 
	WaitForSingleObject(lpData->hOwnEvent,INFINITE);
	while(dwCount != 0)
	{	
			
        bResult = ReadFile(lpData->hMailslot, 
						szBuffer[*lpStrNum - dwCount],
					    dwSize, 
					    &dwRead, 
						NULL); 
		dwCount--;
	}
	g_State[lpData->dwOwnNum] = READ;
	WaitTillDrawState();
	SetEvent(lpData->hOwnEvent);
	Sleep(rand()%10*100);
	return TRUE;
}

BOOL WINAPI WriteMessage(LPTHREADDATA lpData,LPSTR szBuffer)
{
	DWORD dwWritten;
	BOOL bResult;
	g_State[lpData->dwOwnNum] = IDLE;
	WaitForSingleObject(lpData->hSendEvent,INFINITE);
	bResult = WriteFile(lpData->hSendFile, 
						szBuffer, 
						(DWORD)lstrlen(szBuffer) + 1,  // include terminating null 
						 &dwWritten, 
						(LPOVERLAPPED)NULL); 
	if (!bResult) 
	{ 
		SetEvent(lpData->hSendEvent);
		return FALSE;
	}
	else
	{
		g_State[lpData->dwOwnNum] = WRITE;
		WaitTillDrawState();
	}
	Sleep(rand()%50*10);
	SetEvent(lpData->hSendEvent);
	return TRUE;
}

//////////////////////////////////////////////////////////////////////

BOOL Init()
{
	srand(GetTickCount());
	g_ThreadData = new THREADDATA[g_dwThreadNum];
	g_hThread = new HANDLE[g_dwThreadNum];
	g_State = new DWORD[g_dwThreadNum];
	g_hDraw = CreateEvent(NULL,TRUE,FALSE,NULL);
	g_hBuffer = CreateMutex(NULL,FALSE,NULL);
	g_lpszMessage = new TCHAR[MAX_PATH];
	g_lpszMessage[0] = _T('\0'); 
	g_ThreadBuffer = new LPSTR*[g_dwThreadNum];
	for(DWORD i = 0;i < g_dwThreadNum;i++)
	{
		g_ThreadBuffer[i] = new LPSTR[g_dwThreadNum*5];
		for(DWORD j = 0;j < g_dwThreadNum*5;j++)
			g_ThreadBuffer[i][j] = new TCHAR[MAX_PATH];
	}
	for(DWORD i = 0;i < g_dwThreadNum;i++)
	{
		g_ThreadData[i].szOwnName = new TCHAR[MAX_PATH];
		g_ThreadData[i].szSendName = new TCHAR[MAX_PATH];
		g_ThreadData[i].szCreate = new TCHAR[MAX_PATH];
		g_ThreadData[i].szOwnNum = new TCHAR[0xFF];
		g_ThreadData[i].dwOwnNum = i;
		g_ThreadData[i].dwSendNum = (i+1)%g_dwThreadNum;
		g_ThreadData[i].dwThreadNum = g_dwThreadNum;
		MakeName(g_ThreadData+i);
		g_State[i] = NOTINIT;
	}
	if(!CreateSlots(g_ThreadData,g_dwThreadNum)) return FALSE;
	for(DWORD i = 0;i < g_dwThreadNum;i++) 
	{
		g_hThread[i] = CreateThread(NULL,0,&ThreadProc,g_ThreadData+i,0,NULL);
		if(g_hThread[i] == INVALID_HANDLE_VALUE) 
		{
			return FALSE;
		}	
	}
	return TRUE;
}

BOOL Done()
{
	for(DWORD i = 0;i < g_dwThreadNum;i++)
		TerminateThread(g_hThread[i],-1);
	delete[] g_lpszMessage;
	for(DWORD i = 0;i < g_dwThreadNum;i++)
	{
		for(DWORD j = 0;j < g_dwThreadNum*5;j++)
			delete[] g_ThreadBuffer[i][j];
		delete[] g_ThreadBuffer[i];
	}
	delete[] g_ThreadBuffer;
	CloseSlots(g_ThreadData,g_dwThreadNum);
	for(DWORD i = 0;i < g_dwThreadNum;i++)
	{
		delete[] g_ThreadData[i].szOwnName;
		delete[] g_ThreadData[i].szSendName;
		delete[] g_ThreadData[i].szCreate;
		delete[] g_ThreadData[i].szOwnNum;

	}
	delete[] g_ThreadData;
	delete[] g_hThread;
	delete[] g_State;
	return TRUE;
}