#include <ctime>
#include "protolayer_hat.h"
#include "config_new.h"
#include "srvmgrdef.h"
#include "lib\utils.hpp"
#include "syslib.h"
#include "zxmgr.h"
#include "srvmgr.h"
#include "player_info.h"
#include "screenshots.h"

namespace NetHat
{
    std::string HatAddr = "127.0.0.1";
    uint16_t HatPort = 7999;
    std::string ControlAddr = "";
    uint16_t ControlPort = 0;

    SOCKET Socket = NULL;
    bool Connected = false;

    uint32_t LastReconnect = 0;

    PacketReceiver Receiver;

    bool HaveInfo = false;
    ServerInfo Info;

    bool ShuttingDown = false;

    uint32_t LastUpdate = 0;
}

/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! PACKETS SENDED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ///
bool NetCmd_HatAuth()
{
    Packet pack;
    pack.WriteUInt8(0x10);
    pack.WriteUInt32(Config::ServerCaps);

    return (SOCK_SendPacket(NetHat::Socket, pack, Config::ProtocolVersion) == 0);
}

bool NetCmd_Shutdown()
{
    NetHat::ShuttingDown = true;

    Packet pack;
    pack.WriteUInt8(0x11);

    return (SOCK_SendPacket(NetHat::Socket, pack, Config::ProtocolVersion) == 0);
}

bool NetCmd_UpdateInfo()
{
    if (Config::ServerCaps & SVC_DETAILED_INFO)
    {
        Packet pack;
        pack.WriteUInt8(0x12);
        // базовая информация
        if (NetHat::Info.PlayerCount == 0xFF && NetHat::Info.MapLevel == 0xFF &&
            NetHat::Info.GameMode == 0xFF && NetHat::Info.MapSize == 0xFF) // shutdown
        {
            return NetCmd_Shutdown();
        }
        else
        {
            pack.WriteUInt32(NetHat::Info.GameMode);
            pack.WriteString(NetHat::Info.MapName);
            pack.WriteUInt8(NetHat::Info.MapLevel);
            uint32_t p_mapwidth = *(uint32_t*)(*(uint32_t*)(0x006B16A8) + 0x50000);
            uint32_t p_mapheight = *(uint32_t*)(*(uint32_t*)(0x006B16A8) + 0x50004);
            uint32_t p_maptime = *(uint32_t*)(*(uint32_t*)(0x00642C2C) + 0x254) / 1000;
            pack.WriteUInt32(p_mapwidth - 16);  // Map Width
            pack.WriteUInt32(p_mapheight - 16); // Map Height (может быть != Map Width)
            pack.WriteUInt32(p_maptime);        // Map Time
            pack.WriteUInt32(Config::ServerFlags);

            std::vector<byte*> tmp_players_Z = zxmgr::GetPlayers();
            std::vector<byte*> tmp_players;
            // удаляем AI-игроков
            for (std::vector<byte*>::iterator it = tmp_players_Z.begin(); it != tmp_players_Z.end(); ++it)
            {
                byte* player = (*it);
                if (!*(uint32_t*)(player + 0x2C)) // NOT AI
                    tmp_players.push_back(player);
            }

            pack.WriteUInt32(tmp_players.size());
            for (std::vector<byte*>::iterator it = tmp_players.begin(); it != tmp_players.end(); ++it)
            {
                byte* player = (*it);

                const char* player_nickname = *(const char**)(player + 0x18);
                const char* player_login = "artificial";
                if (!*(uint32_t*)(player + 0x2C))
                    player_login = *(const char**)(player + 0xA78);
                uint32_t player_id1 = *(uint32_t*)(player + 0x10);
                uint32_t player_id2 = *(uint32_t*)(player + 0x14);
                pack.WriteString(player_nickname);
                pack.WriteString(player_login);
                pack.WriteUInt32(player_id1);
                pack.WriteUInt32(player_id2);
                byte* vd = zxmgr::GetNetworkStruct(player);
                bool player_connected = (vd);
                pack.WriteUInt8(player_connected);
                if (*(uint32_t*)(player + 0x2C) || !player_connected) // AI or disconnected
                {
                    pack.WriteString("");
                }
                else
                {
                    if (vd)
                    {
                        const char* vd_ip = (const char*)(vd + 8);
                        //pack << std::string(vd_ip);
                        pack.WriteString(vd_ip);
                    }
                    else pack.WriteString("");
                }
            }

            // send the logins to return
            std::vector<std::string> logins;
            std::string mask = Config::ChrBase;
            mask += "*";
            WIN32_FIND_DATA fd;
            HANDLE h = FindFirstFileA(mask.c_str(), &fd);
            if (h != INVALID_HANDLE_VALUE)
            {
                bool found = true;
                while (found)
                {
                    std::string login = std::string(fd.cFileName);
                    found = (FindNextFileA(h, &fd) != 0);
                    
                    if (login.find(".") != std::string::npos)
                        continue;

                    logins.push_back(login);
                }
            }

            FindClose(h);

            pack.WriteUInt32(logins.size());
            for (std::vector<std::string>::iterator it = logins.begin(); it != logins.end(); ++it)
            {
                std::string& login = (*it);
                pack.WriteString(login);
            }
        }

        return (SOCK_SendPacket(NetHat::Socket, pack, Config::ProtocolVersion) == 0);
    }
    else
    {
        Packet pack;
        pack.WriteUInt8(0xD2);
        pack.WriteUInt8(NetHat::Info.PlayerCount);
        pack.WriteUInt8(NetHat::Info.MapLevel);
        pack.WriteUInt8(NetHat::Info.GameMode);
        pack.WriteUInt8(NetHat::Info.MapSize);
        pack.WriteString(NetHat::Info.MapName);

        return (SOCK_SendPacket(NetHat::Socket, pack, Config::ProtocolVersion) == 0);
    }
}

bool NetCmd_Broadcast(std::string what)
{
    Packet pack;
    pack.WriteUInt8(0x13);
    pack.WriteString(what);

    return (SOCK_SendPacket(NetHat::Socket, pack, Config::ProtocolVersion) == 0);
}

bool NetCmd_UnlockLogin(std::string login)
{
    Packet pack;
    pack.WriteUInt8(0x14);
    pack.WriteString(login);

    return (SOCK_SendPacket(NetHat::Socket, pack, Config::ProtocolVersion) == 0);
}

/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! [/PACKETS SENDED ] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ///

/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CONNECTION CONTROL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ///
void _stdcall Net_HatPrepare()
{
    char* stadr = *(char**)(0x006D15B4);
    std::string dadr(stadr);

    std::vector<std::string> ipd = Explode(dadr, ":");
    NetHat::ControlAddr = ipd[0];
    if (ipd.size() == 2)
        NetHat::ControlPort = StrToInt(ipd[1]) + 1000;

    char* stadd = *(char**)(0x006D15B8);
    std::string dadd(stadd);

    ipd = Explode(dadd, ":");
    NetHat::HatAddr = ipd[0];
    if (ipd.size() == 2)
        NetHat::HatPort = StrToInt(ipd[1]);

    NetHat::ShuttingDown = false;
    NetHat::LastReconnect = 0;
}

bool Net_HatInit()
{
    if (NetHat::ControlPort == 0 && !NetHat::ControlAddr.length()) return false;
    if (NetHat::ShuttingDown) return false;

    NetHat::Socket = SOCK_Connect(NetHat::HatAddr, NetHat::HatPort, NetHat::ControlAddr, NetHat::ControlPort);
    if (NetHat::Socket == SERR_NOTCREATED)
    {
        Net_HatShutdown();
        //Printf("Warning: control connection to hat failed.");
        return false;
    }

    NetHat::Connected = true;
    NetHat::Receiver.Connect(NetHat::Socket);
    // number of milliseconds from the moment when system was started
    NetHat::LastUpdate = GetTickCount();

    if (!NetCmd_HatAuth())
    {
        Net_HatShutdown();
        Printf("Warning: control connection to hat failed (disconnected).");
        return false;
    }

    Printf("Control connection to hat established.");

    return true;
}

bool Net_HatProcess()
{
    if (NetHat::ShuttingDown) return false;
    if (!NetHat::Connected) return false;
    if (!NetHat::Receiver.Receive(Config::ProtocolVersion)) return false;

    // number of milliseconds from the moment when system was started - Nethat..
    if (GetTickCount() - NetHat::LastUpdate > 15000 && NetHat::HaveInfo)
    {
        if (!NetCmd_UpdateInfo()) NetHat::Connected = false;
        NetHat::LastUpdate = GetTickCount();
    }

    Packet pack;
    while (NetHat::Receiver.GetPacket(pack))
    {
        uint8_t packet_id = pack.ReadUInt8();

        switch (packet_id)
        {
        case 0x63: // broadcast from hat
        {
            std::string message = pack.ReadString();
            zxmgr::SendMessage(NULL, message.c_str());
            break;
        }
        case 0x64: // screenshot information
        {
            std::string login = pack.ReadString();
            pack.ReadUInt8(); // ????
            uint32_t uid = pack.ReadUInt32();
            bool done = pack.ReadUInt8() != 0;
            std::string url = pack.ReadString();
            ClientScreenshot* cs = ClientScreenshot_FindByUID(uid);
            // means the server crashed, and we are receiving screenshot info from earlier time
            if (!cs)
            {
                if (done)
                {
                    if (!!url.size())
                        Printf("screenshot: %s: Comeback transmission succeeded (saved to %s).", login.c_str(), url.c_str());
                    else Printf("screenshot: %s: Comeback transmission failed.", login.c_str());
                }
                else Printf("screenshot: %s: Comeback transmission initiated.", login.c_str());
            }
            else
            {
                if (cs->SourcePlayer)
                {
                    if (done)
                    {
                        if (!!url.size())
                            zxmgr::SendMessage(cs->SourcePlayer, "screenshot: %s: Transmission succeeded (saved to %s).", login.c_str(), url.c_str());
                        else zxmgr::SendMessage(cs->SourcePlayer, "screenshot: %s: Transmission succeeded (saved to %s).", login.c_str(), url.c_str());
                    }
                    else zxmgr::SendMessage(cs->SourcePlayer, "screenshot: %s: Transmission initiated.", login.c_str());
                }
                else
                {
                    if (done)
                    {
                        if (!!url.size())
                            Printf("screenshot: %s: Transmission succeeded (saved to %s).", login.c_str(), url.c_str());
                        else Printf("screenshot: %s: Transmission succeeded (saved to %s).", login.c_str(), url.c_str());
                    }
                    else Printf("screenshot: %s: Transmission initiated.", login.c_str());
                }
            }
            if (done)
                ClientScreenshot_Drop(uid);
            break;
        }
        case 0x65: // mute player packet
        {
            std::string login = pack.ReadString();
            time_t unmutedate = pack.ReadUInt32();
            struct tm parsedTime;
            localtime_s(&parsedTime, &unmutedate);
            byte* player = zxmgr::FindByLogin(login.c_str());
            std::vector<byte*> players = zxmgr::GetPlayers();
            Player* pi = PI_Get(player);
            if (pi)
            {
                pi->UnmuteDate = unmutedate;
                Printf("Player %s (login %s) should be muted until %02d.%02d.%04d %02d:%02d:%02d.",
                    *(const char**)(player + 0x18), login.c_str(), parsedTime.tm_mday, parsedTime.tm_mon + 1, parsedTime.tm_year + 1900, parsedTime.tm_hour, parsedTime.tm_min, parsedTime.tm_sec);
            }
            break;
        }
        default: break;
        }
    }

    return true;
}

void Net_HatShutdown()
{
    if (NetHat::Connected && !NetHat::Socket) return;
    if (NetHat::Socket) SOCK_Destroy(NetHat::Socket);
    NetHat::Socket = NULL;
    NetHat::Connected = false;
}
/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! [/CONNECTION CONTROL ] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ///

/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CONNECTION IMPORTS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ///
void __declspec(naked) imp_NetLayer_FirstConnect()
{ // 4F0BEF
    __asm
    {
        push    ebp
        mov        ebp, esp
        sub        esp, 0x240
        mov        [ebp-0x230], ecx
        call    Net_HatPrepare

        mov        edx, 0x004F0BFE
        jmp        edx
    }
}

void Net_RegularProc()
{
    if (NetHat::ControlPort != 0 && NetHat::ControlAddr.length() && !NetHat::ShuttingDown)
    {
        if (!Net_HatProcess() && (GetTickCount()-NetHat::LastReconnect > 5000))
        {
            Net_HatShutdown();
            Net_HatInit(); // reconnect
            NetHat::LastReconnect = GetTickCount();
        }
    }

    Sleep(1);
}

void _stdcall imp2_UpdateInfo()
{
    if (!NetCmd_UpdateInfo()) NetHat::Connected = false;
}

void _stdcall imp_UpdateInfo(unsigned char a_pcount, const char* a_name, unsigned char a_level, unsigned char a_type, unsigned char a_size)
{
    NetHat::Info.PlayerCount = a_pcount;
    NetHat::Info.MapLevel = a_level;
    NetHat::Info.MapName = a_name;
    NetHat::Info.GameMode = a_type;
    NetHat::Info.MapSize = a_size;
    NetHat::HaveInfo = true;

    if (!NetCmd_UpdateInfo()) NetHat::Connected = false;
}

void _stdcall imp2_ServerClosed()
{
    if (!NetCmd_Shutdown()) NetHat::Connected = false;
}

/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! [/CONNECTION IMPORTS ] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ///

