#include "syslib.h"
#include "lib\utils.hpp"

void __declspec(naked) imp_PrismaticSprayStruc()
{
    __asm
    {
		push	ebp
		mov		ebp, esp
		sub		esp, 0x1C
		mov		[ebp-0x18], ecx
		mov		[ebp-0x14], 0
		cmp		[ebp+0x08], 0
		jz		pss_skip
		mov		eax, 0x005ADB26
		jmp		eax

pss_skip:
		mov		eax, 0x005ADD5E
		jmp		eax
    }
}

bool _stdcall check_ptr(void* mem, unsigned long size)
{
	return IsBadReadPtr(mem, size);
}

void __declspec(naked) imp_ShopCrash1()
{
	__asm
	{
		mov		eax, [ebp-0x2C]
		push	4
		push	eax
		call	check_ptr
		cmp		eax, 1
		jz		sc1_end_execution
//sc1_ok:
		mov		edx, [ebp-0x34]
		mov		eax, [ebp-0x2C]
		mov		ecx, [eax]
		mov		[edx], ecx
		mov		edx, 0x00549C3E
		jmp		edx
sc1_end_execution:
		mov		[ebp-0x10], 0
		mov		edx, 0x00549C56
		jmp		edx
	}
}

void __declspec(naked) imp_ShopCrash2()
{
	__asm
	{
		mov		eax, [ebp-0x14]
		push	4
		push	eax
		call	check_ptr
		cmp		eax, 1
		jz		sc2_end_execution
		push	1
		mov		ecx, [ebp-0x14]
		mov		edx, [ecx]
		mov		ecx, [ebp-0x14]
		call	dword ptr [edx+4]
		mov		[ebp-0x34], eax
		jmp		sc2_end
sc2_end_execution:
		mov		[ebp-0x34], 0
sc2_end:
		mov		edx, 0x0053D3FC
		jmp		edx
	}
}

void __declspec(naked) imp_ShopCrash3()
{
	__asm
	{
		retn
	}
}

void __declspec(naked) cplex_free_crash()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		test	ecx, ecx
		jz		cpfc_ret
		sub		esp, 8
cpfc_again:
		mov		[ebp-0x08], ecx
		push	4
		push	ecx
		call	check_ptr
		cmp		eax, 1
		jz		cpfc_ret
		mov		ecx, [ebp-0x08]
		mov		eax, [ecx]
		mov		[ebp-0x04], eax
		push	ecx
		mov		edx, 0x005DDF90
		call	edx
		add		esp, 4
		cmp		[ebp-0x04], 0
		mov		ecx, [ebp-0x04]
		jnz		cpfc_again
cpfc_ret:
		mov		esp, ebp
		pop		ebp
		retn
	}
}

char aCrashDetected[] = "Shop crash detected, transaction discarded.\n";

void __declspec(naked) imp_ShopCrash4()
{
	__asm
	{
		mov		eax, [ebp-0x58]
		mov		ecx, [ebp-0x54]
		sub		ecx, [eax+0x08]
		mov		[ebp-0x40], ecx
		mov		edx, [ebp-0x58]
		mov		eax, [edx+0x08]
		mov		ecx, [ebp-0x58]
		mov		edx, [ecx+0x04]
		lea		eax, [edx+eax*0x04]
		mov		[ebp-0x3C], eax
		push	4
		push	eax
		call	check_ptr
		cmp		eax, 1
		jz		sc4_abort
		mov		edx, 0x00544E64
		jmp		edx
sc4_abort:
		push	offset aCrashDetected
		call	Printf
		mov		edx, 0x0054501C
		jmp		edx
	}
}
