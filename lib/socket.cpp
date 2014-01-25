#include "socket.hpp"
#include <winsock2.h>

#include <fstream>
#include "utils.hpp"
#include "hat2.hpp"

/*bool CheckIP(std::string addr)
{
	std::vector<std::string> addr2 = Explode(addr, ".");
	return ((addr2.size() == 4) && CheckInt(addr2[0]) && CheckInt(addr2[1]) && CheckInt(addr2[2]) && CheckInt(addr2[3]));
}*/

SOCKET SOCK_Connect(std::string addr, unsigned short port, std::string localaddr, unsigned short localport)
{
    sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(localaddr.c_str());
    local.sin_port = htons(localport);
    SOCKET out = socket(AF_INET, SOCK_STREAM, 0);
    if(out == INVALID_SOCKET) return SERR_NOTCREATED;
    if(bind(out, (sockaddr*)&local, sizeof(local)) != 0)
    {
        closesocket(out);
        return SERR_NOTCREATED;
    }

    sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    if(CheckIP(addr)) remote.sin_addr.s_addr = inet_addr(addr.c_str());
	else
	{
		struct hostent* host = gethostbyname(addr.c_str());
		if(!host)
		{
			closesocket(out);
			return SERR_NOTCREATED;
		}

		remote.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	}
    remote.sin_port = htons(port);
    if(connect(out, (sockaddr*)&remote, sizeof(remote)) != 0)
    {
        closesocket(out);
        return SERR_NOTCREATED;
    }

    return out;
}

SOCKET SOCK_Listen(std::string addr, unsigned short port)
{
    sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(addr.c_str());
    local.sin_port = htons(port);
    SOCKET out = socket(AF_INET, SOCK_STREAM, 0);
    if(out == INVALID_SOCKET) return SERR_NOTCREATED;
    if(bind(out, (sockaddr*)&local, sizeof(local)) != 0)
    {
        closesocket(out);
        return SERR_NOTCREATED;
    }
    if(listen(out, 128) != 0)
    {
        closesocket(out);
        return SERR_NOTCREATED;
    }

    return out;
}

SOCKET SOCK_Accept(SOCKET listener, sockaddr_in& addr)
{
    int fromlen = sizeof(addr);
    SOCKET client = accept(listener, (sockaddr*)&addr, &fromlen);
    if(client == INVALID_SOCKET) return SERR_NOTCREATED;
    return client;
}

int SOCK_SendPacket(SOCKET socket, Packet& packet, unsigned long protover, bool extended)
{
	if(!extended)
	{
		if(!packet.GetLength()) return 0;
		uint8_t* data = new uint8_t[packet.GetLength()];
		uint32_t size = packet.GetLength();
		std::vector<uint8_t> tbuf = packet.GetBuffer();
		for(uint32_t i = 0; i < size; i++)
			data[i] = tbuf[i];

		int ret_code = send_msg(socket, protover, data, size, 1);
		delete[] data;
		if(ret_code == -2) return SERR_TIMEOUT;
		if(ret_code == -1) return SERR_CONNECTION_LOST;
		return 0;
	}
	else
	{
		uint8_t* data = new uint8_t[packet.GetLength()+8];
		*(uint32_t*)(data) = packet.GetLength();
		*(uint32_t*)(data+4) = 0x80100000;
		std::vector<uint8_t> tbuf = packet.GetBuffer();
		for(uint32_t i = 0; i < packet.GetLength(); i++)
			data[i+8] = tbuf[i];

		int ret_code = send(socket, (const char*)data, packet.GetLength()+8, 0);
		delete[] data;
		if(ret_code == -1) return SERR_CONNECTION_LOST;
		return 0;
	}
}

int SOCK_ReceivePacket(SOCKET socket, Packet& packet, unsigned long protover)
{
    packet.Reset();
    unsigned long origin = 0xFFFFFFFF;

    while(!(origin & 0xFFFF0000) || origin == 0xFFFFFFFF)
    {
        unsigned long _siz;
        unsigned long _org;
        if(!SOCK_WaitEvent(socket, 0) && origin == 0xFFFFFFFF) return SERR_TIMEOUT;
        else if(origin != 0xFFFFFFFF) return SERR_INCOMPLETE_DATA;
        int headersize = recv(socket, (char*)&_siz, 4, 0); // receive packet length
        if(headersize != 4) return SERR_CONNECTION_LOST;
        if(!SOCK_WaitEvent(socket, 0)) return SERR_CONNECTION_LOST;
        headersize = recv(socket, (char*)&_org, 4, 0); // receive packet origin
        if(headersize != 4) return SERR_CONNECTION_LOST;
        if(_siz > 0xFF || !_siz) return SERR_CONNECTION_LOST;
        unsigned char* data = new unsigned char[_siz];
        memset(data, 0, _siz);
        if(!SOCK_WaitEvent(socket, 0)) return SERR_CONNECTION_LOST;
        int packetsize = recv(socket, (char*)data, _siz, 0);
        if(packetsize != (int)_siz)
        {
            packet.Reset();
            delete[] data;
            return SERR_CONNECTION_LOST;
        }
        origin = _org;
        PACKET_XorByKey(data, _siz, protover);
        //packet.AppendData((uint8_t*)data, _siz);
		for(uint32_t i = 0; i < _siz; i++)
			packet.WriteUInt8(data[i]);
        delete[] data;
    }
    packet.Seek(0);
    return 0;
}

void SOCK_Destroy(SOCKET socket)
{
    closesocket(socket);
}

namespace IPFilter
{
    void IPAddress::FromString(std::string str)
    {
        unsigned long mask;
        std::vector<std::string> addr = Explode(str, "/");
        if(addr.size() == 2) mask = StrToInt(addr[1]);
        else if(addr.size() == 1) mask = 32;
        else
        {
            Address = "";
            Addr = 0;
            Masked = 0;
            return;
        }
        std::vector<std::string> at = Explode(addr[0], ".");
        if(at.size() != 4)
        {
            Address = "";
            Addr = 0;
            Masked = 0;
            return;
        }
        Addr = 0;
        Masked = 0;
        uint8_t oc_1 = StrToInt(at[0]);
        uint8_t oc_2 = StrToInt(at[1]);
        uint8_t oc_3 = StrToInt(at[2]);
        uint8_t oc_4 = StrToInt(at[3]);
        Addr |= oc_1;
        Addr <<= 8;
        Addr |= oc_2;
        Addr <<= 8;
        Addr |= oc_3;
        Addr <<= 8;
        Addr |= oc_4;
        Address = addr[0];
        unsigned long lmask = 0;
        for(unsigned long i = 0; i < mask; i++)
            lmask |= (1 << (31-i));
        Masked = Addr & lmask;
        Mask = lmask;
    }

    void IPFFile::ReadIPF(std::string string, bool filename)
    {
        std::vector<std::string> vs;
        if(filename)
        {
            std::ifstream f_temp;
            f_temp.open(string.c_str(), std::ios::in);
            if(!f_temp.is_open()) return;
            std::string str;
            while(f_temp >> str) vs.push_back(Trim(str));
            f_temp.close();
        }
        else vs = Explode(string, "\n");
        std::vector<IPFEntry> ret;

        for(size_t i = 0; i < vs.size(); i++)
        {
            std::string str = Trim(vs[i]);
            if(str.length() < 8) continue;
            char action = str[0];
            str.erase(0, 1);
            IPFEntry ent;
            if(action == '+') ent.Action = true;
            else if(action == '-') ent.Action = false;
            else continue;
            ent.Address.FromString(str);
            ret.push_back(ent);
        }
        Entries = ret;
    }

    int IPFFile::CheckAddress(std::string addr)
    {
        IPAddress caddr;
        caddr.FromString(addr);
        for(std::vector<IPFEntry>::iterator it = Entries.begin(); it != Entries.end(); ++it)
        {
            IPFEntry& ent = (*it);
            if(ent.Address.Masked == (caddr.Addr & ent.Address.Mask))
            {
                if(ent.Action) return 1;
                else return -1;
            }
        }
        return 0;
    }
}

void SOCK_SetBlocking(SOCKET socket, bool blocking)
{
    u_long iMode = 1;
    ioctlsocket(socket, FIONBIO, &iMode);
}

bool SOCK_WaitEvent(SOCKET socket, unsigned long timeout)
{
    fd_set fd;
    timeval tv;
    FD_ZERO(&fd);
    FD_SET(socket, &fd);
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    return select(0, &fd, NULL, NULL, &tv);
}

PacketReceiver::PacketReceiver()
{
    Socket = 0;
}

PacketReceiver::~PacketReceiver()
{
    Queue.clear();
}

void PacketReceiver::Connect(SOCKET socket)
{
    Socket = socket;
}

bool PacketReceiver::Receive(uint32_t version)
{
    if(!Socket) return false;

    Packet pack;
    pack.Reset();

    if(!Queue.size()) Queue.push_back(pack);

    int zz = SOCK_ReceivePacket(Socket, pack, version);

    if(zz == SERR_TIMEOUT) return true;
    else if(zz == SERR_CONNECTION_LOST) return false;

	pack.SaveToStream(Queue.back());

    if(zz == 0)
    {
        // create "next" packet
        Packet pack2;
        pack2.Reset();
        Queue.push_back(pack2);
    }

    return true;
}

bool PacketReceiver::GetPacket(Packet& pack)
{
    if(!Socket) return false;
    if(Queue.size() <= 1) return false;

    Packet pck2 = Queue.front();

    pack = pck2;
    Queue.erase(Queue.begin());

    return true;
}
