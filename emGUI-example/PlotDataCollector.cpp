#include "PlotDataCollector.h"

#include <deque>
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <Windows.h>
#include "FIR-filter-class/filt.h"
#include "IIR_filter.h"

#include "GUI.h"

#define SERIAL_READ_TIMEOUT 500 
#define SERIAL_READ_BUF_SIZE 10

#define TIMEOUT 1000
#define AFE_DATA_RATE 700

static HANDLE serialPort = 0;

static HANDLE serialThread = 0;

static Filter lpf(LPF, 51, AFE_DATA_RATE / 1000.f, 0.05);
static IIR_filter iir_f(1.f / AFE_DATA_RATE);

static unsigned int  dwThreadId = 0;

void handleData(int data, serialThreadParams_t * params) {
	auto pd = params->data;

	//logger->info(data);
	auto fData = (int16_t)(lpf.do_sample(data));
	pd->psData[pd->ulWritePos] = fData;
	params->logger->info(fData);
	params->extraParams->averageCurrent = (float)iir_f.do_sample(fData);
	vGUIUpdateCurrentMonitor();
	pd->ulWritePos++;
	if (pd->ulWritePos >= pd->ulElemCount) {
		pd->ulWritePos = 0;
		pd->bDataFilled = true;
	}
}

void HandleASuccessfulRead(char * lpBuf, WORD dwRead, serialThreadParams_t * params) {
	static std::deque<char> buf;

	for (int i = 0; i < dwRead; i++)
		buf.push_back(lpBuf[i]);

	static std::string line;

	while (buf.size() > 0) {
		char sym = buf.front();
		buf.pop_front();
		if (sym == '\n' || sym == '\r') {
			if (line.length() > 0) {
				try {
					int data = std::stoi(line);
					handleData(data, params);
				}
				catch (std::invalid_argument&) {
					params->logger->error("Invalid string parsing");
				}
				line = "";
			}
			continue;
		}
		line += sym;
	}

}

unsigned __stdcall SecondThreadFunc(void* pArguments) {
	auto prm = (serialThreadParams_t *)pArguments;
	if (!prm)
		return 1;
	auto logger = prm->logger;


	if (lpf.get_error_flag() != 0) // abort in an appropriate manner
		logger->error("Filter not created");
	serialPort = CreateFile(prm->portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		0,
		NULL);
	if (serialPort == INVALID_HANDLE_VALUE) {
		return 2;
	}

	// Serial port settings
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(serialPort, &dcbSerialParams))
	{
		logger->error("getting state error");
	}
	dcbSerialParams.BaudRate = CBR_115200;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	if (!SetCommState(serialPort, &dcbSerialParams))
	{
		logger->error("error setting serial port state");
	}


	DWORD dwRead;
	char lpBuf[10];
	logger->info("Starting serial read thread from {}", (char *)prm->portName);

	// Issue read operation.
	while (ReadFile(serialPort, lpBuf, sizeof(lpBuf), &dwRead, NULL) && !prm->exitFlag) {
		// read completed immediately
		//logger->info("Read 1 byte");
		HandleASuccessfulRead(lpBuf, dwRead, prm);
	}

	logger->info("Leaving serial read thread");
	CloseHandle(serialPort);
	serialPort = INVALID_HANDLE_VALUE;
	;// throw TTYException();
	return 0;
}

bool PlotDataCollectorStart(serialThreadParams_t* params) {
	// TODO: разместите код здесь.
	serialThread = (HANDLE)_beginthreadex(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		&SecondThreadFunc,       // thread function name
		&params,                   // argument to thread function 
		0,                      // use default creation flags 
		&dwThreadId);   // returns the thread identifier 

	if (serialThread)
		return true;

	return false;
}

HANDLE getSerialThreadHandle() {
	return serialThread;
};