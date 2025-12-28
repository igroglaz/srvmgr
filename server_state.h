#pragma once

struct ServerState {
    int map_index;

    ServerState();

    bool Save();
    bool Load();
};

extern ServerState server_state;
