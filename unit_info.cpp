#include "unit_info.h"
#include "srvmgr.h"
#include "lib\utils.hpp"

std::vector<Unit> Units;

void UI_Clear(Unit& unit, byte* cunit, uint32_t type)
{
	unit.Type = type;
	unit.Class = cunit;
	unit.Invalid = (unit.Class && *(byte**)(unit.Class+0x14));
	unit.VisibleFlags = 0;
	unit.VisibleFlagsLast = 0;
}

void UI_Create(byte* cunit, uint32_t type)
{
	bool already_exists = false;
	for(std::vector<Unit>::iterator it = Units.begin();
		it != Units.end(); ++it)
	{
		Unit& unit = (*it);
		if(unit.Class == cunit)
			return;
	}
	
	if(already_exists) return;

	Unit unit;
	UI_Clear(unit, cunit, type);
	Units.push_back(unit);
}

void UI_Delete(byte* cunit)
{
	for(std::vector<Unit>::iterator it = Units.begin();
		it != Units.end(); ++it)
	{
		Unit& unit = (*it);
		if(unit.Class == cunit)
		{
			Units.erase(it);
			return;
		}
	}
}

void UI_Destruct(byte* cunit)
{
	if(cunit)
	{
		for(std::vector<Unit>::iterator it = Units.begin();
			it != Units.end(); ++it)
		{
			Unit& unit = (*it);
			if(unit.Class == cunit)
			{
				uint32_t size = 0x254;
				if(unit.Type == UnitType_Monster) size = 0x208;
				if(!IsBadReadPtr(unit.Class, size))
				{
					__asm
					{
						push	1
						mov		ecx, [cunit]
						mov		edx, [ecx]
						call	dword ptr [edx+4]
					}
				}
			}
		}
	}
}

void UI_Tick()
{
	//...
}

Unit* UI_Get(byte* cunit)
{
	if(!cunit) return NULL;
	for(std::vector<Unit>::iterator it = Units.begin();
		it != Units.end(); ++it)
	{
		Unit& unit = (*it);
		if(unit.Class == cunit)
			return &unit;
	}

	return NULL;
}

void UI_Reset()
{
	Units.clear();
}