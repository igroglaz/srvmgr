#pragma once
#include "a2types.h"
#include "syslib.h"

void SR_UpdateUnit(A2Unit* unit);
void SR_Step();
bool SR_CheckVision(A2Player* player, uint8_t x, uint8_t y);
void SR_DumpToFile(A2Player* player);
