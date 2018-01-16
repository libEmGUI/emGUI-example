#pragma once

#include <memory.h>
#include <Windows.h>
#include "emGUI/emGUI.h"

#include "GUI.h"

#include <spdlog/spdlog.h>

typedef struct {
	LPCWSTR portName;
	bool exitFlag;
	xPlotData_t * data;
	std::shared_ptr<spdlog::logger> logger;
	extraParams_t *extraParams;
}serialThreadParams_t;

bool PlotDataCollectorStart(serialThreadParams_t* params);

HANDLE getSerialThreadHandle();