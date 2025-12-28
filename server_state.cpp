#include "server_state.h"

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
    f << map_index;
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
    f >> map_index;
    f.close();
    if (!f) {
        Printf("[server_state] load: failed to read from '%s': %s", server_state_filename, std::strerror(errno));
        return false;
    }
    return true;
}
