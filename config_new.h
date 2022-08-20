#pragma once

#include <stdint.h>
#include "lib\utils.hpp"

extern unsigned long MAX_SKILL;
extern unsigned long MAX_EXP_ON_SKILL;


struct MainCharacterParameters {
    uint8_t Body, Reaction, Mind, Spirit;
    uint8_t ResistFire, ResistWater, ResistAir, ResistEarth, ResistAstral;
};

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

    extern double DeathExpMult;
    extern double KilledExpMult;
    extern double PKExpMult;
    extern double RespawnExpMult2;

    extern float InventoryDropProbability;
    extern float WearDropProbability;

    extern MainCharacterParameters WarriorMaleMaxParameters;
    extern MainCharacterParameters WarriorFemaleMaxParameters;
    extern MainCharacterParameters MageMaleMaxParameters;
    extern MainCharacterParameters MageFemaleMaxParameters;

    extern uint32_t MinQuestReward;
    extern uint32_t MaxQuestReward;

    extern uint32_t QuestRollInterval;
    extern float mage_pvp_dmg_factor;
    extern int16_t max_pvp_dmg;
    extern float shop_potions_factor;
    extern bool server_rotate_maps;

    extern bool AllowOnlyOneQuest_KillNMonsters;
    extern bool AllowOnlyOneQuest_KillTheMonster;
    extern bool AllowOnlyOneQuest_KillTheGroup;

    extern std::string IPAddress;
    extern uint16_t IPAddressP;

    extern std::string IPAddress2;
    extern uint16_t IPAddress2P;

    extern std::string HatAddress;
    extern uint16_t HatAddressP;

}

int ReadConfig(const char* filename);