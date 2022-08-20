#include "syslib.h"
#include "zxmgr.h"
#include "srvmgr.h"
#include "srvmgrdef.h"
#include "lib\utils.hpp"
#include "player_info.h"

namespace zxmgr
{
    std::vector<byte*> _stdcall GetPlayers()
    {
        std::vector<byte*> vec;
        unsigned long var_8, var_C;
        __asm
        {
            mov        [var_8], 0x00642C2C
            lea        ecx, [var_8]
            mov        edx, 0x00496DC0
            call    edx
            mov        ecx, 0x006CDB24
            mov        ecx, [ecx]
            push    ecx
            lea        ecx, [var_8]
            mov        edx, 0x00496DD0
            call    edx

        doloop:
            test    eax, eax
            jz        ret_break
            
            mov        [var_C], eax
        }

        byte* ch = (byte*)(var_C);
        vec.push_back(ch);

        __asm
        {
            lea        ecx, [var_8]
            mov        edx, 0x00496E20
            call    edx
            jmp        doloop

        ret_break:
        }

        return vec;
    }

    void __declspec(naked) SendMessageRaw(byte* pptr, char* message)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            push    [ebp+0x08]
            lea        eax, [ebp+0x0C]
            push    eax
            mov        ecx, 0x006C3A08
            mov        edx, 0x0051CD89
            call    edx
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __stdcall SendMessage(byte* pptr, const char* mask, ...)
    {
        va_list arglist;
        va_start(arglist, mask);

        char buffer[4096];
        vsprintf(buffer, mask, arglist);

        SendMessageRaw(pptr, buffer);

        va_end(arglist);
    }

    void __declspec(naked) Kick(byte* pptr, bool silent)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            mov        eax, [ebp+0x08] 
            test    eax, eax
            jz        endp
            cmp        dword ptr [eax+2Ch], 0 // AI
            jnz        endp
            mov        eax, [eax+0x38]
            push    eax
            mov        ecx, 0x00642C2C
            mov        ecx, [ecx]
            mov        edx, 0x004EE028
            call    edx
            mov        eax, [ebp+0x08] 
            xor        ecx, ecx
            mov        cx, [eax+4]
            push    ecx
            mov        ecx, 0x006C3A08
            mov        edx, 0x00518544
            call    edx
            push    eax
            mov        ecx, 0x006C3A08
            mov        edx, 0x005170B6
            call    edx
            mov        ecx, 0x006C3A08
            mov        edx, 0x0051800F
            call edx

            // сообщение в чате про кик
            cmp        [ebp+0x0C], 1
            jz        skip_message
            mov        eax, [ebp+0x08]
            mov        eax, [eax+0x14]
            and        eax, 0x3F000800
            cmp        eax, 0x3F000800
            jz        skip_message // невидимый, и кика соотв. видеть не должны
            mov        eax, [ebp+0x08]
            push    eax
            mov        ecx, 0x006C3A08
            mov        edx, 0x0051D49B
            call    edx
            // "»грок был выкикан с сервера" (c) ленд
        skip_message:
            mov        ecx, 0x00642C2C
            mov        ecx, [ecx]
            mov        edx, [ecx]
            dec        edx
            mov        eax, [ebp+0x08]
            mov        [eax+0x0A50], edx
            mov        ecx, 0x006CDB24
            mov        ecx, [ecx]
            mov        edx, 0x00534DDD
            call    edx

        endp:
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void _stdcall KickAllSilent(byte* pptr)
    {
        std::vector<byte*> players = GetPlayers();
        for (std::vector<byte*>::iterator it = players.begin(); it != players.end(); ++it)
        {
            byte* player = (*it);
            if (player && player != pptr && !*(uint32_t*)(player + 0x2C))
                Kick(player, true);
        }
    }

    void _stdcall KickAll(byte* pptr)
    {
        std::vector<byte*> players = GetPlayers();
        for (std::vector<byte*>::iterator it = players.begin(); it != players.end(); ++it)
        {
            byte* player = (*it);
            if (player && player != pptr && !*(uint32_t*)(player + 0x2C))
                Kick(player, false);
        }
    }

    byte* _stdcall FindByNickname(const char* nickname)
    {
        std::vector<byte*> players = GetPlayers();
        for (std::vector<byte*>::iterator it = players.begin(); it != players.end(); ++it)
        {
            byte* player = (*it);
            if (!player) continue;
            const char* pl_name = *(const char**)(player + 0x18);
            std::string pl_nickname = pl_name;
            if (*(uint32_t*)(player + 0x2C))
            {
                // convert to cp-866
                for (size_t i = 0; i < pl_nickname.length(); i++)
                {
                    uint8_t ch = pl_nickname[i];
                    if (ch == 0xA8) ch = 0xF0;
                    else if (ch == 0xB8) ch = 0xF1;
                    else if (ch >= 0xC0 && ch <= 0xEF)
                        ch -= 0x40;
                    else if (ch >= 0xF0 && ch <= 0xFF)
                        ch -= 0x10;
                    pl_nickname[i] = ch;
                }
            }

            if (pl_nickname == nickname) return player;
        }

        return 0;
    }

    byte* _stdcall FindByLogin(const char* login)
    {
        std::vector<byte*> players = GetPlayers();
        for (std::vector<byte*>::iterator it = players.begin(); it != players.end(); ++it)
        {
            byte* player = (*it);
            if (!player) continue;
            if (*(uint32_t*)(player + 0x2C)) continue; // AI check
            const char* pl_login = *(const char**)(player + 0x0A78);
            if (!strcmp(pl_login, login)) return player;
        }

        return 0;
    }

    void _stdcall Kill(byte* pptr, byte* caster)
    {
        std::vector<byte*> units = GetUnits(pptr);
        for (std::vector<byte*>::iterator it = units.begin(); it != units.end(); ++it)
        {
            byte* unit = (*it);
            if (!unit) continue;
            if (*(byte**)(unit + 0x14) != pptr) continue;
            if (caster && unit == *(byte**)(caster + 0x38)) continue;

            *(byte**)(unit + 0x40) = NULL; // damage_by
            *(int16_t*)(unit + 0x94) = -50;
            UpdateUnit(unit, 0, 0xFFFFFFFF, 0xFFB, 0, 0);
        }
    }

    void _stdcall KillAll(byte* pptr, bool ai_only) // ¬Ќ»ћјЌ»≈! в отличие от оригинального #killall, убивает ¬—≈’ за исключением кастера.
    {
        std::vector<byte*> units = GetUnits();
        for (std::vector<byte*>::iterator it = units.begin(); it != units.end(); ++it)
        {
            byte* unit = (*it);
            if (!unit) continue;
            if (pptr && unit == *(byte**)(pptr + 0x38)) continue;
            if (ai_only && *(byte**)(unit + 0x14) &&
                !*(uint32_t*)(*(byte**)(unit + 0x14) + 0x2C)) continue;

            *(byte**)(unit + 0x40) = NULL; // damage_by
            *(int32_t*)(unit + 0x94) = -50;
            UpdateUnit(unit, 0, 0xFFFFFFFF, 0xFFB, 0, 0);
        }
    }

    bool IsConnected(byte* pptr)
    {
        return (GetNetworkStruct(pptr));
    }

    byte __declspec(naked) *Summon(byte* pptr, const char* unitname, byte* pthis, bool ishero, byte* targetptr)
    {
        __asm
        {
            push    ebp
            mov     ebp, esp
            sub     esp, 0x08

            // wellllllll
            push    [ebp+0x0C]
            lea     ecx, [ebp-0x08]
            mov     edx, 0x005DD8F8
            call    edx // CString::CString(const char*)
            //mov     [ebp-0x08], eax

            push    [ebp+0x14]
            mov        eax, [ebp+0x08]
            mov        eax, [ebp+0x08]
            push    [eax+0x38]
            //push    [ebp-0x08]
            lea     ecx, [ebp-0x08]
            push    ecx
            mov        ecx, 0x00642C2C
            mov     ecx, [ecx]
            mov        edx, 0x00509879
            call    edx

            mov        [ebp-0x04], eax
            test    eax, eax
            jz        s_endp

            /*mov        ecx, [ebp+0x14]
            test    ecx, ecx
            jz        s_endp
            mov        ebx, [ecx+0x2C]
            test    ebx, ebx
            jz        s_endp

            mov        ebx, [ebp+0x14]
            mov        [eax+0x14], ebx*/

            push    0
            push    0
            push    0x0FFB
            push    0xFFFFFFFF
            push    0
            push    eax
            mov        ecx, 0x006C3A08
            mov        edx, 0x00519221
            call    edx

    s_endp:
            lea     ecx, [ebp-0x08]
            mov     edx, 0x005DD88A
            call    edx // CString::~CString()

            mov        eax, [ebp-0x04]

            mov     esp, ebp
            pop     ebp
            retn
        }
    }

    // добавить денег игроку
    void __declspec(naked) GiveMoney(byte* pptr, unsigned long count, unsigned long flags)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            push    [ebp+0x10]
            push    [ebp+0x0C]
            mov        ecx, [ebp+0x08]
            mov        edx, 0x00534AC1
            call    edx
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    // 51CEFB - ¤ так пон¤л, обновление (изменение) информации об игроке
    void __declspec(naked) UpdatePlayer(unsigned long flags, unsigned long info, unsigned long unknown, byte* player)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            push    [ebp+0x14]
            push    [ebp+0x10]
            push    [ebp+0x0C]
            push    [ebp+0x08]
            mov        ecx, 0x006C3A08
            mov        edx, 0x0051CEFB
            call    edx
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __stdcall Own(byte* to, byte* from)
    {
        if (!from || !to) return;
        std::vector<byte*> units = GetUnits(from);
        for (std::vector<byte*>::iterator it = units.begin(); it != units.end(); ++it)
        {
            byte* unit = (*it);
            if (!unit) continue;
            if (unit == *(byte**)(*(byte**)(unit + 0x14) + 0x38)) continue;

            *(byte**)(unit + 0x14) = to;
            UpdateUnit(unit, 0, 0xFFFFFFFF, 0xFFB, 0, 0);
        }
    }

    std::vector<byte*> _stdcall GetUnits(byte* player)
    {
        std::vector<byte*> retval;
        unsigned long var_4, var_C;

        __asm
        {
            mov        eax, 0x006CDB3C
            mov        eax, [eax]
            add        eax, 0x04
            mov        edx, [eax+0x04]
            mov        [var_C], edx

    loop_units:
            mov        edx, [var_C]
            test    edx, edx
            jz        exit_loop

            mov        eax, [edx]
            mov        ecx, [edx+0x08]
            mov        [var_C], eax

            mov        [var_4], ecx
            cmp        [player], 0
            jz        add_every_unit
            mov        eax, [ecx+0x14]
            cmp        eax, [player]
            jnz        loop_units // continue

    add_every_unit:
        }

        byte* pl2 = (byte*)var_4;
        retval.push_back(pl2);

        __asm
        {
            jmp        loop_units
    exit_loop:
        }

        return retval;
    }

    void __declspec(naked) PickupFor(unsigned long pptr, unsigned long pthis)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp

            mov        esp, ebp
            pop        ebp
            retn
        }
    }
    
    unsigned long __declspec(naked) GetSpeed()
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            sub        esp, 8
            mov        eax, 0x006D15A8
            mov        eax, [eax]
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __declspec(naked) SetSpeed(unsigned long newspeed)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            sub        esp, 8
            mov        eax, 0x006D15A8
            mov        ebx, [ebp+0x08]
            mov        [eax], ebx
            mov        edx, 0x00401D90
            call    edx
            push    [ebp+0x08]
            mov        ecx, eax
            mov        edx, 0x0048DC02
            call    edx
            test    eax, eax
            jz        nomessage
            push    0
            mov        ecx, 0x006D15A8
            push    [ecx]
            mov        ecx, 0x006C3A08
            mov        edx, 0x0051D837
            call    edx
        nomessage:
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __declspec(naked) ShutdownServer()
    {
        __asm
        {
            call    upd_all
            push    0
            call    zxmgr::KickAllSilent
            push    0
            mov        edx, 0x005C1070 // __exit
            call    edx
        }
    }

    unsigned long __declspec(naked) GetCurrentMapTime()
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            mov        eax, 0x00642C2C
            mov        eax, [eax]
            mov        eax, [eax+0x254]
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    unsigned long __declspec(naked) GetTotalMapTime()
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            mov        eax, 0x006D1634
            push    [eax]
            mov        ecx, 0x006D1618
            mov        edx, 0x00402880
            call    edx
            mov        eax, [eax]
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __declspec(naked) SetTotalMapTime(unsigned long value)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            mov        eax, 0x006D1634
            push    [eax]
            mov        ecx, 0x006D1618
            mov        edx, 0x00402880
            call    edx
            mov        ebx, [ebp+0x08]
            mov        [eax], ebx
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __declspec(naked) ResetMap()
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            sub        esp, 8
            mov        edx, 0x00401D90
            call    edx
            mov        [ebp-0x08], eax // main window?
            push    0
            push    0
            push    0x445
            mov        ecx, [ebp-0x08]
            mov        ecx, [ecx+0x3D0]
            mov        edx, [ebp-0x08]
            mov        eax, [edx+0x3D0]
            mov        edx, [eax]
            call    dword ptr [edx+0x48]
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __declspec(naked) MorphUnit(unsigned long unit, unsigned long kind)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp

            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __declspec(naked) UpdateUnit(byte* unit, byte* player, unsigned long flags, unsigned long flags2, unsigned long flags3, unsigned long flags4)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            push    [ebp+0x1C]
            push    [ebp+0x18]
            push    [ebp+0x14]
            push    [ebp+0x10]
            push    [ebp+0x0C]
            push    [ebp+0x08]
            mov        ecx, 0x006C3A08
            mov        edx, 0x00519221
            call    edx
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    int __declspec(naked) GetUnitSize(byte* unit)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            mov        ecx, [ebp+0x08]
            mov        edx, [ecx]
            call    [edx+0x1C]
            and        eax, 0xFF
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void MakeUnitNoClip(byte* unit)
    {
        *(byte**)(*(byte**)(unit + 0x1C0) + 5) = 0;
        byte* phys = *(byte**)(unit + 0x10);
        int x = phys[0];
        int y = phys[1];
        // clear unit radius
        byte* someTable = *(byte**)(0x006B16A8);
        *(byte**)(*(byte**)(unit + 0x10) + 8) = someTable;
        int unitSize = zxmgr::GetUnitSize(unit);
        // вот тут ¤ не помню что имел в виду. отреверсено из старого-нового сервера
        if (someTable)
        {
            for (int i = x; i < x + unitSize; i++)
            {
                if (i >= 0 && i <= 255)
                {
                    for (int j = y; j < y + unitSize; j++)
                    {
                        if (j >= 0 && j <= 255)
                        {
                            *(byte**)(someTable + ((j + 0x100) << 8) + i) = 0;
                            *(byte**)(someTable + ((j + 0x200) << 8) + i) = 0;
                        }
                    }
                }
            }
        }
    }

    void __declspec(naked) CreateSack(const char* itemname, unsigned long x, unsigned long y, unsigned long money)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            sub        esp, 0x20

            mov        eax, 0x00642C2C
            mov        eax, [eax]
            mov        [ebp-0x14], eax

            // allocate object
            /*push    0x58
            mov        edx, 0x005DDF54
            call    edx // operator new(size)
            mov        [ebp-0x08], eax*/
            push    itemname
            call    ConstructItemN
            mov        [ebp-0x08], eax

            /*push    itemname
            lea        ecx, [ebp-0x04]
            mov        edx, 0x005DD8F8
            call    edx
            lea        ecx, [ebp-0x04]
            push    ecx
            mov        ecx, [ebp-0x08]
            mov        edx, 0x005480E3
            call    edx*/
            
            // allocate sack object
            push    0x24
            mov        edx, 0x005DDF54
            call    edx // operator new(size)
            mov        [ebp-0x0C], eax

            mov        ecx, [ebp-0x0C]
            mov        edx, 0x00551C0A
            call    edx
            mov        [ebp-0x0C], eax

            push    [ebp-0x08]
            mov        ecx, [ebp-0x0C]
            mov        edx, 0x00551FA3
            call    edx

            // create the coords
            push    y
            push    x
            lea        ecx, [ebp-0x20]
            mov        edx, 0x0058A4B1
            call    edx

            // create the sack itself
            // ...
            push    0
            push    money
            push    [ebp-0x0C] // sack
            lea        eax, [ebp-0x20]
            push    eax // coords
            mov        eax, [ebp-0x14]
            mov        eax, [eax+0x7C]
            mov        ecx, [eax+0x08]
            mov        edx, 0x00554460
            call    edx

            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void _stdcall GMLog(const char* format, ...)
    {
        va_list va;
        va_start(va, format);
        size_t count = (_vsnprintf(NULL, 0, format, va) + 1);
        char* line = new char[count+1];
        line[count] = 0;
        _vsnprintf(line, count, format, va);
        va_end(va);

        std::vector<byte*> players = GetPlayers();
        for (std::vector<byte*>::iterator it = players.begin(); it != players.end(); ++it)
        {
            byte* plr = (*it);
            if (!plr) continue;

            uint32_t rights = *(uint32_t*)(plr + 0x14);
            if (CHECK_FLAG(rights, GMF_ANY)) zxmgr::SendMessageRaw(plr, line);
        }

        delete[] line;
    }

    void _stdcall ServerLog(const char* format, ...)
    {
        va_list va;
        va_start(va, format);
        size_t count = (_vsnprintf(NULL, 0, format, va) + 1);
        char* line = new char[count+1];
        line[count] = 0;
        _vsnprintf(line, count, format, va);
        va_end(va);

        unsigned long ecxa;
        unsigned long ecxb;

        __asm
        {
            push    ecx
            mov        ecx, esp
            mov        ecxa, esp
            push    line
            mov        edx, 0x005DD8F8
            call    edx
            mov        ecxb, eax
            mov        edx, 0x0043AA23
            call    edx
            add        esp, 4
        }

        delete[] line;
    }

    void __declspec(naked) Disconnect(byte* pptr)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            mov        eax, [ebp+0x08] 
            test    eax, eax
            jz        endp
            cmp        dword ptr [eax+2Ch], 0 // AI
            jnz        endp
            mov        eax, [eax+0x38]
            push    eax
            mov        ecx, 0x00642C2C
            mov        ecx, [ecx]
            mov        edx, 0x004EE028
            call    edx
            mov        eax, [ebp+0x08] 
            xor        ecx, ecx
            mov        cx, [eax+4]
            push    ecx
            mov        ecx, 0x006C3A08
            mov        edx, 0x00518544
            call    edx
            push    eax
            mov        ecx, 0x006C3A08
            mov        edx, 0x005170B6
            call    edx
            mov        ecx, 0x006C3A08
            mov        edx, 0x0051800F
            call    edx

            /*mov        ecx, 0x00642C2C
            mov        ecx, [ecx]
            mov        edx, [ecx]
            dec        edx
            mov        eax, [ebp+0x08]
            mov        [eax+0x0A50], edx
            mov        ecx, 0x006CDB24
            mov        ecx, [ecx]
            mov        edx, 0x00534DDD
            call    edx*/

        endp:
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    unsigned long _stdcall GetTicks()
    {
        // number of milliseconds from the moment when system was started - Players[i].LastReturn...
        return GetTickCount();
    }

    unsigned long _stdcall CreateUnit(const char* name)
    {
//52931B
        char* cst = new char[0x208];
        unsigned long unit = 0;
        __asm
        {
            mov        ecx, [cst]
            mov        edx, 0x00528AEF
            call    edx
            mov        [unit], eax
            test    eax, eax
            jz        endp
            lea        eax, [name]
            push    eax
            mov        ecx, [unit]
            mov        edx, 0x0052931B
            call    edx
endp:
        }
        return unit;
    }

    void FreeUnit(unsigned long cptr)
    {
        __asm
        {
            mov        ecx, [cptr]
            mov        edx, [ecx]
            call    [edx+0x04]
        }
    }

    byte* GetNetworkStruct(byte* player)
    {
        if(!player) return NULL;
        byte* retval = NULL;
        unsigned long class_1 = 0;
        unsigned long class_2 = 0;
        __asm
        {
            xor        ecx, ecx
            mov        eax, [player]
            mov        cx, word ptr [eax+4]
            push    ecx // player's order
            mov        ecx, 0x006C3A08 // some global struct related to players
            mov        edx, 0x00518544
            call    edx // network data?..

            mov        [class_1], eax // if it's NULL, then !IsConnected
            test    eax, eax
            jz        ret_0

            mov        ecx, [class_1]
            mov        edx, 0x0041F500
            call    edx
            mov        [class_2], eax // not class despite it's name

            mov        edx, [class_2]
            and        edx, 0x3FFF
            push    edx
            mov        ecx, 0x006C3A08
            mov        edx, 0x005185D5
            call    edx
            mov        [retval], eax // something in memory
ret_0:
        }

        return retval;
    }

    void NextMap()
    {
        byte* mainWnd = NULL;

        __asm
        {
            // advance it
            mov     ecx, 0x006D1618
            mov     edx, 0x0041E7A0
            call    edx
            mov     ebx, 0x006D1634
            add     [ebx], 1
            cmp     [ebx], eax
            jl      nm_map_add
            mov     [ebx], 0

        nm_map_add:
            mov     edx, 0x00401D90
            call    edx
            mov     [mainWnd], eax

            push    0
            mov     ecx, 0x00642C2C
            mov     ecx, [ecx]
            mov     edx, 0x004F94C0
            call    edx

        }
    }

    void PrevMap()
    {
        byte* mainWnd = NULL;

        __asm
        {
            mov     ecx, 0x006D1634
            cmp     [ecx], 0
            ja      pm_map_sub

            mov     edx, 0x006D1618
            mov     edx, 0x0041E7A0
            call    edx
            test    eax, eax
            jg      pm_map_sub
            mov     ecx, 0x006D1634
            dec     eax
            mov     [ecx], eax
            jmp     pm_map_cont

        pm_map_sub:
            sub     [ecx], 1

        pm_map_cont:
            mov     edx, 0x00401D90
            call    edx
            mov     [mainWnd], eax

            push    0
            mov     ecx, 0x00642C2C
            mov     ecx, [ecx]
            mov     edx, 0x004F94C0
            call    edx
        }
    }

    const char*  CreateCString(const char* contents)
    {
        const char* cstring = NULL;

        __asm
        {
            push    [contents]
            lea     ecx, [cstring]
            mov     edx, 0x005DD8F8
            call    edx
        }

        return cstring;
    }

    void DeleteCString(const char* string)
    {
        __asm
        {
            lea     ecx, [string]
            mov     edx, 0x005DD88A
            call    edx
        }
    }

    byte* ConstructItemN(const char* definition)
    {
        if (!definition) return NULL;
        return ConstructItem(definition);
    }

    byte* ConstructItem(std::string definition)
    {
        byte* item_pthis = NULL;
        const char* cdefinition = CreateCString(definition.c_str());

        __asm
        {
            lea     eax, [cdefinition]
            push    eax
            mov     ecx, 0x006D0668
            mov     edx, 0x00510502
            call    edx
            mov     [item_pthis], eax
        }

        DeleteCString(cdefinition);

        return item_pthis;
    }

    bool CheckItem(byte* item)
    {
        bool is_good = false;

        __asm
        {
            mov     ecx, [item]
            mov     edx, 0x00548F6A
            call    edx
            mov     [is_good], al
        }

        return is_good;
    }

    void GiveItemTo(byte* item, byte* player)
    {
        __asm
        {
            push    [item]
            mov     eax, [player]
            mov     eax, [eax+0x38]
            mov     ecx, [eax+0x7C]
            mov     edx, 0x00551FA3
            call    edx

            push    0
            mov     ecx, [player]
            mov     ecx, [ecx+0x38]
            mov     edx, 0x0052A790
            call    edx
        }
    }

    void __declspec(naked) DestroyItem(byte* item)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            mov     ecx, [ebp+0x08]
            mov     edx, [ecx]
            call    dword ptr [edx+0x04] // this should be a destructor
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    byte* GetMainWnd()
    {
        byte* mainWnd = NULL;
        __asm
        {
            mov     edx, 0x00401D90
            call    edx
            mov     [mainWnd], eax
        }
        
        return mainWnd;
    }

    byte* _stdcall FindByID(uint16_t id)
    {
        std::vector<byte*> players = GetPlayers();
        for (std::vector<byte*>::iterator it = players.begin(); it != players.end(); ++it)
        {
            byte* player = (*it);
            if (!player) continue;

            if (*(uint16_t*)(player + 4) == id)
                return player;
        }

        return NULL;
    }

    bool ReturnUnit(byte* unit)
    {
        if (!unit) return false;
        bool retval = false;
        __asm
        {
            mov        ecx, [unit]
            mov        edx, 0x0052C409
            call    edx
            mov        retval, al
        }
        
        return retval;
    }

    void CreateSpellbook(byte* unit)
    {
        if (!unit) return;
        byte* spbk = NULL;
        __asm
        {
            push    0x1C
            mov        edx, 0x005DDF54
            call    edx
            add        esp, 4
            mov        spbk, eax
        }

        if (spbk)
        {
            *(uint32_t*)(spbk) = 0x0060EC30;
            *(uint32_t*)(spbk + 0x04) = 0x0060EC48;
            *(uint32_t*)(spbk + 0x08) = 0;
            *(uint32_t*)(spbk + 0x0C) = 0;
            *(uint32_t*)(spbk + 0x10) = 0;
            *(uint32_t*)(spbk + 0x14) = 0;
        }

        *(byte**)(unit + 0x140) = spbk;
    }

    void DeleteSpellbook(byte* unit)
    {
        if (!unit) return;
        byte* spbk = *(byte**)(unit + 0x140);
        if (!spbk) return;

        __asm
        {
            push    1
            mov        ecx, [spbk]
            mov        edx, 0x00573E80
            call    edx
        }

        *(byte**)(unit + 0x140) = NULL;
    }

    uint32_t GetSpells(byte* unit)
    {
        if (!unit) return 0;
        byte* spbk = *(byte**)(unit + 0x140);
        if (!spbk) return 0;

        uint32_t spells = 0;
        __asm
        {
            mov        ecx, [spbk]
            mov        edx, 0x0053DD3D
            call    edx
            mov        [spells], eax
        }

        return spells;
    }

    void SetSpells(byte* unit, uint32_t spells)
    {
        if (!unit) return;
        DeleteSpellbook(unit);
        CreateSpellbook(unit);
        for (int i = 1; i <= 29; i++)
        {
            if ((1 << i) & spells)
            {
                byte* spell = NULL;
                __asm
                {
                    push    0x14
                    mov        edx, 0x005DDF54
                    call    edx
                    add        esp, 4
                    mov        spell, eax
                }
                if (!spell) continue;

                __asm
                {
                    push    i
                    mov        ecx, spell
                    mov        edx, 0x00538FDD
                    call    edx

                    push    eax
                    push    i
                    mov        ecx, unit
                    mov        ecx, [ecx+0x140]
                    mov        edx, 0x0053D7F0
                    call    edx
                }
            }
        }
    }

    void CastPointEffect(byte* from, uint8_t to_x, uint8_t to_y, uint8_t spell)
    {
        byte* ppf = *(byte**)(from + 0x14);
        if (!ppf) return;
        Player* pi = PI_Get(ppf);
        if (!pi) return;

        byte* cspell = pi->CastSpell;

        __asm
        {
            push    spell
            mov        ecx, cspell
            mov        edx, 0x00538FDD
            call    edx

            push    to_y
            push    to_x
            push    0
            push    from
            mov        ecx, cspell
            mov        edx, 0x00539F5A
            call    edx
        }
    }

    uint8_t GetDiplomacy(byte* player1, byte* player2)
    {
        return *(uint8_t*)(*(uint32_t*)(0x006A8B8C) + 0x46 * *(uint16_t*)(player1 + 0x04) +
               0xA8C4 + *(uint16_t*)(player2 + 0x04));
    }

    void SetDiplomacy(byte* player1, byte* player2, uint8_t diplomacy)
    {
        if (*(uint32_t*)(player1+0x2C) && (diplomacy & 0x10)) // vision won't work for AI players
            diplomacy &= ~0x10;

        *(uint8_t*)(*(uint32_t*)(0x006A8B8C) + 0x46 * *(uint16_t*)(player1 + 0x04) + 0xA8C4 +
        *(uint16_t*)(player2 + 0x04)) = diplomacy;

        if (!*(uint32_t*)(player1+0x2C) && !*(uint32_t*)(player2+0x2C) && (diplomacy & 0x10))
        {
            // i don't know how's this related
            *(uint32_t*)(player2+0x32) |= *(uint32_t*)(player1+0x30); // update player2 to player1
            *(uint32_t*)(player1+0x32) |= *(uint32_t*)(player2+0x30); // update player1 to player2
        }
    }

    byte __declspec(naked) *GetItemFromPack(byte* pack, uint16_t index, uint16_t count)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            push    [ebp+0x10]
            push    [ebp+0x0C]
            mov        ecx, [ebp+0x08]
            mov        edx, 0x00552E42
            call    edx
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    void __declspec(naked) SaveCharacter(byte* player)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            push    [ebp+0x08]
            mov        ecx, 0x00642C2C
            mov        ecx, [ecx]
            mov        edx, 0x004EE028
            call    edx
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    byte __declspec(naked) *GetUnitByID(uint16_t player_id, uint16_t unit_id)
    {
        __asm
        {
            push    ebp
            mov        ebp, esp
            push    [ebp+0x0C]
            push    [ebp+0x08]
            mov        edx, 0x00502AD1
            call    edx
            mov        esp, ebp
            pop        ebp
            retn
        }
    }

    SOCKET GetSocket(uint16_t player_id)
    {
        // find player
        byte* mainStruc = (byte*)(0x006D07A0);
        uint32_t structs = *(uint32_t*)(mainStruc + 0x5A4);
        int16_t p_id = -1;
        byte* p_netstruc = NULL;
        for (uint32_t i = 0; i < structs; i++)
        {
            byte* pstruc = *(byte**)(mainStruc + 0x57C) + i * 0x274;
            if (!pstruc) continue;
            p_netstruc = *(byte**)(pstruc + 0x10);
            if (!p_netstruc) continue;

            if (*(uint16_t*)(p_netstruc + 0x108) == player_id)
                return *(SOCKET*)(pstruc + 8);
        }

        return 0;
    }

    SOCKET GetSocket(byte* player)
    {
        if (!player) return 0;
        return GetSocket(*(uint16_t*)(player+0x04));
    }
}