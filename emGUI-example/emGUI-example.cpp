// emGUI-example.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "emGUI-example.h"
#include "emGUI.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include "pics.h"



using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define MAX_LOADSTRING 100

LCD_Glue LCD;
HDC hdc_tmp;
xWidget* wdg;

#define ILI9341_TFTWIDTH 240
#define ILI9341_TFTHEIGHT 240

ARGB convertColor(uint16_t color){
	uint8_t usRed =   ((color >> 11) & 0b00011111) * 8;
	uint8_t usGreen = ((color >> 5)  & 0b00111111) * 4;
	uint8_t usBlue =  ((color & 0b00011111)) * 8;

	ARGB answer = (ARGB)((ARGB)0xFF000000 | (usRed << 16) | (usGreen << 8) | usBlue);
	return answer;

}



void drawPixel(uint16_t x, uint16_t y, uint16_t color) {
	Graphics graphics(hdc_tmp);
	SolidBrush solidBrush(Color(convertColor(color)));
	graphics.FillRectangle(&solidBrush, x, y, 1, 1);
}
extern "C" {
	void vFramebufferRectangle(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, uint16_t usColor, bool bFill) {
		Graphics graphics(hdc_tmp);
		Pen      pen(Color(convertColor(usColor)));
		graphics.DrawRectangle(&pen, usX0, usY0, usX1- usX0, usY1-usY0);
	}
	void vFramebufferPutChar(uint16_t usX, uint16_t usY, char ASCI, xFont pubFont, uint16_t usColor, uint16_t usBackground, bool bFillBg) {
		unsigned const char  *pubBuf = pubFont[(int)ASCI];
		unsigned char charWidth = *pubBuf; //each symbol in NEO fonts contains width info in first byte.
		unsigned char usHeight = *pubFont[0]; //each NEO font contains height info in first byte.
		pubBuf++; //go to first pattern of font
		uint16_t usXt, usYt;
	
		for (uint8_t column = 0; column < charWidth; column++) {
			usXt = usX + column;
			if (usXt >= ILI9341_TFTWIDTH)
				break;
			for (uint8_t row = 0; row < 8; row++) {
				usYt = usY + row;
				if (*pubBuf & (1 << row))
					drawPixel(usXt, usYt, usColor);
				else if (bFillBg)
					drawPixel(usXt, usYt, usBackground);
				if (usYt >= ILI9341_TFTHEIGHT)
					break;
			};

			/* Hack for 16X NEO font */
			if (usHeight == 16) {
				for (uint8_t row = 0; row < 8; row++) {
					usYt = usY + row + 8;
					if (*(pubBuf + charWidth)& (1 << row))
						drawPixel(usXt, usYt, usColor);
					else if (bFillBg)
						drawPixel(usXt, usYt, usBackground);
					if (usYt >= ILI9341_TFTHEIGHT)
						break;
				};
			}
			pubBuf++;
		}
	}
	void vFramebufferHLine(uint16_t usX0, uint16_t usY0, uint16_t usY1, uint16_t usColor) {

	}
	void vFramebufferVLine(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usColor) {

	}
	void bFramebufferPicture(int16_t sX0, int16_t sY0, unsigned short const* pusPicture) {
		int16_t i, j;
		uint16_t x = 2;

		for (j = 0; j < pusPicture[1]; j++) {
			for (i = 0; i < pusPicture[0]; i++) {
				drawPixel(sX0 + j, sY0 + i, pusPicture[x]);
				x++;
			}
		}
	}
}

void onPaint() {
	static uint16_t x = 1;
	static uint16_t y = 1;
	x++;
	y++;
	drawPixel(x, y, 0xFF00);
}
// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
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

    // TODO: разместите код здесь.

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	wdg = pxWidgetCreate(0, 0, 240, 240, NULL, false);
	vWidgetSetTransparency(wdg, false);
	auto l1 = pxLabelCreate(15, 15, 200, 60, "hypothetical rosters of players \
		considered the best in the nation at their respective positions\
		The National Collegiate Athletic Association, a college sports \
		governing body, uses officially recognized All-America selectors \
		to determine the consensus selections. These are based on a point \
		system in which a player is awarded three points for every selector \
		that names him to the first team, two points for the second team, \
		and one point for the third team. The individual who receives the \
		most points at his position is called a consensus All-American.[4] \
		Over time, the sources used to determine the consensus selections \
		have changed, and since 2002, the NCAA has used five selectors, \
		the Associated Press (AP), American Football Coaches Association \
		(AFCA), Football Writers Association of America (FWAA), The Sporting \
		News (TSN), and the Walter Camp Football Foundation (WCFF),   \
		to determine consensus All-Americans.[5]", FONT_ASCII_8_X, 1010, wdg);
	auto b1 = pxButtonCreate(60, 100, rgb_test, wdg);
	
	// Initialize GDI+
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	LCD.vFramebufferRectangle = &vFramebufferRectangle;
	LCD.vFramebufferPutChar = &vFramebufferPutChar;
	LCD.bFramebufferPicture = &bFramebufferPicture;
	LCD.vFramebufferVLine = &vFramebufferVLine;
	LCD.vFramebufferHLine = &vFramebufferHLine;
	vWidgetSetLCD(&LCD);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EMGUIEXAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
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

	GdiplusShutdown(gdiplusToken);
    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EMGUIEXAMPLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EMGUIEXAMPLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

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
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
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
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;

            hdc_tmp = BeginPaint(hWnd, &ps);
			
			onPaint();
			vWidgetInvalidate(wdg);
			vWidgetDraw(wdg);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
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

// Обработчик сообщений для окна "О программе".
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
