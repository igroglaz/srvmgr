#pragma once

#include <vector>
#include "syslib.h"

#define PLAYER_FLAG_AI 1
#define PLAYER_FLAG_HUNT 2

#include <map>
#define CHECK_FLAG(a, b) ((a & b) == b)

namespace zxmgr
{
    std::vector<byte*> _stdcall GetPlayers();
    std::vector<byte*> _stdcall GetUnits(byte* player = NULL);

    void SendMessageRaw(byte* pptr, const char* message);
    void SendMessage(byte* pptr, const char* mask, ...);
    void Kick(byte* pptr, bool silent);
    byte* _stdcall FindByNickname(const char* nickname);
    byte* _stdcall FindByLogin(const char* login);
    byte* _stdcall FindByID(uint16_t id);
    void _stdcall KickAll(byte* caster);
    void _stdcall KickAllSilent(byte* caster);
    void _stdcall Kill(byte* player, byte* caster);
    void _stdcall KillAll(byte* caster, bool ai_only);
    bool IsConnected(byte* player);
    byte* Summon(byte* player, const char* unitname, byte* pthis, bool ishero, byte* targetptr = 0);
    byte* ConstructItemN(const char* definition);
    byte* ConstructItem(std::string definition);
    bool CheckItem(byte* item);
    void GiveItemTo(byte* item, byte* player);
    void DestroyItem(byte* item);
    void GiveMoney(byte* pptr, unsigned long count, unsigned long flags);
    void UpdatePlayer(unsigned long flags, unsigned long info, unsigned long unknown, byte* pptr);
    void __stdcall Own(byte* to, byte* from);
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
    void UpdateUnit(byte* unit, byte* player, unsigned long flags, unsigned long flags2, unsigned long flags3, unsigned long flags4);

    int GetUnitSize(byte* unit);
    void MakeUnitNoClip(byte* unit);

    void CreateSack(const char* itemname, unsigned long x, unsigned long y, unsigned long money);

    void _stdcall GMLog(const char* format, ...);
    void _stdcall ServerLog(const char* format, ...);

    void Disconnect(byte* what);

    unsigned long _stdcall GetTicks();

    //unsigned long _stdcall CreateUnit(const char* name);
    //void FreeUnit(unsigned long cptr);

    byte* GetNetworkStruct(byte* player);

    byte* GetMainWnd();
    bool ReturnUnit(byte* unit);

    uint32_t GetSpells(byte* unit);
    void SetSpells(byte* unit, uint32_t spells);

    void CastPointEffect(byte* from, uint8_t to_x, uint8_t to_y, uint8_t spell);

    uint8_t GetDiplomacy(byte* player1, byte* player2);
    void SetDiplomacy(byte* player1, byte* player2, uint8_t newdip);

    byte* GetItemFromPack(byte* pack, uint16_t index, uint16_t count);
    void SaveCharacter(byte* player);

    byte* GetUnitByID(uint16_t player_id, uint16_t unit_id);

    SOCKET GetSocket(uint16_t player_id);
    SOCKET GetSocket(byte* player);
}
