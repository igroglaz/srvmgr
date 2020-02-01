#include <winsock2.h>
#include <windows.h>

#include "srvmgr.h"
#include "srvmgrdef.h"
#include "charcheck.h"
#include "shared.h"
#pragma warning(disable:4996) // no_deprecate

#include "config_new.h"
#include <string>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <ctime>

#include "lib\utils.hpp"
#include "lib\packet.hpp"
#include "srvmgr_new.h"

/// CDECL нет очистки стека функцией
/// STDCALL есть

char alog_vis[] = " id: 0x%.4X; (%d, %d)[%d] - vision=%d.%d\n";
char camp1[] = "Player %s tried to camp from building.";
char camp2[] = "Player %s initiated camping.";
char player_respawn[] = "Player %s respawned.";
char enter_b1[] = "<cmd: enter to building>\n";
char cancel_camp1[] = "Player %s cancelled camping.";
char enter_shop1[] = "Player %s entered shop ID=%u from (%u, %u).";
char enter_inn1[] = "Player %s entered inn ID=%u from (%u, %u).";
char left_shop1[] = "Player %s left shop.";
char left_inn1[] = "Player %s left inn.";
char aErrPoint[] = "name: %s, error dst\n";
char aArea_Cast[] = "player %s: area_cast from (%d, %d)[%d] to (%d, %d)[%d], vision=%d.%d\n";

char config[BUF_MAX];

bool exiting = false;

int(__cdecl *print_log)(char *message);

HANDLE hThread;
char avisall[] = "name: %s - id: 0x%.2X\n";
DWORD dwThreadId;

char ctl_dir[BUF_MAX];

Encoding encoding = CP1251;
char map_name_buf[0x200];
char log_invalid_packet[]="Invalid packet '%.2X' from '%s'\n";
char log_color_crasher[]="Player %s tried to use color crash bug\n";
unsigned spirit_factor = 256;

#include "bugs.h"

	    
#ifdef SRV_LOG
struct client {
	SOCKET s;
	std::string ip, name;
	struct client *n, *p;
} *c0 = 0;
#endif

char info[100];
char numb[] = "0123456789ABCDEF";

char spirit[MAX_PATH];

char perc_s[]="%s\n";

double exp_death = 1; // 1.0
double exp_full = 0.9; // 0.9

const char * __stdcall get_player_name(void *pl)
{
	return *(const char **)((char *)pl + 0x18);
}

void * __stdcall get_player(void *un)
{
	return *(void **)((char *)un + 0x14);
}

void __stdcall auto_war_cpp(void *unit_from, void *unit_to, int att)
{
	void *player_from = get_player(unit_from);
	void *player_to = get_player(unit_to);
	Printf("Player %s (%s) set auto-war to player %s", get_player_name(player_from), att ? "attacker" : "victim", get_player_name(player_to));
}

void __declspec(naked) auto_war_1()
{ // 005B56EF
	__asm
	{
		push	1
		push	[ebp+0xc]
		push	[ebp+8]
		call	auto_war_cpp
		mov	ecx, [ebp+8]
		mov	edx, [ecx+0x14]
		push	edx
		mov	edx, 0x05B56F6
		jmp	edx
	}
}

void __declspec(naked) auto_war_2()
{ // 005B57C6
	__asm
	{
		push	0
		push	[ebp+8]
		push	[ebp+0xc]
		call	auto_war_cpp
		mov	ecx, [ebp+8]
		mov	edx, [ecx+0x14]
		push	edx
		mov	edx, 0x05B57CD
		jmp	edx
	}
}

void __stdcall log_dip_change_cpp(byte *player_from, byte *player_to, int was, int became)
{
	char names[][7] = {"war", "ally", "ignore", "vision"};
	int flags[4] = {1, 2, 4, 0x10};
	for (int i = 0; i < 4; ++i)
	{
		if ((was & flags[i]) && !(became & flags[i]))
			Printf("Player %s unset %s to player %s", get_player_name(player_from), names[i], get_player_name(player_to));
		if (!(was & flags[i]) && (became & flags[i]))
			Printf("Player %s set %s to player %s", get_player_name(player_from), names[i], get_player_name(player_to));
	}
}

void __declspec(naked) log_dip_change()
{ // 005081DC
	__asm
	{
		// magic here
		mov		ecx, [ebp-0x9E8]
		mov		ecx, dword ptr [ecx+0x14]
		and		ecx, GMF_PLAYERS_VISION
		cmp		ecx, GMF_PLAYERS_VISION
		jnz		do_no_vision

		mov		eax, [ebp-0xA0C]
		mov		ecx, [ebp-0x9DC]
		movzx	ebx, word ptr [ecx+eax*2]
		or		ebx, 0x10
		mov 	word ptr [ecx+eax*2], bx

do_no_vision:
		mov		ecx, [ebp-0x9E8]
		mov		ecx, dword ptr [ecx+0x14]
		and		ecx, GMF_PLAYERS_ALLY
		cmp		ecx, GMF_PLAYERS_ALLY
		jnz		do_normal

		mov		eax, [ebp-0xA0C]
		mov		ecx, [ebp-0x9DC]
		movzx	ebx, word ptr [ecx+eax*2]
		or		ebx, 0x02
		mov 	word ptr [ecx+eax*2], bx

do_normal:
		push	esi
		push	edx
		push	[ebp - 0x9E8]
		push	[ebp - 0x9E4]
		call	log_dip_change_cpp
		mov	edx, [ebp - 0xA08]
		mov	ecx, 0x005081E2
		jmp	ecx
	}
}

char * _stdcall chat_log_cpp(char *msg, char *player_from, char *player_to, unsigned int type)
{
	static char m[5000];
	if (type == 1)
	{
		_snprintf(m, sizeof(m)-1, "--> %s: %s", player_from, msg);
	}
	else if (type == 2)
	{
		_snprintf(m, sizeof(m)-1, "%s >>> %s: %s", player_from, player_to, msg);
	}
	else
	{
		_snprintf(m, sizeof(m)-1, "%s: %s", player_from, msg);
	}
	return m;
}

void _declspec(naked) chat_log()
{ // 0050721C
	__asm
	{
		mov	edx, [ebp-0x528]
		mov	eax, [edx+0x0a]
		sar	eax, 8
		and	eax, 0xFF
		push	eax

		cmp	eax, 2
		jnz	skip_player_to
		mov	edx, [ebp-0x528]
		mov	eax, [edx+0x0a]
		and	eax, 0xFF
		push	eax
		mov	ecx, 0x6CDB24
		mov	ecx, [ecx]
		mov	edx, 0x535B50
		call	edx
		test	eax, eax
		jz	skip_player_to
		mov	eax, [eax+0x18]
		push	eax
		jmp	join_player_to
skip_player_to:
		xor	eax, eax
		push	eax
		jmp	join_player_to
join_player_to:

		mov	ecx, [ebp-0x104]
		mov	eax, [ecx+0x18]
		push	eax

		mov	eax, [ebp-0x108]
		push	eax

		call	chat_log_cpp
		push	eax
		mov	edx, 0x0043A857
		call	edx
		add	esp, 4
		mov	edx, 0x00507267 
		jmp	edx
	}
}

void _declspec(naked) player_respawned_init()
{
	__asm
	{
		mov		eax, [ebp-0x34]
		mov		dword ptr [eax+0x130], 0
		mov		eax, [eax + 0x14]
		test	eax, eax
		jz		pri_skip
		mov		eax, [eax + 0x18]
		test	eax, eax
		jz		pri_skip
		push	eax
		push	offset player_respawn
		call	Printf
pri_skip:
		mov	edx, 0x531197
		jmp	edx
	}
}

unsigned int _stdcall respawn_update_exp_cpp(unsigned int exp)
{
	return static_cast<unsigned int>(exp * Config::RespawnExpMult2);
	//		return static_cast<unsigned int>(exp * (exp_full / exp_death));
}

void _declspec(naked) player_respawned()
{ // 00531278
	__asm
	{
		push	eax
		call	respawn_update_exp_cpp
		mov	edx, 0x0531283
		jmp	edx
	}
}

unsigned int _stdcall death_update_exp_cpp(unsigned int exp)
{
	return static_cast<unsigned int>(exp * Config::DeathExpMult);
	// return static_cast<unsigned int>(exp * exp_death);
}

unsigned int _stdcall _double_to_int32(double val)
{
	return static_cast<unsigned int>(val);
}

void _declspec(naked) death_update_exp(void *unit)
{		//st(0) = exp_Multiplier
	__asm
	{
		push	ebp
		mov	ebp, esp
		sub	esp, 4

		mov	edx, dword ptr [ebp+8] // unit
		mov	dword ptr [edx+0x130], 0
		mov	dword ptr [ebp-4], 1
due_cycle:	cmp	dword ptr [ebp-4], 6
		jge	due_cycle_ex
		mov	edx, dword ptr [ebp+8] // unit
		mov	ecx, dword ptr [ebp-4] // i
		mov	eax, [edx + ecx*4 + 0x23C]

		push	eax
		push	eax
		fild	dword ptr [esp]
		fmul	st,st(1)
		fstp	qword ptr [esp]
		call	_double_to_int32
		//push	eax
		//call	death_update_exp_cpp

		mov	edx, dword ptr [ebp+8] // unit
		mov	ecx, dword ptr [ebp-4] // i
		mov	[edx + ecx*4 + 0x23C], eax
		add	[edx + 0x130], eax

		inc	dword ptr [ebp-4]
		jmp	due_cycle
due_cycle_ex:	
		
		mov	esp, ebp
		pop	ebp
		ret	4
	}
}

void _declspec(naked) player_died()
{ // 00556A56
	__asm
	{
		fld	Config::DeathExpMult
		push	[ebp - 0x18]
		call	death_update_exp
		fstp	st
		mov	edx, 0x556A5D
		jmp	edx
	}
}

void _declspec(naked) player_killed()
{ // 00557005
	__asm
	{
		fld	Config::KilledExpMult
		push	[ebp - 0x18]
		call	death_update_exp
		fstp	st
		mov	edx, 0x55700C
		jmp	edx
	}
}

void _declspec(naked) player_pkilled()
{ // 00556BE6
	__asm
	{
		fld	Config::PKExpMult
		push	[ebp - 0x18]
		call	death_update_exp
		fstp	st
		mov	edx, 0x556BED
		jmp	edx
	}
}

///////////////////////////////////////////////
// extend checking max player parameters

void _declspec(naked) set_max_player_parameters()
{
	__asm
	{
		pop edx
		mov al,[ecx]MainCharacterParameters.Spirit
		mov ss:[ebp+edx],al
		pop edx
		mov al,[ecx]MainCharacterParameters.Mind
		mov ss:[ebp+edx],al
		pop edx
		mov al,[ecx]MainCharacterParameters.Reaction
		mov ss:[ebp+edx],al
		pop edx
		mov al,[ecx]MainCharacterParameters.Body
		mov ss:[ebp+edx],al
		retn
	}
}

// using potions

void _declspec(naked) set_max_player_parameters_for_use_potion()
{
	__asm
	{
		push 0x005316C8
		push -0x20
		push -0x0C
		push -0x24
		push -0x08
		jmp set_max_player_parameters
	}
}

void _declspec(naked) set_warrior_male_max_parameters()
{ // 00531650
	__asm
	{
		lea ecx,Config::WarriorMaleMaxParameters
		jmp set_max_player_parameters_for_use_potion
	}
}

void _declspec(naked) set_warrior_female_max_parameters()
{ // 53166C
	__asm
	{
		lea ecx,Config::WarriorFemaleMaxParameters
		jmp set_max_player_parameters_for_use_potion
	}
}

void _declspec(naked) set_mage_male_max_parameters()
{ // 531696
	__asm
	{
		lea ecx,Config::MageMaleMaxParameters
		jmp set_max_player_parameters_for_use_potion
	}
}

void _declspec(naked) set_mage_female_max_parameters()
{ // 5316B8
	__asm
	{
		lea ecx,Config::MageFemaleMaxParameters
		jmp set_max_player_parameters_for_use_potion
	}
}

// taking potions

void _declspec(naked) set_max_player_parameters_for_take_potion()
{
	__asm
	{
		push 0x00566206
		push -0x110
		push -0x104
		push -0x114
		push -0x100
		jmp set_max_player_parameters
	}
}

void _declspec(naked) set_warrior_male_max_parameters_taking_potion()
{ // 566149
	__asm
	{
		lea ecx,Config::WarriorMaleMaxParameters
		jmp set_max_player_parameters_for_take_potion
	}
}

void _declspec(naked) set_warrior_female_max_parameters_taking_potion()
{ // 56617A
	__asm
	{
		lea ecx,Config::WarriorFemaleMaxParameters
		jmp set_max_player_parameters_for_take_potion
	}
}

void _declspec(naked) set_mage_male_max_parameters_taking_potion()
{ // 5661B6
	__asm
	{
		lea ecx,Config::MageMaleMaxParameters
		jmp set_max_player_parameters_for_take_potion
	}
}

void _declspec(naked) set_mage_female_max_parameters_taking_potion()
{ // 5661EA
	__asm
	{
		lea ecx,Config::MageFemaleMaxParameters
		jmp set_max_player_parameters_for_take_potion
	}
}

///////////////////////////////////////////////
// extend inns

void _declspec(naked) bound_quest_reward()
{ // 565406
	__asm
	{
		mov		eax, [ebp-0x18]
		cmp		eax, Config::MinQuestReward
		jge		not_too_low
		mov		eax, Config::MinQuestReward
not_too_low:
		cmp		eax, Config::MaxQuestReward
		jle		not_too_high
		mov		eax, Config::MaxQuestReward
not_too_high:
		mov		[ebp-0x18], eax
		mov		edx, 0x00565426
        jmp		edx
	}
}

///////////////////////////////////////////////

void _declspec(naked) heal_enemies()
{ // 0053A5CB
	__asm
	{ // skip a check
		mov	edx, 0x0053A5D5
		jmp	edx
	}
}

void _declspec(naked) add_health_potions()
{ // 0054D700
	__asm
	{ // пропустить добавление зельев
		mov	edx, 0x54D8AC
		jmp	edx
	}
}

void _declspec(naked) skip_updating_character_log()
{
	__asm
	{
		mov	edx, 0x4F119B
		jmp	edx
	}
}

void _declspec(naked) skip_returning_character_log()
{
	__asm
	{
		mov	edx, 0x4F1414
		jmp	edx
	}
}

unsigned int get_spirit_level(unsigned int power)
{
	if (spirit_factor == 0) return 5;
	unsigned int level = power / spirit_factor + 1;
	if (level > 5) level = 5;
	return level;
}

char *__stdcall get_zombie(unsigned int power)
{
	unsigned int level = get_spirit_level(power);
	if (level == 1) return "F_Zombie.1";
	_snprintf(spirit, MAX_PATH-1, "%c_Zombie.%d", "AF"[rand()%2], level);
	return spirit;
}

char *__stdcall get_skeleton(unsigned int power)
{
	_snprintf(spirit, MAX_PATH-1, "%c_Skeleton.%d", "AF"[rand()%2], get_spirit_level(power));
	return spirit;
}

char *__stdcall get_ghost(unsigned int power)
{
	unsigned int level = get_spirit_level(power);
	if (level == 1) return "Ghost";
	_snprintf(spirit, MAX_PATH-1, "Ghost.%d", level);
	return spirit;
}

void __declspec(naked) zombie_gen()
{ // 53B6D3
	__asm
	{
		mov	edx, [ebp-0x40]
		push	edx
		call	get_zombie
		push	eax
		lea	ecx, [ebp - 0x0EC]
		mov	edx, 0x53B6DE
		jmp	edx
	}
}

void __declspec(naked) skeleton_gen()
{ // 53B7EB
	__asm
	{
		mov	edx, [ebp-0x40]
		push	edx
		call	get_skeleton
		push	eax
		lea	ecx, [ebp - 0x0F0]
		mov	edx, 0x53B7F6
		jmp	edx
	}
}

void __declspec(naked) ghost_gen()
{ // 53B910
	__asm
	{
		mov	edx, [ebp-0x40]
		push	edx
		call	get_ghost
		push	eax
		lea	ecx, [ebp - 0x0F4]
		mov	edx, 0x53B91B
		jmp	edx
	}
}

void __declspec(naked) redir_log()
{
	__asm
	{
		push	dword ptr [ebp+8]
		push	offset perc_s
		call	log_format
		mov	edx, 0x401D90
		call	edx
		mov	[ebp-0x14],eax
		mov	edx, 0x43A87A
		jmp	edx
	}
}

//char mtap_test[] = "unit %.4X, flags: %.2X, sig: %.2X\n";
//char ucss_test[] = "unit %.4X, flags: %.2X, change\n";

char eghp_test[] = "unit %.4X, eff: %d, bit: %d, ret: %.6X, ret2: %.6X\n";
char ep1t[] = "effect packet 1\n";
char ep2t[] = "effect packet 2, id=%.2X\n";
char ep3t[] = "effect packet 3\n";
char ep4t[] = "effect packet 4\n";

void __declspec(naked) effect_packet_1()
{
	__asm // 0051BAC3
	{
		mov	eax, [ebp-4]
		mov	byte ptr [eax+9], 0x86
		mov	ecx, [ebp-4]
		mov	edx, 0x51baca
		jmp	edx
	}
}

void __declspec(naked) effect_packet_2()
{
	__asm // 0051BBA7
	{
		mov	eax, [ebp+0x0c]
		xor	ecx, ecx
		mov	cl, [eax+8]
		cmp	cl, 0x02
		jz	ep2_send_coords
		cmp	cl, 0x03
		jz	ep2_send_coords
		cmp	cl, 0x11
		jz	ep2_send_coords
		cmp	cl, 0x17
		jz	ep2_send_coords
		cmp	cl, 0x16
		jz	ep2_send_coords
		cmp	cl, 0x07
		jz	ep2_send_coords
		cmp	cl, 0x06
		jz	ep2_send_coords
		jmp	ep2_skip_coords
ep2_send_coords:
		mov	edx, 0xFFB
		xor	ecx, ecx
		push	ecx
		push	ecx
		push	edx
		mov	eax, 0x10000010
		push	eax
		push	ecx
		mov	eax, [ebp + 8]
		push	eax
		mov	ecx, 0x6C3A08
		mov	edx, 0x519221
		call	edx
		
ep2_skip_coords:
		jmp	ep2_skip

		mov	eax, [ebp+0x0c]
		xor	ecx, ecx
		mov	cl, [eax+8]
		push	ecx

		push	offset ep2t
		call	log_format
ep2_skip:
		mov	eax, [ebp-4]
		mov	byte ptr [eax+9], 0x86
		mov	ecx, [ebp-4]
		mov	edx, 0x51bbae
		jmp	edx
	}
}

void __declspec(naked) effect_packet_3()
{
	__asm // 0051BE21
	{
		mov	eax, [ebp-4]
		mov	byte ptr [eax+9], 0x86
		mov	ecx, [ebp+8]
		mov	edx, 0x51be28
		jmp	edx
	}
}

void __declspec(naked) effect_packet_4()
{
	__asm // 0051BEB0
	{
		mov	edx, [ebp-0x18]
		mov	byte ptr [edx+9], 0x86
		mov	eax, [ebp+8]
		mov	edx, 0x51beb7
		jmp	edx
	}
}

char eg1_t[] = "invisibility gone 1\n";
char eg2_t[] = "invisibility gone 2\n";

/*
 * effect packet 2
 * invisibility gone 1
 * invisible has gone packet sent, 5564F6 52A921 53EF9A 51BD9E
 */

void __declspec(naked) effect_gone_1()
{
	__asm
	{
		mov	eax, [ebp-4]
		xor	ecx, ecx
		mov	cx, [eax+0x0c]
		mov	edx, 0x53ef5e
		jmp	edx
	}
}

void __declspec(naked) effect_gone_2()
{
	__asm
	{
		mov	eax, [ebp-0x14]
		xor	ecx, ecx
		mov	cx, [eax+0x0c]
		mov	edx, 0x53F121
		jmp	edx
	}
}

/*
 * [5.12.2010 0:20:29.811] unit 0001, eff: 32, ret: 53F717, ret2: 537624  hang
 * [5.12.2010 0:20:35.062] unit 0001, eff: 32, ret: 51BD9E, ret2: 53EF9A  gone
 */

char ehgp_t[] = "invisible has gone packet sent, %.6X %.6X %.6X %.6X\n";

void __declspec(naked) effect_hang_gone_packet()
{
	__asm
	{
		mov	eax, [ebp+0x10]
		cmp	al, 0x89
		jnz	ehgp_pass  // not gone

		mov	eax, [ebp+8]
		xor	edx, edx
		mov	dx, [eax+0x0C]
		cmp	edx, 12
		jnz	ehgp_pass

		mov	edx, 0xFFB
		xor	ecx, ecx
		push	ecx
		push	ecx
		push	edx
		mov	eax, 0x10
		push	eax
		push	ecx
		mov	eax, [ebp + 0x0c]
		push	eax
		mov	ecx, 0x6C3A08
		mov	edx, 0x519221
		call	edx

		jmp	ehgp_pass
ehgp_pass:
		mov	dword ptr [ebp-4], 0x6CDB28
		mov	edx, 0x51BDB4
		jmp	edx
	}
}

void __declspec(naked) move_turn_attack_packet()
{
	__asm
	{
		push	ebp
		mov	ebp, esp
		sub	esp, 8
		mov	[ebp - 8], ecx
		mov	dword ptr [ebp - 4], 0x70A7D0

		mov	eax, [ebp + 0x18] // destination player
		test	eax, eax
		jz	mtap_skip
		movsx	edx, word ptr [eax + 4]
		cmp	edx, 0x10
		jl	mtap_skip
		cmp	edx, 0x20
		jge	mtap_skip

//		mov	eax, dword ptr [mode]
//		test	eax, TEST_FLAG
//		jz	mtap_pass
		mov	eax, [ebp + 0x14]
		cmp	al, 0x71
		jz	mtap_pass

		mov	eax, [ebp + 8] // unit
		mov	ecx, [eax + 0x144] // effects flags
		and	ecx, 0x1000 // invisibility
		test	ecx, ecx
		jz	mtap_pass // visible

//		jmp	mtap_skip

		mov	ecx, 0x6A8B8C
		mov	edx, [ecx]

		mov	eax, [ebp + 8] // changed unit
		mov	eax, [eax + 0x14] // player of changed unit
		movsx	ecx, word ptr [eax + 4] // id
		imul	ecx, 0x46
		mov	eax, [ebp + 0x18] // destination player
		movsx	eax, word ptr [eax + 4] // id
		add	ecx, eax
		add	ecx, 0xA8C4
		add	ecx, edx
		test	byte ptr [ecx], 0x10 // vision
		jz	mtap_skip
		jmp	mtap_pass

mtap_pass:
		mov	eax, [ebp + 0x14]
		and	eax, 0xFF
		test	eax, eax
		jnz	loc_51B9C3
		mov	ecx, [ebp - 4]
		mov	byte ptr [ecx + 9], 0x6B
		jmp	loc_51B9CC
loc_51B9C3:
		mov	edx, [ebp - 4]
		mov	al, byte ptr [ebp + 0x14]
		mov	[edx + 9], al
loc_51B9CC:
		mov	ecx, [ebp - 4]
		mov	dl, [ebp + 0x10]
		mov	[ecx+0x0D], dl
		mov	dl, [ebp + 0x0C]
		mov	[ecx+0x0C], dl
		mov	ecx, [ebp+8]
		mov	edx, 0x52BABD
		call	edx
		mov	edx, [ebp+0x10]
		and	edx, 0xFF
		mov	eax, 0x642C2C
		mov	eax, [eax]
		mov	ecx, [eax+4]
		add	ecx, edx
		mov	edx, [ebp + 8]
		mov	[edx+0x138], ecx
		mov	eax, [ebp - 4]
		mov	ecx, [ebp + 8]
		mov	dx, [ecx + 4]
		mov	[eax+0x0A], dx
		mov	edx, [ebp + 0x18]
		movsx	edx, word ptr [edx + 4]
		mov	word ptr [eax+7], dx // broadcast
		mov	ecx, [ebp + 8]
		push	ecx
		push	eax // packet
		mov	ecx, [ebp - 8]
		mov	edx, 0x51B0F0
		call	edx
mtap_skip:
		mov	esp, ebp
		pop	ebp
		retn	0x14
	}
}

void __declspec(naked) broadcast_move_turn_attack_packet()
{ // 51B99E
	__asm
	{
		push	ebp
		mov	ebp, esp
		sub	esp, 0x0C
		mov	[ebp - 0x0C], ecx
		lea	ecx, [ebp-0x08]
		mov	edx, 0x496DC0
		call	edx
		mov	ecx, 0x06CDB24
		mov	ecx, [ecx]
		push	ecx
		lea	ecx, [ebp-0x08]
		mov	edx, 0x496DD0
		call	edx
l_50303D:	
		test	eax, eax
		jz	l_503051

		push	eax
		mov	eax, [ebp+0x14]
		push	eax
		mov	eax, [ebp+0x10]
		push	eax
		mov	eax, [ebp+0x0c]
		push	eax
		mov	eax, [ebp+8]
		push	eax
		mov	ecx, [ebp-0x0c]
		call	move_turn_attack_packet

		lea	ecx, [ebp-0x08]
		mov	edx, 0x496E20
		call	edx
		jmp	l_50303D
// ---------------------------------------------------------------------------

l_503051:				 // CODE XREF: sub_50301F+20j
		mov	esp, ebp
		pop	ebp
		ret	0x10
	}
}

// 1) TODO: где-то пролезают координаты
// +2) TODO: при визибле - форсировать отправку поординат и угла посылать
// 3) посылать 6B, 6D как?
// 1c0 : 71 (.text:0051C5E1) , ...

void __declspec(naked) unit_change_single_send()
{ // 00519346
	__asm
	{
		mov	eax, [ebp + 8] // unit
		mov	ecx, [eax + 0x144] // effects flags
		and	ecx, 0x1000
		test	ecx, ecx
		jz	ucss_send // visible

//		jmp	ucss_skip_send

		mov	ecx, 0x6A8B8C
		mov	edx, [ecx]

		mov	eax, [ebp + 8] // changed unit
		mov	eax, [eax + 0x14] // player of changed unit
		movsx	ecx, word ptr [eax + 4] // id
		imul	ecx, 0x46
		mov	eax, [ebp + 0x0C] // destination player
		movsx	eax, word ptr [eax + 4] // id
		add	ecx, eax
		add	ecx, 0xA8C4
		add	ecx, edx
		test	byte ptr [ecx], 0x10 // vision
		jz	ucss_skip_send
		jmp	ucss_send

ucss_skip_send:
		mov	eax, [ebp + 0x10]
		cmp	eax, 0x10000010
		jz	ucss_send
		and	eax, 0xFFFFFFEF
		mov	[ebp + 0x10], eax
ucss_send:
		mov	eax, [ebp + 8]
		xor	ecx, ecx
		mov	cx, [eax + 0x1A4]
		mov	edx, 0x519352
		jmp	edx
//ucss_skip_send_2:
		mov	edx, 0x51A0BB
		jmp	edx
	}
}

char ucss2_t[] = "warn, sending coords to invisible char\n";

void __declspec(naked) unit_change_single_send_2()
{
	__asm
	{
		mov	ecx, [ebp+8]
		mov	ecx, [ecx+0x1C0]
		mov	edx, 0x5196FD
		jmp	edx
	}
}

void __declspec(naked) pkill_msg()
{
	__asm { // from 00556C0F
		mov	eax, [ebp-0x18]
		mov	ecx, [eax+0x40]
		mov	edx, [ecx+0x14]
		mov	eax, [edx+0x18]
		push	eax
		mov	edx, 0x00556C1C
		jmp	edx
	}
}

void __declspec(naked) print_packet_info()
{
	__asm
	{
		mov		eax, [Config::LogMode]
		test	eax, SVL_DIPLOMACY
		jz	ppi_skip_log
		mov	eax, [ebp+8]
		mov	edi, offset info
		mov	byte ptr [edi+4], 0
		mov	edx, offset numb
		xor	ecx, ecx
		mov	cl, [eax+5]
		shr	cl, 4
		mov	cl, [edx+ecx]
		mov	[edi], cl
		mov	cl, [eax+5]
		and	cl, 0x0F
		mov	cl, [edx+ecx]
		mov	[edi+1], cl
		mov	cl, [eax+9]
		shr	cl, 4
		mov	cl, [edx+ecx]
		mov	[edi+2], cl
		mov	cl, [eax+9]
		and	cl, 0x0F
		mov	cl, [edx+ecx]
		mov	[edi+3], cl
		push	edi
		mov	edx, 0x043A857
		call	edx
		add	esp, 4
ppi_skip_log:
		mov	edx, [ebp+8]
		xor	eax, eax
		mov	al, [edx+9]
		mov	edx, 0x050587A
		jmp	edx
	}
}

int __stdcall check_sig_c_s(unsigned char c)
{
//	static int counter = 30;
	unsigned char arr[] = {0x02, 0x04, 0x05, 0x07, 0x08, 0x09, 0x12, 0x13, 0x14, 0x16, 
		0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x21, 0x22, 0x23, 
		0x24, 0x25, 0x26, 0x32, 0x33, 0x34, 0x35, 0x36, 0x38, 0x39, 0x3A, 0x3B, 
		0x3E, 0x3F, 0x45, 0x46, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x91, 0xAE, 
		0xBE, 0xC1, 0xD0, 0x64};
//	if (!--counter) return 0;
	for (size_t i = 0; i < sizeof(arr); ++i)
	{
		if (c == arr[i]) return 1;
	}
	return 0;
}

void __declspec(naked) add_player_to_map()
{
	__asm
	{
		push	ebp
		mov	ebp, esp
		sub	esp, 0x0C
		mov	eax, [ecx + 0x4C] // unit
		test	eax, 8
		jz	aptm_already_on_map
		mov	edx, 0x052C40F
		jmp	edx
aptm_already_on_map:
		mov	edx, 0x052C47B
		jmp	edx
	}
}


void _declspec(naked) packet_recvd()
{ // jmp 00504ABC
	__asm {
		mov	ecx, dword ptr[ebp+8]
		xor	eax, eax
		mov	al, byte ptr [ecx+9]
		push	eax
		call	check_sig_c_s
		test	eax,eax
		jnz	pr_ex
		// –“…Ё’ё…Ќ «ЅƒЅ

		mov	ecx, [ebp+8]
		xor	eax,eax
		mov	ax, [ecx+5]
		push	eax
		mov	ecx, 0x6CDB24
		mov	ecx, [ecx]
		mov	edx, 0x535B50
		call	edx
		test	eax, eax
		jz	pr_ex // skip for now
		mov	[ebp-0xB0C], eax // player
		mov	ecx, [eax + 0xA78]
		push	ecx
		mov	edx, [ebp+8]
		xor	ecx, ecx
		mov	cl, [edx+9]
		push	ecx
		push	offset log_invalid_packet
		call	log_format

		// disconnect him
		mov	eax, [ebp-0xB0C]
		xor	ecx, ecx
		mov	cx, [eax+4]
		push	ecx
		mov	ecx, 0x6C3A08
		mov	edx, 0x518544
		call	edx
		test	eax, eax
		jz	pr_sk
		mov	eax, [eax+29Ch]
		test	eax, eax
		jnz	pr_sk
		mov	ecx, [ebp-0xB0C]
		push	ecx
		mov	ecx, 0x6C3A08
		mov	edx, 0x51D49B
		call	edx
pr_sk:
		mov	edx, 0x50864E
		jmp	edx

pr_ex:
// color crash 
		mov	ecx, dword ptr[ebp+8]
		xor	eax, eax
		mov	al, byte ptr [ecx+9]
		cmp	eax, 2
		jnz	pr_ex2
		mov	al, byte ptr [ecx + 0x13]
		cmp	eax, 0x11
//changed by Las. was cmp eax, 0x12
		jb	pr_ex2
		cmp	eax, 0xE1
		jz	pr_ex2
		mov	byte ptr [ecx + 0x13], 0
// log him
		mov	ecx, [ebp+8]
		xor	eax,eax
		mov	ax, [ecx+5]
		push	eax
		mov	ecx, 0x6CDB24
		mov	ecx, [ecx]
		mov	edx, 0x535B50
		call	edx
		test	eax, eax
		jz	pr_ex2 
// got CPlayer
//		mov	ecx, [eax + 0xA78]
 // player :: login
		mov	ecx, [eax + 0x18] // player :: nickname
		push	ecx
		push	offset log_color_crasher
		call	log_format

pr_ex2:
		mov	dword ptr [ebp - 0xB0C], 0
		mov	edx, 0x504AC6
		jmp	edx
	}
}

void _stdcall msgbox_am(HWND hwnd, char *text, char *caption, u_int uType) 
{
	log_format("MsgBox: %s, %s\n", caption, text);
}

void _declspec(naked) chat_crash_fix() // 50716D
{
	__asm {
		
		mov	ecx, [ebp-0x528]
		add	ecx, 0x0F
ccf_loop:	add	ecx, 1
		cmp	byte ptr [ecx], 0
		jz	ccf_end
		cmp	byte ptr [ecx], 0x20
		jae	ccf_loop
		mov	byte ptr [ecx], 0x20
		jmp	ccf_loop
ccf_end:	
		mov	ecx, [ebp-0x528]
		mov	edx, 0x507173
		jmp	edx		
	}
}

void _stdcall transform_map_name(char *map_name)
{
	char module_fn[0x100];
	GetModuleFileName(0, module_fn, 0xFF);
	std::string s(module_fn);

	size_t k = s.rfind('/');
	if (k == s.npos || k < s.rfind('\\'))
		k = s.rfind('\\');
	if (k == s.npos)
	{
		throw std::logic_error("shit happens");
	}

	s.erase(k + 1);
	s += map_name;

	strcpy(map_name_buf, s.c_str());
}

void _declspec(naked) loading_map_bug()
{
	__asm
	{
		call	transform_map_name
		lea	eax, map_name_buf
		push	eax
		lea	ecx, [ebp-0x20]
		mov	edx, 0x005DF192 // CFile::Open
		call	edx
		mov	edx, 0x004F1D52
		jmp	edx
	}
}

void _declspec(naked) adm_dip(void) {
	__asm {//004FFD77
		mov	edx, [ebp+0x08] // player
		mov	eax, [edx+14h]
		shr	eax, 18h
		cmp	al, 3Fh
		jnz	a_d_c ///не админ, продолжаем
		mov	eax, [edx+14h]
		test	eax, 0x4000 /// monsters_alliance
		jz	a_d_c
		
		mov	edx, [ebp+0x08] // player
		movsx	edx, word ptr [edx + 4]
		imul	edx, 46h
		add	edx, dword ptr [ebp-0x58]
		add	edx, 0xA8C4
		mov	ecx, 0x6A8B8C
		add	edx, dword ptr [ecx]
		mov	byte ptr [edx], 0x2

		mov	edx, dword ptr [ebp-0x58]
		imul	edx, 46h
		mov	eax, [ebp+0x08] // player
		movsx	eax, word ptr [eax + 4]
		add	edx, eax
		add	edx, 0xA8C4
		mov	ecx, 0x6A8B8C
		add	edx, dword ptr [ecx]
		mov	byte ptr [edx], 0x2

		mov	edx, 0x4FFD68
		jmp	edx

a_d_c:
		mov	edx, [ebp-0x058]
		imul	edx, 46h
		mov	eax, 0x004FFD7D
		jmp	eax
	}
}

void _declspec(naked) adm_dip_2(void) {
	__asm {//004FFEA6
			mov edx, dword ptr [ebp+0x08] // player
		  mov ecx, dword ptr [ebp-0x54]
		//  movsx edx, word ptr [edx + 4]
		//  movsx ecx, word ptr [ecx + 4]
		  cmp ecx, edx
		  jz a_d_2_n_a_2 /// себе - вид
	/*__asm {//004FFEA6
		mov	edx, [ebp+0x08] // player
		cmp	edx, dword ptr [ebp-54]
		jz	a_d_2_n_a_2 /// себе - вид*/
		mov	eax, [edx+14h]
		shr	eax, 18h
		cmp	al, 3Fh
		jnz	a_d_2_n_a /// не админ
		mov	eax, [edx+14h]
		test	eax, 0x10000 /// players_alliance
		jz	a_d_2_n_a
		jmp	adm_dip_2_set_dip

a_d_2_n_a:	
		mov	edx, dword ptr [ebp-0x54]
		mov	eax, [edx+14h]
		shr	eax, 18h
		cmp	al, 3Fh
		jnz	a_d_2_n_a_2 /// не админ
		mov	eax, [edx+14h]
		test	eax, 0x10000 /// players_alliance
		jz	a_d_2_n_a_2

adm_dip_2_set_dip:

		mov	edx, [ebp+0x08] // player
		movsx	edx, word ptr [edx + 4]
		imul	edx, 46h
		mov	eax, dword ptr [ebp-0x54]
		movsx	eax, word ptr [eax + 4]
		add	edx, eax
		add	edx, 0xA8C4
		mov	ecx, 0x6A8B8C
		add	edx, dword ptr [ecx]
		mov	byte ptr [edx], 0x2

		mov	edx, dword ptr [ebp-0x54]
		movsx	edx, word ptr [edx + 4]
		imul	edx, 46h
		mov	eax, [ebp+0x08] // player
		movsx	eax, word ptr [eax + 4]
		add	edx, eax
		add	edx, 0xA8C4
		mov	ecx, 0x6A8B8C
		add	edx, dword ptr [ecx]
		mov	byte ptr [edx], 0x2

		mov	edx, 0x4FFF09
		jmp	edx

a_d_2_n_a_2:	

		mov	edx, [ebp+0x08]
		movsx	eax, word ptr [edx+4]
		mov	edx, 0x4FFEAD
		jmp	edx
	}
}

void _declspec(naked) adm_invis(void) {
	__asm { //0051C901
		mov	eax, [ebp-0x04]
		mov	ecx, [eax+0x14]
		shr	ecx, 0x18
		cmp	cl, 0x3F
		jnz	a_i_send
		mov	ecx, [eax+0x14]
		test	ecx, 0x800
		jz	a_i_send
		mov	edx, 0x51C9B5
		jmp	edx
a_i_send:
		mov	eax, [ebp-0x04]
		mov	ecx, [eax+18h]
		mov	[ebp-0x0C], ecx
		mov	edx, 0x0051C90A
		jmp	edx
	}
}

void __stdcall change_dip2(u_char *data, char *pl_name) 
{
//	unsigned int n = *(unsigned int *)(data + 0x0A) * 2;
//	log_format("dip_ch(): %s, ", pl_name);
//	for (size_t i = 0; i < n; ++i) 
//	{
//		log_format2("%.2X ", *(data + 0x0E + i));
//	}
//	log_format2("\n");
}


void _declspec(naked) change_dip(void)
{///00508479
	__asm
	{
		movzx	ax, dword ptr [ebp+0x08]
		push	eax
		mov		ecx, 0x006CDB24
		mov		ecx, [ecx]
		mov		edx, 0x00535B50
		call	edx

		cmp		eax, 0
		jz		do_normal

		push	1
		push	eax
		call	ExtDiplomacy

do_normal:
		mov		[ebp-0x04], 0xFFFFFFFF
		lea		ecx, [ebp-0x9FC]
		mov		edx, 0x005DB831
		call	edx
		mov		edx, 0x0050848B
		jmp		edx
	}
}

void _declspec(naked) left_shop(void) { ///005065E5
	__asm {
		mov		eax, [Config::LogMode]
		test	eax, SVL_BUILDINGS
		jz	l_s_sk

		mov	eax, [ebp-0x0D0]
		mov	eax, [eax + 0x14]
//		mov	eax, [eax + 0x0A78]
 // player :: login
		mov	eax, [eax + 0x18]
		push	eax

		push	offset left_shop1
		call	Printf
l_s_sk:	
		mov	ecx, [ebp-0x0D0]
		push	ecx
		mov	ecx, [ebp-0x0D4]
		mov	edx, 0x544777
		call	edx
		mov	edx, [ebp-0x0D0]
		push	edx
		mov	ecx, [ebp-0x0D4]
		mov	edx, 0x544685
		call	edx
		mov	edx, 0x50864E
		jmp	edx
	}
}

void _declspec(naked) left_inn(void) { ///005063B6
	__asm {
		mov		eax, [Config::LogMode]
		test	eax, SVL_BUILDINGS
		jz	l_i_sk
		mov	eax, [ebp-0x0B0]
		mov	eax, [eax + 0x14]
//		mov	eax, [eax + 0x0A78]
 // player :: login
		mov	eax, [eax + 0x18]
		push	eax

		push	offset left_inn1
		call	Printf
l_i_sk:	
		mov	edx, [ebp+0x08]
		mov	eax, [edx+0Eh]
		push	eax
		mov	ecx, [ebp-0x0B0]
		push	ecx
		mov	ecx, [ebp-0x0B4]
		mov	edx, 0x560DC2
		call	edx
		mov	edx, 0x50864E
		jmp	edx
	}
}


void _declspec(naked) cancel_camp(void) {
	__asm { /// CDECL, 100%TESTED
		push	ebp
		mov	ebp, esp
		push	ecx
		mov	[ebp-0x04], ecx
		mov	eax, [ebp-0x04]
		cmp	dword ptr [eax+0A90h], 0
		jbe	loc_534B98
		mov	ecx, [ebp-0x04]
		mov	dword ptr [ecx+0A90h], 0
		mov	edx, [ebp-0x04]
		mov	dword ptr [edx+0A94h], 0
		mov	eax, [ebp-0x04]
		push	eax
		push	0
		push	0Fh
		mov	ecx, 0x6C3A08
		mov	edx, 0x51CE86
		call	edx

		mov		eax, [Config::LogMode]
		test	eax, SVL_CAMPING
		jz	loc_534B98

		mov	eax, [ebp-0x04]
		mov	eax, [eax+0x18] // player :: nick
		push	eax
		push	offset cancel_camp1
		call	Printf

loc_534B98:				 // CODE XREF: cancel_camp+11j
		mov	esp, ebp
		pop	ebp
		retn
	}
}


void _declspec(naked) enter_inn (void) {
	__asm { /// 005062FB
				 // CODE XREF: sub_504A96+185Ej
		mov	ecx, [ebp-0x0A8]
//		mov	edx, [ecx+38h]
		cmp	word ptr [ecx+94h], 0
		jle	e_i_skip //// если не жив

		mov	ecx, [ebp-0x0A8]
		mov	edx, 0x52C813
		call	edx
		mov	eax, [ebp-0x0A8]
		mov	ecx, [eax+10h]
		mov	edx, 0x58AAF0
		call	edx
		xor	ebx, ebx
		mov	bl, al
		and	ebx, 0FFh
		mov	ecx, [ebp-0x0A8]
		mov	ecx, [ecx+10h]
		mov	edx, 0x58AB00
		call	edx
		and	eax, 0FFh
		shl	eax, 8
		or	ebx, eax
		mov	edx, [ebp-0x0A8]
		mov	eax, [edx+14h]
		mov	[eax+0A74h], bx

		mov		eax, [Config::LogMode]
		test	eax, SVL_BUILDINGS
		jz	e_i_sk
		mov	eax, [ebp-0x0A8]
		mov	edx, [eax + 0x10]

		movzx	eax, byte ptr [edx + 1]
		push	eax
		movzx	eax, byte ptr [edx]
		push	eax

		mov	ecx, [ebp-0xAC]
		mov	ecx, [ecx+0x08]
		and	ecx, 0xFFFF
		push	ecx

		mov	eax, [ebp-0x0A8]
		mov	eax, [eax + 0x14]
 // unit :: player
//		mov	eax, [eax + 0x0A78]
 // player :: login
		mov	eax, [eax + 0x18]
 // player :: nick
		push	eax

		push	offset enter_inn1
		call	Printf
e_i_sk:
		mov	ecx, [ebp-0x0A8]
		push	ecx
		mov	ecx, [ebp-0x0AC]
		mov	edx, 0x560C67
		call	edx
		mov	ecx, [ebp-0x0A8]
		test	ecx, ecx
		jz	e_i_skip
		mov	ecx, [ecx+14h]
		test	ecx, ecx
		jz	e_i_skip

		call	cancel_camp
e_i_skip:
		mov	edx, 0x50864E
		jmp	edx
	}
}

void _declspec(naked) enter_shop (void) {
	__asm { /// 0050623D
		mov	ecx, [ebp-0x0A0]
//		mov	edx, [ecx+38h]
		cmp	word ptr [ecx+94h], 0
		jle	e_s_skip //// если не жив

		mov	ecx, [ebp-0x0A0]
		mov	edx, 0x52C813
		call	edx
		mov	ecx, [ebp-0x0A0]
		mov	ecx, [ecx+10h]
		mov	edx, 0x58AAF0
		call	edx
		xor	ebx, ebx
		mov	bl, al
		and	ebx, 0FFh
		mov	edx, [ebp-0x0A0]
		mov	ecx, [edx+10h]
		mov	edx, 0x58AB00
		call	edx
		and	eax, 0FFh
		shl	eax, 8
		or	ebx, eax
		mov	eax, [ebp-0x0A0]
		mov	ecx, [eax+14h]
		mov	[ecx+0A74h], bx

		mov		eax, [Config::LogMode]
		test	eax, SVL_BUILDINGS
		jz	e_s_sk

		mov	eax, [ebp-0x0A0]
		mov	edx, [eax + 0x10]

		movzx	eax, byte ptr [edx + 1]
		push	eax
		movzx	eax, byte ptr [edx]
		push	eax

		mov	ecx, [ebp-0xA4]
		mov	ecx, [ecx+0x08]
		and	ecx, 0xFFFF
		push	ecx

		mov	eax, [ebp-0x0A0]
		mov	eax, [eax + 0x14]
//		mov	eax, [eax + 0x0A78]
 // player :: login
		mov	eax, [eax + 0x18]
		push	eax

		push	offset enter_shop1
		call	Printf
e_s_sk:
		mov	edx, [ebp-0x0A0]
		push	edx
		mov	ecx, [ebp-0x0A4]
		mov	edx, 0x544655
		call	edx  //// здесь вылет про одновременном входе выходе в лавку 2-х и более человек
		mov	ecx, [ebp-0x0A0]
		test	ecx, ecx
		jz	e_s_skip
		mov	ecx, [ecx+14h]
		test	ecx, ecx
		jz	e_s_skip

		call	cancel_camp
e_s_skip:
		mov	edx, 0x50864E
		jmp	edx
	}
}



void _declspec(naked) enter_b(void) {
	__asm {//00505353
		mov	ecx, 0x6D1648
		cmp     dword ptr [ecx], 2
		mov	edx, 0x0050535A
		jmp	edx
	}
}


void _declspec(naked) camp(void) {
	__asm { //// 50512E
		mov	ecx, 0x06A8B8C
		cmp	dword ptr [ecx], 0
		jz	loc_505146
		mov	ecx, [ebp-0x01C]
		push	ecx
		mov	ecx, 0x06A8B8C
		mov	ecx, [ecx]
		mov	edx, 0x5ACAA7
		call	edx

loc_505146:				 // CODE XREF: sub_504A96+69Fj
		cmp	dword ptr [ebp-0x014], 0
		jz	loc_50515D
		mov	ecx, 0x06D1648
		cmp	dword ptr [ecx], 0
		jnz	loc_50515D
		mov	ecx, [ebp-0x014]
		jmp	loc_50411E
// ---------------------------------------------------------------------------
loc_50411E:				 // CODE XREF: sub_504A96+6C2j
		mov	eax, [ebp-0x018]
		test	byte ptr [eax+4Ch], 8 
		jnz	loc_50412F
		mov	ecx, [ebp-0x014]
		mov	edx, 0x534B17
		call	edx ///// кемпинг

		mov		eax, [Config::LogMode]
		test	eax, SVL_CAMPING
		jz	loc_50515D

		mov	ecx, [ebp+8]
		xor	eax,eax
		mov	ax, [ecx+5]
		push	eax
		mov	ecx, 0x6CDB24
		mov	ecx, [ecx]
		mov	edx, 0x535B50
		call	edx
		test	eax, eax
		jz	loc_50515D // skip for now
		mov	eax, [eax + 0x18] // player::nickname
		push	eax
		push	offset camp2
		call	Printf
		jmp	loc_50515D

loc_50412F:				 // CODE XREF: sub_504A96-971j
		mov		eax, [Config::LogMode]
		test	eax, SVL_CAMPING
		jz	loc_50515D

		mov	ecx, [ebp+8]
		xor	eax,eax
		mov	ax, [ecx+5]
		push	eax
		mov	ecx, 0x6CDB24
		mov	ecx, [ecx]
		mov	edx, 0x535B50
		call	edx
		test	eax, eax
		jz	loc_50515D // skip for now
		mov	eax, [eax + 0x18] // player::nickname
		push	eax
		push	offset camp1
		call	Printf
		jmp	loc_50515D


loc_50515D:				 // CODE XREF: sub_504A96:loc_50412Fj
						// sub_504A96+6B4j ...
		mov	edx, 0x50864E
		jmp	edx
	}
}

void _declspec(naked) sub_5446C7(void) { /// перетаскивание вещи с прилавка себe
	__asm { /// 005446C7 
		// ecx shop
		// [ebp + 0x08] player
		// [ebp + 0x0c] slot
		// [ebp + 0x10] count
		push	ebp
		mov	ebp, esp
		push	ecx

		mov	[ebp-0x04], ecx
		
		mov	eax, [ebp-0x04]
		mov	ecx, [eax+6Ch]
		mov	eax, [ebp+0x08]
		push	eax
		mov	edx, 0x547468
		call	edx  //// получить список вещей прилавка
		lea	ecx, [eax + 0x078]

		mov	eax, [ecx + 4]

		test	eax, eax
		jz	sub_5446C7_ret0

		mov	edx, [eax + 8]
		mov	eax, [eax]

		xor	ecx, ecx

sub_5446C7_start:
		test	edx, edx
		jz	sub_5446C7_ret0
		
		cmp	ecx, [ebp + 0x0c]
		jnz	sub_5446C7_next
		
		cmp	dword ptr [edx + 0x14], 0
		jz	sub_5446C7_ret0 /// магазинна€ вещь

		jmp	sub_5446C7_done


sub_5446C7_next:
		add	ecx, 1

		test	eax, eax
		jz	sub_5446C7_ret0

		mov	edx, [eax + 8]
		mov	eax, [eax]
		jmp	sub_5446C7_start

sub_5446C7_ret0:
		xor	eax, eax

		mov	esp, ebp
		pop	ebp
		retn	0Ch

sub_5446C7_done:
		mov	eax, [ebp+0x010]
		push	eax
		mov	ecx, [ebp+0x0C]
		push	ecx
		mov	edx, [ebp+0x08]
		push	edx
		mov	eax, [ebp-0x04]
		mov	ecx, [eax+6Ch]
		mov	edx, 0x547C5A
		call	edx
		mov	esp, ebp
		pop	ebp
		retn	0Ch
	}
}



void _declspec(naked) public_chat(void) {////50759F  TESTED100%
	__asm {/// реализаци€ запрета чата
		mov	eax, dword ptr [Config::ServerFlags]
		test	eax, SVF_MUTED
		jz	public_chat_norm
		
		mov	ecx, [ebp-0x0104]
		mov	edx, [ecx+0x14]
		and	edx, GMF_ANY
		cmp	edx, GMF_ANY
		jz	public_chat_norm ///флаг admin

		mov	edx, 0x507897
		jmp	edx

public_chat_norm:
		mov	eax, 0x06CDB24
		mov	eax, dword ptr [eax]
		mov	[ebp-0x0BAC], eax
		mov	ecx, [ebp-0x0BAC]
		mov	[ebp-0x0548], ecx

		mov	edx, 0x05075B6
		jmp	edx
	}
}

void _declspec(naked) public_chat_2(void) {////0050764A
	__asm { /// TESTED
		cmp	dword ptr [eax+2Ch], 0
		jnz	p_c_2_sk

		mov	eax, [eax+0x38]
		test	eax, eax
		jz	p_c_2_sk

		mov	dl, [eax+0x4C]
		test	dl, 8 
		jz	p_c_2_ex //// персонаж на карте, разрешаем

p_c_2_sk:
		mov	edx, 0x5076EC /// skip, AI player
                jmp	edx                
p_c_2_ex:
		mov	edx, 0x507654
		jmp	edx
	}
}

void _declspec(naked) private_chat_2(void) {////00507489
	__asm { /// group chat
		cmp	dword ptr [ecx+2Ch], 0
		jnz	pr_c_2_sk /// skip, AI player

		mov	eax, [ecx+0x38]
		test	eax, eax
		jz	pr_c_2_sk

		mov	dl, [eax+0x4C]
		test	dl, 8 
		jz	pr_c_2_ex //// персонаж на карте, разрешаем

pr_c_2_sk:
		mov	edx, 0x50752F 
                jmp	edx                
pr_c_2_ex:
		mov	edx, 0x507493
		jmp	edx
	}
}

void _declspec(naked) private_chat_3(void) {////005073B6
	__asm { /// single private
		xor	edx, edx
		mov	dx, word ptr [ebp-0x010C]
		push	edx
		mov	edx, 0x06CDB24
		mov	ecx, dword ptr [edx]
		mov	edx, 0x535B50
		call	edx /// possibly, player

		test	eax, eax
		jz	pr_c_3_sk

		mov	eax, [eax+0x38]
		test	eax, eax
		jz	pr_c_3_sk

		mov	dl, [eax+0x4C]
		test	dl, 8 
		jz	pr_c_3_ex //// персонаж на карте, разрешаем

pr_c_3_sk: /// не отправл€ем
		mov	edx, 0x5073D8 
                jmp	edx                
pr_c_3_ex: /// отправл€ем
		mov	ecx, [ebp-0x052C]
		mov	edx, 0x5073BC
		jmp	edx
	}
}


void _declspec(naked) private_chat(void) {////005073DD  TESTED100%
	__asm {
		mov	eax, dword ptr [Config::ServerFlags]
		test	eax, SVF_MUTED
		jz	private_chat_norm
		
		mov	ecx, [ebp-0x0104]
		mov	edx, [ecx+0x14]
		and	edx, GMF_ANY
		cmp	edx, GMF_ANY
		jz	private_chat_norm ///флаг admin

		mov	edx, 0x507897
		jmp	edx

private_chat_norm:

		mov	eax, 0x06CDB24
		mov	ecx, dword ptr [eax]
		mov	[ebp-0x0B84], ecx
		mov	edx, [ebp-0x0B84]
		mov	[ebp-0x0538], edx

		mov	edx, 0x005073F5
		jmp	edx

	}
}

void _declspec(naked) shout_chat(void) {////0050775C  TESTED100%
	__asm {
		mov	eax, dword ptr [Config::ServerFlags]
		test	eax, SVF_MUTED
		jz	shout_chat_norm
		
		mov	ecx, [ebp-0x0104]
		mov	edx, [ecx+0x14]
		and	edx, GMF_ANY
		cmp	edx, GMF_ANY
		jz	shout_chat_norm ///флаг admin

		mov	edx, 0x507897
		jmp	edx

shout_chat_norm:
		mov	eax, [ebp-0x0104]
		mov	edx, 0x00507762
		jmp	edx
	}
}

extern char aMode[];


void __stdcall set_mode(int*p, char*s)
{
	/*unsigned int new_mode = 0;/////TESTED100%, STDCALL
	int f_m = 0, f_p = 0;
	char tmp[BUF_MAX];
	s+=strlen(aMode);
	while (*s==' ') s++;
	if (*s=='-') { s++; f_m = 1; } else
	if (*s=='+') { s++; f_p = 1; }
	if (*s=='=') {
		s++;
		while (*s==' ') s++;
		if (*s) {
			if (sscanf(s, "%x", &new_mode)) {
				if (f_m) new_mode = mode & (~new_mode); 
				if (f_p) new_mode = mode | (new_mode); 
				_snprintf(tmp, BUF_MAX-1, "mode is set to 0x%.8x (was: 0x%.8x)", new_mode, mode);
				mode=new_mode;
				log_format("%s\n", tmp);
				send_to_player(p, tmp);
				return;
			}
		}
	}
	_snprintf(tmp, BUF_MAX-1, "mode is set to 0x%.8x", mode);
	log_format("%s\n", tmp);
	send_to_player(p, tmp);*/
	return;
}


void __stdcall set_mode_2(char*s) {
	/*unsigned int new_mode = 0;/////TESTED100%, STDCALL
	int f_m = 0, f_p = 0;
	char tmp[BUF_MAX];
	while (*s==' ') s++;
	if (*s=='-') { s++; f_m = 1; } else
	if (*s=='+') { s++; f_p = 1; }
	if (*s=='=') {
		s++;
		while (*s==' ') s++;
		if (*s) {
			if (sscanf(s, "%x", &new_mode)) {
				if (f_m) new_mode = mode & (~new_mode); 
				if (f_p) new_mode = mode | (new_mode); 
				_snprintf(tmp, BUF_MAX-1, "mode is set to 0x%.8x (was: 0x%.8x)", new_mode, mode);
				mode=new_mode;
				log_format("%s\n", tmp);
				return;
			}
		}
	}
	_snprintf(tmp, BUF_MAX-1, "mode is set to 0x%.8x", mode);
	log_format("%s\n", tmp);*/
	return;
}



void _declspec(naked) max_players(void) {///TESTED100%
	__asm {/////004F7E32   
		cmp	dword ptr [ebp-0x04C], 1
		jl	loc_4F7E3E
		cmp	dword ptr [ebp-4Ch], 20h
		jle	loc_4F7E45

loc_4F7E3E:				 // CODE XREF: sub_4F7188+CAEj
		mov	dword ptr [ebp-0x04C], 20h

loc_4F7E45:				 // CODE XREF: sub_4F7188+CB4j
		mov	ecx, [ebp-0x04C]
		mov	edx, 0x06D163C
		mov	[edx], ecx
		mov	edx, 0x4F836A
		jmp	edx
	}
}

void __stdcall log_saving(char* s) {///TESTED100%
	if (Config::LogMode & SVL_SAVES) {
		/*log_format("Saving character to file \"%s\"\n", s);
		char zzz[BUF_MAX];
		_snprintf(zzz, BUF_MAX-1, "Saving character to file \"%s\"",s);
		print_log(zzz);*/
		log_format("Saving character to file \"%s\"...\n");
	}
}

void __stdcall log_saved(void) {///TESTED100%
	if (Config::LogMode & SVL_SAVES) {
		//log_format("Saved successfully!\n");
		//print_log("Saved successfully!");
		log_format("Saved successfully!\n");
	}
}

void _declspec(naked) part_of_save(void) {///TESTED100%
	__asm {/// 4F62B1
		call	log_saved
		lea	ecx, [ebp-0x020]
		mov	edx, 0x5DF3B8
		call	edx
		mov	dword ptr [ebp-0x0A6C], 1
		mov	dword ptr [ebp-0x04], 0FFFFFFFFh
		lea	ecx, [ebp-0x020]
		mov	edx, 0x5DF0B3
		call	edx
		mov	eax, [ebp-0x0A6C]
		mov	edx, 0x4F62D8
		jmp	edx
	}
}



#pragma warning (disable: 4733)
void _declspec(naked) part_of_save_char(void) {///TESTED100%
	__asm {///
		mov	eax, [ebp-0x44]
		push	eax
		call	log_saving

		push	0
		push	0
		lea	edx, [ebp-0x0100A8]
		push	edx
		lea	eax, [ebp-0x08090]
		push	eax
		mov	ecx, [ebp-0x048]
		add	ecx, 44h
		push	ecx
		lea	edx, [ebp-0x07C]
		push	edx
		lea	eax, [ebp-0x040]
		push	eax
		mov	ecx, [ebp-0x044]
		push	ecx
		mov	edx, 0x4F53EA
		call	edx
		add	esp, 20h
		mov	byte ptr [ebp-0x04], 1
		lea	ecx, [ebp-0x044]
		mov	edx, 0x5DD88A
		call	edx
		mov	byte ptr [ebp-0x04], 0
		lea	ecx, [ebp-0x0100A8]
		mov	edx, 0x527456
		call	edx
		mov	dword ptr [ebp-0x04], 0FFFFFFFFh
		lea	ecx, [ebp-0x08090]
		mov	edx, 0x527456
		call	edx
		mov	ecx, [ebp-0x0C]
		mov	fs:0, ecx
		mov	esp, ebp
		pop	ebp
		retn	4
	}
}
#pragma warning (default: 4733)

void _declspec(naked) part_of_disconnect2(void) {////!!!!!
	__asm { /// 00534E87
/*		mov	eax, [ebp-0x10]
		mov	edx, [eax+0x3C]
		test	[edx + 0x4C
		mov	ecx, [ebp-0x010]
		mov	edx, 0x534778
		call	edx
		mov	edx, [ebp-0x010]
		mov	eax, [edx+0A50h]
		sub	eax, 1
		mov	ecx, [ebp-0x010]
		mov	[ecx+0A50h], eax
		mov	edx, [ebp-0x010]
		mov	dword ptr [edx+0A90h], 0
		jmp	loc_534F04*/
	}
}



void _declspec(naked) part_of_disconnect(void) {////!!!!!
	__asm {///00534F4C
//		mov	eax, 0x0642C2C//////////////////////?????
//		mov	eax, [eax]/////////////////////////????? раскомментировать, если не мешает дюпу
//		cmp	dword ptr [eax+74h], 0/////////////отключена проверка во им€ сохранени€ во врем€ 5-мин. дисконнекта
//		jz	loc_534F82
		mov	ecx, [ebp-0x010]
		cmp	dword ptr [ecx+38h], 0
		jz	loc_534F82
		mov	edx, [ebp-0x010]
		mov	eax, [edx+38h]
		push	eax
		mov	eax, 0x0642C2C
		mov	ecx, [eax]
		mov	edx, 0x4EE028
		call	edx
loc_534F82:
		mov	ecx, [ebp-0x010]
		xor	edx, edx
		mov	dl, [ecx+41h]
		test	edx, edx
		jnz	skip5352B8
		mov	eax, 0x5352B8
		jmp	eax
skip5352B8:
		mov	edx, 0x534F82
		jmp	edx
	}//00534F82
}

void _declspec(naked) part_of_camp(void) {
	__asm { /// 0x00534E87   TESTED100%
		mov	eax, [ebp-0x010]
		mov	eax, [eax+0x38]
		push	eax
		mov	ecx, 0x642C2C
		mov	ecx, [ecx]
		mov	edx, 0x4EE028
		call	edx //// принудительное сохранение
		mov	ecx, [ebp-0x010]
		mov	edx, 0x534778
		call	edx
		mov	edx, [ebp-0x010]
		mov	eax, [edx+0A50h]
		sub	eax, 1
		mov	ecx, [ebp-0x010]
		mov	[ecx+0A50h], eax
		mov	edx, [ebp-0x010]
		mov	dword ptr [edx+0A90h], 0
		mov	edx, 0x534F04
		jmp	edx
	}
}

/*void _stdcall _kick_all(void) {
	__asm
		call	kick_all

}*/

#include "cheat_codes_new.h"

int _declspec(naked) verify_damage(int p1, int p2, int damage) {/// p2 - кого бьем, p1 - кто бьет
	_asm{
		//////// TESTED100%, CDECL
		push	ebp
		mov	ebp, esp

		and		[damage], 0xFFFF
		push	[damage]
		push	[p2]
		push	[p1]
		call	OnDamage
		add		esp, 0x0C
		mov	esp, ebp
		pop	ebp
		ret
	}
}

void setthreadp(void) {
	__asm { /// не повышаем приоритет
		/// STDCALL, TESTED100% ;-)
		retn	8
	}
}

void __stdcall print_error(int*a) {//// TESTED100%
	log_format("error found! 0x%.8x -> 0x%.8x\n", *a, *(a+1));
}

void _declspec(naked) testeaxifzero(void) {
	__asm {///TESTED100%
		test	eax, eax
		jnz	ok111
		call	upd_all
		lea	esi, bugs
		mov	eax, [esp] // адрес возврата
st111:		cmp	dword ptr [esi], 0
		jz	ok1112 // not found
		cmp	[esi], eax
		jz	found111
		add	esi, 8
		jmp	st111
found111:
		push	esi
		call	print_error
		mov	eax, [esi+4]
		mov	[esp], eax
		xor	eax, eax

ok1112:		
ok111:		
		ret
	}
}

SOCKET PASCAL accept0(SOCKET s, sockaddr *addr, int *addrlen) {
	SOCKET r = accept(s, addr, addrlen);
#ifdef SRV_LOG
	if (r) {
		struct client *c;
		c = c0;
		c0 = new struct client;
		c0->n = c;
		c0->p = 0;
		if (c) c->p = c0;
		c0->s = r;
	}
#endif
	return r;
}


void _declspec(naked) part_of_kill(void) { /// 005569EF
	__asm {
		mov	ecx, [ebp-0x018]
		cmp	dword ptr [ecx+40h], 0
		jnz	test_hp_max
		mov	edx, 0x05572C1
		jmp	edx
test_hp_max:
		cmp	word ptr [ecx+0x096], 0
		jle	zeroize_dam_by
		mov	edx, 0x05569F6
		jmp	edx
zeroize_dam_by:
		mov	dword ptr [ecx+40h], 0
		mov	edx, 0x05572C1
		jmp	edx
	}
}



void _declspec(naked) self_damage(void) { /// 005084F5
	__asm {
		mov	ecx, [ebp-0x0A20]
		mov	edx, [ecx+38h]
		mov	dword ptr [edx+40h], 0 /// damage_by = 0
		mov	ecx, [ebp-0x0A20]
		mov	edx, 0x05084FB
		jmp	edx
	}
}

int PASCAL send0(SOCKET s, const char *buf, int len, int flags) {
	/*int r = len;
	char *bb = new char[len], *p = bb;
	memcpy(bb, buf, r);
	do {
		if (r < 8) {
			log_format("bad data discarded!\n");
			return 0;
		}
		if (r < 8 + *p) {
			log_format("bad data discarded!\n");
			return 0;
		}
		log_format("header: %08X %08X\n", *(unsigned long*)p, *(unsigned long*)(p+4));
		cryptver((unsigned char *)p + 8, CUR_VER, *(unsigned char *)p);
		int size = *(unsigned char *)p;
		log_format("s>h ");
		for (int i = 0; i < size; ++i) 
			log_format2("%.2X ", *(unsigned char *)(p + 8 + i));
		log_format2("\n");
		p += size + 8;
		r -= size + 8;
	} while (r);
	delete bb;*/
	return send(s, buf, len, flags);
}

int PASCAL closesocket0(SOCKET s) {
#ifdef SRV_LOG
	struct client *c, *t;
	c = c0;
	while (c) if (c->s != s) c = c->n; else break;
	if (c) { // delete
		t = c->n;
		if (c->p) {
			c->p->n = t;
			if (t) t->p = c->p;
		} else {
			c0 = c->n;
			if (t) t->p = c0;
		}
		delete c;
	}
#endif
	return closesocket(s);
}

#include "zxmgr.h"

int _stdcall recv0(SOCKET s, char* buf, int len, int flags)
{
	SOCKET hat_socket;
	hat_socket = *(SOCKET*)(0x006CDBC0 + 0x0C);

    // find player
    byte* mainStruc = (byte*)(0x006D07A0);
    uint32_t structs = *(uint32_t*)(mainStruc + 0x5A4);
    int16_t p_id = -1;
    byte* p_netstruc = NULL;
    for(uint32_t i = 0; i < structs; i++)
    {
        byte* pstruc = *(byte**)(mainStruc + 0x57C) + i * 0x274;
        p_netstruc = *(byte**)(pstruc + 0x10);

        if(*(SOCKET*)(pstruc + 8) == s)
        {
            if(!p_netstruc)
            {
                Printf("Warning: received packet from player with NULL network structure!\n");
                memset(buf, 0, len);
                return -1;
            }

            p_id = *(uint16_t*)(p_netstruc + 0x108);
        }
    }

    if(p_id == -1 && s != hat_socket)
    {
        Printf("Warning: received packet from unknown player!\n");
        memset(buf, 0, len);
        return -1;
    }

    byte* p_player = NULL;
	if(p_id >= 16) p_player = zxmgr::FindByID(p_id);

	int r = recv(s, buf, 4, 0);
	if(r != 4) return -1;
	if(!*(int*)(buf)) return 0;

	r = recv(s, buf+4, 4, 0);
	if(r != 4) return -1;

	int pkt_size = *(int*)(buf);
	int pkt_flags = *(int*)(buf+4);
	
	if((pkt_flags & 0x80100000) == 0x80100000)
	{
		const char* p_name = (p_player ? *(const char**)(p_player+0x18) : "N/A");
		//log_format("received special packet (size = %u, flags = %08X, player = %s).\n", pkt_size, pkt_flags, p_name);
		uint8_t* rd = new uint8_t[pkt_size];
		r = recv(s, (char*)rd, pkt_size, 0);
		if(r != pkt_size)
		{
			delete[] rd;
			return -1;
		}

		Packet pack;
		pack.Reset();
		for(uint32_t i = 0; i < pkt_size; i++)
			pack.WriteUInt8(rd[i]);
		pack.Seek(0);

		delete[] rd;

		if(!Sv_ProcessClientPacket(p_id, p_player, pack))
			return -1;

		memset(buf, 0, len);
		return recv0(s, buf, len, flags);
	}
	else
	{
		const char* p_name = (p_player ? *(const char**)(p_player+0x18) : "N/A");
		//log_format("received allods packet (size = %u, flags = %08X, player = %s).\n", pkt_size, pkt_flags, p_name);
		r = recv(s, buf+8, pkt_size, 0);
		if(r != pkt_size) return -1;
		r += 8;
	}

	return r;
}

void __declspec(naked) test_if_valid_injection()
{
	// method: jmp
	// instead of: 004FE2ED
	// return to: 004FE2F3 if valid
	// or to: 004FE305 if too strong
	__asm
	{
		jmp test_if_valid_injection_softcore

		mov	edx, dword ptr [ebp - 0x30]
		push	dword ptr [ebp + 0x14]
		push	dword ptr [edx + 0x14]
		push	dword ptr [edx + 0x10]
		call	test_if_valid
		test	eax, eax
		jz	test_if_valid_injection_softcore

		mov	ecx, 0x04FE305
		jmp	ecx

test_if_valid_injection_valid:

		mov	edx, dword ptr [ebp - 0x30]
		mov	eax, dword ptr [ebp - 0x360]
		mov	ecx, 0x004FE2F3
		jmp	ecx

test_if_valid_injection_softcore:

		mov	eax, Config::ServerFlags
		and	eax, SVF_SOFTCORE
		cmp	eax, SVF_SOFTCORE
		jnz	test_if_valid_injection_valid

		mov	edx, 0x004FE61D
		jmp	edx

	}
}

int __stdcall test_if_valid(int id_1, int id_2, char *login_name){//TESTED100%
//	log_format ("login: \"%s\" id1: 0x%.8x id2: 0x%.8x\n", login_name, id_1, id_2);
	if ((id_2 & GMF_ANY) == GMF_ANY) return 0; 
	return 0; 
}

void _declspec(naked) server_closed()
{
	__asm
	{
		mov		eax, [Config::ServerFlags]
		test	eax, SVF_ONEMAP
		jz	sc_cont
		mov	edx, 0x401D90
		call	edx
		mov	dword ptr [eax+0x624], 1
sc_cont:
		mov	dword ptr [ebp-0x98], eax
		mov	edx, 0x4F2FD9
		jmp	edx
	}
}

void _declspec(naked) load_config()
{
	__asm
	{
		push	offset config
		mov	edx, 0x4F7188
		call	edx
		add	esp, 4
		mov	edx, 0x0482CD9
		jmp	edx
	}
}

#include "File.h"
#include "itemex.h"

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	const unsigned char cr20[0x50] =
		{0x0C9,0x086,0x093,0x0FC,0x06A,0x044,0x018,0x0AA,0x0AA,0x012,0x09A,0x060,0x0CF,0x010,0x041,0x0E8,0x0BA,0x0BA,0x01A,0x088,0x010,0x0AF,0x0DE,0x0AD,0x0BE,0x0EF,0x084,0x091,0x05A,0x015,0x020,0x01A,0x055,0x005,0x0E7,0x087,0x001,0x011,0x0E5,0x0D1,0x006,0x0E6,0x068,0x071,0x0CC,0x0CC,0x018,0x080,0x000,0x011,0x03F,0x018,0x0CA,0x016,0x0C9,0x026,0x0AD,0x051,0x05A,0x080,0x0CD,0x0EF,0x034,0x088,0x014,0x081,0x0BE,0x0FE,0x0EE,0x0ED,0x0C0,0x0FF,0x0EE,0x0E8,0x079,0x0A5,0x0E7,0x050,0x041,0x0CA,};

	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		srand(static_cast<unsigned int>(time(0)));
		*((int *)&(print_log))=0x0043A857;

		strcpy(config, "server.cfg");
		strcpy(ctl_dir, ".\\non-existent");

		///////////// считываем из командной строки им€ файла с конфигом
		char *cmd;
		cmd = GetCommandLine();
//		MessageBox(0, cmd, 0, 0);
		memcpy((char *)0x00636368, (const char *)cr20, sizeof(cr10));

		hThread = CreateThread(
			NULL,              // default security attributes
			0,		// use default stack size  
			ThreadProc,        // thread function 
			0,             // argument to thread function 
			0,		// use default creation flags 
			&dwThreadId);   // returns the thread identifier 

		if (!hThread) ExitProcess(0);

		/*if(!Archives.Open("./world.res", "world"))
		{
			log_format("Error: couldn't load \"world.res\".\n");
			return FALSE;
		}

		if(!itemex_Initialize())
		{
			log_format("Error: couldn't initialize ItemEx plugin.\n");
			return FALSE;
		}*/

		break;
	case DLL_PROCESS_DETACH:
		exiting = true;

		WaitForMultipleObjects(1, &hThread, TRUE, INFINITE);
		CloseHandle(hThread);

		if(!Config::ExitingCleanly)
		{
			if(Config::MapLoaded)
			{
				log_format("Trying to save characters...\n");

				__try
				{
					upd_all();
					log_format("upd_all(): Characters saved.\n");
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					log_format("upd_all() failed.\n");
				}
			}
			else
			{
				log_format("Exception was caught, but there is no map. Not saving characters.\n");
			}
		}

		break;
	}
	return TRUE;
}
