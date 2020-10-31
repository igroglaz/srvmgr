#include "player_info.h"
#include "zxmgr.h"
#include "lib\utils.hpp"
#include "forbidden_items.h"
#include "srvmgr.h"
#include "screenshots.h"

Player Players[32];

void PI_Reset()
{
	for(int i = 0; i < 32; i++)
		PI_Clear(Players[i]);

	std::vector<byte*> plrs = zxmgr::GetPlayers();
	for(std::vector<byte*>::iterator it = plrs.begin();
			it != plrs.end(); ++it)
	{
		byte* player = (*it);
		if(!player) continue;
		uint16_t p_id = *(uint16_t*)(player + 4);
		if(!p_id || p_id < 1 || p_id > 32) continue;
		
		PI_Create(player);
	}
}

void PI_Clear(Player& struc)
{
	struc.Exists = false;
	struc.Class = NULL;
	struc.Casted = false;
	struc.ShouldReturn = false;
	struc.LastReturn = 0;
	struc.GodMode = false;
	struc.GodSetter = NULL;
	struc.SetSpells = 0;
	struc.LastSpells = 0;
	struc.SpellSetter = NULL;
	struc.UnmuteDate = 0;
	struc.EnqueuedPackets.clear();
}

void PI_Create(byte* player)
{
	if(!player) return;
	uint16_t p_id = *(uint16_t*)(player + 4);
	if(!p_id || p_id < 1 || p_id > 32) return;

	Player& struc = Players[p_id-1];
	PI_Clear(struc);
	struc.Exists = true;
	struc.Class = player;
	
	/*if(!*(uint32_t*)(player + 0x2C)) // not AI player
	{
		byte* unit = *(byte**)(player + 0x38);
		if(!unit) return;// odd

		struc.SavedItems = ItemRemover_Process(unit);
	}*/
}

void PI_Delete(byte* player)
{
	if(!player) return;
	uint16_t p_id = *(uint16_t*)(player + 4);
	if(!p_id || p_id < 1 || p_id > 32) return;

	for(int i = 0; i < 32; i++)
	{
		if(!Players[i].Exists || !Players[i].Class) continue;
		if(Players[i].GodMode && Players[i].GodSetter == player)
		{
			Players[i].GodMode = false;
			Players[i].GodSetter = NULL;
		}

		byte* unit = *(byte**)(Players[i].Class + 0x38);
		if(unit && Players[i].SetSpells != 0 && Players[i].SpellSetter == player &&
			(i != p_id-1))
		{
			zxmgr::SetSpells(unit, Players[i].LastSpells);
			Players[i].SetSpells = 0;
			Players[i].LastSpells = 0;
			Players[i].SpellSetter = NULL;
		}
	}

	/*for(std::vector<byte*>::iterator it = Players[p_id-1].SavedItems.begin();
		it != Players[p_id-1].SavedItems.end(); ++it)
	{
		byte* item = (*it);
		log_format("restored forbidden item. count: %u\n", *(uint16_t*)(item + 0x42));
		zxmgr::GiveItemTo(item, player);
	}*/

	byte* unit = *(byte**)(player + 0x38);
	bool update = false;

	/*if(Players[p_id-1].SavedItems.size())
	{
		Players[p_id-1].SavedItems.clear();
		update = true;
	}*/

	if(unit && Players[p_id-1].SetSpells != 0)
	{
		zxmgr::SetSpells(unit, Players[p_id-1].LastSpells);
		Players[p_id-1].SetSpells = 0;
		Players[p_id-1].LastSpells = 0;
		Players[p_id-1].SpellSetter = NULL;
		update = true;
	}

	//if(update && unit) zxmgr::SaveCharacter(player);

	PI_Clear(Players[p_id-1]);
	ClientScreenshot_DropPlayer(player);
}

Player* PI_Get(byte* player)
{
	if(!player) return NULL;
	uint16_t p_id = *(uint16_t*)(player + 4);
	if(!p_id || p_id < 1 || p_id > 32) return NULL;
	if(!Players[p_id-1].Exists) return NULL;
	return &Players[p_id-1];
}