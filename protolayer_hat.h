#ifndef PROTOLAYER_HAT_HPP_INCLUDED
#define PROTOLAYER_HAT_HPP_INCLUDED

bool Net_HatInit();
bool Net_HatProcess();
void Net_HatShutdown();

#include "lib\socket.hpp"
#include "lib\utils.hpp"
#include "lib\packet.hpp"

namespace NetHat
{
    extern std::string HatAddr;
    extern uint16_t HatPort;
    extern std::string ControlAddr;
    extern uint16_t ControlPort;
    extern SOCKET Socket;
    extern bool Connected;
    extern uint32_t LastReconnect;
    extern PacketReceiver Receiver;
    extern uint32_t ServerCaps;
    extern bool HaveInfo;
    struct ServerInfo
    {
        unsigned long PlayerCount;
        unsigned long MapLevel;
        std::string MapName;
        unsigned long GameMode;
        unsigned long MapSize;

    };
    extern ServerInfo Info;
    extern bool ShuttingDown;
    extern uint32_t LastUpdate;
}

bool NetCmd_UpdateInfo();
bool NetCmd_Broadcast(std::string what);
bool NetCmd_Shutdown();
bool NetCmd_UnlockLogin(std::string login);

void Net_RegularProc();

#endif // PROTOLAYER_HAT_HPP_INCLUDED
