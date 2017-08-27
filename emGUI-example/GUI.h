#ifndef __GUI_H
#define __GUI_H

#include "emGUI.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include "pics.h"

using namespace Gdiplus;

#define ILI9341_TFTWIDTH 240
#define ILI9341_TFTHEIGHT 240


ARGB convertColor(uint16_t color);
bool myHandler(xWidget *);
bool initGUI();
bool onCloseWAHandler(xWidget *);
void drawPixel(uint16_t x, uint16_t y, uint16_t color);
void setCurrentHDC(HDC a);
#endif