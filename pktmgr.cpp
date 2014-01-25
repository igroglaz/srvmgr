#include "srvmgrdef.h"
#include "pktmgr.h"
#include "lib/socket.hpp"

namespace pktmgr
{
	void __declspec(naked) __sendpacket_determine_size()
	{
		__asm
		{
			push	ebp
			mov		ebp, esp
			push	ecx
			mov		[ebp-0x04], ecx
			mov		eax, [ecx+0x40A]
			mov		esp, ebp
			pop		ebp
			retn
		}
	}

	uint32_t SendPacket(byte* player, Packet& pack)
	{
		byte* mainStruc = (byte*)(0x006D07A0);
		if(!*(byte**)(mainStruc + 0x57C)) return 0;
		uint32_t structs = *(uint32_t*)(mainStruc + 0x5A4);
		if(!structs) return 0;
		byte* p_netstruc = NULL;
		uint32_t sent_cnt = 0;
		for(uint32_t i = 0; i < structs; i++)
		{
			byte* pstruc = *(byte**)(mainStruc + 0x57C) + i * 0x274;
			p_netstruc = *(byte**)(pstruc + 0x10);
			if(!p_netstruc) continue;

			if(!player || (*(uint32_t*)(p_netstruc + 0x108) == *(uint16_t*)(player + 4)))
			{
				SOCKET s = *(SOCKET*)(pstruc + 8);
				if(SOCK_SendPacket(s, pack, 20) == 0) sent_cnt++;
			}
		}

		return sent_cnt;
	}
}