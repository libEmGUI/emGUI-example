/*
	This is main GUI source

*/
#include "GUI.h"

using namespace std;
using namespace Gdiplus;



static xDraw_t LCD;
static xInterface * interface1;
static xLabel * mouseMonitor;
static Graphics *graphics = NULL;
xTouchEvent currentTouch;

static int stride = 0;
BYTE * imgCross = (BYTE *)&rgb_test;
BYTE * imgCross2 = (BYTE *)&rgb_test[23];

// Convert color from emGUI to GDI format
ARGB convertColor(uint16_t color) {
	uint8_t usRed = ((color >> 11) & 0b00011111) * 8;
	uint8_t usGreen = ((color >> 5) & 0b00111111) * 4;
	uint8_t usBlue = ((color & 0b00011111)) * 8;

	ARGB answer = (ARGB)((ARGB)0xFF000000 | (usRed << 16) | (usGreen << 8) | usBlue);
	return answer;

}


extern "C" {
	void vRectangle(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, uint16_t usColor, bool bFill) {
		Pen      pen(Color(convertColor(usColor)));
		graphics->DrawRectangle(&pen, usX0, usY0, usX1 - usX0, usY1 - usY0);
		if (bFill) {
			SolidBrush solidBrush(Color(convertColor(usColor)));
			graphics->FillRectangle(&solidBrush, usX0, usY0, usX1 - usX0, usY1 - usY0);
		}
	}
	void vPutChar(uint16_t usX, uint16_t usY, char ASCI, xFont pubFont, uint16_t usColor, uint16_t usBackground, bool bFillBg) {
		unsigned const char  *pubBuf = pubFont[(int)ASCI];
		unsigned char charWidth = *pubBuf; //each symbol in NEO fonts contains width info in first byte.
		unsigned char usHeight = *pubFont[0]; //each NEO font contains height info in first byte.
		pubBuf++; //go to first pattern of font
		uint16_t usXt, usYt;
		SolidBrush fgBrush(Color(convertColor(usColor)));
		SolidBrush bgBrush(Color(convertColor(usBackground)));
		for (uint8_t column = 0; column < charWidth; column++) {
			usXt = usX + column;
			if (usXt >= SCREEN_WIDTH)
				break;
			for (uint8_t row = 0; row < 8; row++) {
				usYt = usY + row;
				if (*pubBuf & (1 << row))
					graphics->FillRectangle(&fgBrush, usXt, usYt, 1, 1);
				else if (bFillBg)
					graphics->FillRectangle(&bgBrush, usXt, usYt, 1, 1);
				if (usYt >= SCREEN_HEIGHT)
					break;
			};

			/* Hack for 16X NEO font */
			if (usHeight == 16) {
				for (uint8_t row = 0; row < 8; row++) {
					usYt = usY + row + 8;
					if (*(pubBuf + charWidth)& (1 << row))
						graphics->FillRectangle(&fgBrush, usXt, usYt, 1, 1);
					else if (bFillBg)
						graphics->FillRectangle(&bgBrush, usXt, usYt, 1, 1);
					if (usYt >= SCREEN_HEIGHT)
						break;
				};
			}
			pubBuf++;
		}
	}
	void vVLine(uint16_t usX0, uint16_t usY0, uint16_t usY1, uint16_t usColor) {
		Pen      pen(Color(convertColor(usColor)));
		graphics->DrawLine(&pen, usX0, usY0, usX0, usY1);
	}
	void vHLine(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usColor) {
		Pen      pen(Color(convertColor(usColor)));
		graphics->DrawLine(&pen, usX0, usY0, usX1, usY0);
	}
	void bPicture(int16_t sX0, int16_t sY0, unsigned short const* pusPicture) {
		BYTE * imgC = (BYTE *)pusPicture;
		Bitmap cross_pic_bitmap(pusPicture[0], pusPicture[1], pusPicture[0] * sizeof(uint16_t), PixelFormat16bppRGB565, imgC + sizeof(uint16_t) * 2);
		static float angle = 0.0;
		cross_pic_bitmap.RotateFlip(Rotate90FlipX);
		graphics->DrawImage(&cross_pic_bitmap, sX0, sY0);
	}
	bool btn2Handler(xWidget *) {
		char outString[25];
		static int i = 0;
		i++;
		sprintf_s(outString, "pushed: %d times", i);
		pcLabelSetText(mouseMonitor, outString);
		return true;
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
		auto b1 = pxButtonCreate(1, 70, rgb_test, window);

		auto window2 = pxWindowCreate(WINDOW_ABOUT);
		vWindowSetHeader(window2, "Wnd2");
		auto labelAbout = pxLabelCreate(1, 1, 238, 60, "This is Demo for emGUI. 2017", FONT_ASCII_8_X, 50, window2);
		vWindowSetOnCloseRequestHandler(window2, &bGUIOnWindowCloseHandler);

		auto menuBut = pxMenuButtonCreate(120, 70, rgb_test, "push me", &btn2Handler, window);

		vInterfaceOpenWindow(WINDOW_ABOUT);
		vInterfaceOpenWindow(WINDOW_MENU);
		return true;
	}
}

bool bGUIOnWindowCloseHandler(xWidget *) {
	vInterfaceOpenWindow(WINDOW_MENU);
	return true;
}

bool bGUI_InitInterfce() {
	interface1 = pxInterfaceCreate(&bGUIonInterfaceCreateHandler);

	LCD.vRectangle = &vRectangle;
	LCD.vPutChar = &vPutChar;
	LCD.bPicture = &bPicture;
	LCD.vVLine = &vVLine;
	LCD.vHLine = &vHLine;
	vDrawSetHandler(&LCD);
	return true;
}


void vGUIsetCurrentHDC(Graphics *gr) {
	graphics = gr;
}


void vGUIpaintEventHandler() {
	//vInterfaceInvalidate();
	vInterfaceDraw();
}

void vGUIpushClickHandler(LPARAM lParam) {
	char outString[25];
	sprintf_s(outString, "x: %d y: %d\n", LOWORD(lParam), HIWORD(lParam));
	//pcLabelSetText(mouseMonitor, outStri);
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


