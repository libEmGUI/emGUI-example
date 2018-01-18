#ifndef __GUI_H
#define __GUI_H

#include "emGUI/emGUI.h"

#include "emGUIGlue.h"

typedef struct {
	float averageCurrent;
}extraParams_t;

typedef enum {
	WINDOW_BATTERY_STATUS,
	WINDOW_ABOUT,
	WINDOW_LABEL,
	WINDOW_SUBMENU,
	WINDOW_MENU,
	WINDOW_PLOT
}eWindows;

//events

bool bGUI_InitInterfce();
bool MainWindowCloseRequestHdl(xWidget *);
bool bGUIOnWindowCloseHandler(xWidget *);
void vGUIUpdateCurrentMonitor();
bool bGUIonWindowManagerCreateHandler(xWidget *);

void vGUIHandleKeypress(WPARAM keycode);

xPlotData_t * pxGUIGetPlotData();
extraParams_t * pxGUIGetExtraParams();

#endif