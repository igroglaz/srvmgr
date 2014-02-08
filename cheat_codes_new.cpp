#include "cheat_codes_new.h"
#include "srvmgrdef.h"
#include "lib\utils.hpp"
#include "pvm2.h"
#include "config_new.h"
#include "protolayer_hat.h"
#include "srvmgr.h"
#include <fstream>
#include "zxmgr.h"
#include "player_info.h"
#include "pktmgr.h"

#include "scanrange.h"

uint32_t ParseFlags(std::string string)
{
    if(CheckHex(string))
    {
        return HexToInt(string);
    }
    else
    {
        uint32_t flags = 0;
        std::vector<std::string> v = Explode(string, "|");
        for(std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
        {
            std::string par = ToLower(Trim((*it)));
            if(!par.length()) continue;
            uint32_t flag = 0;
            bool erase = false;
            if(par[0] == '-')
            {
                erase = true;
                par.erase(0, 1);
            }

            if(par == "pvm")
                flag = SVF_PVM;
            else if(par == "muted")
                flag = SVF_MUTED;
            else if(par == "closed")
                flag = SVF_CLOSED;
            else if(par == "advanced_pvm")
                flag = SVF_ADVPVM;
			else if(par == "nohealing")
				flag = SVF_NOHEALING;
			else if(par == "noobsrv")
				flag = SVF_NOOBSRV;
			else if(par == "softcore")
				flag = SVF_SOFTCORE;
			else if(par == "nodrop")
				flag = SVF_NODROP;
			else if(par == "fnodrop")
				flag = SVF_FNODROP;

            if(!flag) continue;
            if(!erase) flags |= flag;
            else flags &= ~flag;
        }

        return flags;
    }
}

char SpellNames[32][2][32] =
{
	{"", 0},
	{"Fire_Arrow", 1},
	{"Fire_Ball", 1},
	{"Wall_of_Fire", 1},
	{"Protection_from_Fire", 1},
	{"Ice_Missile", 2},
	{"Poison_Cloud", 2},
	{"Blizzard", 2},
	{"Protection_from_Water", 2},
	{"Acid_Stream", 2},
	{"Lightning", 3},
	{"Prismatic_Spray", 3},
	{"Invisibility", 3},
	{"Protection_from_Air", 3},
	{"Darkness", 3},
	{"Light", 3},
	{"Diamond_Dust", 4},
	{"Wall_of_Earth", 4},
	{"Stone_Curse", 4},
	{"Protection_from_Earth", 4},
	{"Bless", 5},
	{"Haste", 5},
	{"Control_Spirit", 5},
	{"Teleport", 5},
	{"Heal", 5},
	{"Summon", 5},
	{"Drain_Life", 5},
	{"Shield", 5},
	{"Curse", 5},
	{"Slow", 5},
	{"", 0},
	{"", 0},
};

char SpellBooks[6][10] =
{
	"None",
	"Fire",
	"Water",
	"Air",
	"Earth",
	"Astral",
};

void DropEverything(byte* unit, bool full = false)
{
	if(Config::GameMode != 0) return;

    uint32_t p_x = *(uint8_t*)(*(byte**)(unit + 0x10));
    uint32_t p_y = *(uint8_t*)(*(byte**)(unit + 0x10) + 1);

	uint32_t spellmask = zxmgr::GetSpells(unit);
	for(int i = 0; i < 32; i++)
	{
		if((1 << i) & spellmask && strlen(SpellNames[i][0]) && SpellNames[i][1][0])
		{
			std::string cstrp = Format("Book %s {teachSpell=%s}", SpellBooks[SpellNames[i][1][0]], SpellNames[i][0]);
			zxmgr::CreateSack(cstrp.c_str(), p_x, p_y, 0);
		}
	}

	zxmgr::SetSpells(unit, 0);
	zxmgr::UpdateUnit(unit, NULL, 0xA31FFFFF, 0xFFB, 0, 0);
	if(!full) return;
	byte* player = *(byte**)(unit + 0x14);
	if(!player) return;
	uint32_t money = *(uint32_t*)(player + 0x3C);
	if(!money) return;
	if(money > 0x7FFFFFFF) money = 0x7FFFFFFF;
	zxmgr::CreateSack(NULL, p_x, p_y, money);
	*(uint32_t*)(player + 0x3C) = 0;
	zxmgr::GiveMoney(player, 0, 0);
}

void RunCommand(byte* _this, byte* player, const char* ccommand, uint32_t rights, bool console)
{
    if(!ccommand) return;
    if(ccommand[0] != '#') return;

    std::string command(ccommand);
    command = Trim(command);

    std::string rawcmd = command;

    size_t spacepos = command.find_first_of(' ');
    if(spacepos != std::string::npos)
        rawcmd.erase(spacepos);

    if(rawcmd == "#mapinfo")
    {
		const char* map_file = Config::CurrentMapName.c_str();
		const char* map_name = Config::CurrentMapTitle.c_str();

		uint32_t tme = zxmgr::GetCurrentMapTime()/1000;
		uint32_t tm_h = tme / 3600;
		uint32_t tm_m = tme / 60 - tm_h * 60;
		uint32_t tm_s = tme - (tm_h * 3600 + tm_m * 60);
		tme = zxmgr::GetTotalMapTime()*60;
		uint32_t tt_h = tme / 3600;
		uint32_t tt_m = tme / 60 - tt_h * 60;
		uint32_t tt_s = tme - (tt_h * 3600 + tt_m * 60);

        std::string time_left = "infinite";
        if(!Config::Suspended && zxmgr::GetTotalMapTime() != 0x7FFFFFFF) time_left = Format("%u:%02u:%02u", tt_h, tt_m, tt_s);

        if(player) zxmgr::SendMessage(player, "map: \"%s\" in \"%s\", time elapsed: %u:%02u:%02u, time total: %s", map_name, map_file, tm_h, tm_m, tm_s, time_left.c_str());
        else if(console) Printf("Map: \"%s\" in \"%s\", time elapsed: %u:%02u:%02u, time total: %s", map_name, map_file, tm_h, tm_m, tm_s, time_left.c_str());
        goto ex;
    }

    if(rights & GMF_CMD_CHAT)
    {
        if(command.find("#@") == 0)
        {
            command.erase(0, 2);
            std::string what = "";
            if(!player || console) what = Format("[broadcast] <server%u>: %s", Config::ServerID, command.c_str());
            else if(player) what = Format("[broadcast] %s: %s", *(const char**)(player + 0x18), command.c_str());
            else what = Format("[broadcast] %s", command.c_str());
            if(!NetCmd_Broadcast(what)) NetHat::Connected = false;
            goto ex;
        }
        else if(command.find("#!") == 0)
        {
            command.erase(0, 2);
            if(!player || console) zxmgr::SendMessage(NULL, "<server>: %s", command.c_str());
            else if(player) zxmgr::SendMessage(NULL, "%s: %s", *(const char**)(player + 0x18), command.c_str());
            else zxmgr::SendMessage(NULL, command.c_str());
            goto ex;
        }
    }

    if(rights & GMF_CMD_KICK)
    {
        if(rawcmd == "#kick")
        {
            command.erase(0, 5);
            command = TrimLeft(command);
			byte* target = zxmgr::FindByNickname(command.c_str());

			if(target) zxmgr::Kick(target, false);
            goto ex;
        }
        else if(rawcmd == "#kickme")
        {
            if(!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            if(player) zxmgr::Kick(player, true);
            goto ex;
        }
		else if(rawcmd == "#kickall")
		{
			zxmgr::KickAll(NULL);
			goto ex;
		}
        else if(rawcmd == "#kick_silent")
        {
            command.erase(0, 12);
            command = TrimLeft(command);
            byte* target = zxmgr::FindByNickname(command.c_str());

            if(target) zxmgr::Kick(target, true);
            goto ex;
        }
        else if(rawcmd == "#disconnect")
        {
            command.erase(0, 11);
            command = TrimLeft(command);
            byte* target = zxmgr::FindByNickname(command.c_str());

            if(target) zxmgr::Disconnect(target);
            goto ex;
        }
    }

    if(rights & GMF_CMD_INFO)
    {
        if(rawcmd == "#locate")
        {
            command.erase(0, 7);
            command = TrimLeft(command);
			byte* target = zxmgr::FindByNickname(command.c_str());

            if(target && *(byte**)(target + 0x38))
            {
                uint32_t p_x = *(uint8_t*)(*(uint32_t*)(*(byte**)(target + 0x38) + 0x10));
                uint32_t p_y = *(uint8_t*)(*(uint32_t*)(*(byte**)(target + 0x38) + 0x10) + 1);

                if(!player || console)
                    Printf("%s (%u:%u)", *(const char**)(target + 0x18), p_x, p_y);
                else if(player)
                    zxmgr::SendMessage(player, "%s (%u:%u)", *(const char**)(target + 0x18), p_x, p_y);
            }
            goto ex;
        }
        else if(rawcmd == "#info")
        {
            command.erase(0, 5);
            command = TrimLeft(command);
            byte* target = zxmgr::FindByNickname(command.c_str());

            if(!target) goto ex;
            if(!*(byte**)(target + 0x38)) goto ex;

            const char* p_charname = *(const char**)(target + 0x18);
            const char* p_logname = *(const char**)(target + 0x0A78);
            byte* netinf = zxmgr::GetNetworkStruct(target);
            bool p_connected = (netinf);
            const char* p_address = "n/a";
            if(p_connected) p_address = (const char*)(netinf + 8);

            std::string p_state;
            if(p_connected) p_state = Format("connected (%s)", p_address);
            else p_state = "disconnected";

            byte* unit = *(byte**)(target + 0x38);

            uint8_t p_body      = *(uint8_t*)(unit + 0x84);
            uint8_t p_reaction  = *(uint8_t*)(unit + 0x86);
            uint8_t p_mind      = *(uint8_t*)(unit + 0x88);
            uint8_t p_spirit    = *(uint8_t*)(unit + 0x8A);

            const char* p_strong = (vd2_CheckStrong(unit) ? "yes" : "no");

            if(!player || console)
            {
                Printf("nickname: \"%s\", login: \"%s\", state: %s, stats: [%u,%u,%u,%u], strong: %s",
                       p_charname, p_logname, p_state.c_str(),
                       p_body, p_reaction, p_mind, p_spirit, p_strong);
            }
            else if(player)
            {
                zxmgr::SendMessage(player, "nickname: \"%s\", login: \"%s\", state: %s, stats: [%u,%u,%u,%u], strong: %s\n",
                                    p_charname, p_logname, p_state.c_str(),
                                    p_body, p_reaction, p_mind, p_spirit, p_strong);
            }

            goto ex;
        }
		else if(rawcmd == "#scan")
		{
			if(!player) goto ex;
			byte* unit = *(byte**)(player + 0x38);
			if(!unit) goto ex;

			SR_DumpToFile(player);
			goto ex;
		}
    }

    if(rights & GMF_CMD_KILL)
    {
        if(rawcmd == "#kill")
        {
            command.erase(0, 5);
            command = TrimLeft(command);
            byte* target = zxmgr::FindByNickname(command.c_str());

            if(target)
            {
                uint32_t tri = 0;
                if(!*(uint32_t*)(target + 0x2C))
                    tri = *(uint32_t*)(target + 0x14);

                if(CHECK_FLAG(tri, GMF_ANY))
                {
                    if(((tri & GMF_GODMODE) &&
                        (rights & GMF_GODMODE_ADMIN)) ||
                        (tri & GMF_GODMODE_ADMIN)) goto ex;
                }

				zxmgr::Kill(target, player);
            }
            goto ex;
        }
		else if(rawcmd == "#murder")
		{
			command.erase(0, 7);
			command = TrimLeft(command);
            byte* target = zxmgr::FindByNickname(command.c_str());

            if(target)
            {
                uint32_t tri = 0;
                if(!*(uint32_t*)(target + 0x2C))
                    tri = *(uint32_t*)(target + 0x14);

                if(CHECK_FLAG(tri, GMF_ANY))
                {
                    if((((tri & GMF_GODMODE) &&
                       !(rights & GMF_GODMODE_ADMIN)) ||
                        (tri & GMF_GODMODE_ADMIN)) && target != player) goto ex;
                }

				zxmgr::Kill(target, player);
				byte* unit = *(byte**)(target + 0x38);
				if(unit && target != player)
					DropEverything(unit, true);
			}
			goto ex;
		}
        else if(rawcmd == "#killall")
        {
            zxmgr::KillAll(player, false);
        }
        else if(rawcmd == "#killai")
        {
            zxmgr::KillAll(player, true);
        }
    }

    if(rights & GMF_CMD_PICKUP)
    {
        if(rawcmd == "#pickup")
        {
            if(!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            if(player)
            {
                cheat_codes_2(player, ccommand);
            }
            goto ex;
        }
    }

    if(rights & GMF_CMD_SUMMON)
    {
        if(rawcmd == "#summon")
        {
            if(!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            if(player && *(byte**)(player + 0x38))
            {
                command.erase(0, 7);
                command = Trim(command);
				if(!command.length()) goto ex;

                uint32_t count = 1;
                size_t fsp = command.find_first_of(" ");
                if(fsp != std::string::npos)
                {
                    std::string intstr = command;
                    intstr.erase(fsp);
                    if(CheckInt(intstr))
                    {
                        command.erase(0, fsp + 1);
                        command = TrimLeft(command);
                        count = StrToInt(intstr);
                        if(!count) count = 1;
                    }
                }

				if(!command.length()) goto ex;
                for(uint32_t i = 0; i < count; i++)
                {
					byte* unit = zxmgr::Summon(player, command.c_str(), *(byte**)(0x00642C2C), false, NULL);
                    //byte* unit = Map::CreateUnitForEx(player, command.c_str(), false);
                    //zxmgr::SendMessage(NULL, "what: %02X\n", *(uint8_t*)(*(byte**)(unit + 0x1C4) + 0x78));
                    //Unit::SetSpells(unit);
                }
            }
        }
    }

    if(rights & GMF_CMD_SET)
    {
        if(rawcmd == "#nextmap")
        {
            zxmgr::NextMap();
            goto ex;
        }
        else if(rawcmd == "#prevmap")
        {
            zxmgr::PrevMap();
            goto ex;
        }
        else if(rawcmd == "#resetmap")
        {
            if(Config::Suspended)
            {
                zxmgr::SetTotalMapTime(Config::OriginalTime);
                Config::Suspended = false;
            }
            zxmgr::ResetMap();
            goto ex;
        }
        else if(rawcmd == "#shutdown")
        {
            TerminateProcess(GetCurrentProcess(), 100);
            goto ex;
        }
        else if(rawcmd == "#suspend")
        {
            if(!Config::Suspended)
            {
                Config::OriginalTime = zxmgr::GetTotalMapTime();
                zxmgr::SetTotalMapTime(0x7FFFFFFF);

                if(!player || console) Printf("Map timer suspended.");
                else if(player) zxmgr::SendMessage(player, "suspend: map timer suspended.");
            }
            else
            {
                bool change_map = false;
                uint32_t cur_time = zxmgr::GetCurrentMapTime() / 60000;
                if(cur_time >= Config::OriginalTime) change_map = true;

                zxmgr::SetTotalMapTime(Config::OriginalTime);
                Config::OriginalTime = 0;

                if(!player || console) Printf("Map timer released.");
                else if(player) zxmgr::SendMessage(player, "suspend: map timer released.");

                if(change_map)
                {
                    Printf("Note: map timer beyond total map time. Changing map...");
                    zxmgr::NextMap();
                }
            }

            Config::Suspended = !Config::Suspended;
            goto ex;
        }
        else if(rawcmd == "#set" && (command.find("#set mode") == 0))
        {
            command.erase(0, 9);
            command = Trim(command);

            if(!command.length())
            {
                if(!player || console) Printf("Mode is set to %08X.", Config::ServerFlags);
                else if(player) zxmgr::SendMessage(player, "mode is set to %08X", Config::ServerFlags);
            }
            else
            {
                int32_t add_flags = 0;
                if(command[0] == '+') add_flags = 1;
                else if(command[0] == '-') add_flags = -1;
                else if(command[0] == '=') add_flags = 0;
                else goto ex;

                if(add_flags != 0) command.erase(0, 1);
                if(command[0] != '=') goto ex;
                command.erase(0, 1);

                uint32_t old_mode = Config::ServerFlags;

                uint32_t flags = ParseFlags(TrimLeft(command));
                if(add_flags == -1) Config::ServerFlags &= ~flags;
                else if(add_flags == 1) Config::ServerFlags |= flags;
                else Config::ServerFlags = flags;

                if(old_mode != Config::ServerFlags)
                {
                    if(!player || console) Printf("Mode is set to %08X (was: %08X).", Config::ServerFlags, old_mode);
                    else if(player) zxmgr::SendMessage(player, "mode is set to %08X (was: %08X)", Config::ServerFlags, old_mode);

					if(Config::ServerFlags & SVF_SOFTCORE)
					{
						MAX_SKILL = 110;
						Config::ServerCaps |= SVC_SOFTCORE;
					}
					else
					{
						MAX_SKILL = 100;
						Config::ServerCaps &= ~SVC_SOFTCORE;
					}
                }
                else
                {
                    if(!player || console) Printf("Mode is set to %08X.", Config::ServerFlags);
                    else if(player) zxmgr::SendMessage(player, "mode is set to %08X", Config::ServerFlags);
                }
            }

            goto ex;
        }
        else if(command.find("#speed") == 0)
        {
            command.erase(0, 6);
            command = Trim(command);
            if(!CheckInt(command)) goto ex;
            uint32_t speed = StrToInt(command);
            if(!speed) speed = 1;
            if(speed > 8) speed = 8;

			zxmgr::SetSpeed(speed);
            goto ex;
        }
		else if(rawcmd == "#mapwide")
		{
			if(!player || console)
			{
				Printf("This command may not be used from the console.");
				goto ex;
			}

			uint32_t what = 7; // blizzard
			
			command.erase(0, 8);
			command = Trim(command);
			if(CheckInt(command) && command.length()) what = StrToInt(command);

			byte* unit = *(byte**)(player + 0x38);
			if(!unit) goto ex;

			uint8_t p_x = *(uint8_t*)(*(byte**)(unit + 0x10));
            uint8_t p_y = *(uint8_t*)(*(byte**)(unit + 0x10) + 1);

            uint32_t p_mapwidth = *(uint32_t*)(*(uint32_t*)(0x006B16A8) + 0x50000);
            uint32_t p_mapheight = *(uint32_t*)(*(uint32_t*)(0x006B16A8) + 0x50004);

			uint32_t step_x = 1;
			uint32_t step_y = 1;

			switch(what)
			{
			case 7: // blizzard
				step_x = 1;
				step_y = 1;
				break;
			case 14: // darkness
			case 15: // light
			case 6: // poison cloud
				step_x = 1;
				step_y = 1;
				break;
			case 9: // acid steam
				step_x = 1;
				step_y = 1;
				break;
			case 2: // fire ball
				step_x = 1;
				step_y = 1;
				break;
			case 3: // fire wall
				step_x = 1;
				step_y = 1;
				break;
			default: // any other spell is forbidden
				goto ex;			
			}

			uint32_t count_x = 51;
			uint32_t count_y = 51;

			int32_t start_x = p_x - 25;
			int32_t start_y = p_y - 25;

			for(int32_t i = start_x; i <= start_x+count_x; i++)
			{
				for(int32_t j = start_y; j <= start_y+count_y; j++)
				{
					if(i < 8 || i > p_mapwidth-8 ||
						j < 8 || j > p_mapheight-8) continue;
					zxmgr::CastPointEffect(unit, i, j, what);
				}
			}
 
			goto ex;
		}
		else if(rawcmd == "#inn")
		{
			if(!player) goto ex;
			byte* unit = *(byte**)(player + 0x38);
			if(!unit) goto ex;
			byte* item = *(byte**)(unit + 0x74);
			if(!item) goto ex;
			zxmgr::SendMessage(NULL, "%04x", *(uint16_t*)(item+0x40));
			goto ex;
		}
		else if(rawcmd == "#uh")
		{
			if(!player) goto ex;
			SOCKET ps = zxmgr::GetSocket(player);

			Packet testp;
			testp.WriteUInt32(0xBADFACE1);
			SOCK_SendPacket(ps, testp, 0, true);
			goto ex;
		}
    }

    if(rights & GMF_CMD_CREATE)
    {
        if(rawcmd == "#create")
        {
            if(!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            if(!*(byte**)(player + 0x38)) goto ex;

            command.erase(0, 7);
            command = Trim(command);

            uint32_t count = 1;
            size_t fsp = command.find_first_of(" ");
            if(fsp != std::string::npos)
            {
                std::string intstr = command;
                intstr.erase(fsp);
                if(CheckInt(intstr))
                {
                    command.erase(0, fsp + 1);
                    command = TrimLeft(command);
                    count = StrToInt(intstr);
                    if(!count) count = 1;
                }
            }

            if(ToLower(command) == "gold")
            {
                zxmgr::GiveMoney(player, count, 1);
            }
            else
            {
                if(command.find("{") != std::string::npos)
                {
                    for(uint32_t i = 0; i < count; i++)
					{
						byte* t_item = zxmgr::ConstructItem(command);
						if(!t_item) goto ex;
						// special case for scrolls/potions
						if(*(uint8_t*)(t_item + 0x58) == 0 && *(uint8_t*)(t_item + 0x45) == 0 && *(uint8_t*)(t_item + 0x46) == 0 && *(uint16_t*)(t_item + 0x4A) == 1)
						{
							*(uint16_t*)(t_item + 0x42) = count;
							zxmgr::GiveItemTo(t_item, player);
							break;
						}

						*(uint16_t*)(t_item + 0x42) = 1;
	                    zxmgr::GiveItemTo(t_item, player);
					}
                }
                else
                {
					byte* t_item = zxmgr::ConstructItem(command);
					if(!t_item) goto ex;
                    *(uint16_t*)(t_item + 0x42) = count;
                    zxmgr::GiveItemTo(t_item, player);
                }
            }

            zxmgr::UpdateUnit(*(byte**)(player + 0x38), player, 0xFFFFFFFF, 0xFFB, 0, 0);
            goto ex;
        }
    }

	// #modify player +god:
	//  включает годмод. годмод сбрасывается при выходе игрока с карты ИЛИ при выходе установившего ГМа с карты.
	// #modify player ++god: годмод НЕ сбрасывается при выходе ГМа с карты.
	// #modify player -god (--god): отменяет команды выше. количество минусов в данном случае значения не имеет.
	// #modify player +spells: временно добавляет все заклинания. при выходе игрока или установившего ГМа с карты заклинания
    //  сбрасываются обратно на старую книгу. также можно сбросить заклинания через #modify player -spells.
	// #modify player -spells: если книга заклинаний игрока не менялась, временно удаляет все заклинания.
	//  при выходе игрока или ГМа с карты заклинания возвращаются в норму. эффект отменяется через #modify player +spells.
	// #modify player ++spells: то же самое, но навсегда. сбросить заклинания обратно невозможно.
	// #modify player --spells: см. выше про -spells.

	if(rights & GMF_CMD_MODIFY)
	{
		if(rawcmd == "#modify")
		{
			command.erase(0, 7);
			command = Trim(command);

			byte* target = NULL;
			std::string targetname = "";
			for(size_t i = 0; i < command.length(); i++)
			{
				targetname += command[i];
				if(targetname == "self") continue;
				target = zxmgr::FindByNickname(targetname.c_str());
				if(target) break;
			}
			
			if(!target)
			{
				std::string lowercommand = ToLower(command);
				if(lowercommand.find("self") == 0)
				{
					targetname = "self";
					target = player;
				}
				else goto ex;
			}

			Player* pi = PI_Get(target);
			if(!pi) goto ex;

			command.erase(0, targetname.length());
			command = TrimLeft(command);
			if(!command.length()) goto ex;

			int r_change = 0;
			if(command[0] == '+') r_change = 1;
			else if(command[0] == '-') r_change = -1;
			if(!r_change) goto ex;

			command.erase(0, 1);
			if(!command.length()) goto ex;
			if(command[0] == '+') r_change = 2;
			else if(command[0] == '-') r_change = -2;
			
			if(r_change == 2 || r_change == -2)
				command.erase(0, 1);

			command = ToLower(TrimLeft(command));

			byte* unit = *(byte**)(target + 0x38);
			
			if(command == "god")
			{
				if(r_change > 0)
				{
					pi->GodMode = true;
					if(r_change == 1) pi->GodSetter = player;
					else pi->GodSetter = NULL;
				}
				else if(r_change < 0)
				{
					pi->GodMode = false;
					pi->GodSetter = NULL;
				}
			}
			else if(command == "spells" && unit)
			{
				if(r_change > 0)
				{
					if(pi->SetSpells == -1 && r_change != 2) // prev. command removed spells
					{
						pi->SetSpells = 0;
						zxmgr::SetSpells(unit, pi->LastSpells);
						pi->LastSpells = 0;
					}
					else
					{
						if(r_change == 1)
						{
							pi->SetSpells = 1;
							pi->SpellSetter = player;
						}
						else
						{
							pi->SetSpells = 0;
							pi->SpellSetter = NULL;
						}

						pi->LastSpells = zxmgr::GetSpells(unit);
						zxmgr::SetSpells(unit, 0xFFFFFFFF);
					}
				}
				else if(r_change < 0)
				{
					if(pi->SetSpells == 1 && r_change != -2) // prev. command added spells
					{
						pi->SetSpells = 0;
						zxmgr::SetSpells(unit, pi->LastSpells);
						pi->LastSpells = 0;
					}
					else
					{
						if(r_change == -1)
						{
							pi->SetSpells = 1;
							pi->SpellSetter = player;
						}
						else
						{
							pi->SetSpells = 0;
							pi->SpellSetter = NULL;
						}

						pi->SetSpells = -1;
						pi->LastSpells = zxmgr::GetSpells(unit);
						zxmgr::SetSpells(unit, 0);
					}
				}

				zxmgr::UpdateUnit(unit, 0, 0xFFFFFFFF, 0xFFB, 0, 0);
			}
			else if(command == "knowledge" && unit)
			{
				if(r_change > 0)
				{

				}
				else if(r_change < 0)
				{

				}
			}
		}
	}

ex:
    return;
}

//#define _DAMAGE_DEBUG

int32_t OnDamage(byte* unit1, byte* unit2, int16_t damage)
{
    if(damage < 0) return 0;
    if(unit2 && (*(uint8_t*)(unit2 + 0x4C) & 8)) return 0;
    int32_t retval = damage;

    byte* player1 = NULL;
    byte* player2 = NULL;
    if(unit1) player1 = *(byte**)(unit1 + 0x14);
    if(unit2) player2 = *(byte**)(unit2 + 0x14);

#ifdef _DAMAGE_DEBUG
    const char* p1name = "(null)";
    const char* p2name = "(null)";
    if(player1) p1name = *(const char**)(player1 + 0x18);
    if(player2) p2name = *(const char**)(player2 + 0x18);
    zxmgr::SendMessage(NULL, "%s -> %d -> %s", p1name, -damage, p2name);
#endif

    uint32_t rights1 = 0;
    uint32_t rights2 = 0;

    // PvM
    if(Config::ServerFlags & SVF_PVM)
    {
        if((player1 && !*(uint32_t*)(player1 + 0x2C)) &&
           (player2 && !*(uint32_t*)(player2 + 0x2C))) retval = 0;
    }

    // Advanced PvM
    if(Config::ServerFlags & SVF_ADVPVM)
    {
       /*if((player1 && !*(uint32_t*)(player1 + 0x2C)) &&
           (player2 && !*(uint32_t*)(player2 + 0x2C)) &&
           (vd2_CheckStrong(unit1) !=
            vd2_CheckStrong(unit2))) retval = 0;*/
		if(VerifyDamage2(unit1, unit2)) retval = 0;
    }

    if(player1 && CHECK_FLAG(*(uint32_t*)(player1 + 0x14), GMF_ANY))
    {
        retval = damage;
        rights1 = *(uint32_t*)(player1 + 0x14) & 0xFFFFFF;
        if(*(uint32_t*)(player1 + 0x2C)) rights1 = 0;
    }

    if(player2 && CHECK_FLAG(*(uint32_t*)(player2 + 0x14), GMF_ANY))
    {
        rights2 = *(uint32_t*)(player2 + 0x14) & 0xFFFFFF;
        if(*(uint32_t*)(player2 + 0x2C)) rights1 = 0;
    }

    if(rights1 & GMF_MAXDAMAGE) retval = 32767;

    // God mode
    if(((rights2 & GMF_GODMODE) &&
        (!(rights1 & GMF_GODMODE_ADMIN) || (rights2 & GMF_GODMODE_ADMIN))) &&
        (player2 != player1)) retval = 0;

    if(((rights2 & GMF_GODMODE) ||
        (rights2 & GMF_GODMODE_ADMIN)) &&
            (player1 == player2) &&
            (unit2 == *(byte**)(player2 + 0x38))) retval = 0;

    // todo somewhere around: temporary god mode
	Player* pi = NULL;
	if(player2 && (pi = PI_Get(player2)))
	{
		if(pi->GodMode &&	// tmp. god mode set
			(!player1 ||	// cast from nowhere (building/trigger)
			 !CHECK_FLAG(*(uint32_t*)(player1 + 0x14), GMF_ANY))) // or damage from regular player
				retval = 0;
	}
	
    return retval;
}

void __declspec(naked) imp_DropEverything()
{
	__asm
	{
		push	0
		mov		eax, [ebp-0x164]
		push	eax
		call	DropEverything
		add		esp, 4
		mov		edx, 0x00642C2C
		mov		edx, [edx]
		cmp		dword ptr [edx+0x74], 0
		mov		eax, 0x0052E754
		jmp		eax
	}
}