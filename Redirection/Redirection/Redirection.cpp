#include "stdafx.h"
#include "Redirection.h"
#include <string>
#include <vector>

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HANDLE g_hChildStdOutRead = NULL;
std::string g_str = _T("Plz click \"Tool\" -> \"CreateSubProcess\"");

#define BUFSIZE 4096 

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_REDIRECTION, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REDIRECTION));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REDIRECTION));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_REDIRECTION);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

// https://docs.microsoft.com/en-us/windows/win32/ProcThread/creating-a-child-process-with-redirected-input-and-output
bool CreateRedirectedChildProcess(HWND hWnd)
{
	HANDLE hChildStdInRead = NULL;
	HANDLE hChildStdInWrite = NULL;
	HANDLE hChildStdOutWrite = NULL;

	SECURITY_ATTRIBUTES saAttr;

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&g_hChildStdOutRead, &hChildStdOutWrite, &saAttr, 0))
		return false;

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(g_hChildStdOutRead, HANDLE_FLAG_INHERIT, 0))
		return false;

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&hChildStdInRead, &hChildStdInWrite, &saAttr, 0))
		return false;

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, 0))
		return false;

	// Create the child process. 

	TCHAR szCmdline[] = TEXT("child");
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = hChildStdOutWrite;
	siStartInfo.hStdOutput = hChildStdOutWrite;
	siStartInfo.hStdInput = hChildStdInRead;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(NULL,
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

					   // If an error occurs, exit the application. 
	if (!bSuccess)
		return false;
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

		// Close handles to the stdin and stdout pipes no longer needed by the child process.
		// If they are not explicitly closed, there is no way to recognize that the child process has ended.

		CloseHandle(hChildStdOutWrite);
		CloseHandle(hChildStdInRead);
	}

	SetTimer(hWnd, 0, 1000, NULL);

	return true;
}

std::vector<std::string> Split(const std::string _input)
{
	std::vector<std::string> splited;

	size_t ex = 0;
	while (true)
	{
		auto i = _input.find(_T("\r\n"), ex);
		if (i == std::string::npos)
			break;

		splited.push_back(_input.substr(ex, i - ex));
		ex = i + 2;
	}

	return splited;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_TIMER:
		{
			DWORD dwRead, dwWritten;
			CHAR chBuf[BUFSIZE] = { 0, };
			
			bool bSuccess = ReadFile(g_hChildStdOutRead, chBuf, BUFSIZE, &dwRead, NULL);
			if (!bSuccess || dwRead == 0)
			{
				g_str = _T("Finished");
				InvalidateRect(hWnd, nullptr, true);
				break;
			}

			auto infos = Split(chBuf);
			// Paint only last output
			if (!infos.empty())
			{
				g_str = infos.back();
			}
			
			ZeroMemory(chBuf, BUFSIZE);
			InvalidateRect(hWnd, nullptr, true);
		}
		break;
	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			switch (wmId)
			{
			case IDM_CREATE_CHILD:	// CreateSubProcess
				CreateRedirectedChildProcess(hWnd);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			
			TextOut(hdc, 50, 150, g_str.data(), g_str.size());
			
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
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
