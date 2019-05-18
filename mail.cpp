// barb2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "thread.h"
#include <math.h>
#include <time.h>

#define PI 3.1459

HWND	g_hDlg;
extern	DWORD g_dwThreadNum;
extern	DWORD*	g_State;
extern	LPSTR	g_lpszMessage;
extern	HANDLE	g_hDraw;
RECT	g_rcDraw;

// Foward declarations of functions included in this code module:
LRESULT CALLBACK	DlgProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	srand((DWORD)time(NULL));
	DialogBox(hInstance,(LPCTSTR)IDD_DIALOG1,NULL,(DLGPROC)DlgProc);
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_BARB2);
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

// Mesage handler for dialog box.
LRESULT CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
		case WM_INITDIALOG:
			g_hDlg = hDlg;
			EnableWindow(GetDlgItem(hDlg,IDC_END),FALSE);
			SetDlgItemInt(hDlg,IDC_EDIT1,4,FALSE); 
			GetClientRect(hDlg,&g_rcDraw);
			g_rcDraw.right = g_rcDraw.bottom;
			return TRUE;

		case WM_SYSCOMMAND:
			if(wParam == SC_CLOSE) PostQuitMessage(0);
			break;

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam);
			switch (wmId)
			{
				case IDOK:
					PostQuitMessage(0);
					break;
				case IDC_START:
					g_dwThreadNum = GetDlgItemInt(hDlg,IDC_EDIT1,NULL,FALSE);
					if((g_dwThreadNum > 10) || (g_dwThreadNum < 2)) 
					{
						MessageBox(hDlg,"Enter number between 2 and 10","Error",MB_OK|MB_ICONERROR);
						break;
					}
					EnableWindow(GetDlgItem(hDlg,IDC_START),FALSE);
					EnableWindow(GetDlgItem(hDlg,IDC_END),TRUE);
					InvalidateRect(hDlg,NULL,TRUE);
					Init();
					break;
				case IDC_END:
					Done();
					EnableWindow(GetDlgItem(hDlg,IDC_START),TRUE);
					EnableWindow(GetDlgItem(hDlg,IDC_END),FALSE);
					InvalidateRect(hDlg,NULL,TRUE);
					break;
			}
			break;
		case WM_PAINT:
			hdc = BeginPaint(hDlg,&ps);
			DWORD i;
			TCHAR lpszNum[3] = {'\0','\0','\0'};
			HBRUSH hBrush[6];
			hBrush[0] = CreateSolidBrush(RGB(0x0,0x0,0x0));
			hBrush[1] = CreateSolidBrush(RGB(0xFF,0x0,0x0));
			hBrush[2] = CreateSolidBrush(RGB(0x0,0x0,0xFF));
			hBrush[3] = CreateSolidBrush(RGB(0x0,0xFF,0x0));
			hBrush[4] = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
			int dwRadius = g_rcDraw.bottom*1/3 ;
			double fAngle = 2*PI/(double)g_dwThreadNum;
			if(!IsWindowEnabled(GetDlgItem(g_hDlg,IDC_START)))
			{
				SetBkMode(hdc,TRANSPARENT);
				for(i=0;i < g_dwThreadNum;i++)
				{
					ultoa(i,lpszNum,10);
					TextOut(hdc,int(dwRadius*sin(fAngle*i)-10+dwRadius+20),
								int(-dwRadius*cos(fAngle*i)+10+dwRadius+20),
								lpszNum,lstrlen(lpszNum));
					hBrush[5] = (HBRUSH)SelectObject(hdc,hBrush[g_State[i]]);
					Ellipse(hdc,int(dwRadius*sin(fAngle*i)-10+dwRadius+20),
								int(-dwRadius*cos(fAngle*i)-10+dwRadius+20),
								int(dwRadius*sin(fAngle*i)+10+dwRadius+20),
								int(-dwRadius*cos(fAngle*i)+10+dwRadius+20));
					if((g_State[i] == 2) || (g_State[i] == 4))
					{
						MoveToEx(hdc,int(dwRadius*sin(fAngle*i)+dwRadius+20),
								int(-dwRadius*cos(fAngle*i)+dwRadius+20),NULL);
						LineTo(hdc,int(dwRadius*sin(fAngle*(i+1))+dwRadius+20),
								int(-dwRadius*cos(fAngle*(i+1))+dwRadius+20));
					/*	AngleArc(hdc,dwRadius+20,
									  dwRadius+20,
									  dwRadius,
									  float(180*fAngle*i/PI-90),
									  float(180*fAngle/PI));*/
					}
					TextOut(hdc,g_rcDraw.left+10,g_rcDraw.bottom-30,
							g_lpszMessage,lstrlen(g_lpszMessage));
					SelectObject(hdc,hBrush[5]);
				}			
				SetEvent(g_hDraw);
			}
			EndPaint(hDlg,&ps);
			break;
	}
    return FALSE;
}
