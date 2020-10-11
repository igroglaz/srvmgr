#include "syslib.h"
#include "srvmgr_new.h"
#include "config_new.h"
#include "lib\utils.hpp"
#include "srvmgrdef.h"
#include "zxmgr.h"
#include "cheat_codes_new.h"
#include "player_info.h"
#include "srvmgr.h"


int __declspec(naked) imp_inn_quest_roll_interval()
{ // 0053693C
    __asm
    {
        mov     eax, dword ptr [Config::QuestRollInterval]
        cmp		[edx + 0xD8], eax
		ret
    }
}