#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
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

int selected_node = -1;


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
					NewGame();
				EndDialog(dlg, LOWORD(wparam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK SettingsDlg(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL)
			{
				if (LOWORD(wparam) == IDOK)
				{
					
					UpdateSettings();
				}					
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
		{
			if (LOWORD(wparam) == IDOK || LOWORD(wparam) == IDCANCEL)
			{
				EndDialog(dlg, LOWORD(wparam));
				return (INT_PTR)TRUE;
			}
		} break;
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
				case IDM_SETTINGS:
					DialogBox(instance, MAKEINTRESOURCE(IDD_SETTINGS), wnd, SettingsDlg);
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

		case WM_LBUTTONDOWN:
		{
			int xpos = GET_X_LPARAM(lparam);
			int ypos = GET_Y_LPARAM(lparam);
			float nodex = (float)(xpos - g_clientwidth/2.0f + g_clientposx);
			float nodey = (float)(ypos - g_clientheight/2.0f + g_clientposy);
			selected_node = FindNode(nodex, nodey);
			if (selected_node != -1)
				printf("selected node: %d\n", selected_node);
		} break;

		case WM_LBUTTONUP:
			if (selected_node != -1)
			{
				UpdateNodeTangle(selected_node);
				if (CheckUntangled())
					printf("untangled!\n");
				selected_node = -1;
			}
			break;

		case WM_MOUSEMOVE:
			if ((wparam == MK_LBUTTON) && (selected_node != -1))
			{
				int xpos = GET_X_LPARAM(lparam);
				int ypos = GET_Y_LPARAM(lparam);
				float nodex = (float)(xpos - g_clientwidth/2.0f + g_clientposx);
				float nodey = (float)(ypos - g_clientheight/2.0f + g_clientposy);
				SetNode(selected_node, nodex, nodey);
			}
			break;

		case WM_SIZE:
		{
			g_clientwidth = lparam & 0xffff;
			g_clientheight = lparam >> 16;
			RECT r;
			GetWindowRect(wnd, &r);
			g_winwidth = r.right - r.left;
			g_winheight = r.bottom - r.top;
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
	FILE *out;
	AllocConsole();
	freopen_s(&out, "CONOUT$", "w", stdout);
	freopen_s(&out, "CONOUT$", "w", stderr);  	
	
	instance = inst;

	InitSettings(configfile);

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

	HWND wnd = CreateWindowW(winclass, wintitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, g_winwidth, g_winheight,
		nullptr, nullptr, inst, nullptr);
	if (!wnd)
	{
		MessageBox(nullptr, _T("Call to CreateWindow failed!"), _T("Error"), 0);
		return FALSE;
	}

	RECT r;
	GetClientRect(wnd, &r);
	g_clientwidth = r.right - r.left;
	g_clientheight = r.bottom - r.top;

	if (!InitGame())
		return FALSE;

	ShowWindow(wnd, cmdshow);
	UpdateWindow(wnd);

	NewGame();

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
