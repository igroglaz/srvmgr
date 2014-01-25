void _declspec(naked) fix_0051946B()
{
	__asm
	{
		test	edx, edx
		jnz	fix_51946B_cont
		mov	edx, 0x0051950B
		jmp	edx

fix_51946B_cont:
		mov	ecx, [edx+0x0C]
		mov	edx, [ecx+eax]
		mov	[ebp-0x14], edx
		mov	edx, 0x00519474
		jmp	edx
	}
}
