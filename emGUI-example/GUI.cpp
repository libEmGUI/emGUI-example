/*
	This is main GUI source

*/
#include "GUI.h"

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
static xInterface * interface1;
static xLabel * mouseMonitor;
static xLabel * currentMonitor;
static Graphics *graphics = NULL;
xTouchEvent currentTouch;

static int stride = 0;

static xPlotData_t plotLead;
static extraParams_t extraP;

xPlotData_t * pxGUIGetPlotData() {
	return &plotLead;
}

// Convert color from emGUI to GDI format
ARGB convertColor(uint16_t color) {
	uint8_t usRed = ((color >> 11) & 0b00011111) * 800 / 97;
	uint8_t usGreen = (uint32_t)((uint32_t)(color >> 5) & 0b0000111111) * 4000 / 985;
	uint8_t usBlue = ((color & 0b00011111)) * 800 / 97;

	ARGB answer = (ARGB)((ARGB)0xFF000000 | (usRed << 16) | (usGreen << 8) | usBlue);
	return answer;

}
void vGUIWriteToLabel(char* msg) {
	pcLabelSetText(mouseMonitor, msg);
}

extern "C" {
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
		} else {
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
		if (strcmp(picName, "cross") == 0)
			return EM_GUI_PIC_CROSS;

		if (strcmp(picName, "No") == 0)
			return EM_GUI_PIC_NO;

		if (strcmp(picName, "Cancel") == 0)
			return EM_GUI_PIC_REFRESH;

		if (strcmp(picName, "OK") == 0)
			return EM_GUI_PIC_YES;

		return EM_GUI_PIC_CROSS;
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
		case 'o':
			xPicSet.strLabel = "OK";
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

		Bitmap bm(w,h,PixelFormat16bppRGB565);
		Graphics g(&bm);

		g.Clear(convertColor(usBackground));

		for (yy = 0; yy<h; yy++) {
			for (xx = 0; xx<w; xx++) {
				if (!(bit++ & 7)) {
					bits = pgm_read_byte(&bitmap[bo++]);
				}
				if (bits & 0x80) {
					bm.SetPixel(xx, yy, usColor);
				}
				bits <<= 1;
			}
		}

		Rect destRect(x+xo, y+yo, w, h);
		graphics->DrawImage(&bm, destRect);

	}

	xFont getDefaultFont() {
		return &FreeSans9pt7b;
	}

	void doMagic() {
		char outString[25];
		static int i = 0;
		i++;
		sprintf_s(outString, "Magic count: %d times", i);
		pcLabelSetText(mouseMonitor, outString);
	}
	bool btnMagicHDLR(xWidget *) {
		auto dlg = iModalDialogOpen(EMGUI_MODAL_AUTO, "ny", "Magic Button", "Magic will happen. Are you sure?");
		vModalDialogSetHandler(dlg, 'y', &doMagic);
		return true;
	}

	bool btnAboutHDLR(xWidget *) {
		vInterfaceOpenWindow(WINDOW_ABOUT);
		return true;
	}

	bool btnLabelHDLR(xWidget *) {
		vInterfaceOpenWindow(WINDOW_NOTES);
		return true;
	}

	bool btnFolderHDLR(xWidget *) {
		vInterfaceOpenWindow(WINDOW_ARCHIVE);
		return true;
	}

	bool showParrot(xWidget *) {
		vInterfaceOpenWindow(WINDOW_ECG);
		return true;
	}

	// Action on interface creatings
	bool bGUIonInterfaceCreateHandler(xWidget *) {
		auto window = pxWindowCreate(WINDOW_MENU);
		vWindowSetHeader(window, "Main menu");
		mouseMonitor = pxLabelCreate(1, 200, 238, 0, "Magic count: 0", LCD.xGetDefaultFont(), 500, window);
		uint8_t offset = 15;

		uint8_t row1 = offset;
		uint8_t row2 = row1 + 60 + offset;
		uint8_t row3 = row2 + 60 + offset;
		uint8_t column1 = offset;
		uint8_t column2 = SCREEN_WIDTH / 2 - 30;
		uint8_t column3 = SCREEN_WIDTH - offset - 60;
		auto menuBut = pxMenuButtonCreate(column1, row1, EM_GUI_PIC_WRENCH, "Curr. mon.", &showParrot, window);
		//bButtonSetPushPic(menuBut, mail);
		auto menuButAbout = pxMenuButtonCreate(column2, row1, EM_GUI_PIC_HELP, "Info", &btnAboutHDLR, window);
		auto menuButLabel = pxMenuButtonCreate(column3, row1, EM_GUI_PIC_PROCESS, "Test Label", &btnLabelHDLR, window);
		auto menuButFolder = pxMenuButtonCreate(column1, row2, EM_GUI_PIC_OPENFOLDER, "Windows", &btnFolderHDLR, window);

		auto window2_about = pxWindowCreate(WINDOW_ABOUT);
		vWindowSetHeader(window2_about, "About");

		auto window_show_label = pxWindowCreate(WINDOW_NOTES);
		vWindowSetHeader(window_show_label, "Big label");

		auto window_show_folder = pxWindowCreate(WINDOW_ARCHIVE);
		vWindowSetHeader(window_show_folder, "Labels");
		auto menuButAbout2 = pxMenuButtonCreate(column1, row1, EM_GUI_PIC_HELP, "Info", &btnAboutHDLR, window_show_folder);
		auto menuButLabel2 = pxMenuButtonCreate(column2, row1, EM_GUI_PIC_PROCESS, "Test Label", &btnLabelHDLR, window_show_folder);

		auto window_show_ampermeter = pxWindowCreate(WINDOW_ECG);
		vWindowSetHeader(window_show_ampermeter, "Ampermeter, uA");

		plotLead.bDataFilled = false;
		plotLead.bWriteEnabled = false;
		plotLead.sDataDCOffset = -500;
		plotLead.sName = "Test";
		plotLead.ulElemCount = AFE_DATA_RATE * 10;
		plotLead.psData = (short *)malloc(sizeof(*plotLead.psData)*plotLead.ulElemCount);
		plotLead.ulWritePos = 0;


		xPlot * plot = pxPlotCreate(0, 0, EMGUI_LCD_WIDTH, EMGUI_LCD_HEIGHT - EMGUI_STATUS_BAR_HEIGHT -20, window_show_ampermeter, &plotLead);
		vPlotSetScale(plot, 250);
		currentMonitor = pxLabelCreate(10, EMGUI_LCD_HEIGHT - EMGUI_STATUS_BAR_HEIGHT - 20, EMGUI_LCD_WIDTH, 20, "I: _ (0.1 mA)", LCD.xGetDefaultFont(), 100, window_show_ampermeter);
		vLabelSetTextAlign(currentMonitor, LABEL_ALIGN_CENTER);
		vLabelSetVerticalAlign(currentMonitor, LABEL_ALIGN_MIDDLE);
		auto big_label = pxLabelCreate(1, 1, 238, 238, "Sample text: hypothetical rosters of players \
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
	to determine consensus All-Americans.[5]", LCD.xGetDefaultFont(), 1010, window_show_label);

		auto labelAbout = pxLabelCreate(1, 1, 238, 60, "This is Demo for emGUI. 2017", LCD.xGetDefaultFont(), 200, window2_about);

		vWindowSetOnCloseRequestHandler(window, &bGUIOnWindowCloseHandlerMain);
		auto dialog1 = pxModalDialogWindowCreate();
		vInterfaceOpenWindow(WINDOW_MENU);
		return true;
	}
}

void closeWindow() {
	HWND hWnd = GetActiveWindow();
	DestroyWindow(hWnd);
}

bool bGUIOnWindowCloseHandlerMain(xWidget *) {
	auto dial = iModalDialogOpen(EMGUI_MODAL_AUTO, "ny", "Close?", "");
	vModalDialogSetHandler(dial, 'y', closeWindow);
	return true;
}

void vGUIUpdateCurrentMonitor() {
	auto data = plotLead.psData[plotLead.ulWritePos];
	iLabelPrintf(currentMonitor, "I_Avg: %.1f; I: %d.%d (mA)", extraP.averageCurrent / 10.f, data / 10, abs(data % 10));
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
	LCD.xGetDefaultFont = getDefaultFont;

	vDrawSetHandler(&LCD);
	interface1 = pxInterfaceCreate(bGUIonInterfaceCreateHandler);
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

extraParams_t * pxGUIGetExtraParams() {
	return &extraP;
}

