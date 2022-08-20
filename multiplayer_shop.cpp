#include "zxmgr.h"
#include "lib/utils.hpp"
#include <map>

// maps custom ID to shop data
static std::map<uint32_t, byte*> ShopReplacements;

byte* _stdcall CreateShopFromALMStructure(byte* struc, byte* mapdata)
{
    byte* shop = nullptr;
    byte coordsTmp[32]; // 32, right?

    __asm
    {
        push    0xC4
        mov        edx, 0x00401880
        call    edx // malloc(0xC4)
        mov        shop, eax
        
        mov        eax, 0x006B16A8
        push    [eax]
        mov        eax, struc
        mov        cl, [eax+2]
        push    ecx
        mov        eax, struc
        mov        cl, [eax]
        push    ecx
        lea        ecx, coordsTmp
        mov        edx, 0x0058A4B1
        call    edx // init coords object

        push    eax
        mov        eax, struc
        mov        cl, [eax+4]
        push    ecx
        mov        ecx, shop
        mov        edx, 0x00544495
        call    edx
        mov        shop, eax
    }

    // both are done in the calling function, but we need these for completeness of custom shops
    // set structure ID
    uint32_t shop_id = *(uint32_t*)(struc + 0x10);
    *(uint32_t*)(shop + 8) = shop_id;
    // set player pointer
    uint32_t player_id = *(uint32_t*)(struc + 0x08);
    *(byte**)(shop + 0x14) = zxmgr::FindByID(player_id);

    // find shop data in ALM structure
    byte* shop_data_array = mapdata + 0x324;
    uint32_t shop_data_size = *(uint32_t*)(shop_data_array + 8);
    byte** shop_data = *(byte***)(shop_data_array + 4);
    for (uint32_t i = 0; i < shop_data_size; i++)
    {
        byte* sd = shop_data[i];
        if (*(uint32_t*)(sd) == shop_id)
        {
            // each shelf
            for (int j = 0; j < 4; j++)
            {
                *(uint32_t*)(shop + 0x14 * j + 0x70) = *(uint32_t*)(sd + 4 * j + 0x14);
                *(uint32_t*)(shop + 0x14 * j + 0x74) = *(uint32_t*)(sd + 4 * j + 0x24);
                *(uint32_t*)(shop + 0x14 * j + 0x7C) = *(uint32_t*)(sd + 4 * j + 0x44);
                *(uint32_t*)(shop + 0x14 * j + 0x78) = *(uint32_t*)(sd + 4 * j + 0x34);
                *(uint32_t*)(shop + 0x14 * j + 0x80) = *(uint32_t*)(sd + 4 * j + 0x04);
            }
            break;
        }
    }

    return shop;
}

byte* _stdcall CreateMultiShops(byte* struc, byte* mapdata)
{
    byte* mainShop = CreateShopFromALMStructure(struc, mapdata); // return this back
    uint32_t shop_id = *(uint32_t*)(mainShop + 8);
    // create 16 shops, one for each player.
    // give them ID of Player|Struct
    for (uint32_t i = 0; i < 16; i++)
    {
        uint32_t additional_id = (shop_id & 0x00FFFFFF) | ((16+i) << 24);
        byte* addShop = CreateShopFromALMStructure(struc, mapdata);
        *(uint32_t*)(addShop + 8) = additional_id;

        // add structure
        __asm
        {
            push    addShop
            mov        ecx, 0x642C2C
            mov        ecx, [ecx]
            add        ecx, 0x98
            mov        ecx, [ecx+0x24]
            mov        ecx, [ecx]
            mov        edx, 0x00558228
            call    edx
        }

        ShopReplacements[additional_id] = addShop;
    }

    return mainShop;
}

void __declspec(naked) imp_LoadShopData()
{
    __asm
    { // 59C66E
        push    [ebp+0x08] // complete map data
        push    [ebp-0x18] // shop data
        call    CreateMultiShops
        mov        [ebp-0x10], eax
        mov        edx, 0x0059CBFF // continue
        jmp        edx
    }
}

byte* _stdcall FindShopByID(uint32_t id)
{
    byte* shop = ShopReplacements[id];
    return shop;
}

void _stdcall ClearShops()
{
    ShopReplacements.clear();
}

byte* _stdcall CheckReplacementBuilding(byte* shop, byte* coords)
{
    // check which player owns these coords.
    // this is not 100% reliable... to-do: implement proper player check later
    std::vector<byte*> plrs = zxmgr::GetPlayers();
    for (std::vector<byte*>::iterator it = plrs.begin(); it != plrs.end(); ++it)
    {
        byte* plr = *it;
        byte* chr = *(byte**)(plr + 0x38);
        if (!chr) continue;
        if (*(byte**)(chr + 0x10) == coords)
        {
            // player id
            uint16_t player_id = *(uint16_t*)(plr + 0x04);
            uint32_t replacement_id = player_id;
            replacement_id <<= 24;
            replacement_id |= *(uint32_t*)(shop + 8) & 0x00FFFFFF; // shop id
            byte* replacement = FindShopByID(replacement_id);
            return replacement;
        }
    }

    return nullptr;
}

void __declspec(naked) imp_EnterCustomShop()
{
    __asm
    { // 502C50
        
        push    ebp
        mov        ebp, esp
        push    [ebp+0x08]
        call    sub_502C50
        push    [ebp+0x08]
        push    eax
        call    CheckReplacementBuilding
        mov        esp, ebp
        pop        ebp
        retn    0x0004

    sub_502C50:
        push    ebp
        mov        ebp, esp
        sub        esp, 8
        mov        [ebp-0x08], ecx
        mov        edx, 0x00502C59
        jmp        edx
    }
}