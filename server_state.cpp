#include "server_state.h"

#include "a2types.h"
#include "lib/utils.hpp"

#include <fstream>

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
    for (int i = 0; i < a2_map_files->size; ++i) {
        f << ' ' << a2_map_files->data[i];
    }
    f << '\n';

    f.close();
    if (!f) {
        Printf("[server_state] save: failed to write to '%s': %s", server_state_filename, std::strerror(errno));
        return false;
    }
    return true;
}

bool ServerState::Load() {
    std::ifstream f(server_state_filename, std::ios::in);
    if (!f) {
        Printf("[server_state] load: failed to open '%s' for reading: %s", server_state_filename, std::strerror(errno));
        return false;
    }

    f >> this->map_index;

    std::string header;

    f >> header;

    if (header == "map_order") {
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
        for (size_t i = 0; i < count; ++i) {
            f >> this->map_order[i];
        }
    }

    f.close();
    if (!f) {
        Printf("[server_state] load: failed to read from '%s': %s", server_state_filename, std::strerror(errno));
        return false;
    }
    return true;
}
