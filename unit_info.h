#pragma once
#include <vector>
#include "a2types.h"
#include "syslib.h"

#define UnitType_Null     0
#define UnitType_Monster  1
#define UnitType_Human    2
#define UnitType_Humanoid 3

struct Unit
{
    uint32_t Type;
    bool Invalid;
    A2Unit* Class;

    uint32_t VisibleFlags;
    uint32_t VisibleFlagsLast;
};

extern std::vector<Unit> Units;
void UI_Create(A2Unit* cunit, uint32_t type);
void UI_Clear(Unit& unit, A2Unit* cunit, uint32_t type);
void UI_Delete(A2Unit* cunit);
void UI_Tick();
void UI_Destruct(A2Unit* cunit);
void UI_Reset();
Unit* UI_Get(A2Unit* cunit);
