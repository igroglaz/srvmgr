#include <cstdint>

#include "a2types.h"
#include "circle.h"

// Address: 0052a29e.
void __declspec(naked) replace_unit_carrying_body_with_reaction() {
    A2Unit* unit;

    __asm {
        mov [unit], eax
    }

    uint32_t value;
    
    // Check if unit and player are valid, and if it's a circle witch
    if (unit && unit->player && 
        circle::Circle(unit) > 0 && !IsWarrior(unit) && IsFemale(unit)) {
        // Use reaction for circle witches
        value = unit->reaction;
    } else {
        // Vanilla way for everyone else
        value = unit->body;
    }

    __asm {
        mov ecx, DWORD PTR[value]

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

    uint32_t value;
    
    // Check if unit and player are valid, and if it's a circle witch
    if (unit && unit->player && 
        circle::Circle(unit) > 0 && !IsWarrior(unit) && IsFemale(unit)) {
        // Use reaction for circle witches
        value = unit->reaction;
    } else {
        // Vanilla way for everyone else
        value = unit->body;
    }

    __asm {
        mov eax, DWORD PTR[value]

        mov ecx, 0x00531a12
        jmp ecx
    }
}