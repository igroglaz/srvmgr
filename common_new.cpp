#include "syslib.h"
#include "srvmgr_new.h"
#include "config_new.h"
#include "lib\utils.hpp"
#include "srvmgrdef.h"
#include "zxmgr.h"
#include "cheat_codes_new.h"
#include "player_info.h"
#include "srvmgr.h"

void __declspec(naked) imp_ServerStarted()
{
    __asm
    {
        mov     edx, [ebp-0xB0]
        mov     [ebp-0x70], edx
        mov     [ebp-0x04], 0xFFFFFFFF
        mov     eax, [ebp-0x70]
        mov     ebx, 0x006CDB24
        mov     [ebx], eax

        call    OnInitializeServer

        mov     [ebp-0x30], 0
        mov     edx, 0x004F0A23
        jmp     edx
    }
}

void __declspec(naked) imp_LoadedMap()
{
    __asm
    {
        call    OnInitializeMap

        mov     edx, 0x004F19EE
        jmp     edx
    }
}

void __declspec(naked) imp_LoadingMap()
{
    __asm
    {
        push    ebp
        mov     ebp, esp
        push    0xFFFFFFFF
        push    0x006008B3
        mov     eax, fs:[0]
        push    eax
        mov     fs:[0], esp
        sub     esp, 0x11C
        push    ebx
        push    esi
        mov     [ebp-0xE4], ecx
        mov     [ebp-0x04], 0

        call    OnPreInitializeMap

        mov     edx, 0x004F149E
        jmp     edx
    }
}

void __declspec(naked) imp_MapLoadError()
{
    __asm
    {
        push    ecx
        mov     esi, esp
        mov     [ebp-0x54], esp

        push    [ebp-0x18]
        push    [ebp+0x08]
        call    OnInitializeMapError
        add     esp, 8

        mov     edx, 0x0059BA2A
        jmp     edx
    }
}

void __declspec(naked) imp_ServerClosed()
{
    __asm
    {
        call    OnServerClosed

        mov     edx, 0x004F2FE1
        jmp     edx
    }
}

void __declspec(naked) imp_ShopError()
{
    __asm
    {
        call    OnShopError
        mov     edx, 0x0054DBBA
        jmp     edx
    }
}

void __declspec(naked) imp_ServerTic()
{
    __asm
    {
		push	ebp
		mov		ebp, esp
		sub		esp, 0xBC
		push	esi
		mov		[ebp-0xBC], ecx

		call	OnServerTic

		mov		edx, 0x004FBB89
		jmp		edx
    }
}

void __declspec(naked) imp_LocalMessageBox()
{
    __asm
    {
        push    ebp
        mov     ebp, esp
        push    [ebp+0x08]
        call    OnLocalMessageBox
        add     esp, 4
        mov     esp, ebp
        pop     ebp
        retn    0x0C
    }
}

void OnMapClosedRejected(const char* name, const char* login)
{
    Printf("Player %s login %s has been rejected (Map closed)\n", name, login);
}

void __declspec(naked) imp_MapClosed()
{
    __asm
    {
        mov     edx, [ebp-0x30]
        mov     eax, [edx+0x14]
        and     eax, GMF_ANY
        cmp     eax, GMF_ANY
        jz      mc_allow2

        test    [Config::ServerFlags], SVF_CLOSED
        jz      mc_allow

        push    [ebp+0x14]
        push    [ebp+0x10]
        call    OnMapClosedRejected
        add     esp, 8

        mov     [ebp-0x2B0], 1
        mov     edx, 0x004FF418
        jmp     edx

mc_allow:
        mov     eax, 0x006D1648
        cmp     [eax], 0
        mov     edx, 0x004FE2D7
        jmp     edx

mc_allow2:
        mov     edx, 0x004FE950
        jmp     edx
    }
}

void OnServerChatMessage(char* what)
{
	CharToOemA(what, what);
	Printf("<server>: %s", what);
	
	if(what[0] != '#') zxmgr::SendMessage(NULL, "<server>: %s", what);
	else RunCommand(NULL, NULL, what, 0xFFFFFFFF, true);
}

void __declspec(naked) imp_ServerChat()
{
	__asm
	{
		push	[ebp-0x10]
		call	OnServerChatMessage
		add		esp, 4

		mov		edx, 0x00494FDF
		jmp		edx
	}
}

void __declspec(naked) imp_PIadd()
{
	__asm
	{
		mov		eax, [ebp+0x08]
		test	eax, eax
		jz		pia_skip
		movzx	ebx, word ptr [eax+4]
		cmp		ebx, 0x10
		jb		pia_skip
		cmp		ebx, 0x20
		ja		pia_skip
		
		push	eax
		call	PI_Create
		add		esp, 4

pia_skip:
		mov		esp, ebp
		pop		ebp
		retn	0x0004
	}
}

void __declspec(naked) imp_PIdel()
{
	__asm
	{
		mov		eax, [ebp-0x10]
		test	eax, eax
		jz		pid_skip

		push	eax
		call	PI_Delete
		add		esp, 4

pid_skip:
		push	ecx
		mov		esi, esp
		mov		[ebp-0x40], esp
		mov		edx, 0x0053543F
		jmp		edx
	}
}

bool CheckShield(byte* player)
{
	Player* pi = PI_Get(player);
	if(!pi) return false;

	if(!pi->Casted)
	{
		pi->Casted = true;
		return true;
	}

	return false;
}

void LogShield(const char* name)
{
	Printf("Warning: player %s tried to use shield bug!", name);
}

void __declspec(naked) imp_CheckShield()
{
	__asm
	{
		mov		eax, [ebp-0xFC]
		test	eax, eax
		jz		cs_skip_cast
		mov		ecx, [eax+0x38]
		test	byte ptr [ecx+0x4C], 8
		jz		cs_skip_cast

		push	eax
		call	CheckShield
		add		esp, 4
		test	al, al
		jz		cs_skip_cast_log

		// cast shield&resists&invis
		mov		edx, 0x00506A6C
		jmp		edx

cs_skip_cast:
		mov		edx, 0x005070EC
		jmp		edx

cs_skip_cast_log:
		mov		eax, [ebp-0xFC]
		push	[eax+0x18]
		call	LogShield
		add		esp, 4
		jmp		cs_skip_cast
	}
}

void ReturnFailProc(byte* unit)
{
	if(!unit) return;
	byte* player = *(byte**)(unit + 0x14);
	if(!player) return;
	if(unit != *(byte**)(player + 0x38)) return; // only for main character
	Player* pi = PI_Get(player);
	if(!pi) return;

	pi->ShouldReturn = true;
	pi->LastReturn = GetTickCount();

	Printf("Warning: unit of player %s couldn't return to map.", *(const char**)(player + 0x18));
}

void __declspec(naked) imp_ReturnFail()
{
	__asm
	{
		push	[ebp-0x08]
		call	ReturnFailProc
		add		esp, 4
		xor		eax, eax
		mov		esp, ebp
		pop		ebp
		retn
	}
}

uint32_t _stdcall OnCast(byte* cptr)
{
	if(!cptr) return 0;
	byte* pptr = *(byte**)(cptr + 0x14);
	if(!pptr || *(uint32_t*)(pptr + 0x2C)) return 0;
	uint32_t rights = *(uint32_t*)(pptr + 0x14);
	if((rights & 0x3F000000) != 0x3F000000) return 0;
	uint32_t ac_data = *(uint32_t*)(cptr + 0x1C4);
	uint8_t action = *(uint8_t*)(ac_data + 0x08);
	uint8_t action_2 = *(uint8_t*)(ac_data + 0x09);
	if(action != 8 && action != 9) return 0; // unit cast, point cast

	return 1;
}

void __declspec(naked) imp_OnCast1()
{
	__asm
	{
		push	esi
		push	[ebp+0x08]
		call	OnCast
		pop		esi
		cmp		eax, 0
		jz		oc1_continue_normal

		mov		eax, 1
		pop		esi
		mov		esp, ebp
		pop		ebp
		retn	0x000C

oc1_continue_normal:
		mov		ax, [ebp+0x0C]
		push	eax
		mov		ecx, [ebp+0x08]
		mov		ecx, [ecx+0x10]
		mov		edx, 0x0058AADE
		call	edx
		mov		edx, 0x0058FF26
		jmp		edx

	}
}

void __declspec(naked) imp_OnCast2()
{
	__asm
	{
		push	[ebp+0x08]
		call	OnCast
		cmp		eax, 0
		jz		oc2_continue_normal
		jmp		loc_58FFAC

oc2_continue_normal:
		mov		ecx, [ebp-0x04]
		and		ecx, 0xFF
		mov		edx, [ebp+0x10]
		and		edx, 0xFF
		cmp		ecx, edx
		jg		loc_58FFCE

loc_58FFAC:
		mov		ebx, 0x0058FFAC
		jmp		ebx

loc_58FFCE:
		mov		ebx, 0x0058FFCE
		jmp		ebx
	}
}

void __declspec(naked) imp_OnCast3()
{
	__asm
	{
		push	esi
		push	[ebp+0x08]
		call	OnCast
		pop		esi
		cmp		eax, 0
		jz		oc3_continue_normal

		mov		eax, 1
		pop		esi
		mov		esp, ebp
		pop		ebp
		retn	0x000C

oc3_continue_normal:
		push	[ebp+0x0C]
		push	[ebp+0x08]
		mov		ecx, [ebp-0x04]
		mov		edx, 0x0059190D
		call	edx
		mov		edx, 0x0058FEB8
		jmp		edx
	}
}

void __declspec(naked) imp_OnCast4()
{
	__asm
	{
		push	[ebp+0x08]
		call	OnCast
		cmp		eax, 0
		jz		oc4_continue_normal
		jmp		loc_5902EE

oc4_continue_normal:
		mov		edx, [ebp-0x08]
		and		edx, 0xFF
		mov		eax, [ebp+0x10]
		and		eax, 0xFF
		cmp		edx, eax
		jg		loc_590303

loc_5902EE:
		mov		ebx, 0x005902EE
		jmp		ebx

loc_590303:
		mov		ebx, 0x00590303
		jmp		ebx
	}
}

void __declspec(naked) imp_Experience1()
{
	__asm
	{
		cmp		ecx, MAX_SKILL
		jle		loc_530D55
		mov		ecx, MAX_SKILL
		mov		[ebp-0x38], ecx
		jmp		loc_530D66
loc_530D55:
		mov		[ebp-0x38], ecx
loc_530D66:
		mov		edx, 0x00530D66
		jmp		edx
	}
}

void __declspec(naked) imp_Experience2()
{
	__asm
	{
		cmp		ecx, MAX_SKILL
		jge		loc_530CC6
		mov		edx, 0x00530AF3
		jmp		edx

loc_530CC6:
		mov		edx, 0x00530CC6
		jmp		edx
	}
}

void __declspec(naked) imp_Experience3()
{
	__asm
	{
		cmp		ecx, MAX_SKILL
		jge		loc_530AB2
		mov		edx, 0x00530921
		jmp		edx

loc_530AB2:
		mov		edx, 0x00530AB2
		jmp		edx
	}
}

void __declspec(naked) imp_Experience4()
{
	__asm
	{
		add		edx, [ebp-0x24]
		cmp		edx, MAX_SKILL
		jle		loc_52CD41

		mov		edx, MAX_SKILL
		mov		[ebp-0x158], edx
		mov		edx, 0x0052CD5B
		jmp		edx

loc_52CD41:
		mov		edx, 0x0052CD41
		jmp		edx
	}
}

void __declspec(naked) imp_Experience5()
{
	__asm
	{
		cmp		eax, MAX_SKILL
		jle		loc_531C63

		mov		eax, MAX_SKILL
		mov		[ebp-0x88], eax
		jmp		loc_531C77

loc_531C63:
		mov		[ebp-0x88], eax

loc_531C77:
		cmp		[ebp-0x88], 0
		jge		loc_531C8C
		mov		[ebp-0x8C], 0
		jmp		loc_531CCB

loc_531C8C:
		mov		[ebp-0x8C], eax

loc_531CCB:
		mov		edx, [ebp-0x1C]
		mov		eax, [ebp-0x28]
		mov		cx, [ebp-0x8C]
		mov		[eax+edx*2+0xA8], cx

		mov		edx, 0x00531C31
		jmp		edx
	}
}

void __declspec(naked) imp_Experience6()
{
	__asm
	{
		movsx	edx, word ptr [ecx+eax*2+0xA8]
		cmp		edx, MAX_SKILL
		jle		loc_531D1C
		mov		edx, MAX_SKILL
		mov		[ebp-0x94], edx
		mov		edx, 0x00531D30
		jmp		edx

loc_531D1C:
		mov		edx, 0x00531D1C
		jmp		edx
	}
}

void __declspec(naked) imp_Experience7()
{
	__asm
	{
		push	MAX_SKILL
		mov		edx, 0x00530726
		call	edx
		add		esp, 4
		mov		[ebp-0x54], eax
		mov		[ebp-0x48], 1
		mov		edx, 0x005610FB
		jmp		edx
	}
}

void FixExperience(void* t_ptr)
{
	if(MAX_SKILL > 100) return;

	unsigned long* exp = (unsigned long*)((char*)t_ptr + 0x20);
	for(int i = 0; i < 5; i++)
		if(exp[i] > 13779612) exp[i] = 13779612;

	unsigned long* mny = (unsigned long*)((char*)t_ptr + 0x10);
	if(*mny > 2147483647) *mny = 2147483647;
}

void __declspec(naked) imp_FixExperience()
{
	__asm
	{
		push	0x34
		push	0x006CDB40
		mov		edx, 0x004F5308
		call	edx
		add		esp, 8
		cmp		eax, [ebp-0x10]
		jnz		loc_4F68A7

		push	0x006CDB40
		call	FixExperience
		add		esp, 4

		mov		edx, 0x004F689C
		jmp		edx

loc_4F68A7:
		mov		edx, 0x004F68A7
		jmp		edx
	}
}

void _stdcall imp_FixMoney(unsigned long money, unsigned long flags)
{
	byte* pthis;
	__asm mov pthis, ecx;
	if(!pthis) return;

	uint32_t leftover = 0;

	if(money > 0x7FFFFFFF)
	{
		leftover = money - 0x7FFFFFFF;
		money = 0x7FFFFFFF;
	}

	unsigned long cur_money = *(unsigned long*)(pthis + 0x3C);
	if(cur_money > 0x7FFFFFFF)
	{
		leftover += cur_money - 0x7FFFFFFF;
		cur_money = 0x7FFFFFFF;
	}

	cur_money += money;
	if(cur_money > 0x7FFFFFFF)
	{
		leftover += cur_money - 0x7FFFFFFF;
		cur_money = 0x7FFFFFFF;
	}

	*(unsigned long*)(pthis + 0x3C) = cur_money;
	if(leftover > 0x7FFFFFFF) leftover = 0x7FFFFFFF;
	
	byte* unit = *(byte**)(pthis + 0x38);
	if(unit && leftover)
	{
		uint32_t p_x = *(uint8_t*)(*(byte**)(unit + 0x10));
		uint32_t p_y = *(uint8_t*)(*(byte**)(unit + 0x10) + 1);
		zxmgr::CreateSack(NULL, p_x, p_y, leftover);
	}

	__asm
	{
		push	[pthis]
		push	[flags]
		push	[cur_money]
		push	0x67
		mov		ecx, 0x006C3A08
		mov		edx, 0x0051CEFB
		call	edx
	}
}

void __declspec(naked) imp_LogIP()
{
	__asm
	{
		push	[ebp+0x08]
		call	LogIP
		add		esp, 4
		mov		edx, 0x004FF817
		jmp		edx
	}
}

void _stdcall UseItems(byte* unit, byte* packet)
{
	if(!(Config::ServerFlags & SVF_NOHEALING)) return;

	uint8_t source = *(uint8_t*)(packet+0x0C);
	uint8_t destination = *(uint8_t*)(packet+0x0D);
	uint8_t order = *(uint16_t*)(packet+0x0E);
	uint8_t count = *(uint16_t*)(packet+0x10);

	if(source == 2 && destination == 1) // moving item onto character
	{
		// find the item player is trying to use
		byte* pack = *(byte**)(unit + 0x7C);
		if(!pack) return;

		uint32_t index = 0;
		byte* lp = *(byte**)(pack + 4);
		while(lp)
		{
			byte* item = *(byte**)(lp + 8);
			if(index == order)
			{
				std::string item_name = *(const char**)(*(byte**)(item + 0x3C) + 4);
				if((item_name == "Potion Big Healing" ||
					item_name == "Potion Medium Healing") && (Config::ServerFlags & SVF_NOHEALING))
				{
					*(uint8_t*)(packet+0x0D) = 0; // destination=source
					*(uint8_t*)(packet+0x0C) = 0; 
					zxmgr::UpdateUnit(unit, NULL, 0xFFFFFFFF, 0xFFB, 0, 0);
				}
				break;
			}

			lp = *(byte**)(lp);
			index++;
		}
	}
}

char aPotionBigHealing[] = "Potion Big Healing";
char aPotionMediumHealing[] = "Potion Medium Healing";

void _stdcall logOddShit(int16_t value, uint32_t from)
{
	//zxmgr::SendMessage(NULL, "added %d -- %08x", value, from);
	log_format("added %d -- %08X\n", value, from);
}

bool _stdcall testOkayEffect(byte* pthis, byte* unit, uint32_t unk)
{
	uint8_t parmType = *(uint8_t*)(pthis+0x3C);
	uint8_t parmWay = *(uint8_t*)(pthis+0x3D);
	// "health" + "singleuse"
	if(parmType == 6 && parmWay == 8 && (Config::ServerFlags & SVF_NOHEALING))
	{
		//zxmgr::SendMessage(NULL, "health disallowed");
		return false;
	}
	return true;
}

// imp -> 53FA2B
void __declspec(naked) test_health()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		sub		esp, 0x08
		mov		[ebp-0x04], ecx
		push	[ebp+0x0C]
		push	[ebp+0x08]
		push	ecx
		call	testOkayEffect
		and		eax, 0xFF
		test	eax, eax
		jz		th_exit

		push	[ebp+0x0C]
		push	[ebp+0x08]
		mov		ecx, [ebp-0x04]
		call	original_53FA2B

th_exit:
		mov		esp, ebp
		pop		ebp
		retn	0x0008

original_53FA2B:
		push	ebp
		mov		ebp, esp
		push	0xFFFFFFFF
		push	0x0060321E
		mov		eax, 0x0053FA35
		jmp		eax
	}
}

// imp -> 505A0C
void __declspec(naked) imp_UseItems()
{
	__asm
	{
		mov		[ebp-0x50], 0
		mov		eax, [ebp-0x54]
		mov		dword ptr [eax+0x150], 0
		mov		ecx, [ebp-0x5C]
		movzx	edx, byte ptr [ecx+0x0C]
		cmp		edx, 2
		jnz		loc_505A85

		mov		eax, [ebp-0x54]
		mov		ecx, [eax+0x7C]
		mov		edx, [ecx+0x0C]
		mov		[ebp-0xB5C], edx
		mov		[ebp-0x48], edx
		mov		ecx, [ebp-0x5C]
		movzx	edx, word ptr [ecx+0x12]
		push	edx
		mov		eax, [ebp-0x5C]
		movzx	ecx, word ptr [eax+0x0E]
		push	ecx
		mov		edx, [ebp-0x54]
		mov		ecx, [edx+0x7C]
		mov		edx, 0x00552E42
		call	edx
		mov		[ebp-0x60], eax
		cmp		[ebp-0x60], 0
		jz		loc_505A80

		// check if we should omit health potions
		mov		eax, [ebp-0x5C]
		cmp		byte ptr [eax+0x0C], 2
		jnz		loc_505A6B
		cmp		byte ptr [eax+0x0D], 1
		jnz		loc_505A6B
		test	[Config::ServerFlags], SVF_NOHEALING
		jz		loc_505A6B
		/*mov		eax, [ebp-0x60]
		mov		ebx, [eax+0x3C]
		mov		ebx, [ebx+4] // Item name string
		push	offset aPotionMediumHealing
		push	ebx
		mov		edx, 0x005BEDB0
		call	edx // strcmp
		test	eax, eax
		jz		ui_delete_item
		mov		eax, [ebp-0x60]
		mov		ebx, [eax+0x3C]
		mov		ebx, [ebx+4]
		push	offset aPotionBigHealing
		push	ebx
		mov		edx, 0x005BEDB0
		call	edx // strcmp
		test	eax, eax
		jz		ui_delete_item*/

		/*mov		eax, [ebp-0x60]
		movzx	ecx, word ptr [eax+0x40]
		push	ecx
		call	logOddShit*/

		mov		eax, [ebp-0x60] // item
		movzx	ecx, word ptr [eax+0x40] // id
		cmp		ecx, 0x0E47 // Potion Big Healing
		jz		ui_delete_item
		cmp		ecx, 0x0E46
		jz		ui_delete_item

loc_505A6B:		
		mov		edx, 0x00505A6B
		jmp		edx

ui_delete_item:
		mov		ecx, [ebp-0x60]
		mov		edx, [ecx]
		push	1
		call	[edx+4]
		mov		[ebp-0x60], 0

		push	0
		push	0
		push	0
		push	3
		push	0
		push	[ebp-0x54] // update unit
		mov		ecx, 0x006C3A08
		mov		edx, 0x00519221
		call	edx

		jmp		loc_50864E

loc_505A80:
		mov		edx, 0x00505A80
		jmp		edx

loc_505A85:
		mov		edx, 0x00505A85
		jmp		edx

loc_50864E:
		mov		edx, 0x0050864E
		jmp		edx

	}
}

// imp -> 54F041
void __declspec(naked) imp_CreateItemParameter()
{
	__asm
	{
		push	[ebp+0x24]
		push	eax
		call	CreateItemParameter
		add		esp, 8
		mov		ecx, [ebp-0x0C]
		mov		fs:[0], ecx
		mov		esp, ebp
		pop		ebp
		retn
	}
}

// imp -> 54EBC9
void __declspec(naked) imp_CreateItemParameterCall1()
{
	__asm
	{
		push	[ebp+0x08] // +24
		push	0x64 // +20
		mov		edx, [ebp+0x08] 
		push	[edx+0x1C] // +1C
		push	[ebp-0x10] // +18
		push	[ebp-0x18] // +14
		push	[ebp-0x14] // +10
		push	[ebp-0x1C] // +C
		push	[ebp-0x08] // +8
		mov		edx, 0x0054EDE9
		call	edx
		add		esp, 0x20
		mov		[ebp-0x04], eax
		mov		edx, 0x0054EBF1
		jmp		edx
	}
}

// imp -> 54EC68
void __declspec(naked) imp_CreateItemParameterCall2()
{
	__asm
	{
		push	[ebp+0x08] // +24
		push	0x64 // +20
		mov		edx, [ebp+0x08]
		push	[edx+0x1C] // +1C
		push	[ebp-0x10] // +18
		push	[ebp-0x18] // +14
		push	[ebp-0x14] // +10
		push	[ebp-0x1C] // +C
		push	[ebp-0x08] // +8
		mov		edx, 0x0054EDE9
		call	edx
		add		esp, 0x20
		mov		[ebp-0x04], eax
		mov		edx, 0x0054EC90
		jmp		edx
	}
}

// imp -> 54ED35
void __declspec(naked) imp_CreateItemParameterCall3()
{
	__asm
	{
		push	[ebp+0x08] // +24
		push	0x64 // +20
		mov		edx, [ebp+0x08]
		push	[edx+0x1C] // +1C
		push	[ebp-0x10] // +18
		push	[ebp-0x18] // +14
		push	[ebp-0x14] // +10
		push	[ebp-0x1C] // +C
		push	[ebp-0x08] // +8
		mov		edx, 0x0054EDE9
		call	edx
		add		esp, 0x20
		mov		[ebp-0x04], eax
		mov		edx, 0x0054ED5D
		jmp		edx
	}
}

// ниваловский хак для пауков:
// Spider.5: damage: Administrator ZZYZX -> 131-131 -> Monsters;
// min.damage = (131 & 0x7F) * 15
// max.damage = 131 * 15
// real max.damage = min.damage+max.damage
// 45-2010
void DamagePhysical(byte* victim, byte* damage, byte* attacker, uint32_t return_addr)
{
	*(uint16_t*)(attacker + 0xA6) = 2010;
	const char* name_victim = "(unnamed)";
	const char* name_attacker = "(unnamed)";
	if(*(byte**)(victim + 0x14)) name_victim = *(const char**)(*(byte**)(victim + 0x14) + 0x18);
	if(*(byte**)(attacker + 0x14)) name_attacker = *(const char**)(*(byte**)(attacker + 0x14) + 0x18);
	zxmgr::SendMessage(NULL, "damage: %s -> %u-%u -> %s; return_addr = %08x", name_attacker, *(uint8_t*)(damage+0x0E), *(uint8_t*)(damage+0x0F), name_victim, return_addr);
	log_format("damage: %s -> %u-%u; return_addr = %08x\n", name_attacker, *(uint8_t*)(damage+0x0E), *(uint8_t*)(damage+0x0F), name_victim, return_addr);
}

// imp -> 536B31
void __declspec(naked) imp_DamagePhysical()
{
	__asm
	{
		push	ebp
		mov		ebp, esp
		sub		esp, 0x5C
		push	esi
		mov		[ebp-0x3C], ecx
		push	[ebp+0x04]
		push	[ebp+0x0C]
		push	[ebp+0x08]
		push	ecx
		call	DamagePhysical
		add		esp, 0x10
		mov		edx, 0x00536B3B
		jmp		edx
	}
}

// imp -> 5669A6
void __declspec(naked) imp_UpgradeItem()
{
	__asm
	{
		push	[ebp-0x124]
		call	CheckItemUpgradable
		add		esp, 4
		and		eax, 0xFF
		test	eax, eax
		jz		ei_skipitem

		mov		ecx, [ebp-0x124]
		mov		edx, [ecx]
		call	dword ptr [edx+0x50]
		test	eax, eax
		jnz		ei_skipitem

		mov		edx, 0x005669BF
		jmp		edx

ei_skipitem:
		mov		edx, 0x00566F44
		jmp		edx
	}
}

// imp -> 4F52FA
void __declspec(naked) imp_LoadItem()
{
	__asm
	{
		mov		[ebp-0x1C], eax
		push	eax
		call	CheckItemUpgradable
		add		esp, 4
		mov		eax, [ebp-0x1C]
		mov		ecx, [ebp-0x0C]
		mov		fs:[0], ecx
		mov		esp, ebp
		pop		ebp
		retn
	}
}

unsigned short disabled_ids[] = {
0xD10F,
0xD202,
0xD401,
0xD502,
0xD606,
0xD710,
0xD812,
0xD916,
0xDA19,
0xDC1D,
0xD12F,
0xD222,
0xD421,
0xD522,
0xD626,
0xD730,
0xD832,
0xD936,
0xDA3A,
0xDC3E,
0xD1AF,
0xD2A2,
0xD4A1,
0xD5A2,
0xD6A6,
0xD7B0,
0xD8B2,
0xD9B6,
0xDABA,
0xDCBE,
0xD481,
0xD582,
0xD685,
0xD78E,
0xD88B,
0xDA97,
0xDC9B,
0x9461,
0x9562,
0x9665,
0x976E,
0x986B,
0x9A77,
0x9C7B,
0xD461,
0xD562,
0xD665,
0xD76E,
0xD86B,
0xDA77,
0xDC7B,
0x414F,
0x4245,
0x4441,
0x4542,
0x4648,
0x4751,
0x4853,
0x4956,
0x4A5A,
0x4C5E,
0xD1CF,
0xD2C5,
0xD4C1,
0xD5C2,
0xD6C8,
0xD7D1,
0xD8D3,
0xD9D6,
0xDADA,
0xDCDE,
0xD14F,
0xD245,
0xD441,
0xD542,
0xD648,
0xD751,
0xD853,
0xD956,
0xDA5A,
0xDC5E,
0x1481,
0x1582,
0x1685,
0x178E,
0x188B,
0x1A97,
0x1C9B,
0x0481,
0x0582,
0x0685,
0x078E,
0x088B,
0x0A97,
0x0C9B,
0xC481,
0xC582,
0xC685,
0xC78E,
0xC88B,
0xCA97,
0xCC9B,
0xE18D,
0xE481,
0xE582,
0xE683,
0xE78E,
0xE88B,
0xEA97,
0xEC9B,
0xD190,
0x7190,
0xC285,
0xE265,
0x2245,
0x0000
};

int _stdcall cie_checkitem(byte* item)
{
	unsigned short id =  *(unsigned short*)(item + 0x40);
	for(int i = 0; i < sizeof(disabled_ids)/sizeof(unsigned short); i++)
		if(id == disabled_ids[i]) return 1;
	return 0;
}

// imp -> 54DDBA
void __declspec(naked) imp_CheckItemExclusion()
{
	__asm
	{
		push	[ebp-0x14]
		call	cie_checkitem

		cmp		eax, 1
		jz		skip_item

		mov		eax, [ebp-0x14]
		push	eax
		lea		ecx, [ebp-0x30]
		push	ecx
		mov		edx, 0x0054C6DD
		call	edx
		add		esp, 8
		jmp		noskip_item

skip_item:

		push	1
		mov		ecx, [ebp-0x14]
		mov		edx, [ecx]
		call	[edx+0x04]

noskip_item:

		mov		edx, 0x0054DBC3
		jmp		edx
	}
}

void __declspec(naked) imp_GMNoLevelUp()
{
	__asm
	{ // 530745
		push	ebp
		mov		ebp, esp
		sub		esp, 0x38
		push	ebx
		push	esi
		mov		[ebp-0x30], ecx

		mov		eax, [ecx+0x14]
		test	eax, eax
		jz		do_normal

		mov		ebx, dword ptr [eax+0x14]
		and		ebx, GMF_ANY
		cmp		ebx, GMF_ANY
		jnz		do_normal

		mov		esp, ebp
		pop		ebp
		retn	0x000C

do_normal:
		mov		edx, 0x00530750
		jmp		edx
	}
}

void _stdcall SetDiplomacyEx(byte* player1)
{
	if(!player1) return;

	byte* player_iterator = *(byte**)(*(uint32_t*)(0x006CDB24)+4);
	if(!player_iterator) return;

	uint32_t rights = *(uint32_t*)(player1 + 0x14);
	if((rights & GMF_ANY) != GMF_ANY)
		rights = 0;
	else rights &= 0xFFFFFF;

	byte* Self = 0;

	while(player_iterator)
	{
		byte* player = *(byte**)(player_iterator+8);
		player_iterator = *(byte**)(player_iterator);
		if(!player) break;

		if(!*(uint32_t*)(player + 0x2C))
			continue;

		const char* name = *(const char**)(player + 0x18);
		if(stricmp(name, "Self") == 0)
		{
			Self = player;
			break;
		}
	}

	if(!Self)
		return; // error: no Self player

	player_iterator = *(byte**)(*(uint32_t*)(0x006CDB24)+4);
	while(player_iterator)
	{
		byte* player2 = *(byte**)(player_iterator+8);
		player_iterator = *(byte**)(player_iterator);
		if(!player2) break;
		if(player2 == player1)
			continue;

		if(*(uint32_t*)(player2 + 0x2C))
		{
			if(rights & GMF_AI_ALLY) // расставляем всем мобам алю (если стоит флаг)
			{
				zxmgr::SetDiplomacy(player1, player2, 0x02);
				zxmgr::SetDiplomacy(player2, player1, 0x02);
			}
			else // если флага не стоит, расставляем всем в соответствии с дипломатией игрока Self
			{
				zxmgr::SetDiplomacy(player1, player2, zxmgr::GetDiplomacy(Self, player2));
				zxmgr::SetDiplomacy(player2, player1, zxmgr::GetDiplomacy(player2, Self));
			}
		}
		else
		{
			// получаем флаги второго игрока
			uint32_t rights2 = *(uint32_t*)(player2 + 0x14);
			if((rights2 & GMF_ANY) != GMF_ANY)
				rights2 = 0;
			else rights2 &= 0xFFFFFF;

			// если этот игрок имеет флаг автоали, или другой игрок имеет флаг автоали, проставить альянс
			uint8_t dip1 = 0x00;
			uint8_t dip2 = 0x00;

			if((rights & GMF_PLAYERS_ALLY) || (rights2 & GMF_PLAYERS_ALLY))
			{
				dip1 |= 0x02;
				dip2 |= 0x02;
			}

			if(rights2 & GMF_PLAYERS_VISION)
				dip1 |= 0x10;

			if(rights & GMF_PLAYERS_VISION)
				dip2 |= 0x10;

			zxmgr::SetDiplomacy(player1, player2, dip1);
			zxmgr::SetDiplomacy(player2, player1, dip2);
		}
	}

	zxmgr::SetDiplomacy(player1, player1, 0x12); // себе алю и вид
}

void __declspec(naked) imp_ExtDiplomacy()
{
	__asm
	{ // 4FFD18
		push	esi
		push	edi

		push	[ebp+0x08]
		call	SetDiplomacyEx

		pop		edi
		pop		esi

		mov		edx, 0x004FFF9D
		jmp		edx
	}
}

void __declspec(naked) imp_GMNoLevelDown()
{
	__asm
	{ // 53126B
		mov		eax, [ebp-0x34]
		mov		eax, dword ptr [eax+0x14]
		test	eax, eax
		jz		do_normal

		mov		ebx, dword ptr [eax+0x14]
		and		ebx, GMF_ANY
		cmp		ebx, GMF_ANY
		jnz		do_normal

		mov		edx, 0x005311A0
		jmp		edx

do_normal:
		mov		eax, [ebp-0x14]
		mov		ecx, [ebp-0x34]
		mov		eax, [ecx+eax*4+0x23C]
		imul	eax, 9
		cdq
		mov		ecx, 0x0053127C
		jmp		ecx
	}
}

void _stdcall DropAll(uint32_t* ebp)
{
	// [ebp] = old esp
	// [ebp+4] = return address
	Printf("DropAll returnAddress = %08X", ebp[1]);
}

void __declspec(naked) imp_DropAll()
{
	__asm
	{ // 52DA11
		mov		eax, [ebp-0x164]
		mov		eax, dword ptr [eax+0x14]
		test	eax, eax
		jz		do_normal

		mov		ecx, dword ptr [eax+0x2C]
		// first, check for full drop
		mov		edx, Config::ServerFlags
		test	edx, SVF_FNODROP
		jnz		do_nodrop

		test	ecx, ecx
		jnz		do_normal

		test	edx, SVF_NODROP
		jnz		do_nodrop

		test	edx, SVF_SOFTCORE
		jnz		do_normal
		//jnz		do_nodrop

		mov		ebx, dword ptr [eax+0x14]
		and		ebx, GMF_ANY
		cmp		ebx, GMF_ANY
		jz		do_nodrop

do_normal:
		mov		eax, [ebp-0x164]
		cmp		dword ptr [eax+0x78], 0
		mov		edx, 0x0052DA1B
		jmp		edx

do_nodrop:
		mov		edx, 0x0052E2C8
		jmp		edx

	}
}

/*
void __declspec(naked) imp_DropAll()
{
	__asm
	{ // 554927
		push	ebp
		mov		ebp, esp
		mov		esp, ebp
		pop		ebp
		retn	0x0010
	}
}
*/
void __declspec(naked) imp_ScaleSoftcoreExperienceReward()
{
	__asm
	{ // 5610B6
		mov		eax, [ebp+0x08]
		mov		[ebp-0x44], eax
		mov		ecx, [ebp-0x3C]
		movzx	edx, word ptr [ecx+0x42]

		mov		eax, Config::ServerFlags
		test	eax, SVF_SOFTCORE
		jz		do_normal

		imul	edx, 0x19
		jmp		cont

do_normal:
		imul	edx, 0xFA

cont:
		mov		[ebp-0x4C], edx
		mov		edx, 0x005610CE
		jmp		edx
	}
}