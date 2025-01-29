#pragma once

#include <cstdint>
#include <unordered_map>

#include "utils.h"

extern "C" void __fastcall PoisonStapleCell(A2Position* pos);

extern "C" void __fastcall StapleCellOnMobKill(T_UNIT* killed_unit);

T_SRV_LINKED_NODE<A2Bag>* FindSack(int16_t pos_yx);

void ClearStapleCells();

// Is current player in giga mode --- solo mode + 0 deaths.
bool IsGigaPlayer(T_UNIT* unit);
