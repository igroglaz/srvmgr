#include "syslib.h"
#include <math.h>
#include <string>
#include <vector>
#include <sstream>
#include "srvmgr.h"

char akickme[] = "#kickme";
char aDisc[] = "#disconnect ";
char ainfo[] = "#info ";
char aMode[] = "#set mode";

char aown[] = "#own ";
char aownfor[] = "#own_for ";

char * _stdcall get_player_info(char *char_name, char *login_name, bool connected, unsigned char *unit)
{
    static char buf[2000];
    unsigned int st[4];
    unsigned int sum = 0;
    const unsigned int maxSum = 0x8C;
    for (int i = 0; i < 4; ++i)
    {
        st[i] = *(unsigned short *)(unit + 0x84 + i * 2) - *(unsigned char *)(unit + 0xD4 + i);
        if (st[i] > 0x2B)
        {
            sum = maxSum + 1;
        }
        else
        {
            sum += static_cast<unsigned int>(0.349*pow(1.15, double(st[i] - 1)) + 0.5);
        }
    }
    _snprintf(buf, sizeof(buf) - 1, "character: \"%s\", login: \"%s\", %sCONNECTED, [%d,%d,%d,%d]%s", 
        char_name, 
        login_name, 
        connected ? "" : "DIS", st[0], st[1], st[2], st[3], sum > maxSum ? "" : " -> n00b");
    return buf;
}

void parse_space_delimited(const std::string &str, std::vector<std::string> &vs)
{
    vs.clear();
    char quote = 0;
    std::string part;
    for (unsigned int i = 0; i < str.size(); ++i)
    {
        if (quote)
        {
            if (str[i] == quote)
                quote = 0;
            else
            {
                if (str[i] == '\\')
                    if (++i >= str.size()) 
                        throw std::runtime_error("Invalid arguments");
                part += str[i];
            }
        }
        else
        {
            if (std::string("\"'`").find(str[i]) != std::string::npos)
                quote = str[i]; // start quoting
            else if (str[i] != ' ')
            {
                if (str[i] == '\\')
                    if (++i >= str.size()) 
                        throw std::runtime_error("Invalid arguments");
                part += str[i];
            }
            else if (!part.empty())
            {
                vs.push_back(part);
                part.clear();
            }
        }
    }
    if (!part.empty())
    {
        vs.push_back(part);
        part.clear();
    }
}

#pragma warning (disable: 4731)
void __stdcall own_units(void *from, void *to)
{
    __asm
    {
        mov    eax, dword ptr [from]
        mov    ecx, dword ptr [to]
        push    ebp
        mov    ebp, esp
        sub    esp, 0x10
        mov    [ebp-4], eax
        mov    [ebp-8], ecx

        mov    eax, 0x6CDB3C
        mov    eax, [eax]
        add    eax, 4
        mov    edx, [eax+4]
        mov    [ebp-0x0C], edx
l_own_u_loop:
        mov    edx, [ebp-0x0C]
        test    edx, edx
        jz    l_own_u_exit

        mov    eax, [edx]
        mov    ecx, [edx+8]
        mov    [ebp-0x0C], eax

        test    ecx, ecx
        jz    l_own_u_exit
        mov    eax, [ecx + 0x14]
        cmp    eax, [ebp - 4]     // если это юнит жертвы
        jnz    l_own_u_loop
        cmp    ecx, [eax + 0x38]  // проверка на главного персонажа - его не овнить
        jz    l_own_u_loop
        mov    eax, [ebp - 8]
        push    eax
        push    ecx
        mov    ecx, 0x642C2C
        mov    ecx, [ecx]
        mov    ecx, [ecx+0x7C]
        mov    edx, 0x4FB4CA
        call    edx
        jmp    l_own_u_loop
l_own_u_exit:

        mov    esp, ebp
        pop    ebp
    }
}
#pragma warning (default: 4731)

void __stdcall handle_own_for(char *cmd, void *player)
{
    try
    {
        std::vector<std::string> args;
        parse_space_delimited(cmd + strlen(aownfor), args);
        if (args.size() != 2) throw std::runtime_error("Invalid arguments");
        void *from = get_player_by_name(args[0].c_str());
        void *to = get_player_by_name(args[1].c_str());
        if (!from) 
        {
            std::ostringstream ss;
            ss << "Invalid arguments : no such player - " << args[0];
            throw std::runtime_error(ss.str().c_str());
        }
        if (!to)
        {
            std::ostringstream ss;
            ss << "Invalid arguments : no such player - " << args[1];
            throw std::runtime_error(ss.str().c_str());
        }
        own_units(from, to);
    }
    catch (std::runtime_error &e)
    {
        send_to_player(player, e.what());
        send_to_player(player, "Usage: #own_for <player-from> <player-to>");
    }
}

//char print_ecx[] = "ecx = 0x%.8X\n";
//void _stdcall print_stack(void *ebp)
//{
//    for (int i = 0; i < 10; ++i)
//    {
//        void *ret = *(((void**)ebp)+1);
//        if ((unsigned int)ret < 0x400000 || (unsigned int)ret > 0x600000) break;
//        ebp = *(void**)ebp;
//        log_format("--> 0x%.8X\n", (unsigned int)ret);
//    }
//}

#include "cheat_codes_new.h"
#include "srvmgrdef.h"
//void RunCommand(byte* _this, byte* player, const char* command, uint32_t rights, bool console);
#pragma warning (disable: 4102)
#pragma warning (disable: 4733)
void _declspec(naked) cheat_codes(void) {
    __asm
    {
        push    ebp
        mov        ebp, esp
        push    0
        mov        ebx, 0

        mov        eax, [ebp+0x08]
        test    eax, eax
        jz        sk_rights
        mov        edx, [eax+0x14]
        and        edx, GMF_ANY
        cmp        edx, GMF_ANY
        jnz        sk_rights
        mov        ebx, [eax+0x14]
        and        ebx, 0x00FFFFFF

sk_rights:
        push    ebx
        push    [ebp+0x0C]
        push    [ebp+0x08]
        push    ecx
        call    RunCommand      // this function is in another file: cheat_codes_new.cpp
        add        esp, 0x14

        mov        esp, ebp
        pop        ebp
        retn    0x0008
    }
}
#pragma warning (default: 4733)
#pragma warning (default: 4102)

void cheat_codes_unsafe(void);

void __stdcall cheat_say(const char *nick, const char *command)
{
    _asm
    {
        sub    esp, 4
        mov    ecx, esp
        push    dword ptr [command]
        mov    edx, 0x005DD8F8      // CString::CString()
        call    edx
        mov    eax, [eax]

        push    eax
        push    dword ptr [nick]    // name (arg_0)
        call    get_player_by_name  // get player by name
        test    eax, eax
        jz    cs_skip
        push    eax
        call    cheat_codes_unsafe
        jmp    cs_cont
cs_skip:
        pop    eax
cs_cont:
        add    esp, 4
    }
}

void __stdcall cheat_codes_2(byte* player, const char* command)
{
    byte* cstring = NULL;
    __asm
    {
        push    [command]
        lea        ecx, [cstring]
        mov        edx, 0x005DD8F8 // CString::CString()
        call    edx
        push    [eax]
        push    [player]
        call    cheat_codes_unsafe
    }
}

#pragma warning (disable: 4102)
#pragma warning (disable: 4733)
void _declspec(naked) cheat_codes_unsafe(void)
{
    __asm {/////00502D0B
        push    ebp
        mov    ebp, esp
        push    0FFFFFFFFh
        push    0x00601776 // Microsoft VisualC 2-6/net runtime
        mov    eax, fs:0
        push    eax
        mov    fs:0, esp
        sub    esp, 2B0h
        mov    ecx, 0x642C2C
        mov    ecx, dword ptr [ecx]
        mov    [ebp-0x01DC], ecx
        mov    dword ptr [ebp-0x04], 0
        mov    ecx, [ebp+8]
        xor    edx, edx
        mov    dx, [ecx+4]
        push    edx
        mov    ecx, 0x6C3A08
        mov    edx, 0x518544
        call    edx
        mov    [ebp-0x010], eax

        jmp    lu_ready  /// могут все игроки
lu_afterready:
        jmp    lu_test   // только админы
lu_test:                 // CODE XREF: sub_502D0B+53j
        jmp    lu_kick
lu_afterkick:
        jmp    lu_kickme
lu_afterkickme:
        jmp    lu_locate
lu_afterlocate:                 // CODE XREF: sub_502D0B+15Dj
        jmp    lu_create
lu_aftercreate:                 // CODE XREF: sub_502D0B+6ACj
        jmp    lu_modify
lu_aftermodify:                 // CODE XREF: sub_502D0B+9EBj
        jmp    lu_summon
lu_aftersummon:                 // CODE XREF: sub_502D0B+FAFj
        jmp    lu_killall
lu_afterkillall:                // CODE XREF: sub_502D0B+1163j
        jmp    lu_kill
lu_afterkill:                   // CODE XREF: sub_502D0B+14D0j
        jmp    lu_pickupall
lu_afterpickupall:              // CODE XREF: sub_502D0B+160Ej
        jmp    lu_showmap
lu_aftershowmap:                // CODE XREF: sub_502D0B+1AA7j
        jmp    lu_hidemap
lu_afterhidemap:                // CODE XREF: sub_502D0B+1B3Fj
        jmp    lu_info
lu_afterinfo:                   // CODE XREF: sub_502D0B+1B3Fj
        jmp    lu_entermode
lu_afterenter_mode:
        jmp    lu_own
lu_afterown:
        jmp    lu_own_for
lu_afterown_for:
//        jmp    lu_summon_for
lu_aftersummon_for:
        jmp    locu_5049B2
lu_ready:
        mov    ecx, 0x63B124
        push    ecx         // "#ready"
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    locu_5031EF
        mov    ecx, 0x06D1648
        cmp    dword ptr [ecx], 2
        jnz    locu_5031EF
        mov    ecx, 0x06CDB24
        mov    ecx, dword ptr [ecx]
        mov    edx, 0x53636E
        call    edx
        test    eax, eax
        jnz    locu_5031EF
        mov    edx, [ebp+8]
        cmp    dword ptr [edx+0A6Ch], 0
        jnz    locu_5031EF
        mov    eax, [ebp+8]
        mov    dword ptr [eax+0A6Ch], 1
        push    0
        mov    ecx, 0x6C3A08
        mov    edx, 0x51D6B4
        call    edx
        mov    ecx, 0x06CDB24
        mov    ecx, dword ptr [ecx]
        mov    edx, 0x53636E
        call    edx
        test    eax, eax
        jz    locu_5031EF
        push    0
        push    0
        push    0Ah
        mov    ecx, 0x6C3A08
        mov    edx, 0x51CE86
        call    edx
        mov    ecx, [ebp-1DCh]
        mov    edx, 0x4F8F86
        call    edx
        push    0
        push    0
        mov    ecx, [ebp-1DCh]
        mov    edx, 0x4F8FBF
        call    edx
        push    0
        push    1
        mov    ecx, [ebp-1DCh]
        mov    edx, 0x4F8FBF
        call    edx
locu_5031EF:
        jmp    lu_afterready


////////////////
lu_kick:                        // CODE XREF: sub_502D0B+290j
        push    0x63B0BC // "#kick "
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_afterkick
        push    6
        lea    edx, [ebp-90h]
        push    edx
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-1E0h], eax
        mov    [ebp-1E4h], eax
        mov    byte ptr [ebp-4], 1
        push    eax
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-4], 0
        lea    ecx, [ebp-90h]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5DA55C
        call    edx
        push    ecx
        mov    ecx, esp
        mov    [ebp-94h], esp
        lea    edx, [ebp+0Ch]
        push    edx
        mov    edx, 0x5DD74F
        call    edx
        mov    [ebp-1E8h], eax
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        mov    edx, 0x535D39
        call    edx
        push    eax
//        mov    edx, 0x502DE5
        call    kick_char
        jmp    locu_502E54
locu_502E54:                    // CODE XREF: sub_502D0B+D8j
        jmp    locu_5049B2
///////////////
lu_kickme:                      // CODE XREF: sub_502D0B+290j
        push    offset akickme
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_afterkickme
        mov    eax, [ebp+8]
        push    eax
//        mov    edx, 0x502DE5
        call    kick_char
        jmp    locu_5049B2

//////////////////////////////////////////////

                                // CODE XREF: sub_502D0B+29Dj
lu_locate:                      
        push    0x63B0C4        // "#locate "
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_afterlocate
        push    8
        lea    edx, [ebp-0x098]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x01EC], eax
        mov    eax, [ebp-0x01EC]
        mov    [ebp-0x01F0], eax
        mov    byte ptr [ebp-0x04], 2
        mov    ecx, [ebp-0x01F0]
        push    ecx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x098]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        push    ecx
        mov    ecx, esp
        mov    [ebp-0x09C], esp
        lea    edx, [ebp+0x0C]
        push    edx
        mov    edx, 0x5DD74F
        call    edx
        mov    [ebp-0x01F4], eax
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        mov    edx, 0x535D39
        call    edx
        mov    [ebp-0x018], eax
        cmp    [ebp-0x018], 0
        jz    locu_502F7F
        mov    eax, [ebp-0x018]
        cmp    dword ptr [eax+2Ch], 0
        jnz    locu_502F7F
//        mov    ecx, [ebp-0x018]
//        xor    edx, edx
//        mov    dx, [ecx+4]
//        push    edx
//        mov    ecx, 0x6C3A08
//        mov    edx, 0x518544
//        call    edx
//        mov    [ebp-0x010], eax  ////////// проверка на вылет
//        cmp    dword ptr [ebp-0x010], 0
//        jz    locu_502F7F
        lea    ecx, [ebp-0x01C]
        mov    edx, 0x5DD73F
        call    edx
        mov    byte ptr [ebp-0x04], 3
        mov    eax, [ebp-0x018]
        mov    ecx, [eax+38h]
        mov    ecx, [ecx+10h]
        mov    edx, 0x58AB00
        call    edx
        and    eax, 0FFh
        push    eax
        mov    edx, [ebp-0x018]
        mov    eax, [edx+38h]
        mov    ecx, [eax+10h]
        mov    edx, 0x58AAF0
        call    edx
        and    eax, 0FFh
        push    eax
        mov    ecx, [ebp-0x018]
        mov    edx, [ecx+18h]
        push    edx
        push    0x63B0D0 // "%s (%d,%d)"
        lea    eax, [ebp-0x01C]
        push    eax
        mov    edx, 0x5DA3E8
        call    edx
        add    esp, 14h
        mov    ecx, [ebp+0x08]
        push    ecx
        lea    edx, [ebp-0x01C]
        push    edx
        mov    ecx, 0x6C3A08
        mov    edx, 0x51CD89
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x01C]
        mov    edx, 0x5DD88A
        call    edx
locu_502F7F:                    // CODE XREF: sub_502D0B+1DAj
                                // sub_502D0B+1E7j ...
        jmp    locu_5049B2
        
//////////////////////////

lu_create:                      // CODE XREF: sub_502D0B+2ACj
        push    0x63B148        // "#create "
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_aftercreate
        jmp    locu_503424
locu_503424:                    // CODE XREF: sub_502D0B+6C7j
        push    8
        lea    eax, [ebp-0x0B4]
        push    eax
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x0224], eax
        mov    ecx, [ebp-0x0224]
        mov    [ebp-0x0228], ecx
        mov    byte ptr [ebp-0x04], 8
        mov    edx, [ebp-0x0228]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0B4]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        mov    eax, [ebp+0x08]
        mov    ecx, [eax+38h]
        xor    edx, edx
        mov    dl, [ecx+13Ch]
        test    edx, edx
        jle    locu_5034D3
        jmp    locu_5034BF
locu_5034BF:                    // CODE XREF: sub_502D0B+79Cj
        mov    [ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD88A
        call    edx
        jmp    locu_5049C1
// ---------------------------------------------------------------------------

locu_5034D3:                    // CODE XREF: sub_502D0B+776j
        lea    edx, [ebp+0x0C]
        push    edx
        mov    edx, 0x5049D1
        call    edx
        add    esp, 4
        mov    [ebp-0x030], eax
        push    0x63B154        // "Gold"
        mov    eax, [ebp+0x0C]
        push    eax             // char *
        mov    edx, 0x5C0AB0
        call    edx
        add    esp, 8
        mov    [ebp-0x010C], eax
        cmp    dword ptr [ebp-0x010C], 0
        jnz    locu_503560
        push    0
        mov    ecx, [ebp-0x030]
        push    ecx
        mov    ecx, [ebp+0x08]
        mov    edx, 0x534AC1
        call    edx
        jmp    locu_50354C
locu_50354C:                    // CODE XREF: sub_502D0B+829j
        mov    [ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD88A
        call    edx
        jmp    locu_5049C1
// ---------------------------------------------------------------------------

locu_503560:                    // CODE XREF: sub_502D0B+7F5j
        lea    ecx, [ebp+0x0C]
        push    ecx
        mov    ecx, 0x6D0668
        mov    edx, 0x510502
        call    edx
        mov    [ebp-0x034], eax
        cmp    dword ptr [ebp-0x034], 0
        jz    locu_5036A4
        mov    ecx, [ebp-0x034]
        mov    edx, 0x548F6A
        call    edx
        test    eax, eax
        jnz    locu_50361E
        mov    edx, [ebp-0x034]
        mov    [ebp-0x0BC], edx
        mov    eax, [ebp-0x0BC]
        mov    [ebp-0x0B8], eax
        cmp    dword ptr [ebp-0x0B8], 0
        jz    locu_5035C4
        push    1
        mov    ecx, [ebp-0x0B8]
        mov    edx, [ecx]
        mov    ecx, [ebp-0x0B8]
        call    dword ptr [edx+4]
        mov    [ebp-0x0234], eax
        jmp    locu_5035CE
// ---------------------------------------------------------------------------

locu_5035C4:                 // CODE XREF: sub_502D0B+89Cj
        mov    dword ptr [ebp-0x0234], 0

locu_5035CE:                 // CODE XREF: sub_502D0B+8B7j
        jmp    locu_50360A
locu_50360A:                 // CODE XREF: sub_502D0B+8E7j
        mov    [ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD88A
        call    edx
        jmp    locu_5049C1
// ---------------------------------------------------------------------------

locu_50361E:                 // CODE XREF: sub_502D0B+87Aj
        mov    edx, [ebp-0x034]
        mov    ax, word ptr [ebp-0x030]
        mov    [edx+42h], ax
        mov    ecx, [ebp-0x034]
        push    ecx
        mov    edx, [ebp+0x08]
        mov    eax, [edx+38h]
        mov    ecx, [eax+7Ch]
        mov    edx, 0x551FA3
        call    edx
        push    0
        mov    ecx, [ebp+0x08]
        mov    ecx, [ecx+38h]
        mov    edx, 0x52A790
        call    edx
        push    0
        push    0
        push    0FFBh
        push    0FFFFFFFFh
        mov    edx, [ebp+0x08]
        push    edx
        mov    eax, [ebp+0x08]
        mov    ecx, [eax+38h]
        push    ecx
        mov    ecx, 0x6C3A08
        mov    edx, 0x519221
        call    edx
        jmp    locu_5036A4
locu_5036A4:                 // CODE XREF: sub_502D0B+981j
        jmp    locu_5036E2
locu_5036E2:                 // CODE XREF: sub_502D0B:locu_5036A4j
                             // sub_502D0B+9BFj
        jmp    locu_5049B2

///////////////////////////////////////////////////

lu_modify:                      // CODE XREF: sub_502D0B+2BBj
        push    0x63B15C        // "#modify "
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_aftermodify
        push    8
        lea    eax, [ebp-0x0C0]
        push    eax
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x0244], eax
        mov    ecx, [ebp-0x0244]
        mov    [ebp-0x0248], ecx
        mov    byte ptr [ebp-0x04], 9
        mov    edx, [ebp-0x0248]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0C0]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        mov    dword ptr [ebp-0x038], 0
        push    0x63B168 // "self"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    locu_5037C6
        push    0x63B170 // "self"
        mov    edx, 0x5BFCE0
        call    edx
        add    esp, 4
        push    eax
        lea    eax, [ebp-0x0C4]
        push    eax
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x024C], eax
        mov    ecx, [ebp-0x024C]
        mov    [ebp-0x0250], ecx
        mov    byte ptr [ebp-0x04], 0Ah
        mov    edx, [ebp-0x0250]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0C4]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        mov    dword ptr [ebp-0x038], 1
        jmp    locu_50384D
// ---------------------------------------------------------------------------

locu_5037C6:                 // CODE XREF: sub_502D0B+A54j
        push    0x63B178 // "army"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    locu_503839
        push    0x63B180 // "army"
        mov    edx, 0x5BFCE0
        call    edx
        add    esp, 4
        push    eax
        lea    eax, [ebp-0x0C8]
        push    eax
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x0254], eax
        mov    ecx, [ebp-0x0254]
        mov    [ebp-0x0258], ecx
        mov    byte ptr [ebp-0x04], 0Bh
        mov    edx, [ebp-0x0258]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0C8]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        mov    dword ptr [ebp-0x038], 2
        jmp    locu_50384D
// ---------------------------------------------------------------------------

locu_503839:                 // CODE XREF: sub_502D0B+ACAj
        mov    [ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD88A
        call    edx
        jmp    locu_5049C1
// ---------------------------------------------------------------------------

locu_50384D:                 // CODE XREF: sub_502D0B+AB6j
                        // sub_502D0B+B2Cj
        push    0x63B188 // "+god"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    locu_5039E3
        cmp    dword ptr [ebp-0x038], 1
        jnz    locu_503899
        mov    eax, [ebp+0x08]
        mov    ecx, [eax+38h]
        mov    edx, 0x537251
        call    edx
        push    0
        push    0
        push    0FFBh
        push    0A31FFFFFh
        push    0
        mov    ecx, [ebp+0x08]
        mov    edx, [ecx+38h]
        push    edx
        mov    ecx, 0x6C3A08
        mov    edx, 0x519221
        call    edx
        jmp    locu_5039C8
// ---------------------------------------------------------------------------

locu_503899:                 // CODE XREF: sub_502D0B+B5Bj
        cmp    dword ptr [ebp-0x038], 2
        jnz    locu_5039C8
        mov    eax, [ebp+0x08]
        cmp    dword ptr [eax+24h], 0
        jz    locu_5038BD
        mov    ecx, [ebp+0x08]
        mov    edx, [ecx+24h]
        add    edx, 4
        mov    [ebp-0x0124], edx
        jmp    locu_5038C7
// ---------------------------------------------------------------------------

locu_5038BD:                 // CODE XREF: sub_502D0B+B9Fj
        mov    dword ptr [ebp-0x0124], 0

locu_5038C7:                 // CODE XREF: sub_502D0B+BB0j
        mov    eax, [ebp-0x0124]
        mov    [ebp-0x044], eax
        mov    ecx, [ebp-0x044]
        mov    edx, [ecx+4]
        mov    [ebp-0x0110], edx
        mov    eax, [ebp-0x0110]
        mov    [ebp-0x040], eax
        cmp    dword ptr [ebp-0x040], 0
        jz    locu_503935
        lea    ecx, [ebp-0x040]
        mov    [ebp-0x011C], ecx
        mov    edx, [ebp-0x044]
        mov    [ebp-0x0118], edx
        mov    eax, [ebp-0x011C]
        mov    ecx, [eax]
        mov    [ebp-0x0114], ecx
        mov    edx, [ebp-0x011C]
        mov    eax, [ebp-0x0114]
        mov    ecx, [eax]
        mov    [edx], ecx
        mov    edx, [ebp-0x0114]
        mov    eax, [edx+8]
        mov    [ebp-0x0120], eax
        mov    ecx, [ebp-0x0120]
        mov    [ebp-0x03C], ecx
        jmp    locu_50393C
// ---------------------------------------------------------------------------

locu_503935:                 // CODE XREF: sub_502D0B+BDEj
        mov    dword ptr [ebp-0x03C], 0

locu_50393C:                 // CODE XREF: sub_502D0B+C28j
                        // sub_502D0B:locu_5039C3j
        cmp    dword ptr [ebp-0x03C], 0
        jz    locu_5039C8
        mov    ecx, [ebp-0x03C]
        mov    edx, 0x537251
        call    edx
        push    0
        push    0
        push    0FFBh
        push    0A31FFFFFh
        push    0
        mov    edx, [ebp-0x03C]
        push    edx
        mov    ecx, 0x6C3A08
        mov    edx, 0x519221
        call    edx
        cmp    dword ptr [ebp-0x040], 0
        jz    locu_5039BC
        lea    eax, [ebp-0x040]
        mov    [ebp-0x0130], eax
        mov    ecx, [ebp-0x044]
        mov    [ebp-0x012C], ecx
        mov    edx, [ebp-0x0130]
        mov    eax, [edx]
        mov    [ebp-0x0128], eax
        mov    ecx, [ebp-0x0130]
        mov    edx, [ebp-0x0128]
        mov    eax, [edx]
        mov    [ecx], eax
        mov    ecx, [ebp-0x0128]
        mov    edx, [ecx+8]
        mov    [ebp-0x0134], edx
        mov    eax, [ebp-0x0134]
        mov    [ebp-0x03C], eax
        jmp    locu_5039C3
// ---------------------------------------------------------------------------

locu_5039BC:                 // CODE XREF: sub_502D0B+C65j
        mov    dword ptr [ebp-0x03C], 0

locu_5039C3:                 // CODE XREF: sub_502D0B+CAFj
        jmp    locu_50393C
// ---------------------------------------------------------------------------

locu_5039C8:                 // CODE XREF: sub_502D0B+B89j
                        // sub_502D0B+B92j ...
//        push    0
//        mov    ecx, [ebp+0x08]
//        movsx    edx, word ptr [ecx+4]
//        push    edx
//        push    7
//        mov    ecx, 0x6C3A08       ///// ОТПРАВКА СООБЩЕНИЯ ИГРОК РЕШИЛ ИГРАТЬ НЕЧЕСТНО
//        mov    edx, 0x51CE86
//        call    edx
        jmp    locu_503C85
// ---------------------------------------------------------------------------

locu_5039E3:                 // CODE XREF: sub_502D0B+B51j
        push    0x63B190 // "+spell "
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    locu_503B5B
        push    0x63B198 // "+spell "
        mov    edx, 0x5BFCE0
        call    edx
        add    esp, 4
        push    eax
        lea    eax, [ebp-0x0CC]
        push    eax
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x025C], eax
        mov    ecx, [ebp-0x025C]
        mov    [ebp-0x0260], ecx
        mov    byte ptr [ebp-0x04], 0Ch
        mov    edx, [ebp-0x0260]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0CC]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        cmp    dword ptr [ebp-0x038], 1
        jnz    locu_503B56
        mov    eax, [ebp+0x08]
        mov    ecx, [eax+38h]
        mov    [ebp-0x048], ecx
        mov    edx, [ebp-0x048]
        cmp    dword ptr [edx+140h], 0
        jnz    locu_503A84
        mov    [ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD88A
        call    edx
        jmp    locu_5049C1
// ---------------------------------------------------------------------------

locu_503A84:                 // CODE XREF: sub_502D0B+D63j
        mov    eax, [ebp+0x0C]
        push    eax        // char *
        mov    edx, 0x5BF920
        call    edx
        add    esp, 4
        mov    [ebp-0x04C], eax
        cmp    dword ptr [ebp-0x04C], 0
        jle    locu_503B1D
        mov    ecx, 0x06D074C
        mov    ecx, [ecx]
        mov    [ebp-0x0138], ecx
        mov    edx, [ebp-0x04C]
        cmp    edx, [ebp-0x0138]
        jge    locu_503B1D
        push    14h
        mov    edx, 0x5DDF54
        call    edx
        add    esp, 4
        mov    [ebp-0x0D0], eax
        mov    byte ptr [ebp-0x04], 0Dh
        cmp    dword ptr [ebp-0x0D0], 0
        jz    locu_503AEA
        xor    eax, eax
        mov    al, byte ptr [ebp-0x04C]
        push    eax
        mov    ecx, [ebp-0x0D0]
        mov    edx, 0x538FDD
        call    edx
        mov    [ebp-0x0264], eax
        jmp    locu_503AF4
// ---------------------------------------------------------------------------

locu_503AEA:                 // CODE XREF: sub_502D0B+DC4j
        mov    dword ptr [ebp-0x0264], 0

locu_503AF4:                 // CODE XREF: sub_502D0B+DDDj
        mov    ecx, [ebp-0x0264]
        mov    [ebp-0x0D4], ecx
        mov    byte ptr [ebp-0x04], 0
        mov    edx, [ebp-0x0D4]
        push    edx
        mov    eax, [ebp-0x04C]
        push    eax
        mov    ecx, [ebp-0x048]
        mov    ecx, [ecx+140h]
        mov    edx, 0x53D7F0
        call    edx

locu_503B1D:                 // CODE XREF: sub_502D0B+D8Cj
                        // sub_502D0B+DA7j
        push    0
        push    0
        push    0FFBh
        push    0A31FFFFFh
        mov    edx, [ebp-0x048]
        mov    eax, [edx+14h]
        push    eax
        mov    ecx, [ebp-0x048]
        push    ecx
        mov    ecx, 0x6C3A08
        mov    edx, 0x519221
        call    edx
        push    0
        mov    edx, [ebp+0x08]
        movsx    eax, word ptr [edx+4]
        push    eax
        push    7
        mov    ecx, 0x6C3A08
        mov    edx, 0x51CE86
        call    edx

locu_503B56:                 // CODE XREF: sub_502D0B+D4Aj
        jmp    locu_503C85
// ---------------------------------------------------------------------------

locu_503B5B:                 // CODE XREF: sub_502D0B+CE7j
        push    0x63B1A0 // "+spells"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    locu_503C85
        cmp    dword ptr [ebp-0x038], 1
        jnz    locu_503C85
        mov    ecx, [ebp+0x08]
        mov    edx, [ecx+38h]
        mov    [ebp-0x050], edx
        mov    eax, [ebp-0x050]
        cmp    dword ptr [eax+140h], 0
        jnz    locu_503BA3
        mov    [ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD88A
        call    edx
        jmp    locu_5049C1
// ---------------------------------------------------------------------------

locu_503BA3:                 // CODE XREF: sub_502D0B+E82j
        mov    dword ptr [ebp-0x054], 1
        jmp    locu_503BB5
// ---------------------------------------------------------------------------

locu_503BAC:                 // CODE XREF: sub_502D0B+F19j
        mov    ecx, [ebp-0x054]
        add    ecx, 1
        mov    [ebp-0x054], ecx

locu_503BB5:                 // CODE XREF: sub_502D0B+E9Fj
        cmp    dword ptr [ebp-0x054], 1Dh
        jg    locu_503C26
        push    14h
        mov    edx, 0x5DDF54
        call    edx
        add    esp, 4
        mov    [ebp-0x0D8], eax
        mov    byte ptr [ebp-0x04], 0Eh
        cmp    dword ptr [ebp-0x0D8], 0
        jz    locu_503BF1
        xor    edx, edx
        mov    dl, byte ptr [ebp-0x054]
        push    edx
        mov    ecx, [ebp-0x0D8]
        mov    edx, 0x538FDD
        call    edx
        mov    [ebp-0x0268], eax
        jmp    locu_503BFB
// ---------------------------------------------------------------------------

locu_503BF1:                 // CODE XREF: sub_502D0B+ECBj
        mov    dword ptr [ebp-0x0268], 0

locu_503BFB:                 // CODE XREF: sub_502D0B+EE4j
        mov    eax, [ebp-0x0268]
        mov    [ebp-0x0DC], eax
        mov    byte ptr [ebp-0x04], 0
        mov    ecx, [ebp-0x0DC]
        push    ecx
        mov    edx, [ebp-0x054]
        push    edx
        mov    eax, [ebp-0x050]
        mov    ecx, [eax+140h]
        mov    edx, 0x53D7F0
        call    edx
        jmp    locu_503BAC
// ---------------------------------------------------------------------------

locu_503C26:                 // CODE XREF: sub_502D0B+EAEj
        push    0
        push    0
        push    0FFBh
        push    0A31FFFFFh
        mov    ecx, [ebp-0x050]
        mov    edx, [ecx+14h]
        push    edx
        mov    eax, [ebp-0x050]
        push    eax
        mov    ecx, 0x6C3A08
        mov    edx, 0x519221
        call    edx
        jmp    locu_503C85
locu_503C85:                 // CODE XREF: sub_502D0B+CD3j
                        // sub_502D0B:locu_503B56j ...
        push    0x63B1A8 // "+knowledge"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    locu_503CA6
        mov    eax, [ebp+0x08]
        push    eax
        push    0
        mov    ecx, 0x6C3A08
        mov    edx, 0x51D1A8
        call    edx

locu_503CA6:                 // CODE XREF: sub_502D0B+F89j
        jmp    locu_5049B2
/////////////////////////////////////////////////////////
lu_summon:                 // CODE XREF: sub_502D0B+2C8j
        push    0x63B1B4 // "#summon "
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_aftersummon
        jmp    locu_503D27
locu_503D27:                 // CODE XREF: sub_502D0B+FCAj
        push    8
        lea    eax, [ebp-0x0E0]
        push    eax
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x0274], eax
        mov    ecx, [ebp-0x0274]
        mov    [ebp-0x0278], ecx
        mov    byte ptr [ebp-0x04], 0Fh
        mov    edx, [ebp-0x0278]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0E0]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        mov    eax, [ebp+0x08]
        cmp    dword ptr [eax+38h], 0
        jnz    locu_503D91
        mov    dword ptr [ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD88A
        call    edx
        jmp    locu_5049C1
// ---------------------------------------------------------------------------

locu_503D91:                 // CODE XREF: sub_502D0B+1070j
        lea    ecx, [ebp+0x0C]
        push    ecx
        mov    edx, 0x5049D1
        call    edx
        add    esp, 4
        mov    [ebp-0x058], eax
        mov    dword ptr [ebp-0x060], 0
        push    0x63B1C0 // "hero"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    locu_503E13
        mov    dword ptr [ebp-0x060], 1
        mov    dword ptr [ebp-0x058], 1
        push    5
        lea    edx, [ebp-0x0E4]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x027C], eax
        mov    eax, [ebp-0x027C]
        mov    [ebp-0x0280], eax
        mov    byte ptr [ebp-0x04], 10h
        mov    ecx, [ebp-0x0280]
        push    ecx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0E4]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx

locu_503E13:                 // CODE XREF: sub_502D0B+10ABj
        mov    dword ptr [ebp-0x05C], 1
        jmp    locu_503E25
// ---------------------------------------------------------------------------

locu_503E1C:                 // CODE XREF: sub_502D0B+113Cj
        mov    edx, [ebp-0x05C]
        add    edx, 1
        mov    [ebp-0x05C], edx

locu_503E25:                 // CODE XREF: sub_502D0B+110Fj
        mov    eax, [ebp-0x05C]
        cmp    eax, [ebp-0x058]
        jg    locu_503E49
        mov    ecx, [ebp-0x060]
        push    ecx
        mov    edx, [ebp+0x08]
        mov    eax, [edx+38h]
        push    eax
        lea    ecx, [ebp+0x0C]
        push    ecx
        mov    ecx, [ebp-0x01DC]
        mov    edx, 0x509879
        call    edx
        jmp    locu_503E1C
// ---------------------------------------------------------------------------

locu_503E49:                 // CODE XREF: sub_502D0B+1120j
        jmp    locu_5049B2
//////////////////////////////////////////////////////////////
lu_killall:                 // CODE XREF: sub_502D0B+2D5j
        push    0x63B1C8 // "#killall"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_afterkillall

        jmp    locu_503EDB

locu_503EDB:                 // CODE XREF: sub_502D0B+117Ej
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        mov    [ebp-0x0150], ecx
        mov    edx, [ebp-0x0150]
        mov    [ebp-0x068], edx
        mov    eax, [ebp-0x068]
        mov    ecx, [eax+4]
        mov    [ebp-0x013C], ecx
        mov    edx, [ebp-0x013C]
        mov    [ebp-0x064], edx
        cmp    dword ptr [ebp-0x064], 0
        jz    locu_503F55
        lea    eax, [ebp-0x064]
        mov    [ebp-0x0148], eax
        mov    ecx, [ebp-0x068]
        mov    [ebp-0x0144], ecx
        mov    edx, [ebp-0x0148]
        mov    eax, [edx]
        mov    [ebp-0x0140], eax
        mov    ecx, [ebp-0x0148]
        mov    edx, [ebp-0x0140]
        mov    eax, [edx]
        mov    [ecx], eax
        mov    ecx, [ebp-0x0140]
        mov    edx, [ecx+8]
        mov    [ebp-0x014C], edx
        mov    eax, [ebp-0x014C]
        mov    [ebp-0x06C], eax
        jmp    locu_503F5C
// ---------------------------------------------------------------------------

locu_503F55:                 // CODE XREF: sub_502D0B+11FEj
        mov    dword ptr [ebp-0x06C], 0

locu_503F5C:                 // CODE XREF: sub_502D0B+1248j
                        // sub_502D0B:locu_503FEDj
        cmp    dword ptr [ebp-0x06C], 0
        jz    locu_503FF2
        mov    ecx, [ebp-0x06C]
        movsx    edx, word ptr [ecx+4]
        imul    edx, 46h
        mov    eax, 0x06A8B8C
        mov    eax, dword ptr [eax]
        lea    ecx, [eax+edx+0A8C4h]
        mov    edx, [ebp+0x08]
        movsx    eax, word ptr [edx+4]
        movsx    ecx, byte ptr [ecx+eax]
        and    ecx, 1
        test    ecx, ecx
        jz    locu_503F96
        mov    ecx, [ebp-0x06C]
        mov    edx, 0x5346AC
        call    edx

locu_503F96:                 // CODE XREF: sub_502D0B+1281j
        cmp    dword ptr [ebp-0x064], 0
        jz    locu_503FE6
        lea    edx, [ebp-0x064]
        mov    [ebp-0x015C], edx
        mov    eax, [ebp-0x068]
        mov    [ebp-0x0158], eax
        mov    ecx, [ebp-0x015C]
        mov    edx, [ecx]
        mov    [ebp-0x0154], edx
        mov    eax, [ebp-0x015C]
        mov    ecx, [ebp-0x0154]
        mov    edx, [ecx]
        mov    [eax], edx
        mov    eax, [ebp-0x0154]
        mov    ecx, [eax+8]
        mov    [ebp-0x0160], ecx
        mov    edx, [ebp-0x0160]
        mov    [ebp-0x06C], edx
        jmp    locu_503FED
// ---------------------------------------------------------------------------

locu_503FE6:                 // CODE XREF: sub_502D0B+128Fj
        mov    dword ptr [ebp-0x06C], 0

locu_503FED:                 // CODE XREF: sub_502D0B+12D9j
        jmp    locu_503F5C
// ---------------------------------------------------------------------------

locu_503FF2:                 // CODE XREF: sub_502D0B+1255j
        jmp    locu_50402E
locu_50402E:                 // CODE XREF: sub_502D0B+130Bj
        jmp    locu_5049B2
////////////////////////////////////////////////////
lu_kill:                 // CODE XREF: sub_502D0B+2E2j
        push    0x63B1F0 // "#kill "
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_disc
        jmp    locu_504248
locu_504248:                 // CODE XREF: sub_502D0B+14EBj
        push    6
        lea    eax, [ebp-0x0E8]
        push    eax
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x0294], eax
        mov    ecx, [ebp-0x0294]
        mov    [ebp-0x0298], ecx
        mov    byte ptr [ebp-0x04], 11h
        mov    edx, [ebp-0x0298]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0E8]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        push    ecx
        mov    ecx, esp
        mov    [ebp-0x0EC], esp
        lea    eax, [ebp+0x0C]
        push    eax
        mov    edx, 0x5DD74F
        call    edx
        mov    [ebp-0x029C], eax
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        mov    edx, 0x535D39
        call    edx
        mov    [ebp-0x07C], eax
        cmp    dword ptr [ebp-0x07C], 0
        jz    locu_504305
        mov    ecx, [ebp-0x07C]
        mov    edx, 0x5346AC
        call    edx
        jmp    locu_504305
locu_504305:                 // CODE XREF: sub_502D0B+15B4j
                        // sub_502D0B+15E2j
        jmp    locu_5049B2
/////////////////////////////////////////////////////////
lu_own:                 // CODE XREF: sub_502D0B+2E2j
        push    offset aown // "#own "
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123 // strstr
        call    edx
        test    eax, eax
        jnz    lu_afterown

        push    5
        lea    eax, [ebp-0x0E8]
        push    eax
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx  // substr
        mov    [ebp-0x0294], eax
        mov    ecx, [ebp-0x0294]
        mov    [ebp-0x0298], ecx
        mov    byte ptr [ebp-0x04], 11h
        mov    edx, [ebp-0x0298]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x0E8]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        push    ecx
        mov    ecx, esp
        mov    [ebp-0x0EC], esp
        lea    eax, [ebp+0x0C]
        push    eax
        mov    edx, 0x5DD74F
        call    edx
        mov    [ebp-0x029C], eax
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        mov    edx, 0x535D39
        call    edx // player by string
        mov    [ebp-0x07C], eax
        cmp    dword ptr [ebp-0x07C], 0
        jz    locu_504305
// own code ///////////////////////////////////////////////////
        mov    eax, 0x6CDB3C
        mov    eax, [eax]
        add    eax, 4
        mov    [ebp-0x1E0], eax
        mov    edx, [eax+4]
        mov    [ebp-0x1E4], edx
lu_own_c_loop:
        mov    edx, [ebp-0x1E4]
        test    edx, edx
        jz    lu_own_c_exit

        mov    eax, [edx]
        mov    ecx, [edx+8]
        mov    [ebp-0x1E4], eax

        test    ecx, ecx
        jz    lu_own_c_exit
        mov    eax, [ecx + 0x14]
        cmp    eax, [ebp - 0x07C]
        jnz    lu_own_c_loop
        cmp    ecx, [eax + 0x38]  // проверка на главного персонажа - его не овнить
        jz    lu_own_c_loop
        mov    eax, [ebp + 8]
        push    eax  // control by me
        push    ecx
        mov    ecx, 0x642C2C
        mov    ecx, [ecx]
        mov    ecx, [ecx+0x7C]
        mov    edx, 0x4FB4CA
        call    edx
        jmp    lu_own_c_loop
lu_own_c_exit:
        jmp    locu_504305
/////////////////////////////////////////////////////////
lu_own_for:                 // CODE XREF: sub_502D0B+2E2j
        push    offset aownfor // "#own_for "
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123 // strstr
        call    edx
        test    eax, eax
        jnz    lu_afterown_for
        mov    ecx, [ebp+8]
        push    ecx
        mov    ecx, [ebp+0x0C]
        push    ecx
        call    handle_own_for
        jmp    locu_504305
/////////////////////////////////////////////////////////
lu_disc:
        push    offset aDisc /// "#disconnect "
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_afterkill
        
        push    0x0c
        lea    edx, [ebp-90h]
        push    edx
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-1E0h], eax
        mov    [ebp-1E4h], eax
        mov    byte ptr [ebp-4], 1
        push    eax
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-4], 0
        lea    ecx, [ebp-90h]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0Ch]
        mov    edx, 0x5DA55C
        call    edx
        push    ecx
        mov    ecx, esp
        mov    [ebp-94h], esp
        lea    edx, [ebp+0Ch]
        push    edx
        mov    edx, 0x5DD74F
        call    edx
        mov    [ebp-1E8h], eax
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        mov    edx, 0x535D39
        call    edx
        mov    [ebp-14h], eax
        cmp    dword ptr [ebp-14h], 0
        jz    locu_5049B2
        cmp    dword ptr [eax+2Ch], 0
        jnz    locu_5049B2////AI
        mov    eax, [ebp-14h]
        xor    ecx, ecx
        mov    cx, [eax+4]
        push    ecx
        mov    ecx, 0x6C3A08
        mov    edx, 0x518544
        call    edx
        mov    [ebp-10h], eax
        cmp    dword ptr [ebp-10h], 0
        jz    locu_5049B2
        mov    edx, [ebp-10h]
        mov    eax, [edx+29Ch]
        mov    [ebp-0FCh], eax
        cmp    dword ptr [ebp-0FCh], 0
        jnz    locu_5049B2
        mov    ecx, [ebp-14h]
        push    ecx
        mov    ecx, 0x6C3A08
        mov    edx, 0x51D49B
        call    edx
        jmp    locu_5049B2
/////////////////////////////////////////
lu_pickupall:                 // CODE XREF: sub_502D0B+2EFj
        push    0x63B1F8 // "#pickup all"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_afterpickupall
        jmp    locu_504386
locu_504386:                 // CODE XREF: sub_502D0B+1629j
        mov    edx, [ebp+0x08]
        cmp    dword ptr [edx+38h], 0
        jnz    locu_5043A3
        mov    dword ptr [ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD88A
        call    edx
        jmp    locu_5049C1
// ---------------------------------------------------------------------------

locu_5043A3:                 // CODE XREF: sub_502D0B+1682j
        mov    eax, [ebp-0x01DC]
        mov    ecx, [eax+7Ch]
        mov    edx, [ecx+8]
        mov    [ebp-0x01A0], edx
        mov    eax, [ebp-0x01A0]
        mov    [ebp-0x088], eax
        mov    ecx, [ebp-0x088]
        mov    edx, [ecx+4]
        mov    [ebp-0x018C], edx
        mov    eax, [ebp-0x018C]
        mov    [ebp-0x084], eax
        cmp    dword ptr [ebp-0x084], 0
        jz    locu_504435
        lea    ecx, [ebp-0x084]
        mov    [ebp-0x0198], ecx
        mov    edx, [ebp-0x088]
        mov    [ebp-0x0194], edx
        mov    eax, [ebp-0x0198]
        mov    ecx, [eax]
        mov    [ebp-0x0190], ecx
        mov    edx, [ebp-0x0198]
        mov    eax, [ebp-0x0190]
        mov    ecx, [eax]
        mov    [edx], ecx
        mov    edx, [ebp-0x0190]
        mov    eax, [edx+8]
        mov    [ebp-0x019C], eax
        mov    ecx, [ebp-0x019C]
        mov    [ebp-0x080], ecx
        jmp    locu_50443C
// ---------------------------------------------------------------------------

locu_504435:                 // CODE XREF: sub_502D0B+16D8j
        mov    dword ptr [ebp-0x080], 0

locu_50443C:                 // CODE XREF: sub_502D0B+1728j
                        // sub_502D0B:locu_50473Cj
        cmp    dword ptr [ebp-0x080], 0
        jz    locu_504741
        mov    edx, [ebp-0x080]
        push    edx
        mov    ecx, [ebp-0x01DC]
        mov    edx, 0x4F9AD3
        call    edx
        mov    eax, [ebp-0x080]
        push    eax
        mov    ecx, 0x06B16A8
        mov    ecx, [ecx]
        mov    edx, 0x58E525
        call    edx
        mov    ecx, 0x0642C2C
        mov    ecx, [ecx]
        mov    edx, [ecx+7Ch]
        mov    eax, [edx+8]
        mov    [ebp-0x01C8], eax
        mov    ecx, [ebp-0x080]
        mov    [ebp-0x01AC], ecx
        mov    dword ptr [ebp-0x01A8], 0
        cmp    dword ptr [ebp-0x01A8], 0
        jnz    locu_5044A3
        mov    edx, [ebp-0x01C8]
        mov    eax, [edx+4]
        mov    [ebp-0x01A8], eax
        jmp    locu_5044B1
// ---------------------------------------------------------------------------

locu_5044A3:                 // CODE XREF: sub_502D0B+1785j
        mov    ecx, [ebp-0x01A8]
        mov    edx, [ecx]
        mov    [ebp-0x01A8], edx

locu_5044B1:                 // CODE XREF: sub_502D0B+1796j
        jmp    locu_5044C1
// ---------------------------------------------------------------------------

locu_5044B3:                 // CODE XREF: sub_502D0B:locu_5044F0j
        mov    eax, [ebp-0x01A8]
        mov    ecx, [eax]
        mov    [ebp-0x01A8], ecx

locu_5044C1:                 // CODE XREF: sub_502D0B:locu_5044B1j
        cmp    dword ptr [ebp-0x01A8], 0
        jz    locu_5044F2
        mov    edx, [ebp-0x01A8]
        mov    eax, [edx+8]
        sub    eax, [ebp-0x01AC]
        neg    eax
        sbb    eax, eax
        inc    eax
        test    eax, eax
        jz    locu_5044F0
        mov    ecx, [ebp-0x01A8]
        mov    [ebp-0x01A4], ecx
        jmp    locu_5044FC
// ---------------------------------------------------------------------------

locu_5044F0:                 // CODE XREF: sub_502D0B+17D5j
        jmp    locu_5044B3
// ---------------------------------------------------------------------------

locu_5044F2:                 // CODE XREF: sub_502D0B+17BDj
        mov    dword ptr [ebp-0x01A4], 0

locu_5044FC:                 // CODE XREF: sub_502D0B+17E3j
        cmp    dword ptr [ebp-0x01A4], 0
        jz    locu_5046CD
        mov    edx, [ebp-0x01A4]
        mov    [ebp-0x01B0], edx
        mov    eax, [ebp-0x01C8]
        mov    ecx, [ebp-0x01B0]
        cmp    ecx, [eax+4]
        jnz    locu_504539
        mov    edx, [ebp-0x01C8]
        mov    eax, [ebp-0x01B0]
        mov    ecx, [eax]
        mov    [edx+4], ecx
        jmp    locu_50454C
// ---------------------------------------------------------------------------

locu_504539:                 // CODE XREF: sub_502D0B+1819j
        mov    edx, [ebp-0x01B0]
        mov    eax, [edx+4]
        mov    ecx, [ebp-0x01B0]
        mov    edx, [ecx]
        mov    [eax], edx

locu_50454C:                 // CODE XREF: sub_502D0B+182Cj
        mov    eax, [ebp-0x01C8]
        mov    ecx, [ebp-0x01B0]
        cmp    ecx, [eax+8]
        jnz    locu_504571
        mov    edx, [ebp-0x01C8]
        mov    eax, [ebp-0x01B0]
        mov    ecx, [eax+4]
        mov    [edx+8], ecx
        jmp    locu_504585
// ---------------------------------------------------------------------------

locu_504571:                 // CODE XREF: sub_502D0B+1850j
        mov    edx, [ebp-0x01B0]
        mov    eax, [edx]
        mov    ecx, [ebp-0x01B0]
        mov    edx, [ecx+4]
        mov    [eax+4], edx

locu_504585:                 // CODE XREF: sub_502D0B+1864j
        mov    dword ptr [ebp-0x01B8], 1
        mov    eax, [ebp-0x01B0]
        add    eax, 8
        mov    [ebp-0x01B4], eax
        jmp    locu_5045AF
// ---------------------------------------------------------------------------

locu_5045A0:                 // CODE XREF: sub_502D0B+18BDj
        mov    ecx, [ebp-0x01B4]
        add    ecx, 4
        mov    [ebp-0x01B4], ecx

locu_5045AF:                 // CODE XREF: sub_502D0B+1893j
        mov    edx, [ebp-0x01B8]
        mov    eax, [ebp-0x01B8]
        sub    eax, 1
        mov    [ebp-0x01B8], eax
        test    edx, edx
        jz    locu_5045CA
        jmp    locu_5045A0
// ---------------------------------------------------------------------------

locu_5045CA:                 // CODE XREF: sub_502D0B+18BBj
        mov    ecx, [ebp-0x01B0]
        mov    edx, [ebp-0x01C8]
        mov    eax, [edx+10h]
        mov    [ecx], eax
        mov    ecx, [ebp-0x01C8]
        mov    edx, [ebp-0x01B0]
        mov    [ecx+10h], edx
        mov    eax, [ebp-0x01C8]
        mov    ecx, [eax+0Ch]
        sub    ecx, 1
        mov    edx, [ebp-0x01C8]
        mov    [edx+0Ch], ecx
        mov    eax, [ebp-0x01C8]
        cmp    dword ptr [eax+0Ch], 0
        jnz    locu_5046CD
        mov    ecx, [ebp-0x01C8]
        mov    edx, [ecx+4]
        mov    [ebp-0x01BC], edx
        jmp    locu_50462E
// ---------------------------------------------------------------------------

locu_504620:                 // CODE XREF: sub_502D0B:locu_50467Cj
        mov    eax, [ebp-0x01BC]
        mov    ecx, [eax]
        mov    [ebp-0x01BC], ecx

locu_50462E:                 // CODE XREF: sub_502D0B+1913j
        cmp    dword ptr [ebp-0x01BC], 0
        jz    locu_50467E
        mov    dword ptr [ebp-0x01C4], 1
        mov    edx, [ebp-0x01BC]
        add    edx, 8
        mov    [ebp-0x01C0], edx
        jmp    locu_504661
// ---------------------------------------------------------------------------

locu_504652:                 // CODE XREF: sub_502D0B+196Fj
        mov    eax, [ebp-0x01C0]
        add    eax, 4
        mov    [ebp-0x01C0], eax

locu_504661:                 // CODE XREF: sub_502D0B+1945j
        mov    ecx, [ebp-0x01C4]
        mov    edx, [ebp-0x01C4]
        sub    edx, 1
        mov    [ebp-0x01C4], edx
        test    ecx, ecx
        jz    locu_50467C
        jmp    locu_504652
// ---------------------------------------------------------------------------

locu_50467C:                 // CODE XREF: sub_502D0B+196Dj
        jmp    locu_504620
// ---------------------------------------------------------------------------

locu_50467E:                 // CODE XREF: sub_502D0B+192Aj
        mov    eax, [ebp-0x01C8]
        mov    dword ptr [eax+0Ch], 0
        mov    ecx, [ebp-0x01C8]
        mov    dword ptr [ecx+10h], 0
        mov    edx, [ebp-0x01C8]
        mov    dword ptr [edx+8], 0
        mov    eax, [ebp-0x01C8]
        mov    dword ptr [eax+4], 0
        mov    ecx, [ebp-0x01C8]
        mov    ecx, [ecx+14h]
        mov    edx, 0x5DBBD6
        call    edx
        mov    edx, [ebp-0x01C8]
        mov    dword ptr [edx+14h], 0

locu_5046CD:                 // CODE XREF: sub_502D0B+17F8j
                        // sub_502D0B+18FEj
        mov    eax, [ebp-0x080]
        push    eax
        mov    ecx, [ebp+0x08]
        mov    ecx, [ecx+38h]
        mov    edx, 0x52C98B
        call    edx
        cmp    dword ptr [ebp-0x084], 0
        jz    locu_504735
        lea    edx, [ebp-0x084]
        mov    [ebp-0x01D4], edx
        mov    eax, [ebp-0x088]
        mov    [ebp-0x01D0], eax
        mov    ecx, [ebp-0x01D4]
        mov    edx, [ecx]
        mov    [ebp-0x01CC], edx
        mov    eax, [ebp-0x01D4]
        mov    ecx, [ebp-0x01CC]
        mov    edx, [ecx]
        mov    [eax], edx
        mov    eax, [ebp-0x01CC]
        mov    ecx, [eax+8]
        mov    [ebp-0x01D8], ecx
        mov    edx, [ebp-0x01D8]
        mov    [ebp-0x080], edx
        jmp    locu_50473C
// ---------------------------------------------------------------------------

locu_504735:                 // CODE XREF: sub_502D0B+19D8j
        mov    dword ptr [ebp-0x080], 0

locu_50473C:                 // CODE XREF: sub_502D0B+1A28j
        jmp    locu_50443C
// ---------------------------------------------------------------------------

locu_504741:                 // CODE XREF: sub_502D0B+1735j
        jmp    locu_50477D
locu_50477D:                 // CODE XREF: sub_502D0B+1A5Aj
        push    ecx
        mov    ecx, esp
        mov    [ebp-0x0F0], esp
        push    0x63B204 // "All sacks picked up"
        mov    edx, 0x5DD8F8
        call    edx
        mov    [ebp-0x02AC], eax
        mov    edx, 0x43AA23
        call    edx
        add    esp, 4
        jmp    locu_5049B2
///////////////////////////////////////////////
lu_showmap:                 // CODE XREF: sub_502D0B+2FCj
        push    0x63B218 // "#show map"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_aftershowmap
        jmp    locu_5047E3
locu_5047E3:                 // CODE XREF: sub_502D0B+1AC2j
        mov    edx, [ebp+0x08]
        push    edx
        push    0
        push    1
        push    0AAh
        mov    ecx, 0x6C3A08
        mov    edx, 0x51CEFB
        call    edx
        jmp    locu_504836
locu_504836:                 // CODE XREF: sub_502D0B+1B13j
        jmp    locu_5049B2
//////////////////////////////////////////////////////////////////----
lu_hidemap:                 // CODE XREF: sub_502D0B+309j
        push    0x63B224 // "#hide map"
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_afterhidemap
        jmp    locu_50487B
locu_50487B:                 // CODE XREF: sub_502D0B+1B5Aj
        mov    edx, [ebp+0x08]
        push    edx
        push    0
        push    0
        push    0x000AA
        mov    ecx, 0x6C3A08
        mov    edx, 0x51CEFB
        call    edx
        jmp    locu_5048CE
locu_5048CE:                 // CODE XREF: sub_502D0B+1BABj
        jmp    locu_5049B2
/////////////////////////////////////////////////////////
lu_info:                 // CODE XREF: sub_502D0B+29Dj
        push    offset ainfo //
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123
        call    edx
        test    eax, eax
        jnz    lu_afterinfo

        push    6 //////// "#info "
        lea    edx, [ebp-0x098]
        push    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5D9F0C
        call    edx
        mov    [ebp-0x01EC], eax
        mov    eax, [ebp-0x01EC]
        mov    [ebp-0x01F0], eax
        mov    byte ptr [ebp-0x04], 2
        mov    ecx, [ebp-0x01F0]
        push    ecx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DD9D1
        call    edx
        mov    byte ptr [ebp-0x04], 0
        lea    ecx, [ebp-0x098]
        mov    edx, 0x5DD88A
        call    edx
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA55C
        call    edx
        push    ecx
        mov    ecx, esp
        mov    [ebp-0x09C], esp
        lea    edx, [ebp+0x0C]
        push    edx
        mov    edx, 0x5DD74F
        call    edx
        mov    [ebp-0x01F4], eax
        mov    ecx, 0x06CDB24
        mov    ecx, [ecx]
        mov    edx, 0x535D39
        call    edx
        mov    [ebp-0x018], eax
        cmp    [ebp-0x018], 0
        jz    locu_5049B2
        mov    eax, [ebp-0x018]
        cmp    dword ptr [eax+2Ch], 0
        jnz    locu_5049B2
        mov    ecx, [ebp-0x018]
        xor    edx, edx
        mov    dx, [ecx+4]
        push    edx
        mov    ecx, 0x6C3A08
        mov    edx, 0x518544
        call    edx

        mov    ecx, [ebp-0x018]
        mov    ecx, [ecx+0x38]
        push    ecx
        push    eax
        mov    ecx, [ebp-0x018]
        mov    eax, [ecx+0x0A78]
        push    eax////////////// loginname
        mov    edx, [ecx+18h]
        push    edx/////////////// charname
        call    get_player_info

        mov    ecx, [ebp+0x08]
        push    ecx

        lea    edx, [ebp-0x01C]
        push    edx
        mov    [edx], eax

        mov    ecx, 0x6C3A08
        mov    edx, 0x51CD89
        call    edx
        jmp    locu_5049B2
/////////////////////////////////////////////////
lu_entermode:                 // CODE XREF: sub_502D0B+309j
        push    offset aMode //// 
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x5DA123///// CString::Find
        call    edx 
        test    eax, eax
        jnz    lu_afterenter_mode

        mov    ecx, [ebp+0x0c]
        push    ecx
        mov    ecx, [ebp+0x8]
        push    ecx
        call    set_mode

        jmp    locu_5049B2
//////////////////////////////////////////
locu_5049B2:                 // CODE XREF: sub_502D0B:locu_502E54j
                        // sub_502D0B+30Fj ...
        mov    dword ptr[ebp-0x04], 0FFFFFFFFh
        lea    ecx, [ebp+0x0C]
        mov    edx, 0x05DD88A
        call    edx // CString::~CString(void)

locu_5049C1:                 // CODE XREF: sub_502D0B+4Dj
                        // sub_502D0B+7C3j ...
        mov    ecx, [ebp-0x0C]
        mov    fs:0, ecx
        mov    esp, ebp
        pop    ebp
        retn    8
    }
}
#pragma warning (default: 4733)
#pragma warning (default: 4102)
