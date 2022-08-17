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

    //if((Config::ServerCaps & SVC_SAVE_DATABASE) && !SQL_Init()) Quit();
}

void OnInitializeMap()
{
    byte* map_data = *(byte**)(0x00642C2C);
    Config::CurrentMapName  = *(const char**)(map_data+0x90);
    Config::CurrentMapTitle = *(const char**)(map_data+0x1C8);
    Config::MapLoaded = true;

    std::string map_title = "";
    if(!Config::CurrentMapTitle.length()) Config::CurrentMapTitle = Basename(Config::CurrentMapName);
    else if(Config::ServerFlags & SVF_PVM) map_title = "PvM: ";
    map_title += Config::CurrentMapTitle;

    ChangeWndTitle(Format("Server ID %u (%s)", Config::ServerID, map_title.c_str()).c_str());
    Printf("Loaded map \"%s\".", Config::CurrentMapName.c_str());

	// update PlayerInfo structures for new map
	PI_Reset();
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
    if(!NetCmd_Shutdown()) NetHat::Connected = false;
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
	for(int i = 0; i < 32; i++)
	{
		if(!Players[i].Exists) continue;
		if(!Players[i].Class) continue;
		if(*(uint32_t*)(Players[i].Class + 0x2C)) continue;
		if(!*(byte**)(Players[i].Class + 0x38)) continue;

		byte* unit = *(byte**)(Players[i].Class + 0x38);

		if(Players[i].ShouldReturn && GetTickCount()-Players[i].LastReturn > 5000) // every 5 seconds
		{
			if(zxmgr::ReturnUnit(*(byte**)(Players[i].Class + 0x38)))
				Players[i].ShouldReturn = false;
		}

		// check items that should be removed
		/*std::vector<byte*> forbidden_items = ItemRemover_Process(unit);
		for(std::vector<byte*>::iterator it = forbidden_items.begin();
			it != forbidden_items.end(); ++it)
		{
			byte* item_vec = (*it);
			byte* item_vec_info = *(byte**)(item_vec + 0x3C);

			for(std::vector<byte*>::iterator jt = Players[i].SavedItems.begin();
				jt != Players[i].SavedItems.end(); ++jt)
			{
				byte* item_saved = (*jt);
				byte* item_saved_info = *(byte**)(item_saved + 0x3C);

				if(item_saved_info == item_vec_info)
				{
					*(uint16_t*)(item_saved + 0x42) += *(uint16_t*)(item_vec + 0x42);
					zxmgr::DestroyItem(item_vec);
					item_vec = NULL;
					item_vec_info = NULL;
					break;
				}
			}

			// existing item not found
			if(item_vec) Players[i].SavedItems.push_back(item_vec);
		}

		if(forbidden_items.size()) zxmgr::UpdateUnit(unit, Players[i].Class, 0x00282000, 0, 0, 0);*/
	}

	//UI_Tick();
	SR_Step();

	Sleep(1);
}

void OnShopError()
{
    Printf("Shop error: amount > 1000!");
}

void LogIP(byte* player)
{
	const char* player_name = "(null)";
	if(player) player_name = *(const char**)(player + 0x18);
    byte* vd = zxmgr::GetNetworkStruct(player);
    const char* player_addr = "n/a";
	if(vd) player_addr = (const char*)(vd + 8);
	Printf("Player %s has joined the game (from: %s)", player_name, player_addr);
}

byte* CreateItemParameter(byte* param, byte* item)
{
	if(!item) return NULL;
	if(!param) return NULL;

	uint32_t prm1 = *(uint8_t*)(param + 0x3C);
	uint32_t val1 = *(uint16_t*)(param + 0x40);
	uint32_t val2 = *(uint16_t*)(param + 0x42);
	uint32_t item_class = *(uint8_t*)(item + 0x45);
	uint32_t item_material = *(uint8_t*)(item + 0x46);
	uint32_t item_option = *(uint16_t*)(item + 0x0C);
	uint32_t item_slot = *(uint8_t*)(item + 0x58);
	
	/*if((item_slot == 4 ||
		item_slot == 5) &&
		(prm1 == 2))
	{
		if(val1 > 2)
			val1 = 2;
	}*/

	*(uint16_t*)(param + 0x40) = val1;
	*(uint16_t*)(param + 0x42) = val2;

	return param;
}

bool CheckItemUpgradable(byte* item)
{
	if(!item) return false;
	if(*(uint32_t*)(item + 0x1C) == 2) return false; // quest item
	bool upgradable = true;
	uint32_t item_class = *(uint8_t*)(item + 0x45);
	uint32_t item_material = *(uint8_t*)(item + 0x46);
	uint32_t item_option = *(uint16_t*)(item + 0x0C);
	uint32_t item_slot = *(uint8_t*)(item + 0x58);
	byte* parms = *(byte**)(item + 0x28);
	while(parms)
	{
		byte* parm = *(byte**)(parms + 8);
		if(parm)
		{
			uint32_t prm1 = *(uint8_t*)(parm + 0x3C);
			uint32_t val1 = *(uint16_t*)(parm + 0x40);
			uint32_t val2 = *(uint16_t*)(parm + 0x42);

			/*if(prm1 == 2) // body
			{
				if(item_slot == 4 ||
					item_slot == 5)
				{
					if(val1 > 2) val1 = 2;
					if(val1 == 2) upgradable = false;
				}

			}*/

			*(uint8_t*)(parm + 0x3C) = prm1;
			*(uint16_t*)(parm + 0x40) = val1;
			*(uint16_t*)(parm + 0x42) = val2;
		}

		parms = *(byte**)(parms + 4);
	}

	return upgradable;
}

bool Sv_ProcessClientPacket(int16_t id, byte* player, Packet& pack)
{
	if(id == -1) return true; // hat
	return true;
}

void _stdcall ExtDiplomacy(byte* player, uint32_t setd)
{
	std::vector<byte*> players = zxmgr::GetPlayers();

	for(size_t i = 0; i < players.size(); i++)
	{
		byte* player2 = players[i];
		if(player2 == NULL)
			continue;

		if(player == player2)
		{
			zxmgr::SetDiplomacy(player, player2, 0x12);
		}
		else
		{
			// gm ally monsters, gm ally players, gm vision players
			bool has_rights = false;
			uint32_t rights = *(uint32_t*)(player + 0x14);
			if((rights & GMF_ANY) != GMF_ANY)
				rights = 0;
			else has_rights = true;
			bool has_rights2 = false;
			uint32_t rights2 = *(uint32_t*)(player2 + 0x14);
			if((rights2 & GMF_ANY) != GMF_ANY)
				rights2 = 0;
			else has_rights2 = true;
			if(*(uint32_t*)(player2 + 0x2C))
				rights2 = 0;

			rights &= 0xFFFFFF;
			rights2 &= 0xFFFFFF;

			if((rights & GMF_AI_ALLY) && *(uint32_t*)(player2 + 0x2C)) // ai
			{
				zxmgr::SetDiplomacy(player, player2, 0x02); // from this to others
				zxmgr::SetDiplomacy(player2, player, 0x02); // from others to this
			}

			if(((rights & GMF_PLAYERS_ALLY)||(rights2 & GMF_PLAYERS_ALLY)) && !*(uint32_t*)(player2 + 0x2C))// not ai
			{
				zxmgr::SetDiplomacy(player, player2, 0x02); // from this to others
				zxmgr::SetDiplomacy(player2, player, 0x02); // from others to this
			}

			if((rights & GMF_PLAYERS_VISION) && !*(uint32_t*)(player2 + 0x2C)) // not ai, vision
			{
				zxmgr::SetDiplomacy(player2, player, 0x10|zxmgr::GetDiplomacy(player2, player)); // from others to this
			}

			if(rights2 & GMF_PLAYERS_VISION) // other player has vision flag
			{
				zxmgr::SetDiplomacy(player, player2, 0x10|zxmgr::GetDiplomacy(player, player2)); // from this to others
			}
		}
	}
}