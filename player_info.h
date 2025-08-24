#pragma once

#include "syslib.h"
#include <vector>
#include "a2types.h"
#include "lib/packet.hpp"

struct Player
{
    bool Exists;
    A2Player* Class;
    
    bool Casted;
    bool ShouldReturn;
    uint32_t LastReturn;
    bool GodMode;
    A2Player* GodSetter;

    int8_t SetSpells;
    uint32_t LastSpells;
    A2Player* SpellSetter;

    byte CastSpell[0x14];
    std::vector<A2InventoryItem*> SavedItems;

    uint32_t UnmuteDate;

    uint32_t Vision[256][256]; // used in scanrange

    //
    std::vector<Packet> EnqueuedPackets;
};

extern Player Players[32];

Player* PI_Get(A2Player* player);
void PI_Reset();
void PI_Clear(Player& struc);
void PI_Create(A2Player* player);
void PI_Delete(A2Player* player);
