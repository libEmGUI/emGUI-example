#pragma once

#include "resource.h"
#include "GUI.h"


void handleData(int data, uint16_t * data_array);
void HandleAChar(char sym, uint16_t * data_array);
void HandleASuccessfulRead(char * lpBuf, WORD dwRead, uint16_t * data_array);
unsigned __stdcall SecondThreadFunc(void* pArguments);