#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "utils.h"

// Settings set by the player.
struct PlayerSettings {
    std::string quest_filter;
    int quest_mob_count;
    std::string player_name; // This is used to reset settings on relogin.
    uint32_t autobuff_mask; // If bit N is 1, the spell N will not be cast during autobuff casts.
};

// Player settings. Player ID -> settings.
extern std::unordered_map<short, std::unique_ptr<PlayerSettings>> player_settings;

// Initializes `player_settings` by pre-allocating all possible player IDs.
void InitializePlayerSettings();

// Map of "mob type" to normalized mob name. Mob type is: `face << 8 | type_id`.
// This format is used by the "kill N monsters" quests.
extern std::unique_ptr<std::unordered_map<int, std::string>> mob_names;
extern std::unique_ptr<std::unordered_map<int, std::string>> mob_names_raw;
extern std::unordered_map<int, std::string> mob_names_by_server_id;

// Normalizes mob name by lowercasing the string and replacing all dashes with underscores.
std::string NormalizeMobName(const char* name);

// Initializes `mob_names`. Defined in `inn.cpp` because it has the necessary structures.
// If `mob_names` are already initialized, does nothing.
void InitializeMobNames();

// Returns the state for "kill N monsters" quests for a given player.
// Returns formatted user-readable strings.
std::vector<std::string> QuestStateNMonsters(void* player);

// Checks player name and the name in the existing player settings. If they are
// different, it means that these settings were created by another player which
// has already left the game, so we reset the settings.
void __stdcall CheckPlayerSettings(T_PLAYER* player);
