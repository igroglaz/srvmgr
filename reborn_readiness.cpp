#include "reborn_readiness.hpp"

#include <cstdint>
#include <string>
#include <map>
#include <vector>

#include "config_new.h"
#include "lib/utils.hpp"
#include "quests.h"
#include "solo.h"
#include "zxmgr.h"

struct PlayerInfo {
    bool warrior;
    bool female;
    bool solo;
    std::string clan;
    int has_treasures;
    int32_t money;
    uint16_t body;
    uint16_t reaction;
    uint16_t mind;
    uint16_t spirit;
    int32_t experience;
    uint8_t* monster_kills_by_server_id;
    uint32_t deaths;
    int skills[5];
};

const int32_t k = 1000;
const int32_t m = k * k;

// Keeping this list up-to-date would be annoying. We should find a way to request this directly from the hat.
std::map<ServerIDType, std::map<int, uint8_t>> girl_needs_monster_kills{
    {EASY, {
        ////////////////////// 1
        {692, 1},  // Necro_Female1
        {616, 1},  // Ogre
        {620, 1},  // Troll
        ////////////////////// 14
        {657, 14}, // F_Zombie.1
        {664, 14}, // F_Skeleton.1
        {668, 14}, // A_Skeleton.1
        {629, 14}, // Ghost.2
        {715, 14}, // Dino
        {632, 14}, // Bee
        {707, 14}, // Spider
    }},
    {KIDS, {
        ////////////////////// 1
        {617, 1},  // Ogre.2
        {621, 1},  // Troll.2
        {2374, 1}, // Demon
        {711, 1},  // Succubus
        ////////////////////// 14
        {630, 14}, // Ghost.3
        {609, 14}, // Orc_Sword.2
        {633, 14}, // Bee.2
        {707, 14}, // Spider
    }},
    {NIVAL, {
        ////////////////////// 1
        {696, 1},  // Necro_Leader2
        {695, 1},  // Necro_Female2
        {694, 1},  // Necro_Male2
        {712, 1},  // Succubus.2
        ////////////////////// 14
        {669, 14}, // A_Skeleton.2
        {661, 14}, // A_Zombie.2
        {617, 14}, // Ogre.2
        {621, 14}, // Troll.2
        {625, 14}, // Bat_Sonic.2
        {708, 14}, // Spider.2
    }},
    {MEDIUM, {
        ////////////////////// 1
        {701, 1},  // Necro_Female4
        {671, 1},  // A_Skeleton.4
        {667, 1},  // F_Skeleton.4
        ////////////////////// 14
        {666, 14}, // F_Skeleton.3
        {659, 14}, // F_Zombie.3
        {618, 14}, // Ogre.3
        {622, 14}, // Troll.3
        {610, 14}, // Orc_Sword.3
        {614, 14}, // Orc_Bow.3
        {631, 14}, // Ghost.4
        {603, 14}, // Goblin_Pike.4
        {717, 14}, // Dino.3
        {626, 14}, // Bat_Sonic.3
        {634, 14}, // Bee.3
        {709, 14}, // Spider.3
    }},
    {HARD, {
        {2132, 14}, // 2F_KnightLeader4
        {2130, 14}, // 2H_Knight4
        {671, 14}, // A_Skeleton.4
        {663, 14}, // A_Zombie.4
        {667, 14}, // F_Skeleton.4
        {660, 14}, // F_Zombie.4
        {718, 14}, // Dino.4
        {673, 14}, // M_Skeleton.4
        {701, 14}, // Necro_Female4
        {702, 14}, // Necro_Leader4
        {700, 14}, // Necro_Male4
        {619, 14}, // Ogre.4
        {623, 14}, // Troll.4
        {656, 14}, // Orc_Shaman.4
        {611, 14}, // Orc_Sword.4
        {615, 14}, // Orc_Bow.4
        {627, 14}, // Bat_Sonic.4
        {635, 14}, // Bee.4
        {710, 14}, // Spider.4
        {714, 14}, // Succubus.4
        {812, 14}, // Turtle.5
        {808, 14}, // Ghost.5
    }},
};

void CheckRebornReadiness(ServerIDType server_id, const PlayerInfo& player_info, unsigned char* p);

// A2 server only stores the effective unit stats () Walk over all equipped
void SubtractEquippedItems(A2Human* human, PlayerInfo& player_info) {
    for (int i = -2; i < 13; ++i) {
        T_INVENTORY_ITEM* item;

        if (i == -2) {
            item = human->unit.weapon;
        } else if (i == -1) {
            item = human->unit.shield;
        } else {
            item = human->dress[i];
        }

        if (item != nullptr) {
            if (item->effects.size) {
                auto* ptr = item->effects.first_node;
                while (ptr != NULL) {
                    if (ptr->value->effect_id == 2) {
                        player_info.body -= ptr->value->value1;
                    } else if (ptr->value->effect_id == 3) {
                        player_info.mind -= ptr->value->value1;
                    } else if (ptr->value->effect_id == 4) {
                        player_info.reaction -= ptr->value->value1;
                    } else if (ptr->value->effect_id == 5) {
                        player_info.spirit -= ptr->value->value1;
                    }

                    ptr = ptr->next;
                }
            }
        }
    }
}

void RebornReadinessInfo(ServerIDType server_id, T_PLAYER* player, unsigned char* p) {
    T_UNIT* unit = player->current_unit;

    if (unit == nullptr) {
        return zxmgr::SendMessage(p, "no current unit");
    }

    int has_treasures = 0;
    if (unit->inventory) {
        T_SRV_LINKED_NODE<T_INVENTORY_ITEM>* ptr = unit->inventory->list.first_node;
        while (ptr != NULL) {
            if (ptr->value->id == 3667) {
                has_treasures += ptr->value->amount;
            }
            ptr = ptr->next;
        }
    }

    std::string full_name = unit->name;
    std::string clan;
    size_t separator = full_name.find('|');
    if (separator != std::string::npos) {
        clan = full_name.substr(separator + 1);
    }

    PlayerInfo player_info;
    player_info.warrior = IsWarrior(unit);
    player_info.female = IsFemale(unit);
    player_info.solo = IsSoloPlayer(unit);
    player_info.clan = clan;
    player_info.has_treasures = has_treasures;
    player_info.money = player->money;
    player_info.body = unit->body;
    player_info.reaction = unit->reaction;
    player_info.mind = unit->mind;
    player_info.spirit = unit->spirit;
    player_info.experience = unit->exp;
    player_info.monster_kills_by_server_id = player->monster_kills_by_server_id;
    player_info.deaths = player->deaths;

    if (unit->clazz != A2_HUMAN_CLASS) {
        zxmgr::SendMessage(p, "current unit is not a human: %x != %x", unit->clazz, A2_HUMAN_CLASS);
    } else {
        A2Human* human = reinterpret_cast<A2Human*>(unit);
        SubtractEquippedItems(human, player_info);

        for (int i = 0; i < 5; ++i) {
            player_info.skills[i] = unit->skills[i];
        }
    }

    CheckRebornReadiness(server_id, player_info, p);
}

int32_t ServerRequirementsExperience(ServerIDType server_id, const PlayerInfo& player_info) {
    // Reclassed characters.
    if (player_info.female) {
        switch (server_id) {
            case EASY: return 50*k;
            case KIDS: return 500*k;
            case NIVAL: return 2*m;
            case MEDIUM: return 11*m;
            case HARD: return 50*m;
        }
    }

    // Hardcore characters.
    if (player_info.deaths <= 1) {
        switch (server_id) {
            case EASY: return 35*k;
            case KIDS: return 100*k;
            case NIVAL: return 500*k;
            case MEDIUM: return 11*m;
            case HARD: return 50*m;
        }
    }

    // Regular characters.
    return 0;
}

int32_t ServerRequirementsMoney(ServerIDType server_id, const PlayerInfo& player_info) {
    // Reclassed characters.
    if (player_info.female) {
        switch (server_id) {
            case EASY: return 100*k;
            case KIDS: return 1*m;
            case NIVAL: return 5*m;
            case MEDIUM: return 21*m;
            case HARD: return 100*m;
        }
    }

    // Hardcore and regular characters.
    switch (server_id) {
        case EASY: return 30*k;
        case KIDS: return 300*k;
        case NIVAL: return 1500*k;
        case MEDIUM: return 7*m;
        case HARD: return 50*m;
    }

    return 0;
}

uint32_t ServerRequirementsMonsterKills(ServerIDType server_id, const PlayerInfo& player_info) {
    // Reclassed characters.
    if (player_info.female) {
        switch (server_id) {
            case EASY: return 500;
            case KIDS: return 1200;
            case NIVAL: return 1500;
            case MEDIUM: return 2000;
            case HARD: return 4000;
        }
    }

    // Hardcore and regular characters.
    return 0;
}

void CheckReclassReadiness(const PlayerInfo& info, unsigned char* p) {
    bool ready_for_reclass = true;
    std::vector<std::string> info_lines;
    const int32_t need_money = 300000001;
    const int32_t need_experience = 177777778;

    if (info.money < need_money) {
        ready_for_reclass = false;
        info_lines.emplace_back(Format("- Need %d money, you have %d", need_money, info.money));
    } else {
        info_lines.emplace_back(Format("+ You have enough money: need %d, you have %d", need_money, info.money));
    }

    if (info.experience < need_experience) {
        ready_for_reclass = false;
        info_lines.emplace_back(Format("- Need %d experience, you have %d", need_experience, info.experience));
    } else {
        info_lines.emplace_back(Format("+ You have enough experience: need %d, you have %d", need_experience, info.experience));
    }

    info_lines.emplace_back(Format("Also you have: %d body, %d reaction, %d mind, %d spirit (max: 55-76-76-76)", info.body, info.reaction, info.mind, info.spirit));

    if (info.warrior) {
        info_lines.emplace_back(Format("Your skills: %d blade, %d axe, %d bludgeon, %d pike, %d shooting", info.skills[0], info.skills[1], info.skills[2], info.skills[3], info.skills[4]));
    } else {
        info_lines.emplace_back(Format("Your skills: %d fire, %d water, %d air, %d earth, %d astral", info.skills[0], info.skills[1], info.skills[2], info.skills[3], info.skills[4]));
    }

    if (ready_for_reclass) {
        if (info.clan == "reclass") {
            zxmgr::SendMessage(p, "Ready for reclass! Make camp and you will reclass.");
        } else {
            zxmgr::SendMessage(p, "Ready for reclass! To reclass, rename your character to 'reclass'.");
        }
    } else {
        zxmgr::SendMessage(p, "*NOT* ready for reclass. Requirements:");
    }
    for (auto it = info_lines.cbegin(); it != info_lines.cend(); ++it) {
        zxmgr::SendMessage(p, "%s", it->c_str());
    }
}

void CheckAscendReadiness(const PlayerInfo& info, unsigned char* p) {
    bool ready_for_ascend = true;
    std::vector<std::string> info_lines;
    const int32_t need_money = 2147000001;
    const int32_t need_experience = 177777778;
    const int16_t need_total_stats = 284;

    const int16_t total_stats = info.body + info.reaction + info.mind + info.spirit;
    if (total_stats < need_total_stats) {
        ready_for_ascend = false;
        info_lines.emplace_back(Format("- Need stats: you have %d body, %d reaction, %d mind, %d spirit (max: 56-76-76-76)", info.body, info.reaction, info.mind, info.spirit));
    } else {
        info_lines.emplace_back(Format("+ You have maxed out stats"));
    }

    if (info.money < need_money) {
        ready_for_ascend = false;
        info_lines.emplace_back(Format("- Need %d money, you have %d", need_money, info.money));
    } else {
        info_lines.emplace_back(Format("+ You have enough money: need %d, you have %d", need_money, info.money));
    }

    if (info.experience < need_experience) {
        ready_for_ascend = false;
        info_lines.emplace_back(Format("- Need %d experience, you have %d", need_experience, info.experience));
    } else {
        info_lines.emplace_back(Format("+ You have enough experience: need %d, you have %d", need_experience, info.experience));
    }

    if (info.warrior) {
        info_lines.emplace_back(Format("Your skills: %d blade, %d axe, %d bludgeon, %d pike, %d shooting", info.skills[0], info.skills[1], info.skills[2], info.skills[3], info.skills[4]));
    } else {
        info_lines.emplace_back(Format("Your skills: %d fire, %d water, %d air, %d earth, %d astral", info.skills[0], info.skills[1], info.skills[2], info.skills[3], info.skills[4]));
    }

    if (ready_for_ascend) {
        if (info.clan == "ascend") {
            zxmgr::SendMessage(p, "Ready for ascend! Make camp and you will ascend.");
        } else {
            zxmgr::SendMessage(p, "Ready for ascend! To ascend, rename your character to 'ascend'.");
        }
    } else {
        zxmgr::SendMessage(p, "*NOT* ready for ascend. Requirements:");
    }
    for (auto it = info_lines.cbegin(); it != info_lines.cend(); ++it) {
        zxmgr::SendMessage(p, "%s", it->c_str());
    }
}

void CheckRebornReadiness(ServerIDType server_id, const PlayerInfo& player_info, unsigned char* p) {
    int16_t need_mind = 0;
    int16_t need_reaction = 0;
    switch (server_id) {
        case EASY: need_mind = 15; break;
        case KIDS: need_reaction = 20; break;
        case NIVAL: need_reaction = 30; break;
        case MEDIUM: need_reaction = 40; break;
        case HARD: need_reaction = 50; break;
        default:
            if (player_info.female) {
                return CheckAscendReadiness(player_info, p);
            }
            
            return CheckReclassReadiness(player_info, p);
    }

    int32_t treasure_gives_gold = 0; // Note: gold is awarded only once, even for two treasures.
    switch (server_id) {
        case EASY: treasure_gives_gold = 5000; break;
        case KIDS: treasure_gives_gold = 30000; break;
        case NIVAL: treasure_gives_gold = 100000; break;
        case MEDIUM: treasure_gives_gold = 500000; break;
        case HARD: treasure_gives_gold = 1000000; break;
    }

    int32_t need_experience = ServerRequirementsExperience(server_id, player_info);
    int32_t need_money = ServerRequirementsMoney(server_id, player_info);
    uint32_t need_monster_kills = ServerRequirementsMonsterKills(server_id, player_info);
    need_money -= treasure_gives_gold;

    bool ready_for_reborn = true;
    std::vector<std::string> info_lines;
    if (player_info.mind < need_mind) {
        ready_for_reborn = false;
        info_lines.emplace_back(Format("- Need %d mind, you have %d (also you have: %d body, %d reaction, %d spirit)", need_mind, player_info.mind, player_info.body, player_info.reaction, player_info.spirit));
    } else if (need_mind > 0) {
        info_lines.emplace_back(Format("+ You have %d mind (also you have: %d body, %d reaction, %d spirit)", player_info.mind, player_info.body, player_info.reaction, player_info.spirit));
    }
    if (player_info.reaction < need_reaction) {
        ready_for_reborn = false;
        info_lines.emplace_back(Format("- Need %d reaction, you have %d (also you have: %d body, %d mind, %d spirit)", need_reaction, player_info.reaction, player_info.body, player_info.mind, player_info.spirit));
    } else if (need_reaction > 0) {
        info_lines.emplace_back(Format("+ You have %d reaction (also you have: %d body, %d mind, %d spirit)", player_info.reaction, player_info.body, player_info.mind, player_info.spirit));
    }

    if (player_info.solo && server_id <= MEDIUM) {
        if (player_info.has_treasures < 2) {
            ready_for_reborn = false;
            info_lines.emplace_back(Format("- Need two treasures from a boss or a minion, you have %d", player_info.has_treasures));
        } else {
            info_lines.emplace_back(Format("+ You have %d treasures", player_info.has_treasures));
        }
    } else {
        if (!player_info.has_treasures) {
            ready_for_reborn = false;
            info_lines.emplace_back("- Need the treasure from a boss or a minion");
        } else {
            info_lines.emplace_back("+ You have the treasure");
        }
    }

    if (player_info.money < need_money) {
        ready_for_reborn = false;
        info_lines.emplace_back(Format("- Need %d money, you have %d", need_money, player_info.money));
    } else if (need_money > 0) {
        info_lines.emplace_back(Format("+ You have enough money: need %d, you have %d", need_money, player_info.money));
    }

    if (player_info.experience < need_experience) {
        ready_for_reborn = false;
        info_lines.emplace_back(Format("- Need %d experience, you have %d", need_experience, player_info.experience));
    } else if (need_experience > 0) {
        info_lines.emplace_back(Format("+ You have enough experience: need %d, you have %d", need_experience, player_info.experience));
    }

    if (player_info.female) {
        InitializeMobNames();

        auto need_kills = girl_needs_monster_kills[server_id];
        int types_left = 0;
        if (need_kills.size()) {
            for (auto it = need_kills.begin(); it != need_kills.end(); ++it) {
                auto got = player_info.monster_kills_by_server_id[it->first];
                if (got < it->second) {
                    if (++types_left > 5) {
                        info_lines.emplace_back("(other mob kills omitted)");
                        break;
                    }

                    ready_for_reborn = false;
                    info_lines.emplace_back(Format("- Need %d kills of %s, you have %d", it->second, mob_names_by_server_id[it->first].c_str(), got));
                }
            }

            if (types_left == 0) {
                info_lines.emplace_back("+ You have all the mob kills");
            }
        }
    }

    const char* category;
    if (player_info.female) {
        category = "reclassed";
    } else if (player_info.deaths <= 1) {
        category = "hardcore";
    } else {
        category = "regular";
    }

    if (ready_for_reborn) {
        zxmgr::SendMessage(p, "Ready for reborn! Requirements for %s character at server %d:", category, server_id);
    } else {
        zxmgr::SendMessage(p, "*NOT* ready for reborn. Requirements for %s character at server %d:", category, server_id);
    }
    for (std::vector<std::string>::const_iterator it = info_lines.cbegin(); it != info_lines.cend(); ++it) {
        zxmgr::SendMessage(p, "%s", it->c_str());
    }
}
