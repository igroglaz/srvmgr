#include <cstring>
#include <unordered_set>

#include "a2types.h"
#include "lib/utils.hpp"
#include "quests.h"
#include "config_new.h"

// Address in a2serv.exe: 50df19.
// Original `ChooseRewardMob_0050df19` has `A2GameDataRes*` in ECX and `target_experience` on the stack.
// `__fastcall` convention: last two arguments are passed in ECX and EDX, the rest on the stack.
//
// Note that `target_experience` is around reward (in gold) divided by 16.
int __fastcall change_inn_reward_mob(A2GameDataRes *data, int unused, int target_experience) {
	// Original logic:
	//   1) Take all mobs with 63 < `typeId` < 99
	//   2) exclude Ghost.1, F_Zombie.1 and F_Skeleton.1 by `face`
	//   3) exclude all monsters with `.5` in their name.
	//
	// We do it similarly, but for `typeId` up to 108, and we exclude ghosts, flyers and casters.
	//
	// Potentially, instead of all this, we can just filter by unique `serverId`-s (and have this as a server setting?).
	const A2Array<A2MonsterInfo>& monsters = data->monsters;

	int target_mob = 0;
	int max_exp = 0;

	// Type IDs of excluded mobs.
	std::unordered_set<int> excluded_type_ids;
	excluded_type_ids.insert(69); // Ghosts
	excluded_type_ids.insert(70); // Bats
	excluded_type_ids.insert(71); // Dragons
	excluded_type_ids.insert(72); // Urd
	excluded_type_ids.insert(73); // Bees
	excluded_type_ids.insert(81); // Shamans
	excluded_type_ids.insert(82); // F_Zombie.1
	excluded_type_ids.insert(89); // F_Skeleton.1
	excluded_type_ids.insert(97); // M_Skeleton.3
	excluded_type_ids.insert(98); // M_Skeleton.4
	excluded_type_ids.insert(99); // Necromancers
	excluded_type_ids.insert(100); // Druids
	excluded_type_ids.insert(101); // Mercenary orc (campaign)
	excluded_type_ids.insert(102); // Mercenary troll (campaign)
	excluded_type_ids.insert(104); // Spider
	excluded_type_ids.insert(105); // Succubi

	for (uint32_t i = 0; i < monsters.size; ++i) {
		const A2MonsterInfo& m = monsters.data[i];
		if (m.monsterData.data == nullptr) {
			continue;
		}

		const A2MonsterInfoData& d = *m.monsterData.data;

        // Skip mobs that we don't want to see
        if (Config::ServerID >= QUEST_T1 && d.typeId != 76) { // at quest maps only allow turtle1-4
            continue;
        } else {
            if (d.typeId <= 63 || d.typeId >= 108) {
                continue;
            }

            if (excluded_type_ids.count(d.typeId)) {
                continue;
            }
        }

		// Skip all level-5 mobs.
		if (std::strstr(m.name, ".5") != nullptr) {
			continue;
		}

		if (d.experience <= target_experience && d.experience > max_exp) {
			target_mob = i; // We are returning the index in the monster list.
			max_exp = d.experience;
		}
	}

	return target_mob;
}


void InitializeMobNames() {
	if (mob_names.get() != nullptr) {
		return;
	}

	std::unique_ptr<std::unordered_map<int, std::string>> new_mob_names(new std::unordered_map<int, std::string>());
	std::unique_ptr<std::unordered_map<int, std::string>> new_mob_names_raw(new std::unordered_map<int, std::string>());

	const A2GameDataRes* data = (A2GameDataRes*)0x6d0668;
	const A2Array<A2MonsterInfo>& monsters = data->monsters;

	for (uint32_t i = 0; i < monsters.size; ++i) {
		const A2MonsterInfo& m = monsters.data[i];
		if (m.monsterData.data == nullptr) {
			continue;
		}

		const A2MonsterInfoData& d = *m.monsterData.data;

		// Add necros share the same type_id=99. Same with druids (100) and catapults (101).
		std::string name = m.name;
		if (d.typeId == 99) {
			name = "Necro";
		} else if (d.typeId == 100) {
			name = "Druid";
		} else if (d.typeId == 101) {
			name = "Catapult";
		}

		const int mob_type = d.face << 8 | d.typeId;

		(*new_mob_names)[mob_type] = NormalizeMobName(name.c_str());
		(*new_mob_names_raw)[mob_type] = name;

        mob_names_by_server_id[d.serverId] = m.name;
	}

    // Human mobs don't have `A2MonsterInfoData` attached to them. Hardcode these two for `#reborn`.
    mob_names_by_server_id[2130] = "2H_Knight4";
    mob_names_by_server_id[2132] = "2F_KnightLeader4";
	
	if (mob_names.get() != nullptr) {
		return;
	}

	for (auto it = mob_names_by_server_id.begin(); it != mob_names_by_server_id.end(); ++it) {
		mob_names_by_server_id_normed[it->first] = NormalizeMobName(it->second.c_str());
	}

	// Most mobs of the first level don't have a suffix, which makes it
	// impossible to choose only them with filters. For example, the ghost is
	// `Ghost`, but a filter `Ghost` would also match `Ghost.2`.
	// Let's append `.1` to each mob name that doesn't have a level.
	for (auto it = mob_names_by_server_id_normed.begin(); it != mob_names_by_server_id_normed.end(); ++it) {
		const char mob_name_end = it->second.back();
		if (mob_name_end < '0' || mob_name_end > '9') {
			it->second += ".1";
		}
	}
	for (auto it = new_mob_names->begin(); it != new_mob_names->end(); ++it) {
		const char mob_name_end = it->second.back();
		if (mob_name_end < '0' || mob_name_end > '9') {
			it->second += ".1";
		}
	}

	mob_names = std::move(new_mob_names);
	mob_names_raw = std::move(new_mob_names_raw);
}
