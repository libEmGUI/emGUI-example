/*
	This is main GUI source

*/
#include "GUI.h"

#include <windows.h>
#include <iostream>

#include "emGUIGlue.h"

using namespace std;
using namespace Gdiplus;

static xLabel * mouseMonitor;
static xLabel * currentMonitor;

static int stride = 0;

static xPlotData_t plotLead;
static extraParams_t extraP;

xPlotData_t * pxGUIGetPlotData() {
	return &plotLead;
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

// Action on interface creatings
bool bGUIonInterfaceCreateHandler(xWidget *) {
	auto window = pxWindowCreate(WINDOW_MENU);
	vWindowSetHeader(window, "Main menu");
	mouseMonitor = pxLabelCreate(1, 200, 238, 0, "Magic count: 0", xGetDefaultFont(), 500, window);
	uint8_t offset = 15;

	uint8_t row1 = offset;
	uint8_t row2 = row1 + 60 + offset;
	uint8_t row3 = row2 + 60 + offset;
	uint8_t column1 = offset;
	uint8_t column2 = SCREEN_WIDTH / 2 - 30;
	uint8_t column3 = SCREEN_WIDTH - offset - 60;
	auto menuBut = pxButtonCreateFromImageWithText(column1, row1, EM_GUI_PIC_WRENCH, "Curr. mon.", window);
	vButtonSetOnClickHandler(menuBut, 
		[](xWidget *) {
		vInterfaceOpenWindow(WINDOW_ECG);
		return true;
	});
	auto menuButAbout = pxButtonCreateFromImageWithText(column2, row1, EM_GUI_PIC_HELP, "Info", window);
	vButtonSetOnClickHandler(menuButAbout,
		[](xWidget *) {
		vInterfaceOpenWindow(WINDOW_ABOUT);
		return true;
	});
	auto menuButLabel = pxButtonCreateFromImageWithText(column3, row1, EM_GUI_PIC_PROCESS, "Test Label", window);
	vButtonSetOnClickHandler(menuButLabel,
		[](xWidget *) {
		vInterfaceOpenWindow(WINDOW_NOTES);
		return true;
	});
	auto menuButFolder = pxButtonCreateFromImageWithText(column1, row2, EM_GUI_PIC_OPENFOLDER, "Windows", window);
	vButtonSetOnClickHandler(menuButFolder,
		[](xWidget *) {
		vInterfaceOpenWindow(WINDOW_ARCHIVE);
		return true;
	});

	auto window2_about = pxWindowCreate(WINDOW_ABOUT);
	vWindowSetHeader(window2_about, "About");

	auto window_show_label = pxWindowCreate(WINDOW_NOTES);
	vWindowSetHeader(window_show_label, "Big label");

	auto window_show_folder = pxWindowCreate(WINDOW_ARCHIVE);
	vWindowSetHeader(window_show_folder, "Labels");
	auto menuButAbout2 = pxButtonCreateFromImageWithText(column1, row1, EM_GUI_PIC_HELP, "Info", window_show_folder);
	vButtonSetOnClickHandler(menuButAbout2,
		[](xWidget *) {
		vInterfaceOpenWindow(WINDOW_ABOUT);
		return true;
	});
	auto menuButLabel2 = pxButtonCreateFromImageWithText(column2, row1, EM_GUI_PIC_PROCESS, "Test Label", window_show_folder);
	vButtonSetOnClickHandler(menuButLabel2,
		[](xWidget *) {
		vInterfaceOpenWindow(WINDOW_NOTES);
		return true;
	});

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
	currentMonitor = pxLabelCreate(10, EMGUI_LCD_HEIGHT - EMGUI_STATUS_BAR_HEIGHT - 20, EMGUI_LCD_WIDTH, 20, "I: _ (0.1 mA)", xGetDefaultFont(), 100, window_show_ampermeter);
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
to determine consensus All-Americans.[5]", xGetDefaultFont(), 1010, window_show_label);

	auto labelAbout = pxLabelCreate(1, 1, 238, 60, "This is Demo for emGUI. 2017", xGetDefaultFont(), 200, window2_about);

	vWindowSetOnCloseRequestHandler(window, &bGUIOnWindowCloseHandlerMain);
	auto dialog1 = pxModalDialogWindowCreate();
	vInterfaceOpenWindow(WINDOW_MENU);
	return true;
}

bool bGUIOnWindowCloseHandlerMain(xWidget *) {
	auto dial = iModalDialogOpen(EMGUI_MODAL_AUTO, "ny", "Close?", "");
	vModalDialogSetHandler(dial, 'y', []() {
		HWND hWnd = GetActiveWindow();
		DestroyWindow(hWnd);
	});
	return true;
}

void vGUIUpdateCurrentMonitor() {
	auto data = plotLead.psData[plotLead.ulWritePos];
	iLabelPrintf(currentMonitor, "I_Avg: %.1f; I: %d.%d (mA)", extraP.averageCurrent / 10.f, data / 10, abs(data % 10));
}

extraParams_t * pxGUIGetExtraParams() {
	return &extraP;
}

