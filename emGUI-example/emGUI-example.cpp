// emGUI-example.cpp: определ€ет точку входа дл€ приложени€.
//

#include "stdafx.h"
#include "emGUI-example.h"

#include <cstdio>
#include <iostream>

#include <io.h>

#include <fcntl.h>
#include <process.h>
#define SERIAL_READ_TIMEOUT 500 
#define SERIAL_READ_BUF_SIZE 10

#define TIMEOUT 1000

#include <spdlog/spdlog.h>

using namespace std;

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define MAX_LOADSTRING 100

#ifndef DEBUG_ZONE
#define DEBUG_ZONE 0;
#endif

HANDLE serialPort;
HANDLE serialThread;
unsigned int  dwThreadId;
// √лобальные переменные:
HINSTANCE hInst;                                // текущий экземпл€р
WCHAR szTitle[MAX_LOADSTRING];                  // “екст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // им€ класса главного окна
HWND hWnd;

std::shared_ptr<spdlog::logger> logger = nullptr;

typedef struct {
	LPCWSTR portName;
	bool exitFlag;
	uint16_t data[100];
	bool        bDataFilled;      ///< флажок, показывающий, что данные заполнены (сн€ты хот€ бы в 1 проход)
	uint32_t    ulElemCount;      ///< количество отсчЄтов в текущей сессии записи
	uint32_t    ulCapacity;       ///< максимальное количество отсчЄтов в буфере
	uint32_t    ulWritePos;       ///< позици€ указател€ записи в буфере

	std::shared_ptr<spdlog::logger> logger;
}serialThreadParams_t;


// ќтправить объ€влени€ функций, включенных в этот модуль кода:
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

	serialThreadParams_t threadParams;

	threadParams.portName = L"COM9";
	threadParams.exitFlag = false;
	threadParams.logger = logger;

	// TODO: разместите код здесь.
	serialThread = (HANDLE)_beginthreadex(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		&SecondThreadFunc,       // thread function name
		&threadParams,                   // argument to thread function 
		0,                      // use default creation flags 
		&dwThreadId);   // returns the thread identifier 
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	bGUI_InitInterfce(); //TODO: check false

	// »нициализаци€ глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_EMGUIEXAMPLE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ¬ыполнить инициализацию приложени€:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMGUIEXAMPLE));

	MSG msg;

	// ÷икл основного сообщени€:
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
	WaitForSingleObject(serialThread, INFINITE);
	logger->info("Serial thread should be dead!");
	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}

void HandleASuccessfulRead(char * lpBuf, WORD dwRead, uint16_t * data_array) {
	for (int i= 0; i < dwRead; i++)
		HandleAChar(lpBuf[i], data_array);
}

void HandleAChar(char sym, uint16_t * data_array) {
	static char buffer[100];
	static uint8_t i = 0;
	
	if (sym != '\n') {
		buffer[i] = sym;
		i++;
	}
	else {
		string temp(buffer);
		//buffer[i] = '\0';
		i = 0;
		for (int i = 0; i<100; i++)buffer[i] = 0;
		try {
			int data = std::stoi(temp);
			handleData(data, data_array);
		}
		catch (std::invalid_argument& e) {
			;// logger->info("Invalid string for float parcing came from Serial");
		}
	}
}

void handleData(int data, uint16_t * data_array) {
	logger->info(data);
}

//
//  ‘”Ќ ÷»я: MyRegisterClass()
//
//  Ќј«Ќј„≈Ќ»≈: регистрирует класс окна.
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
//   ‘”Ќ ÷»я: InitInstance(HINSTANCE, int)
//
//   Ќј«Ќј„≈Ќ»≈: сохран€ет обработку экземпл€ра и создает главное окно.
//
//    ќћћ≈Ќ“ј–»»:
//
//        ¬ данной функции дескриптор экземпл€ра сохран€етс€ в глобальной переменной, а также
//        создаетс€ и выводитс€ на экран главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // —охранить дескриптор экземпл€ра в глобальной переме
	DWORD dwStyle = WS_BORDER;
	HWND hWnd = CreateWindowW(szWindowClass, 0, dwStyle,
		CW_USEDEFAULT, 0, SCREEN_WIDTH, SCREEN_HEIGHT, nullptr, nullptr, hInstance, nullptr);
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
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

unsigned __stdcall SecondThreadFunc(void* pArguments) {
	auto prm = (serialThreadParams_t *)pArguments;
	if (!prm)
		return 1;

	auto logger = prm->logger;
	serialPort = CreateFile(prm->portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);
	if (serialPort == INVALID_HANDLE_VALUE) {
		return 2;
	}

	static DWORD dwRead;
	static BOOL fWaitingOnRead = FALSE;
	static OVERLAPPED osReader = { 0 };
	static DWORD dwRes;
	static char lpBuf[10];
	logger->info("Starting serial read thread from {}", (char *) prm->portName);
	Sleep(100);

	while (!prm->exitFlag) {
		if (serialPort == INVALID_HANDLE_VALUE) {
			logger->error("Port otvalilsa");
		}

		// Create the overlapped event. Must be closed before exiting
		// to avoid a handle leak.
		osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (osReader.hEvent == NULL) { 
			logger->error("hEvent not created");
		}// Error creating overlapped event; abort.

		
		// Issue read operation.
		while (ReadFile(serialPort, lpBuf, 1, &dwRead, &osReader)) {
			// read completed immediately
			logger->info("Read 1 byte");
			HandleASuccessfulRead(lpBuf, dwRead, (uint16_t *)&prm->data);
		}
		if (GetLastError() != ERROR_IO_PENDING) {
			logger->error("Got some error during read: {}", GetLastError());
		}    // read not delayed?  Error in communications; report it.
		else {
			logger->info("Go to sleep");
			Sleep(100);
		}
	}
	logger->info("Leaving serial read thread");
	CloseHandle(serialPort);
	serialPort = INVALID_HANDLE_VALUE;
	;// throw TTYException();
	return 0;
}
//
//  ‘”Ќ ÷»я: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ќј«Ќј„≈Ќ»≈:  обрабатывает сообщени€ в главном окне.
//
//  WM_COMMAND Ч обработать меню приложени€
//  WM_PAINT Ч отрисовать главное окно
//  WM_DESTROY Ч отправить сообщение о выходе и вернутьс€
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
		vGUIpaintEventHandler();
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		vGUIpushClickHandler(lParam);
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, NULL, NULL);
		return 0;

	case WM_LBUTTONUP:
		vGUIpopClickHandler(lParam);
		InvalidateRect(hWnd, NULL, FALSE);
		SendMessage(hWnd, WM_PAINT, NULL, NULL);
		return 0;
	case WM_ERASEBKGND:
		vGUIeraseBackgroudHandler();
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

