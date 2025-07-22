#pragma once

#include <cstdint>
#include <unordered_map>

#include "a2types.h"

extern "C" void __fastcall PoisonStapleCell(A2Position* pos);

extern "C" void __fastcall StapleCellOnMobKill(A2Unit* killed_unit);

A2Node<A2Bag>* FindSack(uint16_t pos_yx);

void ClearStapleCells();

bool IsSoloPlayer(const A2Unit* unit);
bool IsGigaPlayer(const A2Unit* unit);
bool IsPureSoloPlayer(const A2Unit* unit);
