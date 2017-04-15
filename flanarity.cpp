#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include "flanarity.h"
#include "game.h"


const char *configfile = "flanarity.cfg";

static TCHAR winclass[] = _T("flanarityclass");
static TCHAR wintitle[] = _T("flanarity");
static HINSTANCE instance;
static HDC devicectx;
static HGLRC renderctx;


INT_PTR CALLBACK NewGameDlg(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL)
			{
				if (LOWORD(wparam) == IDOK)
					NewGame(100);
				EndDialog(dlg, LOWORD(wparam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK AboutDlg(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL)
			{
				EndDialog(dlg, LOWORD(wparam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

BOOL SetupPixelFormat(HDC dc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;

	int pixelformat = ChoosePixelFormat(dc, &pfd);

	if (pixelformat == 0)
	{
		MessageBox(NULL, _T("Call to ChoosePixelFormat failed!"), _T("Error"), 0);
		return FALSE;
	}

	if (SetPixelFormat(dc, pixelformat, &pfd) == FALSE)
	{
		MessageBox(NULL, _T("Call to SetPixelFormat failed!"), _T("Error"), 0);
		return FALSE;
	}
	
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			devicectx = GetDC(wnd);
			if (!SetupPixelFormat(devicectx))
				PostQuitMessage(0);
			renderctx = wglCreateContext(devicectx);
			wglMakeCurrent(devicectx, renderctx);
		} break;
		
		case WM_COMMAND:
		{
			int wmid = LOWORD(wparam);
			switch (wmid)
			{
				case IDM_NEWGAME:
					DialogBox(instance, MAKEINTRESOURCE(IDD_NEWGAME), wnd, NewGameDlg);
					break;
				case IDM_ABOUT:
					DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUTBOX), wnd, AboutDlg);
					break;
				case IDM_EXIT:
					PostMessage(wnd, WM_CLOSE, 0, 0);
					break;
				default:
					return DefWindowProc(wnd, msg, wparam, lparam);
			}
		} break;

		case WM_CLOSE:
			if (MessageBox(NULL, _T("Really quit???"), _T("Pfff"), MB_OKCANCEL) == IDOK)
			{
				if (renderctx)
					wglDeleteContext(renderctx);
				if (devicectx)
					ReleaseDC(wnd, devicectx);
				renderctx = 0;
				devicectx = 0;
				DestroyWindow(wnd);
			}
			break;
		
		case WM_DESTROY:
			if (renderctx)
				wglDeleteContext(renderctx);
			if (devicectx)
				ReleaseDC(wnd, devicectx);
			PostQuitMessage(0);
			break;
		
		default:
			return DefWindowProc(wnd, msg, wparam, lparam);
			break;
	}

	return 0;
}

int APIENTRY wWinMain(HINSTANCE inst, HINSTANCE previnst, LPWSTR cmdline, int cmdshow)
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);  	
	
	instance = inst;

	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = inst;
	wcex.hIcon = LoadIcon(inst, MAKEINTRESOURCE(IDI_FLANARITY));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FLANARITY);
	wcex.lpszClassName = winclass;
	wcex.hIconSm = LoadIcon(inst, MAKEINTRESOURCE(IDI_SMALL));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(nullptr, _T("Call to RegisterClassEx failed!"), _T("Error"), 0);
		return FALSE;
	}

	HWND wnd = CreateWindowW(winclass, wintitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
		nullptr, nullptr, inst, nullptr);
	if (!wnd)
	{
		MessageBox(nullptr, _T("Call to CreateWindow failed!"), _T("Error"), 0);
		return FALSE;
	}

	if (!InitGame())
		return FALSE;

	ShowWindow(wnd, cmdshow);
	UpdateWindow(wnd);

	InitSettings(configfile);
	NewGame(100);

	HACCEL acceltable = LoadAccelerators(inst, MAKEINTRESOURCE(IDC_FLANARITY));
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, acceltable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			RenderGame();
			SwapBuffers(devicectx);
		}
	}

	//TODO: save after changing settings instead
	SaveSettings(configfile);

	return (int)msg.wParam;
}
