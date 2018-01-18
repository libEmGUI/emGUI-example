// emGUI-example.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "emGUI-example.h"
#include "emGUIGlue.h"
#include <cstdio>
#include <iostream>

#include "resource.h"
#include "GUI.h" 

#include"PlotDataCollector.h"

#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> logger;

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND hWnd;
UINT_PTR uTimerId;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	

	try
	{
		// Create basic file logger (not rotated)
		logger = spdlog::basic_logger_mt("basic_logger", "app.log");
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log initialization failed: " << ex.what() << std::endl;
	}
	
	logger->info("Hello!");

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	bGUI_InitInterfce(); //TODO: check false

	pxWindowManagerCreate(bGUIonWindowManagerCreateHandler);

	serialThreadParams_t threadParams;

	threadParams.portName = L"COM9";
	threadParams.exitFlag = false;
	threadParams.logger = logger;
	threadParams.data = pxGUIGetPlotData();
	threadParams.extraParams = pxGUIGetExtraParams();

	PlotDataCollectorStart(&threadParams);

	// Инициализация глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EMGUIEXAMPLE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Выполнить инициализацию приложения:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMGUIEXAMPLE));

	MSG msg;

	// Цикл основного сообщения:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	logger->info("Leaving main thread");
	threadParams.exitFlag = true;
	WaitForSingleObject(getSerialThreadHandle(), INFINITE);
	logger->info("Serial thread should be dead!");
	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}

//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  НАЗНАЧЕНИЕ: регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EMGUIEXAMPLE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_EMGUIEXAMPLE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   НАЗНАЧЕНИЕ: сохраняет обработку экземпляра и создает главное окно.
//
//   КОММЕНТАРИИ:
//
//        В данной функции дескриптор экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится на экран главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переме
	DWORD dwStyle = WS_BORDER;
	HWND hWnd = CreateWindowW(szWindowClass, 0, dwStyle,
		CW_USEDEFAULT, 0, EMGUI_LCD_WIDTH, EMGUI_LCD_HEIGHT, nullptr, nullptr, hInstance, nullptr);
	SetWindowLong(hWnd, GWL_STYLE, WS_BORDER);     // stupid winapi maigic. You should reset all styles and set style again
	RECT rc;

	GetWindowRect(hWnd, &rc);

	int xPos = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
	int yPos = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;

	SetWindowPos(hWnd, 0, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	if (!hWnd)
	{
		return FALSE;
	}
	uTimerId = SetTimer(hWnd, 1, 30, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  НАЗНАЧЕНИЕ:  обрабатывает сообщения в главном окне.
//
//  WM_COMMAND — обработать меню приложения
//  WM_PAINT — отрисовать главное окно
//  WM_DESTROY — отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		/*case WM_COMMAND:
			//{
			  //  int wmId = LOWORD(wParam);
				//DestroyWindow(hWnd);
			}
			break;*/
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc_tmp = BeginPaint(hWnd, &ps);
		Graphics gr_tmp(hdc_tmp);
		vGUIsetCurrentHDC(&gr_tmp);
		vWindowManagerDraw();
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_KEYDOWN:
		vGUIHandleKeypress(wParam);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, uTimerId);
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		vGUIMouseHandler(lParam, pushTs);
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, NULL, NULL);
		return 0;
	case WM_TIMER:
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, NULL, NULL);
		return 0;
	case WM_MOUSEMOVE:
		vGUIMouseHandler(lParam, updateTs);
		return 0;
	case WM_LBUTTONUP:
		vGUIMouseHandler(lParam, popTs);
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, NULL, NULL);
		return 0;
	case WM_ERASEBKGND:
		vWindowManagerInvalidate();
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
