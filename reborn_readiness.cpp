#include "reborn_readiness.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include "config_new.h"
#include "lib/utils.hpp"
#include "zxmgr.h"

struct PlayerInfo {
    bool warrior;
    bool female;
    bool has_treasure;
    int32_t money;
    uint16_t reaction;
    uint16_t mind;
    int32_t experience;
    uint32_t monster_kills;
    uint32_t deaths;
};

const int32_t k = 1000;
const int32_t m = k * k;

void CheckRebornReadiness(ServerIDType server_id, const PlayerInfo& player_info, unsigned char* p);

void RebornReadinessInfo(ServerIDType server_id, T_PLAYER* player, unsigned char* p) {
    T_UNIT* unit = player->current_unit;

    if (unit == nullptr) {
        return zxmgr::SendMessage(p, "no current unit");
    }

    bool has_treasure = false;
    if (unit->inventory) {
        T_SRV_LINKED_NODE* ptr = unit->inventory->first_node;
        while (ptr != NULL) {
            if (ptr->value->id == 3667) {
                has_treasure = true;
                break;
            }
            ptr = ptr->next;
        }
    }

    PlayerInfo player_info;
    player_info.warrior = IsWarrior(unit);
    player_info.female = IsFemale(unit);
    player_info.has_treasure = has_treasure;
    player_info.money = player->money;
    player_info.reaction = unit->reaction;
    player_info.mind = unit->mind;
    player_info.experience = unit->exp;
    player_info.monster_kills = player->monster_kills;
    player_info.deaths = player->deaths;

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
        case EASY: return 50*k;
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
            zxmgr::SendMessage(p, "This server does not have a rebirth.");
            return;
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
        info_lines.emplace_back(Format("- Need %d mind, you have %d", need_mind, player_info.mind));
    } else if (need_mind > 0) {
        info_lines.emplace_back(Format("+ You have %d mind", player_info.mind));
    }
    if (player_info.reaction < need_reaction) {
        ready_for_reborn = false;
        info_lines.emplace_back(Format("- Need %d reaction, you have %d", need_reaction, player_info.reaction));
    } else if (need_reaction > 0) {
        info_lines.emplace_back(Format("+ You have %d reaction", player_info.reaction));
    }

    if (!player_info.has_treasure) {
        ready_for_reborn = false;
        info_lines.emplace_back("- Need the treasure from a boss or a minion");
    } else {
        info_lines.emplace_back("+ You have the treasure");
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

    if (player_info.monster_kills < need_monster_kills) {
        ready_for_reborn = false;
        info_lines.emplace_back(Format("- Need %d monster kills, you have %d", need_monster_kills, player_info.monster_kills));
    } else if (need_monster_kills > 0) {
        info_lines.emplace_back(Format("+ You have enough monster kills: need %d, you have %d", need_monster_kills, player_info.monster_kills));
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
    for (const auto& line: info_lines) {
        zxmgr::SendMessage(p, "%s", line.c_str());
    }
}
