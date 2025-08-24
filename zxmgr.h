#pragma once

#include <string>
#include <vector>

#include "a2types.h"
#include "syslib.h"

#define PLAYER_FLAG_AI 1
#define PLAYER_FLAG_HUNT 2

#include <map>
#define CHECK_FLAG(a, b) ((a & b) == b)

namespace zxmgr
{
    std::vector<A2Player*> _stdcall GetPlayers();
    std::vector<A2Unit*> _stdcall GetUnits(A2Player* player = NULL);

    void SendMessageRaw(const A2Player* pptr, const char* message);
    void SendMessage(const A2Player* pptr, const char* mask, ...);
    void Kick(A2Player* pptr, bool silent);
    A2Player* _stdcall FindByNickname(const char* nickname);
    A2Player* _stdcall FindByLogin(const char* login);
    A2Player* _stdcall FindByID(uint16_t id);
    void _stdcall KickAll(A2Player* caster);
    void _stdcall KickAllSilent(A2Player* caster);
    void _stdcall Kill(A2Player* player, A2Player* caster);
    void _stdcall KillAll(A2Player* caster, bool ai_only);
    bool IsConnected(A2Player* player);
    A2Unit* Summon(A2Player* player, const char* unitname, byte* pthis, bool ishero, byte* targetptr = 0);

    // `ConstructItem` returns an object of a corresponding type. It can be a weapon, an armor, and so on.
    A2InventoryItem* ConstructItemN(const char* definition);
    A2InventoryItem* ConstructItem(std::string definition);

    bool CheckItem(A2InventoryItem* item);
    void GiveItemTo(A2InventoryItem* item, A2Player* player);
    void DestroyItem(A2InventoryItem* item);
    void GiveMoney(A2Player* pptr, unsigned long count, unsigned long flags);
    void UpdatePlayer(unsigned long flags, unsigned long info, unsigned long unknown, A2Player* pptr);
    void __stdcall Own(A2Player* to, A2Player* from);
    void PickupFor(unsigned long pptr, unsigned long pthis);

    unsigned long GetSpeed();
    void SetSpeed(unsigned long newspeed);

    void ShutdownServer();
    unsigned long GetCurrentMapTime();
    unsigned long GetTotalMapTime();
    void SetTotalMapTime(unsigned long newtime);
    void ResetMap();
    void NextMap();
    void PrevMap();

    void MorphUnit(unsigned long unit, unsigned long kind);
    void UpdateUnit(A2Unit* unit, A2Player* player, unsigned long flags, unsigned long flags2, unsigned long flags3, unsigned long flags4);

    int GetUnitSize(A2Unit* unit);
    void MakeUnitNoClip(A2Unit* unit);

    void CreateSack(const char* itemname, unsigned long x, unsigned long y, unsigned long money);

    void _stdcall GMLog(const char* format, ...);
    void _stdcall ServerLog(const char* format, ...);

    void Disconnect(A2Player* player);

    unsigned long _stdcall GetTicks();

    //unsigned long _stdcall CreateUnit(const char* name);
    //void FreeUnit(unsigned long cptr);

    byte* GetNetworkStruct(A2Player* player);

    byte* GetMainWnd();
    bool ReturnUnit(A2Unit* unit);

    uint32_t GetSpells(A2Unit* unit);
    void SetSpells(A2Unit* unit, uint32_t spells);

    void CastPointEffect(byte* from, uint8_t to_x, uint8_t to_y, uint8_t spell);

    uint8_t GetDiplomacy(byte* player1, byte* player2);
    void SetDiplomacy(byte* player1, byte* player2, uint8_t newdip);

    A2InventoryItem* GetItemFromPack(A2InventoryList* pack, uint16_t index, uint16_t count);
    void SaveCharacter(byte* player);

    byte* GetUnitByID(uint16_t player_id, uint16_t unit_id);

    SOCKET GetSocket(uint16_t player_id);
    SOCKET GetSocket(byte* player);
}
