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
#include "reborn_readiness.hpp"
#include <math.h>
#include "quests.h"
#include "scanrange.h"
#include "screenshots.h"
#include "a2types.h"


uint32_t ParseFlags(std::string string)
{
    if (CheckHex(string))
        return HexToInt(string);
    else
    {
        uint32_t flags = 0;
        std::vector<std::string> v = Explode(string, "|");

        for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
        {
            std::string par = ToLower(Trim((*it)));

            if (!par.length())
                continue;

            uint32_t flag = 0;
            bool erase = false;

            if (par[0] == '-')
            {
                erase = true;
                par.erase(0, 1);
            }

            if (par == "pvm")
                flag = SVF_PVM;
            else if (par == "muted")
                flag = SVF_MUTED;
            else if (par == "closed")
                flag = SVF_CLOSED;
            else if (par == "advanced_pvm")
                flag = SVF_ADVPVM;
            else if (par == "nohealing")
                flag = SVF_NOHEALING;
            else if (par == "noobsrv")
                flag = SVF_NOOBSRV;
            else if (par == "softcore")
                flag = SVF_SOFTCORE;
            else if (par == "nodrop")
                flag = SVF_NODROP;
            else if (par == "fnodrop")
                flag = SVF_FNODROP;
            else if (par == "nosaving")
                flag = SVF_NOSAVING;
            else if (par == "sandbox")
                flag = SVF_SANDBOX;
            else if (par == "entermage")
                flag = SVF_ENTERMAGE;
            else if (par == "enterwarrior")
                flag = SVF_ENTERWARRIOR;

            if (!flag)
                continue;

            if (!erase)
                flags |= flag;
            else
                flags &= ~flag;
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


void DropEverything(A2Unit* unit, bool full = false)
{
    if (Config::GameMode != 0) return;

    uint32_t p_x = unit->position->x;
    uint32_t p_y = unit->position->y;

    uint32_t spellmask = zxmgr::GetSpells(unit);

    for (int i = 0; i < 32; i++)
    {
        if ((1 << i) & spellmask && strlen(SpellNames[i][0]) && SpellNames[i][1][0])
        {
            std::string cstrp = Format("Book %s {teachSpell=%s}", SpellBooks[SpellNames[i][1][0]], SpellNames[i][0]);
            zxmgr::CreateSack(cstrp.c_str(), p_x, p_y, 0);
        }
    }

    zxmgr::SetSpells(unit, 0);
    zxmgr::UpdateUnit(unit, NULL, 0xA31FFFFF, 0xFFB, 0, 0);

    if (!full) return;

    A2Player* player = unit->player;

    if (!player) return;

    uint32_t money = player->money;

    if (!money) return;

    if (money > 0x7FFFFFFF) money = 0x7FFFFFFF;

    zxmgr::CreateSack(NULL, p_x, p_y, money);
    player->money = 0;
    zxmgr::GiveMoney(player, 0, 0);
}

void ProcessCheat_Quest(A2Player* player, const std::string& args) {
	if (!Config::AllowQuestFilters) {
		zxmgr::SendMessage(player, "Quest filtering is disabled");
		return;
	}

	short player_id = player->id_ext.id;

	std::string filter = NormalizeMobName(TrimLeft(args).c_str());
	int mob_count = 0;

	int spacepos = filter.find(' ');
    if (spacepos != std::string::npos) {
		std::string raw_count = TrimLeft(filter.substr(spacepos+1));
		filter.erase(spacepos);

		if (!CheckInt(raw_count)) {
			zxmgr::SendMessage(player, "'%s' is not an integer, ignored", raw_count.c_str());
		} else {
			mob_count = static_cast<int>(StrToInt(raw_count));
		}
	}

	if (filter.length() > 0) {
		int matching_mobs = 0;

		InitializeMobNames();

		for (auto mob = mob_names_by_server_id_normed.begin(); mob != mob_names_by_server_id_normed.end(); mob++) {
			if (mob->second.find(filter) != std::string::npos) {
				matching_mobs++;
			}
		}
			
		if (matching_mobs == 0) {
			zxmgr::SendMessage(player, "'%s' does not match any mobs, quest filter unchanged", filter.c_str());
			return;
		}
	}

	player_settings[player_id]->quest_filter = filter;
	player_settings[player_id]->quest_mob_count = mob_count;
    player_settings[player_id]->player_name = player->name;

	if (filter.length() > 0) {
		if (mob_count > 0) {
			zxmgr::SendMessage(player, "Quest filter set to '%s', mob count set to %d", filter.c_str(), mob_count);
		} else {
			zxmgr::SendMessage(player, "Quest filter set to '%s'", filter.c_str());
		}
	} else {
		zxmgr::SendMessage(player, "Quest filter reset");
	}
}

void ProcessCheat_QuestsInfo(A2Player* player, const std::string& args) {
	short player_id = player->id_ext.id;

	zxmgr::SendMessage(player, "Current player: %d", player_id);
	for (auto it = player_settings.begin(); it != player_settings.end(); ++it) {
		if (!it->second->quest_filter.empty()) {
			zxmgr::SendMessage(player, "Quest filter for %d: '%s'", it->first, it->second->quest_filter.c_str());
		}
	}

	if (mob_names.get()) {
		zxmgr::SendMessage(player, "Total mob names: %d", mob_names->size());
	}
}

void ProcessCheat_QuestState(A2Player* player, const std::string& args) {
	short player_id = player->id_ext.id;

	std::vector<std::string> messages = QuestStateNMonsters(player);

	for (auto it = messages.begin(); it != messages.end(); ++it) {
		zxmgr::SendMessage(player, "%s", it->c_str());
	}
}

std::unordered_map<std::string, uint32_t> autobuff_spells{
    {"heal", 0x18},
    {"haste", 0x15},
    {"bless", 0x14},
    {"shield", 0x1B},
    {"invisibility", 0x0C},
    {"fire protection", 0x04},
    {"water protection", 0x08},
    {"air protection", 0x0D},
    {"earth protection", 0x13},
};

void ProcessCheat_Autobuff(A2Player* player, const std::string& args) {
    short player_id = player->id_ext.id;

    CheckPlayerSettings(player);
    player_settings[player_id]->player_name = player->name;

    std::string command = ToLower(Trim(args));

    if (command.empty()) {
        player_settings[player_id]->autobuff_mask = 0;
        zxmgr::SendMessage(player, "Autobuff mask reset, all spells are allowed");
        return;
    }

    // The only disambiguation needed is for `h`. Let's do `heal`, maybe
    // somebody will find something fun with turning off healing.
    if (command == "h") {
        command = "heal";
    }

    for (auto it = autobuff_spells.begin(); it != autobuff_spells.end(); ++it) {
        if (it->first.rfind(command, 0) == 0) { // Spell starts with the command
            auto mask = 1 << it->second;
            player_settings[player_id]->autobuff_mask ^= mask;
            if ((player_settings[player_id]->autobuff_mask) & mask) {
                zxmgr::SendMessage(player, "Filtered out *%s*, you will not cast it during autobuff", it->first.c_str());
            } else {
                zxmgr::SendMessage(player, "You will cast *%s* during autobuff normally", it->first.c_str());
            }
            
            return;
        }
    }

    zxmgr::SendMessage(player, "'%s' does not match any spells. Autobuff mask left unchanged.", command.c_str());
}

void RunCommand(byte* _this, A2Player* player, const char* ccommand, uint32_t rights, bool console)
{
    if (!ccommand) return;
    if (ccommand[0] != '#') return;

    std::string command(ccommand);
    command = Trim(command);

    std::string rawcmd = command;
	std::string args;

    size_t spacepos = command.find_first_of(' ');

    if (spacepos != std::string::npos) {
        rawcmd.erase(spacepos);
		args = command.substr(spacepos+1);
	}

    if (rawcmd == "#mapinfo")
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

        if (!Config::Suspended && zxmgr::GetTotalMapTime() != 0x7FFFFFFF)
            time_left = Format("%u:%02u:%02u", tt_h, tt_m, tt_s);

        if (player)
            zxmgr::SendMessage(player, "map: \"%s\" in \"%s\", time elapsed: %u:%02u:%02u, time total: %s",
                               map_name, map_file, tm_h, tm_m, tm_s, time_left.c_str());
        else if (console)
            Printf("Map: \"%s\" in \"%s\", time elapsed: %u:%02u:%02u, time total: %s",
                   map_name, map_file, tm_h, tm_m, tm_s, time_left.c_str());

        goto ex;
	}
	
	if (rawcmd == "#quest" || rawcmd == "#q") {
		ProcessCheat_Quest(player, args);
		return;
	}
	
	if (rawcmd == "#quests_info") {
		ProcessCheat_QuestsInfo(player, args);
		return;
	}

	if (rawcmd == "#quest_state" || rawcmd == "#qs") {
		ProcessCheat_QuestState(player, args);
		return;
	}

    if (rawcmd == "#reborn") {
        return RebornReadinessInfo(Config::ServerID, player, false);
    }

    if (rawcmd == "#hell") {
        return RebornReadinessInfo(Config::ServerID, player, true);
    }

    if (rawcmd == "#autobuff" || rawcmd == "#ab") {
        ProcessCheat_Autobuff(player, args);
    }

    if (rights & GMF_CMD_CHAT)
    {
        if (command.find("#@") == 0)
        {
            command.erase(0, 2);
            std::string what = "";

            if (!player || console)
                what = Format("[broadcast] <server%u>: %s", Config::ServerID, command.c_str());
            else if (player)
                what = Format("[broadcast] %s: %s", player->name, command.c_str());
            else
                what = Format("[broadcast] %s", command.c_str());

            if (!NetCmd_Broadcast(what))
                NetHat::Connected = false;

            goto ex;
        }
        else if (command.find("#!") == 0)
        {
            command.erase(0, 2);

            if (!player || console)
                zxmgr::SendMessage(NULL, "<server>: %s", command.c_str());
            else if (player)
                zxmgr::SendMessage(NULL, "%s: %s", player->name, command.c_str());
            else
                zxmgr::SendMessage(NULL, command.c_str());

            goto ex;
        }
    }

    if(rights & GMF_CMD_KICK)
    {
        if (rawcmd == "#kick")
        {
            command.erase(0, 5);
            command = TrimLeft(command);
            A2Player* target = zxmgr::FindByNickname(command.c_str());

            if (target)
                zxmgr::Kick(target, false);

            goto ex;
        }
        else if (rawcmd == "#kickme")
        {
            if (!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            if (player)
                zxmgr::Kick(player, true);

            goto ex;
        }
        else if (rawcmd == "#kickall")
        {
            zxmgr::KickAll(NULL);
            goto ex;
        }
        else if (rawcmd == "#kick_silent")
        {
            command.erase(0, 12);
            command = TrimLeft(command);
            A2Player* target = zxmgr::FindByNickname(command.c_str());

            if (target)
                zxmgr::Kick(target, true);

            goto ex;
        }
        else if (rawcmd == "#disconnect")
        {
            command.erase(0, 11);
            command = TrimLeft(command);
            A2Player* target = zxmgr::FindByNickname(command.c_str());

            if (target)
                zxmgr::Disconnect(target);

            goto ex;
        }
    }

    if (rights & GMF_CMD_INFO)
    {
        if (rawcmd == "#locate")
        {
            command.erase(0, 7);
            command = TrimLeft(command);
            A2Player* target = zxmgr::FindByNickname(command.c_str());

            if (target && target->current_unit)
            {
                uint32_t p_x = target->current_unit->position->x;
                uint32_t p_y = target->current_unit->position->y;

                if (!player || console)
                    Printf("%s (%u:%u)", target->name, p_x, p_y);
                else if (player)
                    zxmgr::SendMessage(player, "%s (%u:%u)", target->name, p_x, p_y);
            }
            goto ex;
        }
        else if (rawcmd == "#info")
        {
            command.erase(0, 5);
            command = TrimLeft(command);
            A2Player* target = zxmgr::FindByNickname(command.c_str());

            if (!target) goto ex;
            if (!target->current_unit) goto ex;

            const char* p_charname = target->name;
            const char* p_logname = target->account_name;
            byte* netinf = zxmgr::GetNetworkStruct(target);
            bool p_connected = (netinf);
            const char* p_address = "n/a";

            if (p_connected)
                p_address = (const char*)(netinf + 8);

            std::string p_state;

            if (p_connected)
                p_state = Format("connected (%s)", p_address);
            else
                p_state = "disconnected";

            A2Unit* unit = target->current_unit;

            uint16_t p_body      = unit->body;
            uint16_t p_reaction  = unit->reaction;
            uint16_t p_mind      = unit->mind;
            uint16_t p_spirit    = unit->spirit;

            const char* p_strong = (vd2_CheckStrong(unit) ? "yes" : "no");

            if (!player || console)
            {
                Printf("nickname: \"%s\", login: \"%s\", state: %s, stats: [%u,%u,%u,%u], strong: %s",
                       p_charname, p_logname, p_state.c_str(),
                       p_body, p_reaction, p_mind, p_spirit, p_strong);
            }
            else if (player)
            {
                zxmgr::SendMessage(player, "nickname: \"%s\", login: \"%s\", state: %s, stats: [%u,%u,%u,%u], strong: %s\n",
                                   p_charname, p_logname, p_state.c_str(),
                                   p_body, p_reaction, p_mind, p_spirit, p_strong);
            }

            goto ex;
        }
        else if (rawcmd == "#scan")
        {
            Printf("#scan command entered.\n");
            if (!player) goto ex;
            Printf("#scan performed. player found.\n");
            A2Unit* unit = player->current_unit;
            if (!unit) goto ex;
            Printf("check for unit OK.. getting dump\n");         

            SR_DumpToFile(player);
            Printf("dump finished\n");         
            goto ex;
        }
    }

    if (rights & GMF_CMD_KILL)
    {
        if (rawcmd == "#kill")
        {
            command.erase(0, 5);
            command = TrimLeft(command);
            A2Player* target = zxmgr::FindByNickname(command.c_str());

            if (target)
            {
                uint32_t tri = 0;
                if (!target->unitType)
                    tri = target->flags;

                if (CHECK_FLAG(tri, GMF_ANY))
                {
                    if (((tri & GMF_GODMODE) &&
                        (rights & GMF_GODMODE_ADMIN)) ||
                        (tri & GMF_GODMODE_ADMIN)) goto ex;
                }

                zxmgr::Kill(target, player);
            }
            goto ex;
        }
        else if (rawcmd == "#murder")
        {
            command.erase(0, 7);
            command = TrimLeft(command);
            A2Player* target = zxmgr::FindByNickname(command.c_str());

            if (target)
            {
                uint32_t tri = 0;
                if (!target->unitType)
                    tri = target->flags;

                if (CHECK_FLAG(tri, GMF_ANY))
                {
                    if ((((tri & GMF_GODMODE) && !(rights & GMF_GODMODE_ADMIN)) ||
                       (tri & GMF_GODMODE_ADMIN)) && target != player)
                            goto ex;
                }

                zxmgr::Kill(target, player);
                A2Unit* unit = target->current_unit;

                if (unit && target != player)
                    DropEverything(unit, true);
            }
            goto ex;
        }
        else if (rawcmd == "#killall")
        {
            zxmgr::KillAll(player, false);
        }
        else if (rawcmd == "#killai")
        {
            zxmgr::KillAll(player, true);
        }
    }

    if (rights & GMF_CMD_PICKUP)
    {
        if (rawcmd == "#pickup")
        {
            if (!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            if (player)
            {
                cheat_codes_2(reinterpret_cast<byte*>(player), ccommand);
            }
            goto ex;
        }
    }

    if (rights & GMF_CMD_SUMMON)
    {
        if (rawcmd == "#summon")
        {
            if (!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            if (player && player->current_unit)
            {
                command.erase(0, 7);
                command = Trim(command);
                if (!command.length()) goto ex;

                uint32_t count = 1;
                size_t fsp = command.find_first_of(" ");
                if (fsp != std::string::npos)
                {
                    std::string intstr = command;
                    intstr.erase(fsp);
                    if (CheckInt(intstr))
                    {
                        command.erase(0, fsp + 1);
                        command = TrimLeft(command);
                        count = StrToInt(intstr);
                        if (!count) count = 1;
                    }
                }

                if (!command.length()) goto ex;
                for (uint32_t i = 0; i < count; i++)
                {
                    A2Unit* unit = zxmgr::Summon(player, command.c_str(), *(byte**)(0x00642C2C), false, NULL);
                    /*if (rights & GMF_UNITS_NOCLIP)
                        zxmgr::MakeUnitNoClip(unit);*/
                    //byte* unit = Map::CreateUnitForEx(player, command.c_str(), false);
                    //zxmgr::SendMessage(NULL, "what: %02X\n", *(uint8_t*)(*(byte**)(unit + 0x1C4) + 0x78));
                    //Unit::SetSpells(unit);
                }
            }
        }
    }

    if (rights & GMF_CMD_SET)
    {
        if (rawcmd == "#nextmap")
        {
            zxmgr::NextMap();
            goto ex;
        }
        else if (rawcmd == "#prevmap")
        {
            zxmgr::PrevMap();
            goto ex;
        }
        else if (rawcmd == "#resetmap")
        {
            if (Config::Suspended)
            {
                zxmgr::SetTotalMapTime(Config::OriginalTime);
                Config::Suspended = false;
            }
            zxmgr::ResetMap();
            goto ex;
        }
        else if (rawcmd == "#shutdown")
        {
            TerminateProcess(GetCurrentProcess(), 100);
            goto ex;
        }
        else if (rawcmd == "#suspend")
        {
            if (!Config::Suspended)
            {
                Config::OriginalTime = zxmgr::GetTotalMapTime();
                zxmgr::SetTotalMapTime(0x7FFFFFFF);

                if (!player || console)
                    Printf("Map timer suspended.");
                else if (player)
                    zxmgr::SendMessage(player, "suspend: map timer suspended.");
            }
            else
            {
                bool change_map = false;
                uint32_t cur_time = zxmgr::GetCurrentMapTime() / 60000;
                if (cur_time >= Config::OriginalTime) change_map = true;

                zxmgr::SetTotalMapTime(Config::OriginalTime);
                Config::OriginalTime = 0;

                if (!player || console)
                    Printf("Map timer released.");
                else if (player)
                    zxmgr::SendMessage(player, "suspend: map timer released.");

                if (change_map)
                {
                    Printf("Note: map timer beyond total map time. Changing map...");
                    zxmgr::NextMap();
                }
            }

            Config::Suspended = !Config::Suspended;
            goto ex;
        }
        else if (rawcmd == "#set" && (command.find("#set mode") == 0))
        {
            command.erase(0, 9);
            command = Trim(command);

            if (!command.length())
            {
                if (!player || console)
                    Printf("Mode is set to %08X.", Config::ServerFlags);
                else if (player)
                    zxmgr::SendMessage(player, "mode is set to %08X", Config::ServerFlags);
            }
            else
            {
                int32_t add_flags = 0;
                if (command[0] == '+') add_flags = 1;
                else if (command[0] == '-') add_flags = -1;
                else if (command[0] == '=') add_flags = 0;
                else goto ex;

                if (add_flags != 0) command.erase(0, 1);
                if (command[0] != '=') goto ex;
                command.erase(0, 1);

                uint32_t old_mode = Config::ServerFlags;

                uint32_t flags = ParseFlags(TrimLeft(command));
                if (add_flags == -1) Config::ServerFlags &= ~flags;
                else if (add_flags == 1) Config::ServerFlags |= flags;
                else Config::ServerFlags = flags;

                if (old_mode != Config::ServerFlags)
                {
                    if (!player || console)
                        Printf("Mode is set to %08X (was: %08X).", Config::ServerFlags, old_mode);
                    else if (player)
                        zxmgr::SendMessage(player, "mode is set to %08X (was: %08X)", Config::ServerFlags, old_mode);

                    if (Config::ServerFlags & SVF_SOFTCORE)
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
                    if (!player || console)
                        Printf("Mode is set to %08X.", Config::ServerFlags);
                    else if (player)
                        zxmgr::SendMessage(player, "mode is set to %08X", Config::ServerFlags);
                }
            }

            goto ex;
        }
        else if (command.find("#speed") == 0)
        {
            command.erase(0, 6);
            command = Trim(command);
            if (!CheckInt(command)) goto ex;
            uint32_t speed = StrToInt(command);
            if (!speed) speed = 1;
            if (speed > 8) speed = 8;

            zxmgr::SetSpeed(speed);
            goto ex;
        }
        else if (rawcmd == "#mapwide")
        {
            if (!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            uint32_t what = 7; // blizzard
            
            command.erase(0, 8);
            command = Trim(command);

            if (CheckInt(command) && command.length())
                what = StrToInt(command);

            A2Unit* unit = player->current_unit;
            if (!unit) goto ex;

            uint8_t p_x = unit->position->x;
            uint8_t p_y = unit->position->y;

            uint32_t p_mapwidth = *(uint32_t*)(*(uint32_t*)(0x006B16A8) + 0x50000);
            uint32_t p_mapheight = *(uint32_t*)(*(uint32_t*)(0x006B16A8) + 0x50004);

            uint32_t step_x = 1;
            uint32_t step_y = 1;

            switch (what)
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

            for (uint32_t i = start_x; i <= start_x+count_x; i++)
            {
                for (uint32_t j = start_y; j <= start_y+count_y; j++)
                {
                    if (i < 8 || i > p_mapwidth - 8 ||
                        j < 8 || j > p_mapheight - 8) continue;
                    zxmgr::CastPointEffect(unit, i, j, what);
                }
            }
 
            goto ex;
        }
        else if(rawcmd == "#inn")
        {
            if (!player) goto ex;
            A2Unit* unit = player->current_unit;
            if (!unit) goto ex;
            A2InventoryItem* item = unit->weapon;
            if (!item) goto ex;

            zxmgr::SendMessage(NULL, "%04x", item->id);

            goto ex;
        }
    }

    if (rights & GMF_CMD_SCREENSHOT)
    {
        if (rawcmd == "#screenshot")
        {

            command.erase(0, 11);
            command = TrimLeft(command);
            A2Player* target = zxmgr::FindByNickname(command.c_str());
            if (!target)
            {
                if (player) zxmgr::SendMessage(player, "screenshot: Player %s not found", command.c_str());
                else Printf("screenshot: Player %s not found", command.c_str());
                goto ex;
            }

            Player* p = PI_Get(target);
            if (!p)
            {
                if (player) zxmgr::SendMessage(player, "screenshot: Player %s has no playerinfo!", target->name);
                else Printf("screenshot: Player %s has no playerinfo!", target->name);
                goto ex;
            }

            SOCKET ps = zxmgr::GetSocket(target);
            if (!ps)
            {
                if (player) zxmgr::SendMessage(player, "screenshot: Player %s has no socket (AI or disconnected?)", target->name);
                else Printf("screenshot: Player %s has no socket (AI or disconnected?)", target->name);
                goto ex;
            }

            uint32_t uid = ClientScreenshot_Enqueue(player, target);
            Packet cmd;
            cmd.WriteUInt8(0x01);
            cmd.WriteString(target->account_name);
            cmd.WriteUInt32(uid);
            p->EnqueuedPackets.push_back(cmd);

            if (player) zxmgr::SendMessage(player, "screenshot: Request sent to player %s", target->name, uid);
            else Printf("screenshot: Request sent to player %s", target->name, uid);

            goto ex;
        }
    }

    if(rights & GMF_CMD_CREATE)
    {
        if (rawcmd == "#create")
        {
            if (!player || console)
            {
                Printf("This command may not be used from the console.");
                goto ex;
            }

            if (!player->current_unit) goto ex;

            command.erase(0, 7);
            command = Trim(command);

            uint32_t count = 1;
            size_t fsp = command.find_first_of(" ");
            if (fsp != std::string::npos)
            {
                std::string intstr = command;
                intstr.erase(fsp);
                if (CheckInt(intstr))
                {
                    command.erase(0, fsp + 1);
                    command = TrimLeft(command);
                    count = StrToInt(intstr);
                    if (!count) count = 1;
                }
            }

            if (ToLower(command) == "gold")
            {
                zxmgr::GiveMoney(player, count, 1);
            }
            else
            {
                if (command.find("{") != std::string::npos)
                {
                    for (uint32_t i = 0; i < count; i++)
                    {
                        A2InventoryItem* t_item = zxmgr::ConstructItem(command);
                        if (!t_item) goto ex;

                        A2Armor* armor = reinterpret_cast<A2Armor*>(t_item); // To check the slot. Should work with all item types, I guess.

                        // special case for scrolls/potions
                        if (armor->slot == 0 && t_item->shape == 0 && t_item->material == 0 && t_item->weight == 1)
                        {
                            t_item->amount = count;
                            zxmgr::GiveItemTo(t_item, player);
                            break;
                        }

                        *(uint16_t*)(t_item + 0x42) = 1;
                        zxmgr::GiveItemTo(t_item, player);
                    }
                }
                else
                {
                    A2InventoryItem* t_item = zxmgr::ConstructItem(command);
                    if (!t_item) goto ex;
                    t_item->amount = count;
                    zxmgr::GiveItemTo(t_item, player);
                }
            }

            zxmgr::UpdateUnit(player->current_unit, player, 0xFFFFFFFF, 0xFFB, 0, 0);
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

    if (rights & GMF_CMD_MODIFY)
    {
        if (rawcmd == "#modify")
        {
            command.erase(0, 7);
            command = Trim(command);

            A2Player* target = NULL;
            std::string targetname = "";
            for (size_t i = 0; i < command.length(); i++)
            {
                targetname += command[i];
                if (targetname == "self") continue;
                target = zxmgr::FindByNickname(targetname.c_str());
                if (target) break;
            }
            
            if (!target)
            {
                std::string lowercommand = ToLower(command);
                if (lowercommand.find("self") == 0)
                {
                    targetname = "self";
                    target = player;
                }
                else goto ex;
            }

            Player* pi = PI_Get(target);
            if (!pi) goto ex;

            command.erase(0, targetname.length());
            command = TrimLeft(command);
            if (!command.length()) goto ex;

            int r_change = 0;
            if (command[0] == '+') r_change = 1;
            else if (command[0] == '-') r_change = -1;
            if (!r_change) goto ex;

            command.erase(0, 1);
            if (!command.length()) goto ex;
            if (command[0] == '+') r_change = 2;
            else if (command[0] == '-') r_change = -2;
            
            if (r_change == 2 || r_change == -2)
                command.erase(0, 1);

            command = ToLower(TrimLeft(command));

            A2Unit* unit = target->current_unit;
            
            if (command == "god")
            {
                if (r_change > 0)
                {
                    pi->GodMode = true;
                    if (r_change == 1) pi->GodSetter = player;
                    else pi->GodSetter = NULL;
                }
                else if (r_change < 0)
                {
                    pi->GodMode = false;
                    pi->GodSetter = NULL;
                }
            }
            else if (command == "spells" && unit)
            {
                if (r_change > 0)
                {
                    if (pi->SetSpells == -1 && r_change != 2) // prev. command removed spells
                    {
                        pi->SetSpells = 0;
                        zxmgr::SetSpells(unit, pi->LastSpells);
                        pi->LastSpells = 0;
                    }
                    else
                    {
                        if (r_change == 1)
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
                else if (r_change < 0)
                {
                    if (pi->SetSpells == 1 && r_change != -2) // prev. command added spells
                    {
                        pi->SetSpells = 0;
                        zxmgr::SetSpells(unit, pi->LastSpells);
                        pi->LastSpells = 0;
                    }
                    else
                    {
                        if (r_change == -1)
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
            else if (command == "knowledge" && unit)
            {
                if (r_change > 0)
                {

                }
                else if (r_change < 0)
                {

                }
            }
        }
    }

ex:
    return;
}

//#define _DAMAGE_DEBUG

uint32_t GetDamageBonus(A2Unit* unit) {
    if (!unit) {
        return 0;
    }

    // check if this is human
    if (unit->clazz != A2_CLASS_HUMAN) {
        return 0;
    }

    uint32_t retval = 0;

    if (!unit->inventory) {
        return 0;
    }

    A2Human* human = (A2Human*)unit;

    for (uint32_t i = 1; i <= 12; i++) {
        A2InventoryItem* item = NULL;

        if (i == 1) {
            item = unit->weapon;
        } else if (i == 2) {
            item = unit->shield;
        } else {
            item = human->dress[i];
        }

        if (!item) {
            continue;
        }

        // iterate item stats
        A2Node<A2Effect>* effect = item->effects.first_node;

        while (effect) {
            if (effect->value) {
                if (effect->value->effect_id == 26) {
                    retval += effect->value->value1;
                }
            }
            effect = effect->next;
        }
    }

    return retval;
}

int32_t OnDamage(A2Unit* attacker, A2Unit* victim, int16_t damage) {
    if (damage < 0) {
        return 0;
    }

    if (victim && (victim->unit_attrs & 8)) {
        return 0;
    }

    // calculate damage bonus
    uint32_t damage_bonus = GetDamageBonus(attacker);
    if (damage_bonus && damage > 0) {
        damage = static_cast<int16_t>(double(damage) * (double(damage_bonus) / 100 + 1.0));
    }

    int32_t retval = damage;

    A2Player* attacker_player = attacker ? attacker->player : nullptr;
    A2Player* victim_player = victim ? victim->player : nullptr;
    
    // Damage modificators in PvP
    if (attacker_player && victim_player &&
        !attacker_player->unitType &&
        !victim_player->unitType &&
        attacker_player != victim_player) // Ensure it's not self-inflicted damage
    {
        if (attacker->unit_attrs & 4) { // if mage or witch
            int old_dmg = damage;
            // This method is being called twice, so it needs to reduce the factor like shown below.
            // If the method would be called once, it would be unnecessary
            // factor*x = k*(k*x) => k = sqrt(factor)
            float factor = sqrt(Config::mage_pvp_dmg_factor);
            damage = static_cast<uint16_t>(damage * factor);
            retval = damage;
            Printf("dmg changed: %d -> %d", old_dmg, retval);
        }

        // limit maximum damage dealt to player
        int16_t pvp_dmg_lim = Config::max_pvp_dmg;
        if (damage > pvp_dmg_lim) {
            retval = pvp_dmg_lim;
        }
    }

#ifdef _DAMAGE_DEBUG
    const char* p1name = "(null)";
    const char* p2name = "(null)";
    if (attacker_player) p1name = attacker_player->name;
    if (victim_player) p2name = victim_player->name;
    zxmgr::SendMessage(NULL, "%s -> %d -> %s", p1name, -damage, p2name);
#endif

    uint32_t rights1 = 0;
    uint32_t rights2 = 0;

    // PvM
    if (Config::ServerFlags & SVF_PVM) {
        if((attacker_player && !attacker_player->unitType &&
           (victim_player && !victim_player->unitType)))
                retval = 0;
    }

    // Advanced PvM
    if (Config::ServerFlags & SVF_ADVPVM) {
        if (VerifyDamage2(attacker, victim)) {
            retval = 0;
        }
    }

    if (attacker_player && CHECK_FLAG(attacker_player->flags, GMF_ANY)) {
        retval = damage;
        rights1 = attacker_player->flags & 0xFFFFFF;

        if (attacker_player->unitType)
            rights1 = 0;
    }

    if (victim_player && CHECK_FLAG(victim_player->flags, GMF_ANY)) {
        rights2 = victim_player->flags & 0xFFFFFF;

        if (victim_player->unitType)
            rights1 = 0;
    }

    if (rights1 & GMF_MAXDAMAGE) {
        retval = 32767;
    }

    // God mode
    if (((rights2 & GMF_GODMODE) &&
         (!(rights1 & GMF_GODMODE_ADMIN) || (rights2 & GMF_GODMODE_ADMIN))) &&
         (victim_player != attacker_player)) {
            retval = 0;
    }

    if (((rights2 & GMF_GODMODE) ||
         (rights2 & GMF_GODMODE_ADMIN)) &&
         (attacker_player == victim_player) &&
         (victim == victim_player->current_unit)) {
            retval = 0;
    }

    // todo somewhere around: temporary god mode
    Player* pi = NULL;
    if (victim_player && (pi = PI_Get(victim_player))) {
        if (pi->GodMode &&    // tmp. god mode set
           (!attacker_player ||    // cast from nowhere (building/trigger)
            !CHECK_FLAG(attacker_player->flags, GMF_ANY))) { // or damage from regular player
                retval = 0;
        }
    }

    return retval;
}


void __declspec(naked) imp_DropEverything()
{
    __asm
    {
        push    0
        mov        eax, [ebp-0x164]
        push    eax
        call    DropEverything
        add        esp, 4
        mov        edx, 0x00642C2C
        mov        edx, [edx]
        cmp        dword ptr [edx+0x74], 0
        mov        eax, 0x0052E754
        jmp        eax
    }
}
