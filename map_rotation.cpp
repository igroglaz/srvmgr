#include "config_new.h"
#include <stdlib.h>
#include "lib\utils.hpp"

void stop_server()
{
	exit(0);
}

void __stdcall reset_map_counter()
{
	if(Config::server_rotate_maps){
		Printf("Rolling maps");
		int *server_map_index = (int *)(void *)0x006D1634;
		*server_map_index = 0;
	}else{
		Printf("Stopping server after the last map");
		stop_server();
	}
}

int __declspec(naked) imp_reset_map_counter()
{ 
	__asm{
		mov		edx, reset_map_counter
		call	edx
		ret
	}
}