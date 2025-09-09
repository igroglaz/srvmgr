#include "a2types.h"

void __fastcall PreventSuicideRegular(A2Unit* unit, A2Unit* attacker, A2HitInfo* hit_info) {
    if (unit != attacker) {
        unit->last_hit_by = attacker;

        if (IsWarrior(attacker)) {
            unit->last_hit_spell = 0;
        } else if (hit_info) {
            unit->last_hit_spell = hit_info->spell_id;
        }
    }
}

// Address: 005370ed.
void __declspec(naked) prevent_suicide_regular() {
    __asm {
        mov ecx, DWORD PTR [ebp+0x8]  // Hit info
        push ecx
        mov ecx, DWORD PTR[ebp-0x3c]  // Target
        mov edx, DWORD PTR[ebp+0xc]  // Attacker
        call PreventSuicideRegular

        mov eax, 0x00537126
        jmp eax
    }
}

void __fastcall PreventSuicideDrainLife(A2Unit* unit, A2Unit* attacker, A2Spell* spell) {
    if (unit != attacker) {
        unit->last_hit_by = attacker;
        unit->last_hit_spell = spell->spell_id;
    }
}

// Address: 0053a7cc.
void __declspec(naked) prevent_suicide_drain_life() {
    __asm {
        mov ecx, DWORD PTR [ebp-0x2fc] // Spell
        push ecx
        mov ecx, DWORD PTR[ebp-0x5c]  // Target
        mov edx, DWORD PTR[ebp+0x8]  // Attacker
        call PreventSuicideDrainLife

        mov ecx, 0x0053a7e4
        jmp ecx
    }
}

void __fastcall PreventSuicidePointEffect(A2Unit* unit, A2Unit* attacker, uint8_t spell_id) {
    if (unit != attacker) {
        unit->last_hit_by = attacker;
        unit->last_hit_spell = spell_id;
    }
}

// Address: 0053766c.
void __declspec(naked) prevent_suicide_point_effect() {
    __asm {
        mov edx, DWORD PTR [ebp-0xc]  // Point effect
        mov ecx, DWORD PTR [edx+0x4c]  // Its effect (maybe it's A2Effect*, not sure)
        mov cl, BYTE PTR [ecx+0xc]  // Spell ID
        push ecx

        mov ecx, DWORD PTR[ebp-0x4]  // Target
        mov edx, DWORD PTR[edx+0x3c]  // Point effect caster
        call PreventSuicidePointEffect

        mov eax, 0x00537687
        jmp eax
    }
}

void __fastcall PreventSuicideAreaEffect(A2Unit* unit, A2Unit* attacker, uint8_t spell_id) {
    if (unit != attacker) {
        unit->last_hit_by = attacker;
        unit->last_hit_spell = spell_id;
    }
}

// Address: 00538473.
void __declspec(naked) prevent_suicide_area_effect() {
    __asm {
        mov edx, DWORD PTR[ebp-0x78]  // Area effect
        mov ecx, DWORD PTR[edx+0x48]  // Its effect (maybe it's A2Effect*, not sure)
        mov cl, BYTE PTR[ecx+0xc]  // Spell ID
        push ecx

        mov ecx, DWORD PTR[ebp+0x8]  // Target
        mov edx, DWORD PTR[edx+0x3c]  // Effect caster
        call PreventSuicideAreaEffect

        mov edx, 0x0053849a
        jmp edx
    }
}
