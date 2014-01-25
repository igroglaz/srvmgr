#pragma once
#include "lib/packet.hpp"
#include "srvmgrdef.h"
#include "syslib.h"

namespace pktmgr
{
	uint32_t SendPacket(byte* player, Packet& pack);
}