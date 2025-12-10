#include "a2types.h"

void __stdcall distance_bug_fix_internal(A2Unit *unit){
    // State 13 here is "cast point spell from the book onto an enemy that is too far".
    if (unit->state == 13) {
        // Offensive spells are not allowed, defensive ones are ok.
        if (!unit->eye2 || !unit->eye2->spell || !unit->eye2->spell->is_defensive) {
            unit->state = 12; // State 12 is "stop".
        }
    }
}

int __declspec(naked) imp_distance_bug_fix()
{ // 005AAA8D
    __asm
    {
        mov     [ebp-4], ecx    // prefix
        mov        ecx, [ebp+8]
        push    ecx
        call    distance_bug_fix_internal

        mov        eax, [ebp+8]    // postfix
        ret
    }
}
