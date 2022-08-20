#include "scanrange.h"
#include "zxmgr.h"
#include "player_info.h"
#include "srvmgr.h"
#include "lib/ScanrangeCalc.hpp"

CScanrangeCalc Scanrange;
bool Scanrange_Initialized = false;
byte* Scanrange_Player = NULL;

bool srvmgr_CheckValid(int16_t x, int16_t y)
{
    uint32_t mapwidth = *(uint32_t*)(*(uint32_t*)(0x006B16A8) + 0x50000);
    uint32_t mapheight = *(uint32_t*)(*(uint32_t*)(0x006B16A8) + 0x50004);
    return (x >= 7 && y >= 7 &&
        x <= mapwidth-7 &&
        y <= mapheight-7);
}

uint8_t srvmgr_GetHeight(int16_t x, int16_t y)
{
    if (!srvmgr_CheckValid(x, y)) return 0;
    uint8_t* mapheights = (uint8_t*)(*(byte**)(0x006B16A8) + 0x944F4);
    return *(mapheights+y*256+x);
}

void SR_UpdateUnit(byte* unit)
{
    if (!Scanrange_Initialized)
    {
        Scanrange.InitializeTables();
        Scanrange_Initialized = true;
    }

    if (!unit) return;
    byte* unit_player = *(byte**)(unit+0x14);
    if (!unit_player) return;
    Scanrange_Player = unit_player;
    // number of milliseconds from the moment when system was started
    uint32_t ticks_count = GetTickCount();
    bool dCalc = false;
    for (int i = 0; i < 32; i++)
    {
        if (!Players[i].Exists) continue;
        if (!Players[i].Class) continue;
        if (*(uint32_t*)(Players[i].Class + 0x2C)) continue;
        if (!*(byte**)(Players[i].Class + 0x38)) continue;

        if (Players[i].Class == unit_player || (zxmgr::GetDiplomacy(unit_player, Players[i].Class) & 0x10))
        {
            uint8_t unit_x = *(uint8_t*)(*(byte**)(unit + 0x10));
            uint8_t unit_y = *(uint8_t*)(*(byte**)(unit + 0x10)+1);
            uint16_t unit_vision = *(uint16_t*)(unit+0xA4);

            if (!dCalc)
            {
                Scanrange.CalculateVision(unit_x, unit_y, unit_vision, &srvmgr_GetHeight, &srvmgr_CheckValid);
                dCalc = true;
            }

            int16_t gen_x = unit_x - 20;
            int16_t gen_y = unit_y - 20;

            for (uint8_t x = 0; x < 41; x++)
            {
                for (uint8_t y = 0; y < 41; y++)
                {
                    if (srvmgr_CheckValid(gen_x+x, gen_y+y) && (Scanrange.pTablesVision[x][y] > 0))
                        Players[i].Vision[gen_x+x][gen_y+y] = ticks_count;
                }
            }
        }
    }
}

void SR_Step()
{
    std::vector<byte*> units = zxmgr::GetUnits();

    for (int i = 0; i < 32; i++)
    {
        if (!Players[i].Exists) continue;
        if (!Players[i].Class) continue;
        memset(Players[i].Vision, 0, sizeof(Players[i].Vision));
    }

    for (std::vector<byte*>::iterator it = units.begin();
        it != units.end(); ++it)
    {
        byte* unit = (*it);
        if (unit && unit != *(byte**)(*(byte**)(unit+0x14)+0x38))
            SR_UpdateUnit(unit);
    }

    for (int i = 0; i < 32; i++)
    {
        if (!Players[i].Exists) continue;
        if (!Players[i].Class) continue;

        if (*(byte**)(Players[i].Class+0x38))
            SR_UpdateUnit(*(byte**)(Players[i].Class+0x38));
    }
}

bool SR_CheckVision(byte* player, uint8_t check_x, uint8_t check_y)
{
    Player* pi = PI_Get(player);
    if (!pi) return false;
    if (!srvmgr_CheckValid(check_x, check_y)) return false;
    uint32_t ticks_count = GetTickCount();
    for (int x = -1; x <= 1; x++)
        for (int y = -1; y <= 1; y++)
            if (ticks_count - pi->Vision[check_x+x][check_y+y] <= 3000) return true;
    return false;
}


// can not be used from console
void SR_DumpToFile(byte* player)
{
    Player* pi = PI_Get(player); // get player ID
    if (!pi) return;    
    uint32_t ticks_count = GetTickCount(); // ms from the moment when system was started

    // 'player+0x18' - player name
    log_format("SR_DumpToFile: dump initiated (for player %s).\n", *(const char**)(player+0x18));

    for (uint32_t y = 0; y < 256; y++)
    {
        for (uint32_t x = 0; x < 256; x++)
        {
            if (ticks_count - (pi->Vision[x][y]) <= 3000) // uint32_t Vision[256][256];
                log_format2("\x7F"); // ASCII 127 (delete)
            else log_format2(" ");
        }
        log_format2("\n");
    }
    log_format("SR_DumpToFile: dump finished.\n");
}
