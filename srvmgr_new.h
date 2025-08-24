#pragma once

#include "a2types.h"
#include "lib\packet.hpp"

void OnInitializeServer();
void OnInitializeMap();
void OnPreInitializeMap();
void OnInitializeMapError(const char* mapfile, const char* error);
void OnServerClosed();
void OnShopError();
void OnLocalMessageBox(const char* message);
void OnServerTic();
void LogIP(A2Player* player);
A2Effect* CreateItemParameter(A2Effect* param, A2InventoryItem* item);
bool CheckItemUpgradable(A2InventoryItem* item);
bool Sv_ProcessClientPacket(int16_t id, A2Player* player, Packet& pack);
void _stdcall ExtDiplomacy(A2Player* player, uint32_t setd);
