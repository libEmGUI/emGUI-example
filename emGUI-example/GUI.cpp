/*
	This is main GUI source

*/
#include "GUI.h"

using namespace std;
using namespace Gdiplus;



static LCD_Glue LCD;
static xInterface * interface1;
static xLabel * mouseMonitor;
static HDC hdc_tmp;
xTouchEvent currentTouch;

// Convert color from emGUI to GDI format
ARGB convertColor(uint16_t color) {
	uint8_t usRed = ((color >> 11) & 0b00011111) * 8;
	uint8_t usGreen = ((color >> 5) & 0b00111111) * 4;
	uint8_t usBlue = ((color & 0b00011111)) * 8;

	ARGB answer = (ARGB)((ARGB)0xFF000000 | (usRed << 16) | (usGreen << 8) | usBlue);
	return answer;

}


extern "C" {
	void vFramebufferRectangle(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, uint16_t usColor, bool bFill) {
		Graphics graphics(hdc_tmp);
		Pen      pen(Color(convertColor(usColor)));
		graphics.DrawRectangle(&pen, usX0, usY0, usX1 - usX0, usY1 - usY0);
		if (bFill) {
			SolidBrush solidBrush(Color(convertColor(usColor)));
			graphics.FillRectangle(&solidBrush, usX0, usY0, usX1 - usX0, usY1 - usY0);
		}
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
					vGUIdrawPixel(usXt, usYt, usColor);
				else if (bFillBg)
					vGUIdrawPixel(usXt, usYt, usBackground);
				if (usYt >= ILI9341_TFTHEIGHT)
					break;
			};

			/* Hack for 16X NEO font */
			if (usHeight == 16) {
				for (uint8_t row = 0; row < 8; row++) {
					usYt = usY + row + 8;
					if (*(pubBuf + charWidth)& (1 << row))
						vGUIdrawPixel(usXt, usYt, usColor);
					else if (bFillBg)
						vGUIdrawPixel(usXt, usYt, usBackground);
					if (usYt >= ILI9341_TFTHEIGHT)
						break;
				};
			}
			pubBuf++;
		}
	}
	void vFramebufferVLine(uint16_t usX0, uint16_t usY0, uint16_t usY1, uint16_t usColor) {
		Graphics graphics(hdc_tmp);
		Pen      pen(Color(convertColor(usColor)));
		graphics.DrawLine(&pen, usX0, usY0, usX0, usY1);
	}
	void vFramebufferHLine(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usColor) {
		Graphics graphics(hdc_tmp);
		Pen      pen(Color(convertColor(usColor)));
		graphics.DrawLine(&pen, usX0, usY0, usX1, usY0);
	}
	void bFramebufferPicture(int16_t sX0, int16_t sY0, unsigned short const* pusPicture) {
		int16_t i, j;
		uint16_t x = 2;

		for (j = 0; j < pusPicture[1]; j++) {
			for (i = 0; i < pusPicture[0]; i++) {
				vGUIdrawPixel(sX0 + j, sY0 + i, pusPicture[x]);
				x++;
			}
		}
	}
}


// Action on interface creatings
bool bGUIonInterfaceCreateHandler(xWidget *) {
	auto window = pxWindowCreate(WINDOW_MENU);
	vWindowSetHeader(window, "Wnd1");
	vWidgetSetBgColor(window, 0xFF, false);
	auto l1 = pxLabelCreate(1, 1, 238, 60, "hypothetical rosters of players \
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
    to determine consensus All-Americans.[5]", FONT_ASCII_8_X, 1010, window);

	mouseMonitor = pxLabelCreate(1, 200, 238, 0, "x:   y:   ", FONT_ASCII_8_X, 500, window);
	auto b1 = pxButtonCreate(60, 100, rgb_test, window);

	auto window2 = pxWindowCreate(WINDOW_ABOUT);
	auto labelAbout = pxLabelCreate(1, 1, 238, 60, "This is Demo for emGUI. 2017", FONT_ASCII_8_X, 50, window2);

	vWindowSetOnCloseRequestHandler(window2, &bGUIOnWindowCloseHandler);
	vInterfaceOpenWindow(WINDOW_ABOUT);
	vInterfaceOpenWindow(WINDOW_MENU);
	return true;
}


bool bGUIOnWindowCloseHandler(xWidget *) {
	vInterfaceOpenWindow(WINDOW_MENU);
	return true;
}

bool bGUI_InitInterfce() {
	interface1 = pxInterfaceCreate(&bGUIonInterfaceCreateHandler);
	LCD.vFramebufferRectangle = &vFramebufferRectangle;
	LCD.vFramebufferPutChar = &vFramebufferPutChar;
	LCD.bFramebufferPicture = &bFramebufferPicture;
	LCD.vFramebufferVLine = &vFramebufferVLine;
	LCD.vFramebufferHLine = &vFramebufferHLine;
	vWidgetSetLCD(&LCD);
	return true;
}

void vGUIdrawPixel(uint16_t x, uint16_t y, uint16_t color){
	Graphics graphics(hdc_tmp);
	SolidBrush solidBrush(Color(convertColor(color)));
	graphics.FillRectangle(&solidBrush, x, y, 1, 1);
}

void vGUIsetCurrentHDC(HDC a) {
	hdc_tmp = a;
}


void vGUIpaintEventHandler() {
	//vInterfaceInvalidate();
	vInterfaceDraw();
}

void vGUIpushClickHandler(LPARAM lParam) {
	char outString[25];
	sprintf_s(outString, "x: %d y: %d\n", LOWORD(lParam), HIWORD(lParam));
	pcLabelSetText(mouseMonitor, outString);
	currentTouch.eventTouchScreen = pushTs;
	currentTouch.xTouchScreen = LOWORD(lParam);
	currentTouch.yTouchScreen = HIWORD(lParam);
	bInterfaceCheckTouchScreenEvent(&currentTouch);
	
}
void vGUIpopClickHandler(LPARAM lParam) {
	currentTouch.eventTouchScreen = popTs;
	currentTouch.xTouchScreen = LOWORD(lParam);
	currentTouch.yTouchScreen = HIWORD(lParam);
	bInterfaceCheckTouchScreenEvent(&currentTouch);
}

void vGUIeraseBackgroudHandler() {
	vInterfaceInvalidate();
}
