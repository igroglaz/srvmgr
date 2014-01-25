#pragma once

#include "syslib.h"
#include <stdint.h>
#include <string>

uint32_t ParseFlags(std::string string);
void RunCommand(byte* _this, byte* player, const char* command, uint32_t rights, bool console);
bool AllyPlayers(byte* player1, byte* player2);
int32_t OnDamage(byte* unit1, byte* unit2, int16_t damage);
void __stdcall cheat_codes_2(byte* player, const char* command);