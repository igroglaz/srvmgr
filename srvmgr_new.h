#pragma once

#include "lib\packet.hpp"

void OnInitializeServer();
void OnInitializeMap();
void OnPreInitializeMap();
void OnInitializeMapError(const char* mapfile, const char* error);
void OnServerClosed();
void OnShopError();
void OnLocalMessageBox(const char* message);
void OnServerTic();
void LogIP(byte* player);
byte* CreateItemParameter(byte* param, byte* item);
bool CheckItemUpgradable(byte* item);
bool Sv_ProcessClientPacket(int16_t id, byte* player, Packet& pack);
void _stdcall ExtDiplomacy(byte* player, uint32_t setd);
