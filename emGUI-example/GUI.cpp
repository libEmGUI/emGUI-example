/*
	This is main GUI source

*/
#include "GUI.h"

#include <windows.h>
#include <iostream>
#include <memory>

#include "emGUIGlue.h"

#define xstr(a) str(a)
#define str(a) #a


using namespace std;
using namespace Gdiplus;

static xLabel * mouseMonitor;
static xLabel * currentMonitor;

static int stride = 0;

static xPlotData_t plotLead;
static extraParams_t extraP;

//static std::shared_ptr<std::wstring> ctx;

xPlotData_t * pxGUIGetPlotData() {
	return &plotLead;
}

// Action on interface creatings
bool bGUIonWindowManagerCreateHandler(xWidget *) {

	// ============= Main Menu ==============
	auto window = pxWindowCreate(WINDOW_MENU);
	vWindowSetHeader(window, "Main menu");
	mouseMonitor = pxLabelCreate(1, 190, 238, 0, "EmGUI v" xstr(EMGUI_VERSION), xGetDefaultFont(), 500, window);
	uint8_t offset = 10;

	uint8_t row1 = offset;
	uint8_t row2 = row1 + 80 + offset;
	uint8_t column1 = offset;
	uint8_t column2 = SCREEN_WIDTH / 2 - 30;
	uint8_t column3 = SCREEN_WIDTH - offset - 60;
	auto menuBut = pxButtonCreateFromImageWithText(column1, row1, "DemoImages/plot.png", "Plot", window);
	vButtonSetOnClickHandler(menuBut, 
		[](xWidget *) {
		vWindowManagerOpenWindow(WINDOW_PLOT);
		return true;
	});

	auto menuButAbout = pxButtonCreateFromImageWithText(column2, row1, "DemoImages/about.png", "About", window);
	vButtonSetOnClickHandler(menuButAbout,
		[](xWidget *) {
		vWindowManagerOpenWindow(WINDOW_ABOUT);
		return true;
	});

	auto menuButLabel = pxButtonCreateFromImageWithText(column3, row1, "DemoImages/label.png", "Label", window);
	vButtonSetOnClickHandler(menuButLabel,
		[](xWidget *) {
		vWindowManagerOpenWindow(WINDOW_LABEL);
		return true;
	});

	auto menuButFolder = pxButtonCreateFromImageWithText(column1, row2, "DemoImages/folder.png", "Windows", window);
	vButtonSetOnClickHandler(menuButFolder,
		[](xWidget *) {
		vWindowManagerOpenWindow(WINDOW_SUBMENU);
		return true;
	});

	vWindowSetOnCloseRequestHandler(window, &MainWindowCloseRequestHdl);

	vWindowManagerOpenWindow(WINDOW_MENU);

	// ============= About ==============

	auto window2_about = pxWindowCreate(WINDOW_ABOUT);
	vWindowSetHeader(window2_about, "About");

	auto labelAbout = pxLabelCreate(1, 1, 238, 60, "This is Demo for emGUI v" xstr(EMGUI_VERSION) ". 2017", xGetDefaultFont(), 200, window2_about);

	// ============= Multiline Label ==============

	auto window_show_label = pxWindowCreate(WINDOW_LABEL);
	vWindowSetHeader(window_show_label, "Big label");

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
to determine consensus All-Americans.[5]", xGetDefaultFont(), 1010, window_show_label);

	// ============= Submenu ==============

	auto window_show_folder = pxWindowCreate(WINDOW_SUBMENU);
	vWindowSetFullScreen(window_show_folder, true);
	vWindowSetHeader(window_show_folder, "Labels");
	auto menuButAbout2 = pxButtonCreateFromImageWithText(column1, row1, "DemoImages/about.png", "About", window_show_folder);
	vButtonSetOnClickHandler(menuButAbout2,
		[](xWidget *) {
		vWindowManagerOpenWindow(WINDOW_ABOUT);
		return true;
	});

	auto menuButLabel2 = pxButtonCreateFromImageWithText(column2, row1, "DemoImages/label.png", "Label", window_show_folder);
	vButtonSetOnClickHandler(menuButLabel2,
		[](xWidget *) {
		vWindowManagerOpenWindow(WINDOW_LABEL);
		return true;
	});

	auto lab = pxLabelCreate(10, 100, 200, 100, "Window is in FullScreen mode! Use ESC to go back.", xGetDefaultFont(), 0, window_show_folder);
	bLabelSetMultiline(lab, true);

	// ============= Plot ==============

	auto window_show_ampermeter = pxWindowCreate(WINDOW_PLOT);

	vWindowSetHeader(window_show_ampermeter, "Plot, mA");

	plotLead.bDataFilled = false;
	plotLead.bWriteEnabled = false;
	plotLead.sDataDCOffset = -500;
	plotLead.sName = "Test";
	plotLead.ulElemCount = AFE_DATA_RATE * 10;
	plotLead.psData = (short *)malloc(sizeof(*plotLead.psData)*plotLead.ulElemCount);
	plotLead.ulWritePos = 0;

	xPlot * plot = pxPlotCreate(0, 0, EMGUI_LCD_WIDTH, EMGUI_LCD_HEIGHT - EMGUI_STATUS_BAR_HEIGHT -20, window_show_ampermeter, &plotLead);
	vPlotSetScale(plot, 250);
	currentMonitor = pxLabelCreate(10, EMGUI_LCD_HEIGHT - EMGUI_STATUS_BAR_HEIGHT - 20, EMGUI_LCD_WIDTH, 20, "I: _ (0.1 mA)", xGetDefaultFont(), 100, window_show_ampermeter);
	vLabelSetTextAlign(currentMonitor, LABEL_ALIGN_CENTER);
	vLabelSetVerticalAlign(currentMonitor, LABEL_ALIGN_MIDDLE);
	
	return true;
}

bool MainWindowCloseRequestHdl(xWidget *) {
	auto dial = iModalDialogOpen(0, "ny", "Close?", "You are about to close main app! Are you sure?");
	
	vModalDialogSetHandler(dial, NULL, [](char cBtn, void* ctx) -> bool {

		if (cBtn != 'y')
			return true;  // dialog can close on this button

		HWND hWnd = GetActiveWindow();
		DestroyWindow(hWnd);
		return true; // dialog can close on this button
	});

	return false; //suppress window close!
}

void vGUIUpdateCurrentMonitor() {
	auto data = plotLead.psData[plotLead.ulWritePos];
	iLabelPrintf(currentMonitor, "I_Avg: %.1f; I: %d.%d (mA)", extraP.averageCurrent / 10.f, data / 10, abs(data % 10));
}

extraParams_t * pxGUIGetExtraParams() {
	return &extraP;
}

void vGUIHandleKeypress(WPARAM keycode) {

	string q;

	switch (keycode) {
	case VK_ESCAPE:
		vWindowManagerCloseActiveWindow();
		return;
	}
	
	/*if (keycode != 'R')
		return;

	q += (char) keycode;
	q += " was pressed. Bring main menu back?";

	auto dial = iModalDialogOpen(0, "ny", "Restore?", q.c_str());

	vModalDialogSetHandler(dial, NULL, [](char cBtn, void* ctx) -> bool {

		if (cBtn != 'y')
			return true;  // dialog can close on this button

		vWindowManagerOpenWindow(WINDOW_MENU);
		return true; // dialog can close on this button
	});*/
}

