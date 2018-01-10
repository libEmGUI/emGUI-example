#ifndef __GUI_H
#define __GUI_H

#include "emGUI/emGUI.h"

#include <windows.h>

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

bool bGUI_InitInterfce();
bool bGUIOnWindowCloseHandlerMain(xWidget *);
bool bGUIOnWindowCloseHandler(xWidget *);
void vGUIpushClickHandler(LPARAM lParam);
void vGUIpopClickHandler(LPARAM lParam);
void vGUIUpdateCurrentMonitor();
bool bGUIonInterfaceCreateHandler(xWidget *);

xPlotData_t * pxGUIGetPlotData();
extraParams_t * pxGUIGetExtraParams();

#endif