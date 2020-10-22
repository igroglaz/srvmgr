#include "utils.h"

void __stdcall distance_bug_fix_internal(T_UNIT *unit){
	if(unit->dword50 == 13){
		unit->dword50 = 12;
	}
}

int __declspec(naked) imp_distance_bug_fix()
{ // 005AAA8D
    __asm
    {
        mov     [ebp-4], ecx	// prefix
		mov		ecx, [ebp+8]
		push	ecx
		call	distance_bug_fix_internal







ret_point:
		mov		eax, [ebp+8]	// postfix
		ret
    }
}