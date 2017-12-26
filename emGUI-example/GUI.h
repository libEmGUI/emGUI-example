#ifndef __GUI_H
#define __GUI_H

#include "emGUI/emGUI.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <iostream>

using namespace Gdiplus;

#define SCREEN_WIDTH EMGUI_LCD_WIDTH
#define SCREEN_HEIGHT EMGUI_LCD_HEIGHT

typedef struct {
	float averageCurrent;
}extraParams_t;

typedef enum {
	WINDOW_BATTERY_STATUS,
	WINDOW_ABOUT,
	WINDOW_NOTES,
	WINDOW_ARCHIVE,
	WINDOW_MENU,
	WINDOW_ECG
}eWindows;

//events

ARGB convertColor(uint16_t color);
bool bGUI_InitInterfce();
bool bGUIOnWindowCloseHandlerMain(xWidget *);
bool bGUIOnWindowCloseHandler(xWidget *);
void vGUIsetCurrentHDC(Graphics * gr);
void vGUIpaintEventHandler();
void vGUIpushClickHandler(LPARAM lParam);
void vGUIpopClickHandler(LPARAM lParam);
void vGUIeraseBackgroudHandler();
void vGUIUpdateCurrentMonitor();
void vGUIWriteToLabel(char * msg);

xPlotData_t * pxGUIGetPlotData();
extraParams_t * pxGUIGetExtraParams();

#endif