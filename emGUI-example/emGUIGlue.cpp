#include "emGUIGlue.h"

#include <windows.h>
#include <objidl.h>
#include <iostream>

using namespace Gdiplus;

static Graphics *graphics = NULL;
xTouchEvent currentTouch;

#define PROGMEM

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#include "Adafruit-GFX/Fonts/FreeSans9pt7b.h"

using namespace std;
using namespace Gdiplus;

static xDraw_t LCD;

// Convert color from emGUI to GDI format
static ARGB convertColor(uint16_t color, uint8_t alpha = 0xFF) {
	uint8_t usRed = ((color >> 11) & 0b00011111) * 800 / 97;
	uint8_t usGreen = (uint32_t)((uint32_t)(color >> 5) & 0b0000111111) * 4000 / 985;
	uint8_t usBlue = ((color & 0b00011111)) * 800 / 97;

	ARGB answer = (ARGB)((ARGB)(alpha << 24) | (usRed << 16) | (usGreen << 8) | usBlue);
	return answer;

}

static void vRectangle(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usY1, uint16_t usColor, bool bFill) {
	Pen      pen(Color(convertColor(usColor)));
	graphics->DrawRectangle(&pen, usX0, usY0, usX1 - usX0, usY1 - usY0);
	if (bFill) {
		SolidBrush solidBrush(Color(convertColor(usColor)));
		graphics->FillRectangle(&solidBrush, usX0, usY0, usX1 - usX0, usY1 - usY0);
	}
}

static void vVLine(uint16_t usX0, uint16_t usY0, uint16_t usY1, uint16_t usColor) {
	Pen      pen(Color(convertColor(usColor)));
	if (usY0 == usY1) usY1++;
	graphics->DrawLine(&pen, usX0, usY0, usX0, usY1);
}
static void vHLine(uint16_t usX0, uint16_t usY0, uint16_t usX1, uint16_t usColor) {
	Pen      pen(Color(convertColor(usColor)));
	if (usX0 == usX1) usX1++;
	graphics->DrawLine(&pen, usX0, usY0, usX1, usY0);
}

static uint16_t usGetPictureH(xPicture pusPicture) {
	const WCHAR *pwcsName;
	int nChars = MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, NULL, 0);
	pwcsName = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, (LPWSTR)pwcsName, nChars);
	Image img(pwcsName);
	uint16_t H = img.GetHeight();
	delete[] pwcsName;
	return H;
}

static uint16_t usGetPictureW(xPicture pusPicture) {
	const WCHAR *pwcsName;
	int nChars = MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, NULL, 0);
	pwcsName = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, (LPWSTR)pwcsName, nChars);
	Image img(pwcsName);
	uint16_t W = img.GetWidth();
	delete[] pwcsName;
	return W;
}

static void bPicture(int16_t sX0, int16_t sY0, xPicture pusPicture) {

	const WCHAR *pwcsName;
	int nChars = MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, NULL, 0);
	pwcsName = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, (LPWSTR)pwcsName, nChars);
	Image img(pwcsName);
	delete[] pwcsName;
	uint16_t we, he;
	if (usGetPictureW(pusPicture) < 220 && usGetPictureH(pusPicture) < 220) {
		we = usGetPictureW(pusPicture);
		he = usGetPictureH(pusPicture);
	}
	else {
		we = 220;
		he = 220;
	}
	Rect destRect(sX0, sY0, we, he);
	graphics->DrawImage(&img, destRect);

}

static uint16_t usFontGetH(xFont pubFont) {
	const GFXfont *gfxFont = pubFont;
	uint8_t  height = pgm_read_byte(&gfxFont->yAdvance);
	return (uint16_t)height;
}

static xPicture prvGetPicture(const char * picName) {
	if (strcmp(picName, "SB_CROSS.pic") == 0)
		return "DemoImages/cross16px.bmp";

	if (strcmp(picName, "No") == 0)
		return "DemoImages/no.png";

	if (strcmp(picName, "Service") == 0)
		return "DemoImages/service.bmp";

	if (strcmp(picName, "Cancel") == 0)
		return "DemoImages/undo.png";

	if (strcmp(picName, "Yes") == 0)
		return "DemoImages/ok.png";

	if (strcmp(picName, "OK") == 0)
		return "DemoImages/ok.png";

	return "DemoImages/cross16px.bmp";
}

static xModalDialogPictureSet prvGetPicSet(char cType) {
	xModalDialogPictureSet xPicSet;
	switch (cType) {
	case 'n':
		xPicSet.strLabel = "No";
		break;
	case 'c':
		xPicSet.strLabel = "Cancel";
		break;
	case 'y':
		xPicSet.strLabel = "Yes";
		break;
	default:
		xPicSet.strLabel = "OK";
		break;
	}

	xPicSet.xPicMain = prvGetPicture(xPicSet.strLabel);

	return xPicSet;
}

static uint16_t ucFontGetCharW(char cChar, xFont pubFont) {
	const GFXfont *gfxFont = pubFont;
	cChar -= (uint8_t)pgm_read_byte(&gfxFont->first);
	GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[cChar]);

	uint8_t  w = pgm_read_byte(&glyph->xAdvance);
	return (uint16_t)w;
}

static uint16_t usFontGetStrW(char const * pcStr, xFont pubFont) {
	uint16_t usWidth = 0;

	while (*pcStr) {
		usWidth += ucFontGetCharW(*pcStr, pubFont);
		pcStr++;
	}
	return usWidth;
}

static uint16_t usFontGetStrH(const char * pcStr, xFont pubFont) {
	//TODO: implement multistring height.
	return usFontGetH(pubFont);
}

static void vPutChar(uint16_t usX, uint16_t usY, char ASCII, xFont pubFont, uint16_t usColor, uint16_t usBackground, bool bFillBg) {

	const GFXfont *gfxFont = pubFont;
	ASCII -= (uint8_t)pgm_read_byte(&gfxFont->first);
	GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[ASCII]);
	uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

	uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
	uint8_t  w = pgm_read_byte(&glyph->width),
		h = pgm_read_byte(&glyph->height);
	int8_t   xo = pgm_read_byte(&glyph->xOffset),
		yo = pgm_read_byte(&glyph->yOffset);
	uint8_t  xx, yy, bits = 0, bit = 0;

	auto x = usX;
	auto y = usY + usFontGetH(pubFont) * 3 / 4 - 1;

	Bitmap bm(w, h, PixelFormat32bppARGB);
	Graphics g(&bm);

	if(bFillBg)
		g.Clear(convertColor(usBackground));

	for (yy = 0; yy < h; yy++) {
		for (xx = 0; xx < w; xx++) {
			if (!(bit++ & 7)) {
				bits = pgm_read_byte(&bitmap[bo++]);
			}
			if (bits & 0x80) {
				bm.SetPixel(xx, yy, convertColor(usColor));
			}
			bits <<= 1;
		}
	}

	Rect destRect(x + xo, y + yo, w, h);
	graphics->DrawImage(&bm, destRect);

}

xFont xGetDefaultFont() {
	return &FreeSans9pt7b;
}

bool bGUI_InitInterfce() {
	vDrawHandlerInit(&LCD);
	LCD.vRectangle = vRectangle;
	LCD.vPutChar = vPutChar;
	LCD.bPicture = bPicture;
	LCD.vVLine = vVLine;
	LCD.vHLine = vHLine;
	LCD.usGetPictureH = usGetPictureH;
	LCD.usGetPictureW = usGetPictureW;

	LCD.usFontGetH = usFontGetH;
	LCD.ucFontGetCharW = ucFontGetCharW;
	LCD.usFontGetStrW = usFontGetStrW;
	LCD.usFontGetStrH = usFontGetStrH;
	LCD.xGetDialogPictureSet = prvGetPicSet;
	LCD.xGetPicture = prvGetPicture;
	LCD.xGetDefaultFont = xGetDefaultFont;

	vDrawSetHandler(&LCD);
	return true;
}


void vGUIsetCurrentHDC(Graphics *gr) {
	graphics = gr;
	//graphics->SetCompositingMode(CompositingModeSourceCopy);
	graphics->SetCompositingQuality(CompositingQualityHighSpeed);
	graphics->SetPixelOffsetMode(PixelOffsetModeNone);
	graphics->SetSmoothingMode(SmoothingModeNone);
	graphics->SetInterpolationMode(InterpolationModeDefault);
}

void vGUIpushClickHandler(LPARAM lParam) {
	char outString[25];
	sprintf_s(outString, "x: %d y: %d\n", LOWORD(lParam), HIWORD(lParam));
	//pcLabelSetText(mouseMonitor, outStri);
	currentTouch.eventTouchScreen = pushTs;
	currentTouch.xTouchScreen = LOWORD(lParam);
	currentTouch.yTouchScreen = HIWORD(lParam);
	bWindowManagerCheckTouchScreenEvent(&currentTouch);

}
void vGUIpopClickHandler(LPARAM lParam) {
	currentTouch.eventTouchScreen = popTs;
	currentTouch.xTouchScreen = LOWORD(lParam);
	currentTouch.yTouchScreen = HIWORD(lParam);
	bWindowManagerCheckTouchScreenEvent(&currentTouch);
}