#include "stdafx.h"
#include "flanarity.h"

using namespace std;

static TCHAR winclass[] = _T("flanarityclass");
static TCHAR wintitle[] = _T("flanarity");
static HINSTANCE instance;
static HDC devicectx;
static HGLRC renderctx;

GLint uniTime;

float vertices[] = {
	-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
};

GLuint elements[] = {
	0, 1, 2,
	2, 3, 0
};

void LoadTextFile(char *file, const char **data)
{
	ifstream f(file);
	string line, str;
	while (getline(f, line))
		str += line + "\n";
	*data = _strdup(str.c_str());
}


BOOL OpenGLSetup()
{
	glewExperimental = GL_TRUE;
	glewInit();
	
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	
	const char *vshdata;
	LoadTextFile("test.vsh", &vshdata);
	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vshdata, nullptr);
	glCompileShader(vshader);
	
	const char *pshdata;
	LoadTextFile("test.psh", &pshdata);
	GLuint pshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(pshader, 1, &pshdata, nullptr);
	glCompileShader(pshader);
	
	GLuint shprogram = glCreateProgram();
	glAttachShader(shprogram, vshader);
	glAttachShader(shprogram, pshader);
	glBindFragDataLocation(shprogram, 0, "outColor");
	glLinkProgram(shprogram);
	glUseProgram(shprogram);
	
	GLint posattrib = glGetAttribLocation(shprogram, "position");
	glEnableVertexAttribArray(posattrib);
	glVertexAttribPointer(posattrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
	GLint colattrib = glGetAttribLocation(shprogram, "color");
	glEnableVertexAttribArray(colattrib);
	glVertexAttribPointer(colattrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(2 * sizeof(float)));
	GLint texattrib = glGetAttribLocation(shprogram, "texcoord");
	glEnableVertexAttribArray(texattrib);
	glVertexAttribPointer(texattrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(5 * sizeof(float)));
	
	GLuint textures[2];
	glGenTextures(2, textures);
	
	int width, height;
	unsigned char *image;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shprogram, "texKitten"), 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shprogram, "texPuppy"), 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	uniTime = glGetUniformLocation(shprogram, "time");

	return TRUE;
}

void Render(float time)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(uniTime, time);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	SwapBuffers(devicectx);
}


INT_PTR CALLBACK About(HWND dlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	UNREFERENCED_PARAMETER(lparam);
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
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;

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

LRESULT CALLBACK WndProc(_In_ HWND wnd, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam)
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
			case IDM_ABOUT:
				DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUTBOX), wnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(wnd);
				break;
			default:
				return DefWindowProc(wnd, msg, wparam, lparam);
			}
		} break;
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(wnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(wnd, &ps);
		} break;
		
		case WM_CLOSE:
			if (renderctx)
				wglDeleteContext(renderctx);
			if (devicectx)
				ReleaseDC(wnd, devicectx);
			renderctx = 0;
			devicectx = 0;
			DestroyWindow(wnd);
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

int APIENTRY wWinMain(_In_ HINSTANCE inst, _In_opt_ HINSTANCE previnst, _In_ LPWSTR cmdline, _In_ int cmdshow)
{
    UNREFERENCED_PARAMETER(previnst);
    UNREFERENCED_PARAMETER(cmdline);

	instance = inst;

	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
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

	if (!OpenGLSetup())
		return FALSE;

	ShowWindow(wnd, cmdshow);
	UpdateWindow(wnd);

	auto t_start = chrono::high_resolution_clock::now();

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
			auto t_now = chrono::high_resolution_clock::now();
			float time = chrono::duration_cast<chrono::duration<float>>(t_now - t_start).count();
			Render(time);
		}
	}

	return (int)msg.wParam;
}
