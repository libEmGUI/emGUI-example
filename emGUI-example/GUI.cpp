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
	uint8_t usRed = ((color >> 11) & 0b00011111) * 800 / 97;
	uint8_t usGreen = (uint32_t)((uint32_t)(color >> 5) & 0b0000111111) * 4000 / 985;
	uint8_t usBlue = ((color & 0b00011111)) * 800 / 97;

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
	#ifdef EM_GUI_OVERRIDE_DEFAULT_PICS
	uint16_t usGetPictureH(xPicture pusPicture) {
		const WCHAR *pwcsName;
		int nChars = MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, NULL, 0);
		pwcsName = new WCHAR[nChars];
		MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, (LPWSTR)pwcsName, nChars);
		Image img(pwcsName);
		uint16_t H = img.GetHeight();
		delete[] pwcsName;
		return H;
	}

	uint16_t usGetPictureW(xPicture pusPicture) {
		const WCHAR *pwcsName;
		int nChars = MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, NULL, 0);
		pwcsName = new WCHAR[nChars];
		MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, (LPWSTR)pwcsName, nChars);
		Image img(pwcsName);
		uint16_t W = img.GetWidth();
		delete[] pwcsName;
		return W;
	}
	#endif

	void bPicture(int16_t sX0, int16_t sY0, xPicture pusPicture) {
		#ifndef EM_GUI_OVERRIDE_DEFAULT_PICS
			BYTE * imgC = (BYTE *)pusPicture;
			Bitmap cross_pic_bitmap(pusPicture[0], pusPicture[1], pusPicture[0] * sizeof(uint16_t), PixelFormat16bppRGB565, imgC + sizeof(uint16_t) * 2);
			static float angle = 0.0;
			cross_pic_bitmap.RotateFlip(Rotate90FlipX);
			graphics->DrawImage(&cross_pic_bitmap, sX0, sY0);
		#else
			const WCHAR *pwcsName;
			int nChars = MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, NULL, 0);
			pwcsName = new WCHAR[nChars];
			MultiByteToWideChar(CP_ACP, 0, pusPicture, -1, (LPWSTR)pwcsName, nChars);
			Image img(pwcsName);
			graphics->DrawImage(&img, sX0, sY0);
			delete[] pwcsName;
		#endif

	}

	void doMagic() {
		char outString[25];
		static int i = 0;
		i++;
		sprintf_s(outString, "Magic count: %d times", i);
		pcLabelSetText(mouseMonitor, outString);
	}
	bool btnMagicHDLR(xWidget *) {
		auto dlg = iModalDialogOpen(MODAL_AUTO, "ny", "Magic Button", "Magic will happen. Are you sure?");
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

	// Action on interface creatings
	bool bGUIonInterfaceCreateHandler(xWidget *) {
		auto window = pxWindowCreate(WINDOW_MENU);
		vWindowSetHeader(window, "Main menu");
		mouseMonitor = pxLabelCreate(1, 200, 238, 0, "Magic count: 0", FONT_ASCII_8_X, 500, window);
		uint8_t offset = 15;

		uint8_t row1 = offset;
		uint8_t row2 = row1 + 60 + offset;
		uint8_t row3 = row2 + 60 + offset;
		uint8_t column1 = offset;
		uint8_t column2 = SCREEN_WIDTH / 2 - 30;
		uint8_t column3 = SCREEN_WIDTH - offset - 60;
		auto menuBut		= pxMenuButtonCreate(column1, row1, EM_GUI_PIC_MAGIC, "Do magic", &btnMagicHDLR, window);
		auto menuButAbout	= pxMenuButtonCreate(column2, row1, EM_GUI_PIC_HELP, "Info", &btnAboutHDLR, window);
		auto menuButLabel	= pxMenuButtonCreate(column3, row1, EM_GUI_PIC_PROCESS, "Test Label", &btnLabelHDLR, window);
		auto menuButFolder  = pxMenuButtonCreate(column1, row2, EM_GUI_PIC_OPENFOLDER, "Windows", &btnFolderHDLR, window);

		auto window2_about = pxWindowCreate(WINDOW_ABOUT);
		vWindowSetHeader(window2_about, "About");

		auto window_show_label = pxWindowCreate(WINDOW_NOTES);
		vWindowSetHeader(window_show_label, "Big label");

		auto window_show_folder = pxWindowCreate(WINDOW_ARCHIVE);
		vWindowSetHeader(window_show_folder, "Labels");
		auto menuButAbout2 = pxMenuButtonCreate(column1, row1, EM_GUI_PIC_HELP, "Info", &btnAboutHDLR, window_show_folder);
		auto menuButLabel2 = pxMenuButtonCreate(column2, row1, EM_GUI_PIC_PROCESS, "Test Label", &btnLabelHDLR, window_show_folder);

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
	to determine consensus All-Americans.[5]", FONT_ASCII_8_X, 1010, window_show_label);

		auto labelAbout = pxLabelCreate(1, 1, 238, 60, "This is Demo for emGUI. 2017", FONT_ASCII_8_X, 200, window2_about);

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
	auto dial = iModalDialogOpen(MODAL_AUTO, "ny", "Close?", "");
	vModalDialogSetHandler(dial, 'y', closeWindow);
	return true;
}


bool bGUI_InitInterfce() {
	vDrawHandlerInit(&LCD);
	LCD.vRectangle = &vRectangle;
	LCD.vPutChar = &vPutChar;
	LCD.bPicture = &bPicture;
	LCD.vVLine = &vVLine;
	LCD.vHLine = &vHLine;
	#ifdef EM_GUI_OVERRIDE_DEFAULT_PICS
		LCD.usGetPictureH = &usGetPictureH;
		LCD.usGetPictureW = &usGetPictureW;
	#endif
	vDrawSetHandler(&LCD);
	interface1 = pxInterfaceCreate(&bGUIonInterfaceCreateHandler);
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


