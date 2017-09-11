#pragma once

#include "resource.h"
#include "GUI.h"


#include <spdlog/spdlog.h>

using namespace std;
#define DATA_ARRAY_SIZE 100
std::shared_ptr<spdlog::logger> logger;

typedef struct {
	LPCWSTR portName;
	bool exitFlag;
	int16_t data[DATA_ARRAY_SIZE];
	bool        bDataFilled;      ///< флажок, показывающий, что данные заполнены (сняты хотя бы в 1 проход)
	uint32_t    ulElemCount;      ///< количество отсчётов в текущей сессии записи
	uint32_t    ulCapacity;       ///< максимальное количество отсчётов в буфере
	uint32_t    ulWritePos = 0;       ///< позиция указателя записи в буфере

	std::shared_ptr<spdlog::logger> logger;
}serialThreadParams_t;
void handleData(int data, serialThreadParams_t * params);
void HandleAChar(char sym, serialThreadParams_t * params);
void HandleASuccessfulRead(char * lpBuf, WORD dwRead, serialThreadParams_t * params);

unsigned __stdcall SecondThreadFunc(void* pArguments);