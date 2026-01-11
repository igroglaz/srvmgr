#include "a2types.h"
#include "lib/utils.hpp"
#include "player_settings.h"
#include "quests.h"

auto a2PickSpell = (void* (__stdcall *)(void* unit, int spell_number, int unused))(0x005a79d6);

bool CanCast(A2Unit* unit, int spell_number) {
    if (unit->player->unitType) {
        return true;
    }
    auto player_settings = settings::Find(unit->player->name);
    if (player_settings && (player_settings->autobuff_mask & (1 << spell_number))) {
        return false;
    }
    return true;
}

void* __stdcall autobuff(A2Unit* unit, int spell_number, int unused) {
    if (!CanCast(unit, spell_number)) {
        return nullptr;
    }
    return a2PickSpell(unit, spell_number, unused);
}
