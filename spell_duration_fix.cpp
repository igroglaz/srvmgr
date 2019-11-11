#define FTOL 0x005BF1AC
void __declspec(naked) fix_spell_duration_overflow_0053973E()
{ // 0053973E
    __asm
    {
		cmp		eax, 0xFFFF
		jle		short ret_point1
		mov		eax, 0xFFFF
ret_point1:
		// restore overridden code begin
		mov     edx, [ebp-0xC]
		mov     [edx+10h], ax
		// restore overridden code 
		// jump back
        mov     edx, 0x00539745
        jmp     edx
    }
}
// FUNCTIONS BELOW ARE DIFFERENT
void __declspec(naked) fix_spell_duration_overflow()
{ // 53BF9C
    __asm
    {
		mov		eax, FTOL
		call    eax
		cmp		eax, 0xFFFF
		jle		short ret_point
		mov		eax, 0xFFFF
ret_point:
		ret
    }
}
// FUNCTIONS BELOW ARE DIFFERENT
void __declspec(naked) fix_spell_duration_overflow_0053BFA1()
{ // 0053BFA1
    __asm
    {
		cmp		eax, 0xFFFF
		jle		short ret_point2
		mov		eax, 0xFFFF
ret_point2:
		// restore overridden code begin
		mov     edx, [ebp-0x44]
		mov     [edx+0x42], ax
		// restore overridden code 
		// jump back
        mov     edx, 0x0053BFA8
        jmp     edx
    }
}

void __declspec(naked) fix_spell_duration_overflow_0053C0E1()
{ // 0053C0E1
    __asm
    {
		cmp		eax, 0xFFFF
		jle		short ret_point3
		mov		eax, 0xFFFF
ret_point3:
		// restore overridden code begin
		mov     edx, [ebp-0x44]
		mov     [edx+0x42], ax
		// restore overridden code 
		// jump back
        mov     edx, 0x0053C0E8
        jmp     edx
    }
}

void __declspec(naked) fix_spell_duration_overflow_0053C21E()
{ // 0053C21E
    __asm
    {
		cmp		eax, 0xFFFF
		jle		short ret_point4
		mov		eax, 0xFFFF
ret_point4:
		// restore overridden code begin
		mov     edx, [ebp-0x44]
		mov     [edx+0x42], ax
		// restore overridden code 
		// jump back
        mov     edx, 0x0053C225
        jmp     edx
    }
}

void __declspec(naked) fix_spell_duration_overflow_0053C52A()
{ // 0053C52A
    __asm
    {
		cmp		eax, 0xFFFF
		jle		short ret_point5
		mov		eax, 0xFFFF
ret_point5:
		// restore overridden code begin
		mov     edx, [ebp-0x44]
		mov     [edx+0x42], ax
		// restore overridden code 
		// jump back
        mov     edx, 0x0053C531
        jmp     edx
    }
}