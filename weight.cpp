#include <cstdint>

#include "a2types.h"

// Address: 0052a29e.
void __declspec(naked) replace_unit_carrying_body_with_reaction() {
    A2Unit* unit;

    __asm {
        mov [unit], eax
    }

    uint32_t reaction;
    reaction = unit->reaction;

    __asm {
        mov ecx, DWORD PTR[reaction]

        mov edx, 0x0052a2a5
        jmp edx
    }
}

// Address: 00531a0b.
void __declspec(naked) replace_human_carrying_body_with_reaction() {
    A2Unit* unit;

    __asm {
        mov [unit], edx
    }

    uint32_t reaction;
    reaction = unit->reaction;

    __asm {
        mov eax, DWORD PTR[reaction]

        mov ecx, 0x00531a12
        jmp ecx
    }
}
