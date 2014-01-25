#ifndef SOCKET_HPP_INCLUDED
#define SOCKET_HPP_INCLUDED

#include <string>
#include "packet.hpp"
#include "serialize.hpp"

#if !defined ( _BSDTYPES_DEFINED )
/* also defined in gmon.h and in cygwin's sys/types */
typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;
typedef unsigned long	u_long;
#define _BSDTYPES_DEFINED
#endif /* ! def _BSDTYPES_DEFINED  */
typedef u_int	SOCKET;

#define SERR_NOTCREATED 0xFFFFFFFF
#define SERR_CONNECTION_LOST   100
#define SERR_TIMEOUT           101
#define SERR_INCOMPLETE_DATA   102

struct sockaddr_in;

SOCKET SOCK_Connect(std::string address, unsigned short port, std::string localaddress, unsigned short localport);
SOCKET SOCK_Listen(std::string address, unsigned short port);
SOCKET SOCK_Accept(SOCKET listener, sockaddr_in& addr);
int SOCK_SendPacket(SOCKET socket, Packet& packet, unsigned long protover, bool extended = false);
int SOCK_ReceivePacket(SOCKET socket, Packet& packet, unsigned long protover);
void SOCK_Destroy(SOCKET socket);
void SOCK_SetBlocking(SOCKET socket, bool blocking);
bool SOCK_WaitEvent(SOCKET socket, unsigned long timeout);

namespace IPFilter
{
    struct IPAddress
    {
        std::string Address;
        unsigned long Addr;
        unsigned long Masked;
        unsigned long Mask;

        void FromString(std::string str);
    };

    struct IPFEntry
    {
        bool Action;
        IPAddress Address;
    };

    struct IPFFile
    {
        std::vector<IPFEntry> Entries;

        // +1 = explicitly allowed
        // -1 = explicitly blocked
        // 0 = not set in this IPF
        int CheckAddress(std::string addr);
        void ReadIPF(std::string string, bool filename = false);
    };
}

class PacketReceiver
{
    public:
        PacketReceiver();
        ~PacketReceiver();

        void Connect(SOCKET sock);
        bool Receive(uint32_t version);

        bool GetPacket(Packet& pack);

    private:
        std::vector<Packet> Queue;

        SOCKET Socket;
};

#endif // SOCKET_HPP_INCLUDED
