#pragma once

#include <string>
#include <vector>

struct ServerState {
    int map_index;
    std::vector<std::string> map_order;

    ServerState();

    bool Save();
    bool Load();
};

extern ServerState server_state;
