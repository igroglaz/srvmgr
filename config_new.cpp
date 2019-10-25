#include "config_new.h"
#include "lib\utils.hpp"
#include "srvmgrdef.h"
#include "cheat_codes_new.h"

unsigned long MAX_SKILL = 100;

///////////////////////////////////////////////////////////////////////////////
// A scope for the Config variables
//
namespace Config {
  std::string ChrBase;

  uint32_t LogMode;
  std::string LogFile;

  uint32_t ServerID;
  bool ServerStarted = false;
  std::string CurrentMapName;
  std::string CurrentMapTitle;

  uint32_t ServerFlags = 0;
  bool MapLoaded = false;
  uint32_t ProtocolVersion;

  std::string IPAddress;
  std::string IPAddress2;
  std::string HatAddress;

  uint32_t ServerCaps = 0;
  uint32_t GameMode = 0;

  bool Suspended = false;
  uint32_t OriginalTime = 0;

  std::string ControlDirectory;
  uint32_t MaxPlayers;

  bool ExitingCleanly = false;
}

///////////////////////////////////////////////////////////////////////////////
// This function parses LogMode string and converts it to corresponding uint32_t flag representation
// Example:
//    LogMode:"SVL_ALL" => 0x0000000F
// ToDo: Review and refactor this function
//
uint32_t ParseLogFlags(std::string string)
{
  if (CheckHex(string))
  {
    return HexToInt(string);
  }
  else
  {
    uint32_t flags = 0;
    std::vector<std::string> v = Explode(string, "|");
    for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
    {
      std::string par = ToLower(Trim((*it)));
      if (!par.length())
        continue;
      uint32_t flag = 0;
      bool erase = false;
      if (par[0] == '-')
      {
        erase = true;
        par.erase(0, 1);
      }

      if (par == "camping")
        flag = SVL_CAMPING;
      else if (par == "buildings")
        flag = SVL_BUILDINGS;
      else if (par == "saves")
        flag = SVL_SAVES;
      else if (par == "dimplomacy")
        flag = SVL_DIPLOMACY;
      else if (par == "all")
        flag = SVL_ALL;

      if (!flag)
        continue;
      if (!erase)
        flags |= flag;
      else
        flags &= ~flag;
    }

    return flags;
  }
}

///////////////////////////////////////////////////////////////////////////////
// This function injects a code that calls ReadConfig function
//
void __declspec(naked) imp_ParseConfig()
{
  __asm
  {
    push    [esp+0x04]
    call    ReadConfig
    add     esp, 4
    retn
  }
}

///////////////////////////////////////////////////////////////////////////////
// This function is responsible for overwriting CString variables in memory
//
void SetCString(byte *cstring, const char *value)
{
  __asm
  {
    push    value
    mov     ecx, [cstring]
    mov     edx, 0x005DDA20
    call    edx
  }
}

///////////////////////////////////////////////////////////////////////////////
// This function adds map and it's duration to the maps's rotation list
//
void AppendMaplist(const char *cMapName, uint32_t cMapDuration)
{
  __asm
  {
    push    [cMapName]
    mov     eax, 0x006D15F8
    push    [eax]
    mov     ecx, 0x006D15F0
    mov     edx, 0x005DAEAB
    call    edx

    push    [cMapDuration]
    mov     eax, 0x006D1620
    push    [eax]
    mov     ecx, 0x006D1618
    mov     edx, 0x005DB2FC
    call    edx
  }
}

///////////////////////////////////////////////////////////////////////////////
// Splits the output of GetPrivateProfileSection function into keys and values pairs.
// INPUT:
//    A pointer to a buffer that contains the key name and value pairs.
//    The buffer is filled with one or more null-terminated strings;
//    the last string is followed by a second null character.
//    Buffer example: ['k','e','y','=','v','a','l','u','e','\0','k','e','y','=','v','a','l','u','e','\0','\0']
char *ReadKeyValuePair(char *buf, char *key, char *value)
{
  unsigned long sz = strlen(buf);
  if (sz == 0)
    return NULL;
  while (*buf != '=')
    *key++ = *buf++;
  *key = '\0';
  ++buf;
  while (*value++ = *buf++)
    ;
  return buf;
}

///////////////////////////////////////////////////////////////////////////////
// This function reads config file and sets corresponding values into memory and Config namespace
// It uses native WinAPI functions to parse ini file:
// GetPrivateProfileInt ()
//    Purpose:
//      Retrieves an integer associated with a key in the specified section of an ini file
//    Input:
//      SECTION_NAME : The name of the section in the ini file
//      KEY_NAME: The name of the key whose associated string is to be retrieved
//      DEFAULT_VALUE: The default value to return if the key cannot be found
//      FILENAME : The name of the ini file
// GetPrivateProfileString ()
//    Purpose:
//      Retrieves a string associated with a key in the specified section of an ini file
//    Input:
//      SECTION_NAME : The name of the section in the ini file
//      KEY_NAME: The name of the key whose associated string is to be retrieved
//      DEFAULT_VALUE: The default value to return if the key cannot be found
//      RETURN_BUFFER : A pointer to a buffer that receives the key name and value pairs
//      RETURN_BUFFER_SIZE : The size of the buffer
//      FILENAME : The name of the ini file
// GetPrivateProfileSection
//    Purpose:
//      Retrieves all the keys and values for the specified section of an ini file
//    Input:
//      SECTION_NAME : The name of the section in the ini file
//      RETURN_BUFFER : A pointer to a buffer that receives the key name and value pairs
//      RETURN_BUFFER_SIZE : The size of the buffer
//      FILENAME : The name of the ini file

int ReadConfig(const char *filename)
{
  // Reserve a buffer for output of GetPrivateProfileString
  char cLogFile[256];
  // Read value of "LogFile" key in section "Settings" in the buffer.
  // Set default value "srvmgr.log" if corresponding key is missing
  GetPrivateProfileString("Settings", "LogFile", "srvmgr.log", cLogFile, sizeof(cLogFile), filename);
  // Set LogFile variable in Config namespace
  Config::LogFile = cLogFile;
  // Print output to the logfile
  Printf("[CONFIG] LogFile: %s", cLogFile);

  Printf("[CONFIG] CONFIG FILE: %s", filename);

  char cLogMode[256];
  GetPrivateProfileString("Settings", "LogMode", "SVL_ALL", cLogMode, sizeof(cLogMode), filename);
  Printf("[CONFIG] LogMode: %s", cLogMode);
  Config::LogMode = ParseLogFlags(cLogMode);

  char cMaps[2048], cMapName[128], cMapDuration[4], *pMaps;
  GetPrivateProfileSection("Maps", cMaps, sizeof(cMaps), filename);
  pMaps = cMaps;
  // Read another map=duration key value pair
  while (pMaps = ReadKeyValuePair(pMaps, cMapName, cMapDuration))
  {
    // Convert char buffer to string
    std::string sMapDuration(cMapDuration);
    // Convert string to float. Convert hours to minutes
    float fMapDuration = std::stof(sMapDuration) * 60.0f;
    Printf("[CONFIG] Map to load: %s Duration: %imin", cMapName, (uint32_t)fMapDuration);
    // Add map to rotation list
    AppendMaplist(cMapName, (uint32_t)fMapDuration);
  }

  char cGameType[256];
  GetPrivateProfileString("Settings", "GameType", "COOPERATIVE", cGameType, sizeof(cGameType), filename);
  Printf("[CONFIG] GameType: %s", cGameType);
  // Convert "GameType" string to corresponding uint32_t flag representation
  uint32_t uGameType;
  if (0 == strcmp(cGameType, "COOPERATIVE"))
    uGameType = GAMEMODE_COOPERATIVE;
  else if (0 == strcmp(cGameType, "DEATHMATCH"))
    uGameType = GAMEMODE_DEATHMATCH;
  else if (0 == strcmp(cGameType, "TEAMPLAY"))
    uGameType = GAMEMODE_TEAMPLAY;
  else if (0 == strcmp(cGameType, "ARENA"))
    uGameType = GAMEMODE_ARENA;
  else if (0 == strcmp(cGameType, "CTF"))
    uGameType = GAMEMODE_CTF;
  Config::GameMode = uGameType;
  // If "GameType" == "CTF", set corresponging uint32_t flag to "ARENA".
  // ToDo: Review this logic
  uGameType = uGameType == GAMEMODE_CTF ? GAMEMODE_ARENA : uGameType;
  Printf("[CONFIG] GameMode Flag: %u", uGameType);
  *(uint32_t *)(ADDR_GAME_MODE) = uGameType;

  char cSrvAddress[256];
  GetPrivateProfileString("Network", "IPAddress", "127.0.0.1:8001", cSrvAddress, sizeof(cSrvAddress), filename);
  Printf("[CONFIG] IPAddress: %s", cSrvAddress);
  SetCString((byte *)(ADDR_NETWORK_SRV_IP), cSrvAddress);

  char cSrvAddress2[256];
  GetPrivateProfileString("Network", "IPAddress2", "127.0.0.1:8002", cSrvAddress2, sizeof(cSrvAddress2), filename);
  Printf("[CONFIG] IPAddress2: %s", cSrvAddress2);
  SetCString((byte *)(ADDR_NETWORK_SRV_IP_2), cSrvAddress2);

  char cHatAddress[256];
  GetPrivateProfileString("Network", "HatAddress", "127.0.0.1:7999", cHatAddress, sizeof(cHatAddress), filename);
  Printf("[CONFIG] HatAddress: %s", cHatAddress);
  SetCString((byte *)(ADDR_NETWORK_HAT_IP), cHatAddress);

  int iProtocolVersion = GetPrivateProfileInt("Network", "ProtocolVersion", 20, filename);
  iProtocolVersion = iProtocolVersion < 8 ? 8 : iProtocolVersion;
  iProtocolVersion = iProtocolVersion > 20 ? 20 : iProtocolVersion;
  Printf("[CONFIG] ProtocolVersion: %i", iProtocolVersion);
  Config::ProtocolVersion = iProtocolVersion;

  int iGameSpeed = GetPrivateProfileInt("Settings", "GameSpeed", 5, filename);
  iGameSpeed = (iGameSpeed < 0 || iGameSpeed > 8) ? 4 : iGameSpeed;
  Printf("[CONFIG] GameSpeed: %i", iGameSpeed);
  *(int32_t *)(ADDR_GAME_SPEED) = iGameSpeed;

  int iRepopDelay = GetPrivateProfileInt("Settings", "RepopDelay", 100, filename);
  iRepopDelay = iRepopDelay < 20 ? 20 : iRepopDelay;
  iRepopDelay = iRepopDelay > 500 ? 500 : iRepopDelay;
  Printf("[CONFIG] RepopDelay: %i", iRepopDelay);
  *(int32_t *)(ADDR_REPOP_DELAY) = iRepopDelay;

  int iServerID = GetPrivateProfileInt("Settings", "ServerID", 0, filename);
  Printf("[CONFIG] ServerID: %i", iServerID);
  *(uint32_t *)(ADDR_SRV_ID) = iServerID;
  Config::ServerID = iServerID;

  char cDescription[256];
  GetPrivateProfileString("Settings", "Description", "pvm", cDescription, sizeof(cDescription), filename);
  Printf("[CONFIG] Description: %s", cDescription);
  SetCString((byte *)(ADDR_DESCRIPTION), cDescription);

  char cChrBase[256];
  GetPrivateProfileString("Settings", "ChrBase", "chr", cChrBase, sizeof(cChrBase), filename);
  Printf("[CONFIG] ChrBase: %s", cChrBase);
  SetCString((byte *)(ADDR_CHR_BASE), cChrBase);

  char cControlDirectory[256];
  GetPrivateProfileString("Settings", "ControlDirectory", "ctl", cControlDirectory, sizeof(cControlDirectory), filename);
  Printf("[CONFIG] ControlDirectory: %s", cControlDirectory);
  Config::ControlDirectory = cControlDirectory;

  int iSayRange = GetPrivateProfileInt("Settings", "SayRange", 512, filename);
  iSayRange = iSayRange > 255 ? 255 : iSayRange;
  Printf("[CONFIG] SayRange: %i", iSayRange);
  *(uint32_t *)(ADDR_SAY_RANGE) = iSayRange;

  int iShoutDelay = GetPrivateProfileInt("Settings", "ShoutDelay", 20, filename);
  iShoutDelay = iShoutDelay < 0 ? 0 : iShoutDelay;
  Printf("[CONFIG] ShoutDelay: %i", iShoutDelay);
  *(int32_t *)(ADDR_SHOUT_DELAY) = iShoutDelay;

  int iMaxPlayers = GetPrivateProfileInt("Settings", "MaxPlayers", 16, filename);
  iMaxPlayers = iMaxPlayers > 32 ? 32 : iMaxPlayers;
  Printf("[CONFIG] MaxPlayers: %i", iMaxPlayers);
  *(uint32_t *)(ADDR_MAX_PLAYERS) = iMaxPlayers;
  Config::MaxPlayers = iMaxPlayers;

  int iLoginTimeout = GetPrivateProfileInt("Settings", "LoginTimeout", 300, filename);
  iLoginTimeout = iLoginTimeout < 10 ? 10 : iLoginTimeout;
  iLoginTimeout = iLoginTimeout > 300 ? 300 : iLoginTimeout;
  Printf("[CONFIG] LoginTimeout: %i", iLoginTimeout);
  *(int32_t *)(ADDR_LOGIN_TIMEOUT) = iLoginTimeout;

  int iReconnectDelay = GetPrivateProfileInt("Settings", "ReconnectDelay", 0, filename);
  iReconnectDelay = iReconnectDelay < 1 ? 0 : iReconnectDelay;
  Printf("[CONFIG] ReconnectDelay: %i", iReconnectDelay);
  *(int32_t *)(ARRD_RECONNECT_DELAY) = iReconnectDelay;

  int iShutdownDelay = GetPrivateProfileInt("Settings", "ShutdownDelay", 60, filename);
  iShutdownDelay = (iShutdownDelay < 1 || iShutdownDelay > 60) ? 5 : iShutdownDelay;
  Printf("[CONFIG] ShutdownDelay: %i", iShutdownDelay);
  *(int32_t *)(ARRD_SHUTDOWN_DELAY) = iShutdownDelay;

  int iFragLimit = GetPrivateProfileInt("Settings", "FragLimit", 2147483647, filename);
  iFragLimit = iFragLimit < 1 ? 2147483647 : iFragLimit;
  Printf("[CONFIG] FragLimit: %i", iFragLimit);
  *(int32_t *)(ADDR_FRAG_LIMIT) = iFragLimit;

  int iTimeLimit = GetPrivateProfileInt("Settings", "TimeLimit", 2147483647, filename);
  iTimeLimit = iTimeLimit < 1 ? 2147483647 : iTimeLimit;
  Printf("[CONFIG] TimeLimit: %i", iTimeLimit);
  *(int32_t *)(ADDR_TIME_LIMIT) = iTimeLimit;

  ///////////////////////////////////////////////////////////////////////////////
  // ToDo: Review flag purpose
  //
  int iScaledMaps = GetPrivateProfileInt("Settings", "ScaledMaps", 0, filename);
  Printf("[CONFIG] ScaledMaps: %i", iScaledMaps);
  *(uint32_t *)(ADDR_SCALED_MAPS) = iScaledMaps;

  ///////////////////////////////////////////////////////////////////////////////
  // ToDo: Review flag purpose
  //
  int iTreasureProbability = GetPrivateProfileInt("Settings", "TreasureProbability", 50, filename);
  iTreasureProbability = iTreasureProbability < 0 ? 0 : iTreasureProbability;
  iTreasureProbability = iTreasureProbability > 100 ? 100 : iTreasureProbability;
  Printf("[CONFIG] TreasureProbability: %i", iTreasureProbability);
  *(int32_t *)(ADDR_TREASURE_PROBABILITY) = iTreasureProbability;

  char cServerFlags[256];
  GetPrivateProfileString("Settings", "ServerFlags", NULL, cServerFlags, sizeof(cServerFlags), filename);
  Printf("[CONFIG] ServerFlags: %s", cServerFlags);
  Config::ServerFlags = ParseFlags(cServerFlags);
  if (Config::ServerFlags & SVF_SOFTCORE)
  {
    MAX_SKILL = 110;
    Config::ServerCaps |= SVC_SOFTCORE;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Parses "ServerCaps" string and converts it to corresponding uint32_t flag representation
  // Example:
  //    ServerCaps:"SVL_ALL" => 0x0000000B
  // ToDo: Refactor pasring code
  //
  char cServerCaps[256];
  GetPrivateProfileString("Settings", "ServerCaps", NULL, cServerCaps, sizeof(cServerCaps), filename);
  Printf("[CONFIG] ServerCaps: %s", cServerCaps);
  std::vector<std::string> svcap = Explode(cServerCaps, "|");
  uint32_t caps = 0;
  for (std::vector<std::string>::iterator it = svcap.begin(); it != svcap.end(); ++it)
  {
    std::string &cap = (*it);
    if (!cap.length())
      continue;
    uint32_t flag = 0;
    bool erase = false;
    if (cap[0] == '-')
    {
      erase = true;
      cap.erase(0, 1);
    }

    if (cap == "ALL")
      flag = SVC_ALL;
    else if (cap == "DETAILED_INFO")
      flag = SVC_DETAILED_INFO;
    else if (cap == "FIXED_MAPLIST")
      flag = SVC_FIXED_MAPLIST;
    else if (cap == "SAVE_DATABASE")
      flag = SVC_SAVE_DATABASE;

    if (!erase)
      caps |= flag;
    else
      caps &= ~flag;
  }
  Config::ServerCaps &= SVC_SOFTCORE;
  Config::ServerCaps |= caps & ~SVC_SOFTCORE;

  return 0;
}
