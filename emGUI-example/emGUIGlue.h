#pragma once

#include "emGUI/emGUI.h"

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#define SCREEN_WIDTH EMGUI_LCD_WIDTH
#define SCREEN_HEIGHT EMGUI_LCD_HEIGHT

xFont xGetDefaultFont();

void vGUIsetCurrentHDC(Gdiplus::Graphics * gr);