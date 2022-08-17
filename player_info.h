#pragma once

#include "syslib.h"
#include <vector>

struct Player
{
	bool Exists;
	byte* Class;
	
	bool Casted;
	bool ShouldReturn;
	uint32_t LastReturn;
	bool GodMode;
	byte* GodSetter;

	int8_t SetSpells;
	uint32_t LastSpells;
	byte* SpellSetter;

	byte CastSpell[0x14];
	std::vector<byte*> SavedItems;

	uint32_t Vision[256][256];
};

extern Player Players[32];

Player* PI_Get(byte* player);
void PI_Reset();
void PI_Clear(Player& struc);
void PI_Create(byte* player);
void PI_Delete(byte* player);