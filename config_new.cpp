#include "config_new.h"
#include "lib\utils.hpp"
#include "srvmgrdef.h"
#include "cheat_codes_new.h"

#include <fstream>
#include <algorithm>

unsigned long MAX_SKILL = 100;

uint32_t ParseLogFlags(std::string string)
{
	if(CheckHex(string))
    {
        return HexToInt(string);
    }
    else
    {
        uint32_t flags = 0;
        std::vector<std::string> v = Explode(string, "|");
        for(std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it)
        {
            std::string par = ToLower(Trim((*it)));
            if(!par.length()) continue;
            uint32_t flag = 0;
            bool erase = false;
            if(par[0] == '-')
            {
                erase = true;
                par.erase(0, 1);
            }

            if(par == "camping")
                flag = SVL_CAMPING;
            else if(par == "buildings")
                flag = SVL_BUILDINGS;
            else if(par == "saves")
                flag = SVL_SAVES;
            else if(par == "dimplomacy")
                flag = SVL_DIPLOMACY;
			else if(par == "all")
				flag = SVL_ALL;

            if(!flag) continue;
            if(!erase) flags |= flag;
            else flags &= ~flag;
        }

        return flags;
    }
}

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

void SetCString(byte* cstring, const char* value)
{
    __asm
    {
        push    value
        mov     ecx, [cstring]
        mov     edx, 0x005DDA20
        call    edx
    }
}

void AppendMaplist(const char* filename, uint32_t duration)
{
    __asm
    {
        push    [filename]
        mov     eax, 0x006D15F8
        push    [eax]
        mov     ecx, 0x006D15F0
        mov     edx, 0x005DAEAB
        call    edx

        push    [duration]
        mov     eax, 0x006D1620
        push    [eax]
        mov     ecx, 0x006D1618
        mov     edx, 0x005DB2FC
        call    edx
    }
}

namespace Config
{
    uint32_t ExceptionCount = 0;

	std::string ChrBase = "chr";

	uint32_t LogMode = SVL_ALL;
    std::string LogFile = "server.log";

    uint32_t ServerID = 0;
	bool ServerStarted = false;
    std::string CurrentMapName = "N/A";
    std::string CurrentMapTitle = "N/A";

    uint32_t ServerFlags = 0;
    bool MapLoaded = false;
    uint32_t ProtocolVersion = 7;

    std::string IPAddress = "0.0.0.0";
    uint16_t IPAddressP = 8001;

    std::string IPAddress2 = "0.0.0.0";
    uint16_t IPAddress2P = 8002;

    std::string HatAddress = "127.0.0.1";
    uint16_t HatAddressP = 7999;

    uint32_t MaxPaletteAllowed = 16;

    std::string SqlAddress = "127.0.0.1";
    uint16_t SqlPort = 3306;
    std::string SqlLogin = "root";
    std::string SqlPassword = "";
    std::string SqlDatabase = "logins";

    uint32_t ServerCaps = 0;
    uint32_t GameMode = 0;

    bool Suspended = false;
    uint32_t OriginalTime = 0;

    std::vector<std::string> Includes;

	std::string ControlDirectory = "ctl";

    uint32_t MaxPlayers = 16;

	bool ExitingCleanly = false;
}

int ReadConfig(const char* filename)
{
    if(!Config::Includes.size()) // root config
    {
        // set defaults
        SetCString((byte*)(0x006D15B0), "127.0.0.1:8001");     // IPAddress
        SetCString((byte*)(0x006D15B4), "127.0.0.1:8002");     // IPAddress2
        SetCString((byte*)(0x006D15B8), "127.0.0.1:7999");   // HatAddress
        SetCString((byte*)(0x006D15BC), "chr");              // ChrBase
    }

    using namespace std;
    ifstream f_cfg;
    f_cfg.open(filename, ios::in);
    if(!f_cfg.is_open()) return -1;

    string line;
    int32_t lnid = 0;
    std::string section = "root";

    while(getline(f_cfg, line))
    {
        lnid++;
        bool enc = false;
        for(size_t i = 0; i < line.length(); i++)
        {
            if(line[i] == '"') enc = !enc;
            if(enc) continue;
            if(line[i] == '#' || line[i] == ';' ||
                (line[i] == '/' && i != line.length()-1 && line[i+1] == '/'))
            {
                line.erase(i);
                break;
            }
        }

        line = Trim(line);
        if(!line.length()) continue;

        if(line[0] == '!')
        {
            if(line.find("!include ") == 0)
            {
                line.erase(0, 9);
                line = Trim(line);
                if(line[0] == '"')
                {
                    line.erase(0, 1);
                    line.erase(line.length()-1, 1);
                }

                if(std::find(Config::Includes.begin(), Config::Includes.end(), line) == Config::Includes.end())
                {
                    Config::Includes.push_back(line);
                    int32_t result = ReadConfig(line.c_str());
                    if(result == -1)
                        Printf("ReadConfig: file not found in \"%s\"!", line.c_str());
                    else if(result > 0)
                        Printf("ReadConfig: syntax error at \"%s\":%u!", line.c_str(), result);
                }
                else
                {
                    Printf("ReadConfig: recursive inclusion detected in \"%s\":%u.", filename, lnid);
                }
            }
            continue;
        }

        if(line[0] == '[')
        {
            line.erase(0, 1);
            size_t whs = line.find_first_of(']');
            if(whs == string::npos)
            {
                return lnid;
            }
            line.erase(whs);
            section = ToLower(Trim(line));

            if(section == "bannedips" ||
               section == "bannedplayers" ||
               section == "reporttowww") Printf("ReadConfig: obsolete section (%s)", section.c_str());
            else if(section != "settings" &&
                    section != "maps" &&
                    section != "network" &&
                    section != "mysql") return lnid;
        }
        else
        {
            std::string parameter, value;
            size_t whd = line.find_first_of('=');
            if(whd != string::npos)
            {
                parameter = line;
                parameter.erase(whd);
                value = line;
                value.erase(0, whd+1);
            }
            else
            {
                value = "";
                parameter = line;
            }

            parameter = ToLower(Trim(parameter));
            value = Trim(value);
            if(value[0] == '"')
            {
                if(value[value.length()-1] != '"')
                {
                    return lnid;
                }

                value.erase(0, 1);
                value.erase(value.length()-1, 1);
            }

            if(section == "settings")
            {
                if(parameter == "chrbase")
                {
                    if(value[value.length()-1] != '\\') value += '\\';
					Config::ChrBase = value;
                    SetCString((byte*)(0x006D15BC), value.c_str());
                }
                else if(parameter == "description")
                {
                    SetCString((byte*)(0x006D15C0), value.c_str());
                }
                else if(parameter == "repopdelay")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 20) val = 20;
                    if(val > 500) val = 500;
                    *(int32_t*)(0x006D15A0) = val;
                }
                else if(parameter == "gamespeed")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 0 || val > 8) val = 4;
                    *(int32_t*)(0x006D15A8) = val;
                }
                else if(parameter == "logfile")
                {
                    Config::LogFile = value;
                }
				else if(parameter == "logmode")
				{
					Config::LogMode = ParseLogFlags(value);
				}
                else if(parameter == "serverid")
                {
                    if(!CheckInt(value)) return lnid;
                    uint32_t val = StrToInt(value);
                    Config::ServerID = val;
                    *(uint32_t*)(0x006D15C4) = val;
                }
                else if(parameter == "sayrange")
                {
                    if(!CheckInt(value)) return lnid;
                    uint32_t val = StrToInt(value);
                    if(val > 255 || !val) val = 255;
                    *(uint32_t*)(0x006D162C) = val;
                }
                else if(parameter == "shoutdelay")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 0) val = 0;
                    *(int32_t*)(0x006D1630) = val;
                }
                else if(parameter == "maxplayers")
                {
                    if(!CheckInt(value)) return lnid;
                    uint32_t val = StrToInt(value);
                    if(val > 32) val = 32;
                    Config::MaxPlayers = val;
                    *(uint32_t*)(0x006D163C) = val;
                }
                else if(parameter == "gametype" ||
                        parameter == "gamemode")
                {
                    uint32_t gmode = 0;
                    if(CheckInt(value))
                    {
                        uint32_t val = StrToInt(value);
                        if(val > 3) return lnid;
                        gmode = val;
                    }
                    else
                    {
                        value = ToLower(value);
                        if(value == "cooperative")
                            gmode = GAMEMODE_COOPERATIVE;
                        else if(value == "deathmatch")
                            gmode = GAMEMODE_DEATHMATCH;
                        else if(value == "teamplay")
                            gmode = GAMEMODE_TEAMPLAY;
                        else if(value == "arena")
                            gmode = GAMEMODE_ARENA;
                        else if(value == "ctf")
                            gmode = GAMEMODE_CTF;
                        else return lnid;
                    }

                    Config::GameMode = gmode;
                    if(gmode == GAMEMODE_CTF)
                        *(uint32_t*)(0x006D1648) = GAMEMODE_ARENA; // для сервера это арена
                    else *(uint32_t*)(0x006D1648) = gmode;
                }
                else if(parameter == "logintimeout")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 10) val = 10;
                    if(val > 300) val = 300;

                    *(int32_t*)(0x006D1640) = val;
                }
                else if(parameter == "reconnectdelay")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 1) val = 0;
                    *(int32_t*)(0x006D1644) = val;
                }
                else if(parameter == "shutdowndelay")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 1 || val > 60)
                        val = 5;
                    *(int32_t*)(0x006D1658) = val;
                }
                else if(parameter == "fraglimit")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 1) val = 2147483647;
                    *(int32_t*)(0x006D164C) = val;
                }
                else if(parameter == "arenatimelimit" ||
                        parameter == "timelimit")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 1) val = 2147483647;
                    *(int32_t*)(0x006D1660) = val;
                }
                else if(parameter == "scaledmaps")
                {
                    if(!CheckBool(value)) return lnid;
                    bool bv = StrToBool(value);
                    if(bv) *(uint32_t*)(0x006D1654) = 1;
                }
                else if(parameter == "treasureprobability")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 0) val = 0;
                    if(val > 100) val = 100;
                    *(int32_t*)(0x006D1664) = val;
                }
                else if(parameter == "serverflags")
                {
                    Config::ServerFlags = ParseFlags(value);
					if(Config::ServerFlags & SVF_SOFTCORE)
					{
						MAX_SKILL = 110;
						Config::ServerCaps |= SVC_SOFTCORE;
					}
                }
                else if(parameter == "maxpaletteallowed")
                {
                    int32_t val = StrToInt(value);
                    if(val < 0) val = 0;
                    Config::MaxPaletteAllowed = val;
                }
                else if(parameter == "servercaps")
                {
                    value = Trim(ToLower(value));
                    std::vector<std::string> svcap = Explode(value, "|");
                    uint32_t caps = 0;
                    for(std::vector<std::string>::iterator it = svcap.begin(); it != svcap.end(); ++it)
                    {
                        std::string& cap = (*it);
                        if(!cap.length()) continue;
                        uint32_t flag = 0;
                        bool erase = false;
                        if(cap[0] == '-')
                        {
                            erase = true;
                            cap.erase(0, 1);
                        }

                        if(cap == "all") flag = SVC_ALL;
                        else if(cap == "detailed_info") flag = SVC_DETAILED_INFO;
                        else if(cap == "fixed_maplist") flag = SVC_FIXED_MAPLIST;
                        else if(cap == "save_database") flag = SVC_SAVE_DATABASE;

                        if(!flag) return lnid;
                        if(!erase) caps |= flag;
                        else caps &= ~flag;
                    }

                    Config::ServerCaps &= SVC_SOFTCORE;
					Config::ServerCaps |= caps & ~SVC_SOFTCORE;
                }
				else if(parameter == "controldirectory")
				{
					Config::ControlDirectory = value;
				}
                else if(parameter == "flagscore" ||
                        parameter == "protocol" ||
                        parameter == "ipaddress" ||
                        parameter == "ipaddress2" ||
                        parameter == "save" ||
                        parameter == "hataddress")
                {
                    Printf("ReadConfig: obsolete parameter (settings/%s)", parameter.c_str());
                }
            }
            else if(section == "network")
            {
                if(parameter == "protocolversion")
                {
                    if(!CheckInt(value)) return lnid;
                    int32_t val = StrToInt(value);
                    if(val < 8) val = 8;
                    if(val > 20) val = 20;
                    Config::ProtocolVersion = val;
                }
                else if(parameter == "ipaddress" ||
                        parameter == "ipaddress2" ||
                        parameter == "hataddress")
                {
                    vector<string> ipd = Explode(value, ":");
                    if(ipd.size() > 2) return lnid;
                    if(!CheckIP(ipd[0])) return lnid;
                    uint32_t port = 8001;
                    if(ipd.size() == 2)
                    {
                        if(!CheckInt(ipd[1])) return lnid;
                        port = StrToInt(ipd[1]);
                        if(port > 65535) return lnid;
                    }

                    if(parameter == "ipaddress")
                    {
                        SetCString((byte*)(0x006D15B0), value.c_str());
                        Config::IPAddress = ipd[0];
                        Config::IPAddressP = port;
                    }
                    else if(parameter == "ipaddress2")
                    {
                        SetCString((byte*)(0x006D15B4), value.c_str());
                        Config::IPAddress2 = ipd[0];
                        Config::IPAddress2P = port;
                    }
                    else
                    {
                        SetCString((byte*)(0x006D15B8), value.c_str());
                        Config::HatAddress = ipd[0];
                        Config::HatAddressP = port;
                    }
                }
            }
            else if(section == "mysql")
            {
                if(parameter == "server")
                {
                    vector<string> ipd = Explode(value, ":");
                    if(ipd.size() > 2) return lnid;
                    if(!CheckIP(ipd[0])) return lnid;
                    uint32_t port = 3306;
                    Config::SqlAddress = ipd[0];
                    Config::SqlPort = port;
                }
                else if(parameter == "login")
                    Config::SqlLogin = value;
                else if(parameter == "password")
                    Config::SqlPassword = value;
                else if(parameter == "database")
                    Config::SqlDatabase = value;
                else return lnid;
            }
            else if(section == "maps")
            {
                std::string m_filename = parameter;
                uint32_t m_time = 2147483647;
                if(value.length()) m_time = (uint32_t)(StrToFloat(value) * 60.0);
                AppendMaplist(m_filename.c_str(), m_time);
            }
            else
            {
                Printf("ReadConfig: unknown parameter (%s/%s)", section.c_str(), parameter.c_str());
            }
        }
    }

    f_cfg.close();

    return 0;
}
