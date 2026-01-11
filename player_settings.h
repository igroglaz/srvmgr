#pragma once

#include <functional>
#include <string>

namespace settings {
    
// Settings set by the player.
struct PlayerSettings {
    // Quest filter string. If non-empty, the player will only get quests for matching mobs. Example: `skeleton.4`.
    std::string quest_filter;
    // Quest mob count. If > 0, the player will only get quests for this many mobs (if allowed by the inn).
    int quest_mob_count;
    // If bit N is 1, the spell N will not be cast during autobuff casts. Persisted to server state.
    uint32_t autobuff_mask; 
    // The name of the map this player voted for.
    std::string map_vote;
    // Timestamp of the last modification of these settings.
    time_t last_modified;
};

PlayerSettings* Find(const char* player_name);
PlayerSettings* FindOrCreate(const char* player_name);

void ForEachReadonly(std::function<void(const std::string&, PlayerSettings*)> func);
void ForEachMutable(std::function<void(const std::string&, PlayerSettings*)> func);

} // namespace settings
