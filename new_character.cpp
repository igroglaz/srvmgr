#include "lib/utils.hpp"
#include "a2types.h"

extern "C" __declspec(naked) void skip_new_character_stats() {
    __asm {
        // Jump to `else` block.
        //
        // Decompiled code for reference:
        //
        // if (local_24 < 0) { // We inject at this instruction ...
        //   (local_1c->_).Body = 0x19;
        //   (local_1c->_).Reaction = 0x19;
        //   (local_1c->_).Mind = 0x19;
        //   (local_1c->_).Spirit = 0x19;
        // } else { // ... and jump here.
        //   (local_1c->_).Body = (ushort)body;
        //   (local_1c->_).Reaction = (ushort)reaction;
        //   (local_1c->_).Mind = (ushort)mind;
        //   (local_1c->_).Spirit = (ushort)spirit;
        // }
        mov edx, 0x0050100b
        jmp edx
    }
}

void AddSpell(void* spellbook, int spell_id) {
    // We allocate memory and trust that server will clean it up later. Vanilla does the same way.
    // But here we're using default C++ allocator which might be different from what A2 expects.
    int* area = new int[20]();

    uint8_t spell_id_byte = static_cast<uint8_t>(spell_id);
    void* spell;

    // I couldn't get C++ code to follow the weird calling conventions here, so I'm calling via assembly.
    __asm {
        // Taken from 0x004efc7e.

        // Last argument: spell ID, 1 byte, so pass it in EDX via DL.
        mov edx, 0
        mov dl, spell_id_byte
        push edx

        // First argument: allocated memory region for the spell. Passed in ECX --- fastcall convention.
        mov ecx, area

        // Call Spell::Spell.
        mov eax, 0x00538fdd
        call eax

        // Store result.
        mov spell, eax
    }

    __asm {
        // Taken from 0x004efc97.

        // Last argument: return value from Spell::Spell.
        push spell

        // Second argument: spell ID, 4 bytes.
        push spell_id

        // First argument: spellbook. Passed in ECX --- fastcall convention.
        mov ecx, spellbook

        // Call SpellBook::SetAtGrow.
        mov eax, 0x0053d7f0
        call eax
    }
}

void __stdcall CreateMagicBook(A2Unit* unit, int main_sphere) {
    if (IsWarrior(unit)) {
        return;
    }

    auto spellbook = unit->spellbook;
    if (spellbook == nullptr) {
        Printf("CreateMagicBook: the spellbook is not initialized, cannot add the spell");
        return;
    }

    int spell_id = 0;
    switch (main_sphere) {
        case 1:
            spell_id = 1; // Fire arrow.
            break;
        case 2:
            spell_id = 5; // Ice missile.
            break;
        case 3:
            spell_id = 10; // Lightning.
            break;
        case 4:
            spell_id = 16; // Diamond dust, aka stone missile.
            break;
    }

    if (spell_id == 0) {
        Printf("CreateMagicBook: cannot add the spell for sphere %d", main_sphere);
        return;
    }

    AddSpell(spellbook, spell_id);
}

extern "C" __declspec(naked) void skip_new_character_weapon() {
    // Add the magic arrow to the mage's spellbook.
    __asm {
        push DWORD PTR [ebp+0x8]  // Human object.
        push DWORD PTR [ebp-0x80] // Main sphere.
        call CreateMagicBook
    }

    // Jump to the end of the original function to skip creating the weapon.
    __asm {
        mov edx, 0x0053396e
        jmp edx
    }
}
