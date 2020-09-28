int __declspec(naked) __stdcall this_call_impl()
{
    __asm
    {
        pop     eax     // pop esp
		pop		eax		// pop eax from the wrapper
        pop     edx     // pop method ptr
        pop     ecx     // pop this
		push	eax		// restore esp from call to the wrapper
        jmp     edx     // jump to the method
    }
}
int __declspec(naked) __stdcall this_call(int func, void* thisPtr)
{
    __asm
    {
        call	this_call_impl
    }
}
int __declspec(naked) __stdcall this_call(int func, void* thisPtr, void* a1, void* a2)
{
    __asm
    {
        call	this_call_impl
    }
}
int __declspec(naked) __stdcall this_call(int func, void* thisPtr, void* a1, void* a2, void* a3)
{
    __asm
    {
        call	this_call_impl
    }
}