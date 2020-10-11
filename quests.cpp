#include "config_new.h"

int __declspec(naked) imp_inn_quest_roll_interval()
{ // 0053693C
    __asm
    {
        mov     eax, dword ptr [Config::QuestRollInterval]
        cmp		[edx + 0xD8], eax
		ret
    }
}