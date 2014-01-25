#pragma once
#include "syslib.h"

void SR_UpdateUnit(byte* unit);
void SR_Step();
bool SR_CheckVision(byte* player, uint8_t x, uint8_t y);
void SR_DumpToFile(byte* player);