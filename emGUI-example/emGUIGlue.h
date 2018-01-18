#pragma once

#include "emGUI/emGUI.h"

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <windows.h>
#include <iostream>

#include <Windows.h>

#ifndef F_WRP(x)
#define F_WRP(x) x
#endif

using namespace std;
using namespace Gdiplus;

xFont xGetDefaultFont();


void vGUIMouseHandler(LPARAM lParam, xTouchEventType ev);

void vGUIsetCurrentHDC(Gdiplus::Graphics * gr);