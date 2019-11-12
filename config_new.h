#pragma once

#include <stdint.h>
#include "lib\utils.hpp"

extern unsigned long MAX_SKILL;

namespace Config
{
    extern uint32_t ExceptionCount;

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

    extern uint32_t MaxPaletteAllowed;

    extern std::string SqlAddress;
    extern uint16_t SqlPort;
    extern std::string SqlLogin;
    extern std::string SqlPassword;
    extern std::string SqlDatabase;

    extern uint32_t ServerCaps;
    extern uint32_t GameMode;

    extern bool Suspended;
    extern uint32_t OriginalTime;
    extern uint32_t MaxPlayers;

	extern std::string ControlDirectory;

	extern bool ExitingCleanly;
}

int ReadConfig(const char* filename);