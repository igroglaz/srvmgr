#include "zxmgr.h"
#include "srvmgr.h"
bool block_invis = false;

char aAttackInvisible[] = "[invis_fix] %s/%s attacks %s!\n";

void __declspec(naked) stop_unit_attack() // fixes attack on invisible units
{
    __asm
    {
        // [ebp-0x14] - attacking player
        // [ebp-0x28] - target unit
        cmp        [ebp-0x14], 0
        jz        continue_cmd
        cmp        [ebp-0x28], 0
        jz        continue_cmd
        mov        eax, [ebp-0x28] // unit
        mov        eax, [eax+0x144]
        and        eax, 0x1000
        test    eax, eax
        jz        continue_cmd // visible
        mov        edx, 0x006A8B8C
        mov        edx, [edx]
        mov        eax, [ebp-0x28]
        mov        eax, [eax+0x14]
        movsx    ebx, [eax+0x04]
        imul    ebx, 0x46
        mov        eax, [ebp-0x14]
        movsx    ecx, [eax+0x04]
        add        ebx, ecx
        add        ebx, 0xA8C4
        add        ebx, edx
        test    byte ptr [ebx], 0x10
        jnz        continue_cmd
        mov        eax, [ebp-0x28]
        mov        eax, [eax+0x14]
        push    [eax+0x18] // target nickname
        mov        eax, [ebp-0x14]
        push    [eax+0x18] // attacker nickname
        push    [eax+0x0A78] // attacker login
        push    offset aAttackInvisible
        call    log_format // log cheater
        // в бан всех читоров
        cmp        byte ptr [block_invis], 0
        jz        continue_cmd
        mov        edx, 0x0050864E // exit from sub
        jmp        edx
continue_cmd:
        mov        edx, 0x006D1648
        cmp        dword ptr [edx], 0x02
        jnz        l_505580
        mov        edx, 0x0050556C
        jmp        edx
l_505580:
        mov        edx, 0x00505580
        jmp        edx
    }
}

 char aCastInvisible[] = "[invis_fix] %s/%s casts spell onto %s!\n";

void __declspec(naked) stop_unit_cast() // fixes spellcasting onto invisible units
{
    __asm
    {
        // [ebp-0x14] - attacking player
        // [ebp-0x28] - target unit
        cmp        [ebp-0x14], 0
        jz        continue_cmd
        cmp        [ebp-0x28], 0
        jz        continue_cmd
        mov        eax, [ebp-0x28] // unit
        mov        eax, [eax+0x144]
        and        eax, 0x1000
        test    eax, eax
        jz        continue_cmd // visible
        mov        edx, 0x006A8B8C
        mov        edx, [edx]
        mov        eax, [ebp-0x28]
        mov        eax, [eax+0x14]
        movsx    ebx, [eax+0x04]
        imul    ebx, 0x46
        mov        eax, [ebp-0x14]
        movsx    ecx, [eax+0x04]
        add        ebx, ecx
        add        ebx, 0xA8C4
        add        ebx, edx
        test    byte ptr [ebx], 0x10
        jnz        continue_cmd
        mov        eax, [ebp-0x28]
        mov        eax, [eax+0x14]
        push    [eax+0x18] // target nickname
        mov        eax, [ebp-0x14]
        push    [eax+0x18] // attacker nickname
        push    [eax+0x0A78] // attacker login
        push    offset aCastInvisible
        call    log_format // log cheater
        // в бан всех читоров
        cmp        byte ptr [block_invis], 0
        jz        continue_cmd
        mov        edx, 0x0050864E // exit from sub
        jmp        edx
continue_cmd:
        mov        edx, 0x006D1648
        cmp        dword ptr [edx], 0x02
        jnz        l_50566A
        mov        edx, 0x00505656
        jmp        edx
l_50566A:
        mov        edx, 0x0050566A
        jmp        edx
    }
}

void __declspec(naked) stop_mage_cast() // fixes spellcasting onto invisible units
{
    __asm
    {
        // [ebp-0x14] - attacking player
        // [ebp-0x28] - target unit
        cmp        [ebp-0x14], 0
        jz        continue_cmd
        cmp        [ebp-0x28], 0
        jz        continue_cmd
        mov        eax, [ebp-0x28] // unit
        mov        eax, [eax+0x144]
        and        eax, 0x1000
        test    eax, eax
        jz        continue_cmd // visible
        mov        edx, 0x006A8B8C
        mov        edx, [edx]
        mov        eax, [ebp-0x28]
        mov        eax, [eax+0x14]
        movsx    ebx, [eax+0x04]
        imul    ebx, 0x46
        mov        eax, [ebp-0x14]
        movsx    ecx, [eax+0x04]
        add        ebx, ecx
        add        ebx, 0xA8C4
        add        ebx, edx
        test    byte ptr [ebx], 0x10
        jnz        continue_cmd
        mov        eax, [ebp-0x28]
        mov        eax, [eax+0x14]
        push    [eax+0x18] // target nickname
        mov        eax, [ebp-0x14]
        push    [eax+0x18] // attacker nickname
        push    [eax+0x0A78] // attacker login
        push    offset aAttackInvisible
        call    log_format // log cheater
        // в бан всех читоров
        cmp        byte ptr [block_invis], 0
        jz        continue_cmd
        mov        edx, 0x0050864E // exit from sub
        jmp        edx
continue_cmd:
        mov        edx, 0x006D1648
        cmp        dword ptr [edx], 0x02
        jnz        l_50561A
        mov        edx, 0x00505606
        jmp        edx
l_50561A:
        mov        edx, 0x0050561A
        jmp        edx
    }
}