#include "srvmgr.h"

void _stdcall say_all(const char *s) 
{
    _asm 
    {
        mov    eax, dword ptr [s]
        push    eax
        call    say_all2
        add    esp, 4
    }
}

void _declspec(naked) say_all2(void) 
{
    __asm 
    {///TESTED100%  
        push    ebp
        mov    ebp, esp
        sub    esp, 8
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496DC0
        call    edx    // ??0Iostream_init@@QAE@XZ
                        // doubtful name
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        push    ecx
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496DD0
        call    edx

loc_50303D:                 // CODE XREF: sub_50301F+30j
        test    eax, eax
        jz    loc_503051
        mov    edx, [ebp + 8]
        push    edx
        push    eax
        call    send_to_player
//        add    esp, 8
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496E20
        call    edx
        jmp    loc_50303D
// ---------------------------------------------------------------------------

loc_503051:                 // CODE XREF: sub_50301F+20j
        mov    esp, ebp
        pop    ebp
        ret
    }
}



void _declspec(naked) kick_all(void) 
{
    __asm 
    {///TESTED100%  
        push    ebp
        mov    ebp, esp
        sub    esp, 8
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496DC0
        call    edx    // ??0Iostream_init@@QAE@XZ
                        // doubtful name
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        push    ecx
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496DD0
        call    edx

loc_50303D:                 // CODE XREF: sub_50301F+30j
        test    eax, eax
        jz    loc_503051
        push    eax
        call    kick_char
        add    esp, 4
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496E20
        call    edx
        jmp    loc_50303D
// ---------------------------------------------------------------------------

loc_503051:                 // CODE XREF: sub_50301F+20j
        mov    esp, ebp
        pop    ebp
        ret
    }
}


void _declspec(naked) upd_all(void) 
{ 
    __asm 
    {///TESTED100%  
        push    ebp
        mov    ebp, esp
        sub    esp, 8
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496DC0
        call    edx    // ??0Iostream_init@@QAE@XZ
                        // doubtful name
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        push    ecx
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496DD0
        call    edx

loc1_50303D:                 // CODE XREF: sub_50301F+30j
        test    eax, eax
        jz    loc1_503051
        cmp    dword ptr [eax+2Ch], 0
        jnz    loc_skip///AI
        mov    eax, [eax+0x38]
        push    eax
        mov    ecx, 0x642C2C
        mov    ecx, [ecx]
        mov    edx, 0x4EE028
        call    edx //// принудительное сохранение
loc_skip:
        lea    ecx, [ebp-0x08]
        mov    edx, 0x496E20
        call    edx
        jmp    loc1_50303D
// ---------------------------------------------------------------------------

loc1_503051:                 // CODE XREF: sub_50301F+20j
        mov    esp, ebp
        pop    ebp
        retn
    }
}


void _declspec(naked) map_exit(void) 
{
    __asm 
    {///// STDCALL TESTED100%
        push    ebp  /// 04F94C0
        mov    ebp, esp
        push    ecx
        mov    [ebp-0x04], ecx
        mov    ecx, 0x06D1648 ///тип игры
        cmp    dword ptr [ecx], 1 
        jz    loc_4F94D9
        cmp    dword ptr [ecx], 2
        jnz    loc_4F94E5    // обычная

loc_4F94D9:                 // CODE XREF: sub_4F94C0+Ej
        push    0
        mov    ecx, 0x6C3A08
        mov    edx, 0x51D6B4
        call    edx

loc_4F94E5:                 // CODE XREF: sub_4F94C0+17j
        call    kick_all
        mov    ecx, [ebp-0x04]
        mov    dword ptr [ecx+208h], 1

//loc_4F9509:                 // CODE XREF: sub_4F94C0+37j
        mov    esp, ebp
        pop    ebp
        retn    4
    }
}


/*
void _declspec(naked) kick_char(void) 
{
    __asm 
    {////TESTED100%, CDECL
        push    ebp
        mov    ebp, esp
        mov    eax, [ebp+0x08] 
        test    eax, eax
        jz    loc_502E48
        cmp    dword ptr [eax+2Ch], 0
        jnz    loc_502E48
        mov    eax, [eax+0x38]
        push    eax
        mov    ecx, 0x642C2C
        mov    ecx, [ecx]
        mov    edx, 0x4EE028
        call    edx //// принудительное сохранение
        mov    eax, [ebp+0x08] 
        xor    ecx, ecx
        mov    cx, [eax+4]
        push    ecx
        mov    ecx, 0x6C3A08
        mov    edx, 0x518544
        call    edx
        push    eax
        mov    ecx, 0x6C3A08
        mov    edx, 0x5170B6
        call    edx
        mov    ecx, 0x6C3A08
        mov    edx, 0x51800F
        call    edx
        mov    eax, [ebp+0x08]
        push    eax
        mov    ecx, 0x6C3A08
        mov    edx, 0x51D49B
        call    edx
        mov    ecx, 0x0642C2C
        mov    ecx, [ecx]
        mov    edx, [ecx]
        dec    edx
        mov    eax, [ebp+0x08]
        mov    [eax+0A50h], edx
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        mov    edx, 0x534DDD
        call    edx

loc_502E48:    
        mov    esp, ebp
        pop    ebp
        retn
    }
}

*/
 void _declspec(naked) kick_char(void) 
 {
    __asm 
    {////TESTED100%, CDECL
        push ebp
        mov ebp, esp
        mov eax, [ebp+0x08] 
        test eax, eax
        jz loc_502E48
        cmp dword ptr [eax+2Ch], 0
        jnz loc_502E48
        mov eax, [eax+0x38]
        push eax
        mov ecx, 0x642C2C
        mov ecx, [ecx]
        mov edx, 0x4EE028
        call edx //// принудительное сохранение
        mov eax, [ebp+0x08] 
        xor ecx, ecx
        mov cx, [eax+4]
        push ecx
        mov ecx, 0x6C3A08
        mov edx, 0x518544
        call edx
        push eax
        mov ecx, 0x6C3A08
        mov edx, 0x5170B6
        call edx
        mov ecx, 0x6C3A08
        mov edx, 0x51800F
        call edx

        /// уведомление "Игрок был выкикан с сервера 
        mov eax, [ebp+0x08]
        mov eax, [eax+14h]
        shr eax, 18h
        cmp al, 3Fh
        jnz k_c_c ///не админ, продолжаем
        mov eax, [edx+14h]
        test eax, 0x800 /// 
        jz k_c_c

        jmp k_c_s
        k_c_c:
        mov eax, [ebp+0x08]
        push eax
        mov ecx, 0x6C3A08
        mov edx, 0x51D49B
        call edx 
        /// уведомление "Игрок был выкикан с сервера 
        k_c_s:
        mov ecx, 0x0642C2C
        mov ecx, [ecx]
        mov edx, [ecx]
        dec edx
        mov eax, [ebp+0x08]
        mov [eax+0A50h], edx
        mov ecx, 0x06CDB24
        mov ecx, [ecx]
        mov edx, 0x534DDD
        call edx

        loc_502E48: 
        mov esp, ebp
        pop ebp
        retn
    }
}   

 void kick_by_name(const char *s) 
 {
    __asm 
    {
        push dword ptr [s]
        call kick_by_name_2
        add esp, 4
    }
}

void _declspec(naked) kick_by_name_2()  /// doesnt work :(((
{
    __asm 
    {///TESTED100%  
        push ebp
        mov ebp, esp
        sub esp, 0x0C
        lea ecx, [ebp-0x08]
        mov edx, 0x496DC0
        call edx // ??0Iostream_init@@QAE@XZ
        // doubtful name
        mov ecx, 0x06CDB24
        mov ecx, [ecx]
        push ecx
        lea ecx, [ebp-0x08]
        mov edx, 0x496DD0
        call edx

        loc_50303D: // CODE XREF: sub_50301F+30j
        test eax, eax
        jz loc_503051
        cmp dword ptr [eax+2Ch], 0
        jnz loc_503051

        mov [ebp-0x0c], eax // unit (?)
        //mov eax, [eax + 0x14] // player
        mov eax, [eax + 0x0A78] // вроде как имя логина
        push eax
        mov eax, [ebp+0x08]
        push eax
        mov edx, 0x05BEDB0
        call edx
        test eax, eax
        jnz loc_503051

        mov eax, [ebp-0x0C]
        push eax
        call kick_char
        add esp, 4
        lea ecx, [ebp-0x08]
        mov edx, 0x496E20
        call edx
        jmp loc_50303D
        // ---------------------------------------------------------------------------

        loc_503051: // CODE XREF: sub_50301F+20j
        mov esp, ebp
        pop ebp
        ret
    }
}  

void kick(const char *name) // nick name
{
    _asm
    { ///TESTED100%
            push    dword ptr [name]    // name (arg_0)
            call    get_player_by_name    // get player by name
            push    eax
            call    kick_char
            add    esp, 4
    }
}

void __stdcall send_to_player(void *p, const char *msg) {
    __asm {///TESTED100%, STDCALL
        mov    ecx, dword ptr [p]
        push    ecx
        lea    edx, dword ptr [msg]
        push    edx
        mov    ecx, 0x6C3A08
        mov    edx, 0x51CD89
        call    edx
    }
}


void * __stdcall get_player_by_name(const char *name)
{
    __asm
    {
        sub    esp, 4
        mov    ecx, esp
        push    dword ptr [name]    // name (arg_0)
        mov    edx, 0x005DD8F8 // CString::CString()
        call    edx
        mov    eax, [eax]
        push    eax
        mov    ecx, 0x6CDB24
        mov    ecx, [ecx]
        mov    edx, 0x535D39  /// stdcall
        call    edx        // get player by name
        add    esp, 4
    }
}
