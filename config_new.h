#pragma once

#include <stdint.h>
#include "lib\utils.hpp"

extern unsigned long MAX_SKILL;

///////////////////////////////////////////////////////////////////////////////
// These constants represents memory addresses for corresponding variables
// Example: ADDR_NETWORK_HAT_IP
//          refers to the memory address containing Hat's IP_ADDRESS:PORT
//
const int ADDR_NETWORK_SRV_IP = 0x006D15B0;
const int ADDR_NETWORK_SRV_IP_2 = 0x006D15B4;
const int ADDR_NETWORK_HAT_IP = 0x006D15B8;
const int ADDR_GAME_MODE = 0x006D1648;
const int ADDR_GAME_SPEED = 0x006D15A8;
const int ADDR_REPOP_DELAY = 0x006D15A0;
const int ADDR_DESCRIPTION = 0x006D15C0;
const int ADDR_CHR_BASE = 0x006D15BC;
const int ADDR_SRV_ID = 0x006D15C4;
const int ADDR_SAY_RANGE = 0x006D162C;
const int ADDR_SHOUT_DELAY = 0x006D1630;
const int ADDR_MAX_PLAYERS = 0x006D163C;
const int ADDR_LOGIN_TIMEOUT = 0x006D1640;
const int ARRD_RECONNECT_DELAY = 0x006D1644;
const int ARRD_SHUTDOWN_DELAY = 0x006D1658;
const int ADDR_FRAG_LIMIT = 0x006D164C;
const int ADDR_TIME_LIMIT = 0x006D1660;
const int ADDR_SCALED_MAPS = 0x006D1654;
const int ADDR_TREASURE_PROBABILITY = 0x006D1664;

namespace Config {
  extern uint32_t LogMode;
  extern std::string LogFile;

  extern std::string ChrBase;

  extern uint32_t ServerID;

  extern bool ServerStarted;

  extern std::string CurrentMapName;
  extern std::string CurrentMapTitle;

  extern uint32_t ServerFlags;
  extern bool MapLoaded;
  extern uint32_t ProtocolVersion;

  extern uint32_t ServerCaps;
  extern uint32_t GameMode;

  extern bool Suspended;
  extern uint32_t OriginalTime;
  extern uint32_t MaxPlayers;

  extern std::string ControlDirectory;

  extern bool ExitingCleanly;
} // namespace Config

int ReadConfig(const char *filename);