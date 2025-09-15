#include "a2types.h"
#include "lib\utils.hpp"
#include "config_new.h"
#include "syslib.h"
#include "srvmgrdef.h"
#include "protolayer_hat.h"
#include "crash_filter.h"
#include "srvmgr.h"
#include "zxmgr.h"
#include "player_info.h"
#include "scanrange.h"
#include "unit_info.h"
#include "forbidden_items.h"
#include "multiplayer_shop.h"
#include "solo.h"
#include "quests.h"

void ChangeWndTitle(const char* title)
{
    byte* mainWnd = zxmgr::GetMainWnd();
    HWND hWnd = *(HWND*)(mainWnd + 0x1C);
    SetWindowTextA(hWnd, title);
}

void OnInitializeServer()
{
    SetExceptionFilter();

    ChangeWndTitle(Format("Server ID %u (map not loaded)", Config::ServerID).c_str());
    Printf("Server started.");

    Config::MapLoaded = false;
    Config::ServerStarted = true;

    //if ((Config::ServerCaps & SVC_SAVE_DATABASE) && !SQL_Init()) Quit();
}

void OnInitializeMap()
{
    byte* map_data = *(byte**)(0x00642C2C);
    Config::CurrentMapName  = *(const char**)(map_data+0x90);
    Config::CurrentMapTitle = *(const char**)(map_data+0x1C8);
    Config::MapLoaded = true;

    std::string map_title = "";
    
    if (!Config::CurrentMapTitle.length())
        Config::CurrentMapTitle = Basename(Config::CurrentMapName);
    else if (Config::ServerFlags & SVF_PVM)
        map_title = "PvM: ";

    map_title += Config::CurrentMapTitle;

    ChangeWndTitle(Format("Server ID %u (%s)", Config::ServerID, map_title.c_str()).c_str());
    Printf("Loaded map \"%s\".", Config::CurrentMapName.c_str());

    // update PlayerInfo structures for new map
    PI_Reset();

    ClearStapleCells();

    // Clear map votes.
    for (auto& ps: player_settings) {
        ps.second->map_vote.clear();
    }
}

void OnPreInitializeMap()
{
    // update UnitInfo
    ClearShops();
    UI_Reset();
    Config::MapLoaded = false;
    ChangeWndTitle(Format("Server ID %u (loading map...)", Config::ServerID).c_str());
}

void OnInitializeMapError(const char* mapfile, const char* error)
{
    Config::MapLoaded = false;
    ChangeWndTitle(Format("Server ID %u (map not loaded)", Config::ServerID).c_str());
    Printf("Error: map \"%s\" not loaded (%s)", mapfile, error);
}

void OnServerClosed()
{
    Config::ExitingCleanly = true;
    Config::MapLoaded = false;
    ChangeWndTitle(Format("Server ID %u (map not loaded)", Config::ServerID).c_str());
    if (!NetCmd_Shutdown())
        NetHat::Connected = false;
    Printf("Server closed.");
    log_format("\n");
}

void OnLocalMessageBox(const char* message)
{
    Printf("Error: %s", message);
}

void OnServerTic()
{
    Net_RegularProc();

    // check for unit in astral & forbidden items in pack
    for (int i = 0; i < 32; i++)
    {
        if (!Players[i].Exists) continue;
        if (!Players[i].Class) continue;
        if (Players[i].Class->unitType) continue;
        if (!Players[i].Class->current_unit) continue;

        A2Unit* unit = Players[i].Class->current_unit;

        // milliseconds from system start - Players[i].LastReturn...
        if (Players[i].ShouldReturn && GetTickCount()-Players[i].LastReturn > 5000) // every 5 seconds
        {
            if (zxmgr::ReturnUnit(unit))
                Players[i].ShouldReturn = false;
        }

        // check items that should be removed
        /*std::vector<byte*> forbidden_items = ItemRemover_Process(unit);
        for (std::vector<byte*>::iterator it = forbidden_items.begin();
            it != forbidden_items.end(); ++it)
        {
            byte* item_vec = (*it);
            byte* item_vec_info = *(byte**)(item_vec + 0x3C);

            for (std::vector<byte*>::iterator jt = Players[i].SavedItems.begin();
                jt != Players[i].SavedItems.end(); ++jt)
            {
                byte* item_saved = (*jt);
                byte* item_saved_info = *(byte**)(item_saved + 0x3C);

                if (item_saved_info == item_vec_info)
                {
                    *(uint16_t*)(item_saved + 0x42) += *(uint16_t*)(item_vec + 0x42);
                    zxmgr::DestroyItem(item_vec);
                    item_vec = NULL;
                    item_vec_info = NULL;
                    break;
                }
            }

            // existing item not found
            if (item_vec) Players[i].SavedItems.push_back(item_vec);
        }

        if (forbidden_items.size()) zxmgr::UpdateUnit(unit, Players[i].Class, 0x00282000, 0, 0, 0);*/
    }

    //UI_Tick();
    SR_Step();

    Sleep(1);
}

void OnShopError()
{
    Printf("Shop error: amount > 1000!");
}

void LogIP(A2Player* player)
{
    const char* player_name = "(null)";
    if (player) player_name = player->name;
    byte* vd = zxmgr::GetNetworkStruct(player);
    const char* player_addr = "n/a";
    if (vd) player_addr = (const char*)(vd + 8);
    Printf("Player %s has joined the game (from: %s)", player_name, player_addr);
}

A2Effect* CreateItemParameter(A2Effect* param, A2InventoryItem* item)
{
    if (!item) return NULL;
    if (!param) return NULL;

    uint8_t prm1 = param->effect_id;
    uint8_t val1 = param->value1;
    uint8_t val2 = param->value2;
    uint32_t item_class = item->shape;
    uint32_t item_material = item->material;
    uint32_t item_option = item->option;
    uint32_t item_slot = reinterpret_cast<A2Armor*>(item)->slot;
    
    /*if ((item_slot == 4 ||
        item_slot == 5) &&
        (prm1 == 2))
    {
        if (val1 > 2)
            val1 = 2;
    }*/

    param->value1 = val1;
    param->value2 = val2;

    return param;
}

bool CheckItemUpgradable(A2InventoryItem* item)
{
    if (!item) return false;
    if (item->price == 2) return false; // quest item
    return true;
}

bool Sv_ProcessClientPacket(int16_t id, A2Player* player, Packet& pack)
{
    if (id == -1) return true; // hat
    return true;
}

void _stdcall ExtDiplomacy(A2Player* player, uint32_t setd)
{
    std::vector<A2Player*> players = zxmgr::GetPlayers();

    for (size_t i = 0; i < players.size(); i++)
    {
        A2Player* player2 = players[i];
        if (player2 == NULL)
            continue;

        if (player == player2)
        {
            zxmgr::SetDiplomacy(player, player2, 0x12);
        }
        else
        {
            // gm ally monsters, gm ally players, gm vision players
            bool has_rights = false;
            uint32_t rights = player->flags;
            if ((rights & GMF_ANY) != GMF_ANY)
                rights = 0;
            else has_rights = true;
            bool has_rights2 = false;
            uint32_t rights2 = player2->flags;
            if ((rights2 & GMF_ANY) != GMF_ANY)
                rights2 = 0;
            else has_rights2 = true;
            if (player2->unitType)
                rights2 = 0;

            rights &= 0xFFFFFF;
            rights2 &= 0xFFFFFF;

            // ai
            if ((rights & GMF_AI_ALLY) && player2->unitType)
            {
                zxmgr::SetDiplomacy(player, player2, 0x02); // from this to others
                zxmgr::SetDiplomacy(player2, player, 0x02); // from others to this
            }

            // not ai
            if (((rights & GMF_PLAYERS_ALLY)||(rights2 & GMF_PLAYERS_ALLY)) && !player2->unitType)
            {
                zxmgr::SetDiplomacy(player, player2, 0x02); // from this to others
                zxmgr::SetDiplomacy(player2, player, 0x02); // from others to this
            }

            if ((rights & GMF_PLAYERS_VISION) && !player2->unitType) // not ai, vision
            {
                // from others to this
                zxmgr::SetDiplomacy(player2, player, 0x10|zxmgr::GetDiplomacy(player2, player)); 
            }

            if (rights2 & GMF_PLAYERS_VISION) // other player has vision flag
            {
                // from this to others
                zxmgr::SetDiplomacy(player, player2, 0x10|zxmgr::GetDiplomacy(player, player2));
            }
        }
    }
}
