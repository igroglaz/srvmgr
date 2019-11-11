void __declspec(naked) fix_spell_duration_overflow_0053973E()
{ // 0053973E
    __asm
    {
		cmp		eax, 0xFFFF
		jle		short ret_point
		mov		eax, 0xFFFF
ret_point:
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
{ // 0053BFA1
    __asm
    {
		cmp		eax, 0xFFFF
		jle		short ret_point
		mov		eax, 0xFFFF
ret_point:
		mov     edx, [ebp-0x44]
		mov     [edx+0x42], ax
		pop 	eax
		inc 	eax
		push 	eax
		ret
    }
}