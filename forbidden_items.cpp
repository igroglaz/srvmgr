#include "config_new.h"
#include "zxmgr.h"
#include "srvmgr.h"
#include "lib\utils.hpp"

bool ItemRemover_CheckForbidden(byte* item)
{
	return false;
	/*
	std::string item_name = *(const char**)(*(byte**)(item + 0x3C) + 4);
	uint16_t item_count = *(uint16_t*)(item + 0x42);
	if(item_name == "Potion Big Healing" ||
		item_name == "Potion Medium Healing") return true;
	return false;*/
}

std::vector<byte*> ItemRemover_Process(byte* unit)
{
	std::vector<byte*> saved_items;

	byte* pack = *(byte**)(unit + 0x7C);
	if(!pack) return saved_items;

	uint32_t index = 0;
	byte* lp = *(byte**)(pack + 4);
	byte* last_lp = NULL;
	while(lp)
	{
		byte* item = *(byte**)(lp + 8);
		uint16_t item_count = *(uint16_t*)(item + 0x42);
		if(item_count && ItemRemover_CheckForbidden(item))
		{
			byte* p_item = zxmgr::GetItemFromPack(pack, index, item_count);
			if(p_item)
			{
				saved_items.push_back(p_item);
				if(last_lp) lp = last_lp;
				else lp = *(byte**)(pack + 4);
			}
		}

		last_lp = lp;
		lp = *(byte**)(lp);
		index++;
	}

	return saved_items;
}