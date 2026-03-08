#pragma once

#include <cstdint>
#include <unordered_map>

#include "a2types.h"

void __fastcall PoisonStapleCell(A2Position* pos);

void __fastcall StapleCellOnMobKill(A2Unit* killed_unit);

A2Node<A2Bag>* FindSack(uint16_t pos_yx);

void ClearStapleCells();

bool IsSoloPlayer(const A2Unit* unit);
bool IsGigaPlayer(const A2Unit* unit);
bool IsIronPlayer(const A2Unit* unit);
