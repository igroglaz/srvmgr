#include "a2types.h"

bool __fastcall UnsummonUnit(void* unused, A2Unit* unit) {
    if (!unit || !unit->player || !unit->player->unit_list) {
        return true;
    }

    int summoned_units = 0;

    auto& l = unit->player->unit_list->list;
    for (auto it = l.first_node; it != nullptr; it = it->next) {
        if (it->value && it->value->summoned && it->value->summon_id) {
            ++summoned_units;
        }
    }

    return summoned_units > 1;
}

// Address: 00557985
void __declspec(naked) unsummon_unit() {
    __asm {
        // EDX holds the pointer to the unit.
        call UnsummonUnit

        cmp eax, 0
        jz skip

        // Original instruction.
        mov edx, DWORD PTR[ebp-0x1c]
        mov eax, DWORD PTR[edx+0x148]
        mov ecx, 0x0055798b
        jmp ecx

    skip:
        mov ecx, 0x005579ae
        jmp ecx
    }
}
