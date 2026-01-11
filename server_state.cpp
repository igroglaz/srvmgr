#include "server_state.h"

#include <fstream>
#include <mutex>

#include "a2types.h"
#include "lib/utils.hpp"
#include "player_settings.h"

ServerState server_state;

const char* server_state_filename = "server_state.dat";

ServerState::ServerState() : map_index(0) {
}

bool ServerState::Save() {
    std::ofstream f(server_state_filename, std::ios::out | std::ios::trunc);
    if (!f) {
        Printf("[server_state] save: failed to open '%s' for writing: %s", server_state_filename, std::strerror(errno));
        return false;
    }

    f << this->map_index << '\n';

    // Save map order directly from the game's map array.
    const auto* a2_map_files = (A2Array<const char*>*)0x006d15f0;
    f << "map_order " << a2_map_files->size;
    for (uint32_t i = 0; i < a2_map_files->size; ++i) {
        f << ' ' << a2_map_files->data[i];
    }
    f << '\n';

    // Save player settings. We store only autobuff masks for now.
    settings::ForEachReadonly([&f](const std::string& player_name, settings::PlayerSettings* ps) {
        if (ps->autobuff_mask != 0) {
            f << "player_settings_autobuff_mask " << player_name << ' ' << ps->last_modified << ' ' << ps->autobuff_mask << '\n';
        }
    });

    f.close();
    if (!f) {
        Printf("[server_state] save: failed to write to '%s': %s", server_state_filename, std::strerror(errno));
        return false;
    }
    return true;
}

std::mutex throttled_save_mutex;
time_t last_save_time = 0;

bool ServerState::ThrottledSave() {
    std::lock_guard<std::mutex> lock(throttled_save_mutex);
    if (time(NULL) - last_save_time < 60) {
        return false;
    }
    last_save_time = time(NULL);
    return Save();
}

bool ServerState::Load() {
    std::ifstream f(server_state_filename, std::ios::in);
    if (!f) {
        Printf("[server_state] load: failed to open '%s' for reading: %s", server_state_filename, std::strerror(errno));
        return false;
    }

    f >> this->map_index;

    std::string section;

    while (true) {
        f >> section;
        if (!f) {
            break;
        }

        if (section == "map_order") {
            int count;
            f >> count;

            if (!f) {
                Printf("[server_state] load: failed to read map count from '%s': %s", server_state_filename, std::strerror(errno));
                return false;
            }

            if (count < 0 || count > 1000) {
                Printf("[server_state] load: invalid map count %d in '%s'", count, server_state_filename);
                return false;
            }

            this->map_order.resize(count);
            for (int i = 0; i < count; ++i) {
                f >> this->map_order[i];
            }

            if (!f) {
                Printf("[server_state] load: failed to read map order from '%s': %s", server_state_filename, std::strerror(errno));
                return false;
            }
        } else if (section == "player_settings_autobuff_mask") {
            std::string player_name;
            time_t last_modified;
            uint32_t mask;
            f >> player_name >> last_modified >> mask;

            if (!f) {
                Printf("[server_state] load: failed to read player_settings_autobuff_mask from '%s': %s", server_state_filename, std::strerror(errno));
                return false;
            }

            // Skip timestamps older than 1 week.
            if (time(NULL) - last_modified > 7 * 24 * 3600) {
                continue;
            }

            auto* ps = settings::FindOrCreate(player_name.c_str());
            ps->autobuff_mask = mask;
            ps->last_modified = last_modified;
        } else {
            Printf("[server_state] load: unknown section '%s' in '%s'", section.c_str(), server_state_filename);
            return false;
        }
    }

    return true;
}
