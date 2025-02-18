#pragma once

#include <cstdint>
#include <unordered_map>

#include "utils.h"

extern "C" void __fastcall PoisonStapleCell(A2Position* pos);

extern "C" void __fastcall StapleCellOnMobKill(T_UNIT* killed_unit);

T_SRV_LINKED_NODE<A2Bag>* FindSack(uint16_t pos_yx);

void ClearStapleCells();

bool IsSoloPlayer(T_UNIT* unit);
bool IsGigaPlayer(T_UNIT* unit);
bool IsPureSoloPlayer(T_UNIT* unit);
