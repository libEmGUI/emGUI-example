#ifndef __GUI_H
#define __GUI_H

#include "emGUI.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include "pics.h"
#include <iostream>

using namespace Gdiplus;

#define ILI9341_TFTWIDTH 240
#define ILI9341_TFTHEIGHT 240


ARGB convertColor(uint16_t color);
bool bGUI_InitInterfce();
bool bGUIOnWindowCloseHandler(xWidget *);
void vGUIsetCurrentHDC(Graphics * gr);
void vGUIpaintEventHandler();
void vGUIpushClickHandler(LPARAM lParam);
void vGUIpopClickHandler(LPARAM lParam);
void vGUIeraseBackgroudHandler();

#endif