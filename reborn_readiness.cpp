#include "reborn_readiness.hpp"

#include <cmath>
#include <cstdint>
#include <string>
#include <map>
#include <vector>

#include "circle.h"
#include "config_new.h"
#include "lib/utils.hpp"
#include "quests.h"
#include "solo.h"
#include "thresholds.h"
#include "zxmgr.h"

int SkillLevelForExperience(uint32_t experience) {
    return static_cast<int>(floor(log10(experience / 1000.0 + 1.0) / log10(1.1)));
}

struct PlayerInfo {
    bool warrior;
    bool female;
    bool solo;
    bool legend;
    int circle;
    std::string clan;
    uint32_t has_treasures;
    uint32_t money;
    uint16_t body;
    uint16_t reaction;
    uint16_t mind;
    uint16_t spirit;
    uint32_t experience;
    const uint8_t* monster_kills_by_server_id;
    uint32_t deaths;
    int skills[5];
    int32_t main_sphere;
    int32_t experience_per_sphere[5];
    A2Unit* unit;
};

std::string WithSpaces(int value) {
    std::string s = std::to_string(value);
    int position = static_cast<int>(s.length()) - 3;
    while (position > 0) {
        s.insert(position, " ");
        position -= 3;
    }
    return s;
}

void CheckRebornReadiness(ServerIDType server_id, const PlayerInfo& player_info, const A2Player* p, bool hell);

// A2 server only stores the effective unit stats () Walk over all equipped
void SubtractEquippedItems(A2Human* human, PlayerInfo& player_info) {
    for (int i = -2; i < 13; ++i) {
        A2InventoryItem* item;

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

void RebornReadinessInfo(ServerIDType server_id, const A2Player* player, bool hell) {
    A2Unit* unit = player->current_unit;

    if (unit == nullptr) {
        return zxmgr::SendMessage(player, "no current unit");
    }

    int has_treasures = 0;
    if (unit->inventory) {
        A2Node<A2InventoryItem>* ptr = unit->inventory->list.first_node;
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
    player_info.legend = IsGigaPlayer(unit);
    player_info.circle = circle::Circle(unit);
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
    player_info.unit = unit;

    if (unit->clazz != A2_CLASS_HUMAN) {
        zxmgr::SendMessage(player, "current unit is not a human: %x != %x", unit->clazz, A2_CLASS_HUMAN);
    } else {
        A2Human* human = reinterpret_cast<A2Human*>(unit);
        SubtractEquippedItems(human, player_info); 

        for (int i = 0; i < 5; ++i) {
            player_info.skills[i] = unit->skills[i];
            player_info.main_sphere = human->main_sphere;
            player_info.experience_per_sphere[i] = human->experience_per_sphere[i];
        }
    }

    thresholds::thresholds.MaybeReload(thresholds::THRESHOLDS_FILE);

    CheckRebornReadiness(server_id, player_info, player, hell);
}

int32_t ServerRequirementsExperience(const PlayerInfo& player_info) {
    return thresholds::thresholds.Value("reborn.experience", player_info.unit);
}

int32_t ServerRequirementsMoney(const PlayerInfo& player_info) {
    return thresholds::thresholds.Value("reborn.money", player_info.unit);
}

bool CheckMonsterKills(const PlayerInfo& info, const std::unordered_map<uint16_t, int>* need_kills, std::vector<std::string>& info_lines) {
    if (!need_kills) {
        return true;
    }

    InitializeMobNames();

    bool ready = true;
    int types_left = 0;

    for (auto it = need_kills->begin(); it != need_kills->end(); ++it) {
        auto got = info.monster_kills_by_server_id[it->first];
        if (got < it->second) {
            if (++types_left > 5) {
                info_lines.emplace_back("(other mob kills omitted)");
                break;
            }

            ready = false;
            info_lines.emplace_back(Format("- Need %d kills of %s, you have %d", it->second, mob_names_by_server_id[it->first].c_str(), got));
        }
    }

    if (types_left == 0) {
        info_lines.emplace_back("+ You have all the mob kills");
    }

    return ready;
}

bool CheckMoney(const PlayerInfo& info, uint32_t need_money, std::vector<std::string>& info_lines) {
    bool result = true;
    if (info.money < need_money) {
        info_lines.emplace_back(Format("- Need %s money, you have %s", WithSpaces(need_money).c_str(), WithSpaces(info.money).c_str()));
        result = false;
    } else {
        info_lines.emplace_back(Format("+ You have enough money: need %s, you have %s", WithSpaces(need_money).c_str(), WithSpaces(info.money).c_str()));
    }

    return result;
}

bool CheckExperience(const PlayerInfo& info, uint32_t need_experience, std::vector<std::string>& info_lines) {
    bool result = true;
    if (info.experience < need_experience) {
        info_lines.emplace_back(Format("- Need %s experience, you have %s", WithSpaces(need_experience).c_str(), WithSpaces(info.experience).c_str()));
        result = false;
    } else {
        info_lines.emplace_back(Format("+ You have enough experience: need %s, you have %s", WithSpaces(need_experience).c_str(), WithSpaces(info.experience).c_str()));
    }

    if (info.warrior) {
        info_lines.emplace_back(Format("Your skills: %d blade, %d axe, %d bludgeon, %d pike, %d shooting", info.skills[0], info.skills[1], info.skills[2], info.skills[3], info.skills[4]));
    } else {
        info_lines.emplace_back(Format("Your skills: %d fire, %d water, %d air, %d earth, %d astral", info.skills[0], info.skills[1], info.skills[2], info.skills[3], info.skills[4]));
    }

    return result;
}

void SkillsAndCeilings(const PlayerInfo& info, std::vector<std::string>& messages) {
    if (info.warrior) {
        messages.emplace_back(Format("Your skills: %d blade, %d axe, %d bludgeon, %d pike, %d shooting", info.skills[0], info.skills[1], info.skills[2], info.skills[3], info.skills[4]));
    } else {
        messages.emplace_back(Format("Your skills: %d fire, %d water, %d air, %d earth, %d astral", info.skills[0], info.skills[1], info.skills[2], info.skills[3], info.skills[4]));
    }

    const uint32_t limit_main = thresholds::thresholds.Value("experience_limit.main_skill", info.unit);
    const uint32_t limit_secondary = thresholds::thresholds.Value("experience_limit.secondary", info.unit);
    if (limit_main != 0 && limit_secondary != 0) {
        int max_skill_main = SkillLevelForExperience(limit_main);
        int max_skill_secondary = SkillLevelForExperience(limit_secondary);

        messages.emplace_back(Format("Max skills at this server: %d primary and astral/shooting, %d secondary", max_skill_main, max_skill_secondary));
    }
    
    const uint32_t experience_cutoff = thresholds::thresholds.Value("reborn.experience_cutoff", info.unit);
    if (experience_cutoff == 0) {
        return;
    }

    // If the player were to reborn right now, how much experience per sphere will they have?
    int32_t new_exp[5];

    // TODO: All this logic is better be in thresholds as well.
    if (!info.female && info.circle == 0) {
        for (int i = 0; i < 4; ++i) {
            new_exp[i] = info.experience_per_sphere[i] / 2;
        }

        new_exp[info.main_sphere - 1] = 0;

        if (info.deaths == 0) {
            new_exp[4] = info.experience_per_sphere[4] / 2;
        } else if (info.warrior) {
            new_exp[4] = info.experience_per_sphere[4] / static_cast<int>(Config::ServerID + 1);
        } else {
            new_exp[4] = 0;
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            new_exp[i] = 0;
        }
    }

    int skills[5];
    for (int i = 0; i < 5; ++i) {
        if (new_exp[i] > static_cast<int32_t>(experience_cutoff)) {
            new_exp[i] = experience_cutoff;
        }

        skills[i] = SkillLevelForExperience(new_exp[i]);
    }

    if (info.warrior) {
        messages.emplace_back(Format("Upon reborn, your skills will be: %d blade, %d axe, %d bludgeon, %d pike, %d shooting", skills[0], skills[1], skills[2], skills[3], skills[4]));
    } else {
        messages.emplace_back(Format("Upon reborn, your skills will be: %d fire, %d water, %d air, %d earth, %d astral", skills[0], skills[1], skills[2], skills[3], skills[4]));
    }
}

void CheckReclassReadiness(const PlayerInfo& info, const A2Player* p) {
    bool ready_for_reclass = true;
    std::vector<std::string> info_lines;
    const uint32_t need_money = thresholds::thresholds.Value("reclass.money", info.unit);
    const uint32_t need_experience = thresholds::thresholds.Value("reclass.experience", info.unit);

    ready_for_reclass &= CheckMoney(info, need_money, info_lines);
    ready_for_reclass &= CheckExperience(info, need_experience, info_lines);

    info_lines.emplace_back(Format("Also you have: %d body, %d reaction, %d mind, %d spirit (max: 55-76-76-76)", info.body, info.reaction, info.mind, info.spirit));

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

void CheckAscendReadiness(const PlayerInfo& info, const A2Player* p) {
    bool ready_for_ascend = true;
    std::vector<std::string> info_lines;
    const uint32_t need_money = thresholds::thresholds.Value("ascend.money", info.unit);
    const uint32_t need_experience = thresholds::thresholds.Value("ascend.experience", info.unit);
    const uint32_t need_body = thresholds::thresholds.Value("ascend.stats.body", info.unit);
    const uint32_t need_reaction = thresholds::thresholds.Value("ascend.stats.reaction", info.unit);
    const uint32_t need_mind = thresholds::thresholds.Value("ascend.stats.mind", info.unit);
    const uint32_t need_spirit = thresholds::thresholds.Value("ascend.stats.spirit", info.unit);

    const int16_t need_total_stats = need_body + need_reaction + need_mind + need_spirit;
    const int16_t total_stats = info.body + info.reaction + info.mind + info.spirit;
    if (total_stats < need_total_stats) {
        ready_for_ascend = false;
        info_lines.emplace_back(Format("- Need stats: you have %d body, %d reaction, %d mind, %d spirit (max: 56-76-76-76)", info.body, info.reaction, info.mind, info.spirit));
    } else {
        info_lines.emplace_back(Format("+ You have maxed out stats"));
    }

    ready_for_ascend &= CheckMoney(info, need_money, info_lines);
    ready_for_ascend &= CheckExperience(info, need_experience, info_lines);

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

std::vector<std::string> no_more_circles_snark{
    "Congratulations! You've officially run out of hell. Please see yourself out through the gift shop.",
    "You've reached the end of hell. There is literally nothing past this. Good job!",
    "Well done. You've completed your tour of eternal damnation. No refunds.",
    "You really thought there was more? This isn't a buffet, it's hell. Portions are controlled.",
    "Beyond this point is only your own imagination. Frankly, that's probably worse.",
    "Beyond this point lies the void... and your own foolish ambition. Turn back, overachiever.",
    "You have reached Level: Overachiever. Quest: Stop trying to break the Universe. Status: Failed.",
    "How did you beat all of hell? Are you a doomguy or something?",
};

void CheckCircleReadiness(const PlayerInfo& info, const A2Player* p) {
    if (info.circle == 8) {
        int snark = std::rand() % no_more_circles_snark.size();
        zxmgr::SendMessage(p, "%s", no_more_circles_snark[snark].c_str());
        return;
    }

    bool ready = true;
    std::vector<std::string> info_lines;
    const uint32_t need_money = thresholds::thresholds.Value("hell.money", info.unit);
    const uint32_t need_experience = thresholds::thresholds.Value("hell.experience", info.unit);
    const uint32_t need_reaction = thresholds::thresholds.Value("hell.stats.reaction", info.unit);
    const uint32_t need_mind = thresholds::thresholds.Value("hell.stats.mind", info.unit);
    const uint32_t need_spirit = thresholds::thresholds.Value("hell.stats.spirit", info.unit);

    if (info.reaction < need_reaction || info.mind < need_mind || info.spirit < need_spirit) {
        ready = false;
        info_lines.emplace_back(Format("- Need stats: you have %d body, %d reaction, %d mind, %d spirit (need 76 in reaction, mind and spirit)", info.body, info.reaction, info.mind, info.spirit));
    } else {
        info_lines.emplace_back(Format("+ You have maxed out stats"));
    }

    auto hell_needs_kills = thresholds::thresholds.Mobs("hell.mobs", info.unit);

    ready &= CheckMoney(info, need_money, info_lines);
    ready &= CheckExperience(info, need_experience, info_lines);
    ready &= CheckMonsterKills(info, hell_needs_kills, info_lines);

    if (ready) {
        if (info.circle == 0) {
            if (info.clan == "circle" || info.clan == "hell") {
                zxmgr::SendMessage(p, "Ready for hell! Make camp and you will go to the next circle of hell.");
            } else if (info.clan == "miss_hell") {
                zxmgr::SendMessage(p, "Ready for hell! Make camp and you will go to the next circle of hell as a female character.");
            } else {
                zxmgr::SendMessage(p, "Ready for hell! To go to the next circle of hell, rename your character to 'hell' or 'miss_hell'.");
            }
        } else {
            if (info.clan == "circle" || info.clan == "hell" || info.clan == "miss_hell") {
                zxmgr::SendMessage(p, "Ready for hell! Make camp and you will go to the next circle of hell.");
            } else {
                zxmgr::SendMessage(p, "Ready for hell! To go to the next circle of hell, rename your character to 'hell'.");
            }
        }
    } else {
        zxmgr::SendMessage(p, "*NOT* ready for hell. Requirements:");
    }
    for (auto it = info_lines.cbegin(); it != info_lines.cend(); ++it) {
        zxmgr::SendMessage(p, "%s", it->c_str());
    }
}

void CheckRebornReadiness(ServerIDType server_id, const PlayerInfo& player_info, const A2Player* p, bool hell) {
    if (server_id >= NIGHTMARE) {
        if (player_info.circle != 0 || hell) {
            return CheckCircleReadiness(player_info, p);
        }

        if (player_info.female) {
            return CheckAscendReadiness(player_info, p);
        }

        return CheckReclassReadiness(player_info, p);
    }

    uint32_t need_reaction = thresholds::thresholds.Value("reborn.stats.reaction", player_info.unit);
    uint32_t need_mind = thresholds::thresholds.Value("reborn.stats.mind", player_info.unit);
    uint32_t need_spirit = thresholds::thresholds.Value("reborn.stats.spirit", player_info.unit);

    uint32_t treasure_gives_gold = thresholds::thresholds.Value("treasure_award", player_info.unit);

    uint32_t need_experience = ServerRequirementsExperience(player_info);
    uint32_t need_money = ServerRequirementsMoney(player_info);
    need_money -= treasure_gives_gold;

    bool ready_for_reborn = true;
    std::vector<std::string> info_lines;

    if (server_id == HARD) {
        if (player_info.reaction < need_reaction || player_info.mind < need_mind || player_info.spirit < need_spirit) {
            ready_for_reborn = false;
            info_lines.emplace_back(Format("- Need %d reaction, %d mind and %d spirit, you have: %d reaction, %d mind, %d spirit (also you have %d body)", need_reaction, need_mind, need_spirit, player_info.reaction, player_info.mind, player_info.spirit, player_info.body));
        } else {
            info_lines.emplace_back(Format("+ You have %d reaction, %d mind and %d spirit (also you have %d body)", player_info.reaction, player_info.mind, player_info.spirit, player_info.body));
        }
    } else {
        if (need_mind) {
            if (player_info.mind < need_mind) {
                ready_for_reborn = false;
                info_lines.emplace_back(Format("- Need %d mind, you have %d (also you have: %d body, %d reaction, %d spirit)", need_mind, player_info.mind, player_info.body, player_info.reaction, player_info.spirit));
            } else if (need_mind > 0) {
                info_lines.emplace_back(Format("+ You have %d mind (also you have: %d body, %d reaction, %d spirit)", player_info.mind, player_info.body, player_info.reaction, player_info.spirit));
            }
        }

        if (need_reaction) {
            if (player_info.reaction < need_reaction) {
                ready_for_reborn = false;
                info_lines.emplace_back(Format("- Need %d reaction, you have %d (also you have: %d body, %d mind, %d spirit)", need_reaction, player_info.reaction, player_info.body, player_info.mind, player_info.spirit));
            } else if (need_reaction > 0) {
                info_lines.emplace_back(Format("+ You have %d reaction (also you have: %d body, %d mind, %d spirit)", player_info.reaction, player_info.body, player_info.mind, player_info.spirit));
            }
        }
    }

    uint32_t need_treasures = thresholds::thresholds.Value("reborn.treasures", player_info.unit);

    if (need_treasures > 1) {
        if (player_info.has_treasures < need_treasures) {
            ready_for_reborn = false;
            info_lines.emplace_back(Format("- Need %d treasures from a boss or a minion, you have %d", need_treasures, player_info.has_treasures));
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
        info_lines.emplace_back(Format("- Need %s money, you have %s", WithSpaces(need_money).c_str(), WithSpaces(player_info.money).c_str()));
    } else if (need_money > 0) {
        info_lines.emplace_back(Format("+ You have enough money: need %s, you have %s", WithSpaces(need_money).c_str(), WithSpaces(player_info.money).c_str()));
    }

    if (player_info.experience < need_experience) {
        ready_for_reborn = false;
        info_lines.emplace_back(Format("- Need %s experience, you have %s", WithSpaces(need_experience).c_str(), WithSpaces(player_info.experience).c_str()));
    } else if (need_experience > 0) {
        info_lines.emplace_back(Format("+ You have enough experience: need %s, you have %s", WithSpaces(need_experience).c_str(), WithSpaces(player_info.experience).c_str()));
    }

    auto needs_kills = thresholds::thresholds.Mobs("reborn.mobs", player_info.unit);
    ready_for_reborn &= CheckMonsterKills(player_info, needs_kills, info_lines);

    SkillsAndCeilings(player_info, info_lines);

    const char* category;
    if (player_info.circle) {
        category = "hell";
    } else if (player_info.female) {
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
