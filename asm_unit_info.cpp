#include "unit_info.h"
#include "lib\utils.hpp"

// Human::construct() #1
void __declspec(naked) imp_UICreate1()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x00602C6D
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		sub		esp, 0x0C
		mov		[ebp-0x14], ecx
		push	UnitType_Human
		push	ecx
		call	UI_Create
		add		esp, 8
		mov		edx, 0x0053234B
		jmp		edx
	}
}

// Human::construct() #2
void __declspec(naked) imp_UICreate2()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x00602C80
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		sub		esp, 0x0C
		mov		[ebp-0x14], ecx
		push	UnitType_Human
		push	ecx
		call	UI_Create
		add		esp, 8
		mov		edx, 0x005323B8
		jmp		edx
	}
}

// Human::delete()
void __declspec(naked) imp_UIDelete1()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	ecx
		mov		[ebp-0x04], ecx
		call	UI_Delete // this should NOT be stdcall!
		mov		edx, 0x0057BBA7
		jmp		edx
	}
}

// CUnit::construct() #1
void __declspec(naked) imp_UICreate3()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x0060282A
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		sub		esp, 0x10
		mov		[ebp-0x1C], ecx
		push	UnitType_Monster
		push	ecx
		call	UI_Create
		add		esp, 8
		mov		edx, 0x00528B0D
		jmp		edx
	}
}

// CUnit::construct() #2
void __declspec(naked) imp_UICreate4()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x00602879
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		sub		esp, 0x0C
		mov		[ebp-0x18], ecx
		push	UnitType_Monster
		push	ecx
		call	UI_Create
		add		esp, 8
		mov		edx, 0x00528CB5
		jmp		edx
	}
}

// CUnit::construct() #3
void __declspec(naked) imp_UICreate5()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x006028C8
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		sub		esp, 0x0C
		mov		[ebp-0x18], ecx
		push	UnitType_Monster
		push	ecx
		call	UI_Create
		add		esp, 8
		mov		edx, 0x00528E42
		jmp		edx
	}
}

// CUnit::construct() #4
void __declspec(naked) imp_UICreate6()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x00602920
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		sub		esp, 0x10
		mov		[ebp-0x1C], ecx
		push	UnitType_Monster
		push	ecx
		call	UI_Create
		add		esp, 8
		mov		edx, 0x00528FD5
		jmp		edx
	}
}

// CUnit::construct() #5
void __declspec(naked) imp_UICreate7()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x00602978
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		sub		esp, 0x10
		mov		[ebp-0x1C], ecx
		push	UnitType_Monster
		push	ecx
		call	UI_Create
		add		esp, 8
		mov		edx, 0x00529185
		jmp		edx
	}
}

// CUnit::delete()
void __declspec(naked) imp_UIDelete2()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	ecx
		mov		[ebp-0x04], ecx
		call	UI_Delete
		mov		edx, 0x0057B867
		jmp		edx
	}
}

// Humanoid::construct() #1
void __declspec(naked) imp_UICreate8()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x00602BEC
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		push	ecx
		mov		[ebp-0x10], ecx
		push	UnitType_Humanoid
		push	ecx
		call	UI_Create
		mov		edx, 0x0052FFBA
		jmp		edx
	}
}

// Humanoid::construct() #2
void __declspec(naked) imp_UICreate9()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x00602BFF
		mov		eax, fs:[0]
		push	eax
		mov		fs:[0], esp
		push	ecx
		mov		[ebp-0x10], ecx
		push	UnitType_Humanoid
		push	ecx
		call	UI_Create
		mov		edx, 0x0053000E
		jmp		edx
	}
}

// Humanoid::delete()
void __declspec(naked) imp_UIDelete3()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		push	ecx
		mov		[ebp-0x04], ecx
		call	UI_Delete
		mov		edx, 0x0057BB77
		jmp		edx
	}
}