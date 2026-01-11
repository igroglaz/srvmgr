#include "player_settings.h"

#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace settings {

std::shared_mutex player_settings_mutex;
// Player settings. Short player name (without clan) -> settings.
std::unordered_map<std::string, std::unique_ptr<PlayerSettings>> player_settings;

std::string ShortPlayerName(const char* player_name) {
    std::string short_name = player_name;
    auto pos = short_name.find('|');
    if (pos != std::string::npos) {
        short_name = short_name.substr(0, pos);
    }
    return short_name;
}

PlayerSettings* Find(const char* player_name) {
    std::string short_name = ShortPlayerName(player_name);

    std::shared_lock<std::shared_mutex> read_lock(player_settings_mutex);
    auto it = player_settings.find(short_name);
    if (it != player_settings.end()) {
        return it->second.get();
    }
    return nullptr;
}

PlayerSettings* FindOrCreate(const char* player_name) {
    std::string short_name = ShortPlayerName(player_name);

    // Fast path: read existing settings.
    {
        std::shared_lock<std::shared_mutex> read_lock(player_settings_mutex);
        auto it = player_settings.find(short_name);
        if (it != player_settings.end()) {
            return it->second.get();
        }
    }

    // Slow path: create new settings under an exclusive lock.
    std::unique_lock<std::shared_mutex> write_lock(player_settings_mutex);

    auto inserted = player_settings.try_emplace(short_name, std::make_unique<PlayerSettings>());
    inserted.first->second->last_modified = time(NULL);
    return inserted.first->second.get();
}

void ForEachReadonly(std::function<void(const std::string&, PlayerSettings*)> func) {
    std::shared_lock<std::shared_mutex> read_lock(player_settings_mutex);
    for (auto& ps: player_settings) {
        func(ps.first, ps.second.get());
    }
}

void ForEachMutable(std::function<void(const std::string&, PlayerSettings*)> func) {
    std::unique_lock<std::shared_mutex> read_lock(player_settings_mutex);
    for (auto& ps: player_settings) {
        func(ps.first, ps.second.get());
    }
}

} // namespace settings
