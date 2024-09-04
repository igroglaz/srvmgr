#include <cstring>
#include <unordered_set>

#include "lib/utils.hpp"
#include "quests.h"

// A couple data types used in the inn logic.
namespace {

template <typename T>
struct Array {
    int whatever0;
    T *data;
    int size;
    int whatever1[2];
};

// Full `MonsterInfoData` definition taken from a2serv.exe via Ghidra.
// We need just a few fields, but it's easier to dump the full struct.
struct MonsterInfoData {
    int body;
    int reaction;
    int mind;
    int spirit;
    int healthMax;
    int healthRegeneration;
    int manaMax;
    int manaRegeneration;
    int speed;
    int rotationSpeed;
    int scanRange;
    int physicalMin;
    int physicalMax;
    int attackType;
    int toHit;
    int defence;
    int absorbtion;
    int charge;
    int relax;
    int protectionFire;
    int protectionWater;
    int protectionAir;
    int protectionEarth;
    int protectionAstral;
    int protectionBlade;
    int protectionAxe;
    int protectionBludgeon;
    int protectionPike;
    int protectionShooting;
    int typeId;
    int face;
    int tokenSize;
    int movementType;
    int dyingTime;
    int withdraw;
    int wimpy;
    int seeInvisible;
    int experience;
    int treasureGold;
    int treasureGoldMin;
    int treasureGoldMax;
    int treasureItem;
    int treasureItemMin;
    int treasureItemMax;
    int treasureItemMask;
    int notUsed;
    int notUsed1;
    int power;
    int spell1ID;
    int spellProbability1;
    int spell2ID;
    int spellProbability2;
    int spell3ID;
    int spellProbability3;
    int spellPower;
    int serverId;
    int knownSpellsMask;
    int skillFire;
    int skillWater;
    int skillAir;
    int skillEarth;
    int skillAstral;
};

struct MonsterInfo {
    int whatever0;
    const char* name;
    Array<MonsterInfoData> monsterData;
    Array<char*> equipment;
};

struct GameDataRes {
    char whatever[0x8c];
    Array<MonsterInfo> monsters;
};

} // anonymous namespace

// Address in a2serv.exe: 50df19.
// Original `ChooseRewardMob_0050df19` has `GameDataRes*` in ECX and `target_experience` on the stack.
// `__fastcall` convention: last two arguments are passed in ECX and EDX, the rest on the stack.
//
// Note that `target_experience` is around reward (in gold) divided by 16.
int __fastcall change_inn_reward_mob(GameDataRes *data, int unused, int target_experience) {
	// Original logic:
	//   1) Take all mobs with 63 < `typeId` < 99
	//   2) exclude Ghost.1, F_Zombie.1 and F_Skeleton.1 by `face`
	//   3) exclude all monsters with `.5` in their name.
	//
	// We do it similarly, but for `typeId` up to 108, and we exclude ghosts, flyers and casters.
	//
	// Potentially, instead of all this, we can just filter by unique `serverId`-s (and have this as a server setting?).
	const Array<MonsterInfo>& monsters = data->monsters;

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

	for (int i = 0; i < monsters.size; ++i) {
		const MonsterInfo& m = monsters.data[i];
		if (m.monsterData.data == nullptr) {
			continue;
		}

		const MonsterInfoData& d = *m.monsterData.data;

		if (d.typeId <= 63 || d.typeId >= 108) {
			continue;
		}

		// Skip the mobs that we don't want to see.
		if (excluded_type_ids.count(d.typeId)) {
			continue;
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

	const GameDataRes* data = (GameDataRes*)0x6d0668;
	const Array<MonsterInfo>& monsters = data->monsters;

	for (int i = 0; i < monsters.size; ++i) {
		const MonsterInfo& m = monsters.data[i];
		if (m.monsterData.data == nullptr) {
			continue;
		}

		const MonsterInfoData& d = *m.monsterData.data;

		const int mob_type = d.face << 8 | d.typeId;

		(*new_mob_names)[mob_type] = ToLower(m.name);
	}
	
	if (mob_names.get() != nullptr) {
		return;
	}

	mob_names = std::move(new_mob_names);
}