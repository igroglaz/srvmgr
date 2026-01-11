#pragma once

#include <string>
#include <vector>
#include <map>

struct ServerState {
    int map_index;
    std::vector<std::string> map_order;

    ServerState();

    bool Save();
    bool ThrottledSave(); // Saves at most once per minute.
    bool Load();
};

extern ServerState server_state;
