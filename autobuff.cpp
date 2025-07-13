#include "a2types.h"
#include "lib/utils.hpp"
#include "quests.h"

auto a2PickSpell = (void* (__stdcall *)(void* unit, int spell_number, int unused))(0x005a79d6);

bool CanCast(A2Unit* unit, int spell_number) {
    CheckPlayerSettings(unit->player);

    auto autobuff_mask = player_settings[unit->player->id_ext.id]->autobuff_mask;
    if (autobuff_mask & (1 << spell_number)) {
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
