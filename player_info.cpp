#include "a2types.h"
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

    std::vector<A2Player*> plrs = zxmgr::GetPlayers();
    for (auto it = plrs.begin(); it != plrs.end(); ++it) {
        A2Player* player = (*it);
        if(!player) continue;
        uint16_t p_id = player->id_ext.id;
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

void PI_Create(A2Player* player)
{
    if(!player) return;
    uint16_t p_id = player->id_ext.id;
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

void PI_Delete(A2Player* player)
{
    if(!player) return;
    uint16_t p_id = player->id_ext.id;
    if(!p_id || p_id < 1 || p_id > 32) return;

    for(int i = 0; i < 32; i++)
    {
        if(!Players[i].Exists || !Players[i].Class) continue;
        if(Players[i].GodMode && Players[i].GodSetter == player)
        {
            Players[i].GodMode = false;
            Players[i].GodSetter = NULL;
        }

        A2Unit* unit = Players[i].Class->current_unit;
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

    A2Unit* unit = player->current_unit;
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

Player* PI_Get(A2Player* player)
{
    if(!player) return NULL;
    uint16_t p_id = player->id_ext.id;
    if(!p_id || p_id < 1 || p_id > 32) return NULL;
    if(!Players[p_id-1].Exists) return NULL;
    return &Players[p_id-1];
}
