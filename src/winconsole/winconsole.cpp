// winconsole.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "winconsole.h"
#include "winserver.h"
#include "winutils.h"
#include <commctrl.h>
#include <windowsx.h>

#include <iostream>
#include <sstream>

#include <measureserver/version.h>

#include <contrib/crashinfo.h>
#include <string>
#include <fstream>
#include <iostream>

#include <sstream>
#include <stringop.h>

#define MAX_LOADSTRING 100

#define TIMER_1 1
#define TIMER_2 2

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hwndEdit;
HWND hWnd;

HWND hWndInfo;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void ConsolePrint(HWND hwndEdit, const std::wstring& data, unsigned int maxLineCount = 500);

WinServer* gpWinServer;

VOID CALLBACK ConsoleTimer(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime);

class ConsoleBuffer : public std::streambuf
{
public:
	ConsoleBuffer() : rread(0), mTimerStarted(false)
	{
		mMutex = CreateMutex(NULL, FALSE, NULL);
	}
	~ConsoleBuffer()
	{
		CloseHandle(mMutex);
	}
	int overflow(int c)
	{
		if (WaitForSingleObject(mMutex, INFINITE) != WAIT_OBJECT_0)
		{
			MessageBox(hWnd, L"Mutex error", L"Mutex error", MB_OK);
			return 1;
		}

		if (c == '\n') mBuffer += L'\r';
		mBuffer += (wchar_t)c;

		if (!mTimerStarted)
		{
			SetTimer(hWnd, TIMER_1, 100, ConsoleTimer);
			mTimerStarted = true;
		}

		ReleaseMutex(mMutex);

		return 1;
	}

	void Print()
	{
		if (WaitForSingleObject(mMutex, INFINITE) != WAIT_OBJECT_0)
		{
			MessageBox(hWnd, L"Mutex error", L"Mutex error", MB_OK);
			return;
		}

		ConsolePrint(hwndEdit, mBuffer);
		mBuffer.clear();
		mTimerStarted = false;
		KillTimer(hWnd, TIMER_1);

		ReleaseMutex(mMutex);
	}
private:
	int rread;
	std::wstring mBuffer;
	bool mTimerStarted;
	HANDLE mMutex;
} *gConsoleBuffer;

VOID CALLBACK ConsoleTimer(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime)
{
	gConsoleBuffer->Print();
}

VOID CALLBACK InfoTimer(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime)
{
	SendMessage(hWndInfo, WM_SETTEXT, 0, (WPARAM)ConvertToUnicode(gpWinServer->GetInfoText()).c_str());
}

// Entrada del programa en modo ventana
int APIENTRY realWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WinServer pWinServer;
	gpWinServer = &pWinServer;

	ConsoleBuffer conbuf;
	gConsoleBuffer = &conbuf;
	std::cout.rdbuf(&conbuf);
	std::cerr.rdbuf(&conbuf);

	MSG msg;
	HACCEL hAccelTable;

	// set the current work dir to the binary path
	WCHAR binaryFileName[MAX_PATH];
	WCHAR binaryPath[MAX_PATH];
	WCHAR* filePart = NULL;

	GetModuleFileName(NULL, binaryFileName, sizeof(binaryFileName) / sizeof(binaryFileName[0]));
	GetFullPathName(binaryFileName, sizeof(binaryPath) / sizeof(binaryPath[0]), binaryPath, &filePart);
	filePart[0] = '\0'; // this will cut away the file path bit
	SetCurrentDirectory(binaryPath);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	lstrcat(szTitle, L" ");
	lstrcat(szTitle, ConvertToUnicode(VersionString()).c_str());

	LoadString(hInstance, IDC_WINCONSOLE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINCONSOLE));

	pWinServer.StartThreaded();

	SetTimer(hWnd, TIMER_2, 500, InfoTimer);

	/*BOOL done = FALSE;
	while (!done)
	{
		BOOL hasMsg = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (hasMsg && !TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			if (msg.message == WM_QUIT) done = true;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}*/

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	pWinServer.Shutdown();

	return (int) msg.wParam;
}

void save_exception_buffer(const std::string & buffer) {
	std::ofstream ofs("crashlog_win.txt");
	if (ofs) ofs << buffer;
}

int seh_helper(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow,
	std::string & buffer)
{
	__try {
		return realWinMain(hInstance,hPrevInstance, lpCmdLine, nCmdShow);
	} __except (seh_filter(GetExceptionInformation(), buffer)) {
		if (!buffer.empty()) {
			save_exception_buffer(buffer);
			MessageBoxA(0, buffer.c_str(), "Abnormal Termination", MB_OK);
		}
		return -1;
	}
} 

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	//SymInit sym;
	std::string buffer;
	return seh_helper(hInstance,hPrevInstance, lpCmdLine, nCmdShow, buffer);
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINCONSOLE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BACKGROUND); //COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINCONSOLE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   //HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_EX_COMPOSITED,
   //   CW_USEDEFAULT, 0, 800, 300, NULL, NULL, hInstance, NULL);
   hWnd = CreateWindowEx(WS_EX_COMPOSITED, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
      CW_USEDEFAULT, 0, 800, 300, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

BOOL CreateDialogContents(HWND hWnd)
{
	hwndEdit = CreateWindow(
	//hwndEdit = CreateWindowEx(WS_EX_LAYERED,
		TEXT("EDIT"),
		(LPCWSTR) NULL,        // no window title 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | /*WS_HSCROLL |*/
		ES_LEFT | ES_MULTILINE /*| ES_AUTOVSCROLL*/ | ES_READONLY | ES_NOHIDESEL, 
		0, 0, 0, 0,  // set size in WM_SIZE message 
		hWnd,        // parent window 
		NULL, //(HMENU) ID_EDITCHILD,   // edit control ID 
		hInst,	//(HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE), 
		NULL);       // pointer not needed 

	if (hwndEdit == NULL) return FALSE;

	SendMessage(hwndEdit, WM_SETFONT, (WPARAM) GetStockObject(ANSI_FIXED_FONT), 0); 
	SendMessage(hwndEdit, EM_SETLIMITTEXT, (WPARAM) 0, (LPARAM) 0); // set maximal text size

	HFONT hfont0 = CreateFont(-11, 0, 0, 0, 400, FALSE, FALSE, FALSE, 1, 400, 0, 0, 0, L"Ms Shell Dlg 2");
	hWndInfo = CreateWindowEx(
		0,
		WC_STATIC,
		L"Starting up",
		WS_VISIBLE | WS_CHILD | WS_GROUP | SS_LEFT | SS_SUNKEN,
		0, 0, 10, 10, hWnd, (HMENU)IDC_STATIC, hInst, 0);

	SendMessage(hWndInfo, WM_SETFONT, (WPARAM)hfont0, FALSE);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	//case WM_ERASEBKGND:
	//	return 1;
	//	break;
	case WM_CREATE:
		CreateDialogContents(hWnd);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	
	case WM_SIZE:
		MoveWindow(hwndEdit, 
			0, 0,                  // starting x- and y-coordinates 
			LOWORD(lParam),        // width of client area 
			HIWORD(lParam)-20,        // height of client area 
			TRUE);                 // repaint window 
		MoveWindow(hWndInfo,
			0, HIWORD(lParam)-18, LOWORD(lParam), 18, TRUE);
		return 1;
		break;
	/*case WM_MOVING:
	case WM_SIZING:
		return 1;
		break;*/
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		{
			std::stringstream versioninfo;
			versioninfo << "Measurement server, Version " << VersionString().c_str();
			SendMessage(GetDlgItem(hDlg, IDC_VERSION), WM_SETTEXT, (WPARAM) 0, (LPARAM) ConvertToUnicode(versioninfo.str()).c_str());
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

////

void ConsolePrint(HWND hwndEdit, const std::wstring& data, unsigned int maxLineCount)
{
	SendMessage(hwndEdit, WM_SETREDRAW, (WPARAM) 0, (LPARAM) 0);
	//SendMessage(hWnd, WM_SETREDRAW, (WPARAM) 0, (LPARAM) 0);

	int sp = 0, ep = 0;
	SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&sp, (LPARAM)&ep);	

	int ndx = GetWindowTextLength (hwndEdit);

	SendMessage(hwndEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
	SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM) data.c_str());

	unsigned int lc = Edit_GetLineCount(hwndEdit);
	if (lc > maxLineCount)
	{
		int idx = Edit_LineIndex(hwndEdit, lc - maxLineCount);
		Edit_SetSel(hwndEdit, 0, idx);
		Edit_ReplaceSel(hwndEdit, "");
		sp -= idx;
		if (sp < 0) sp = 0;
		ep -= idx;
		if (ep < 0) ep = 0;
	}

	SendMessage(hwndEdit, EM_SETSEL, (WPARAM)sp, (LPARAM)ep);
	SendMessage(hwndEdit, EM_LINESCROLL, (WPARAM)0, (LPARAM)Edit_GetLineCount(hwndEdit));

	SendMessage(hwndEdit, WM_SETREDRAW, (WPARAM) 1, (LPARAM) 0);
	//SendMessage(hWnd, WM_SETREDRAW, (WPARAM) 1, (LPARAM) 0);

}
