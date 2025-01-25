#include "config_new.h"
#include "lib/utils.hpp"
#include "utils.h"
#include "zxmgr.h"

// Characters with `@` as the first letter of the name are in the solo mode.
bool IsSoloPlayer(T_UNIT* unit) {
    return unit->name2[0] == '@';
}

A2Server* a2server_instance = nullptr;

// Remember the address of the A2Server instance. We're doing it in the packet
// handler for sack pickup, so it's guaranteed to be executed before the sack pickup.
//
// It's a global variable, loaded via `mov eax, ds:0x642c2c`. but I can't
// access it. No idea why. I guess DLL uses a different data segment and I
// can't read the data segment of the EXE?
extern "C" void __fastcall RememberA2Server(A2Server* server) {
    if (a2server_instance != server) {
        Printf("remembering a2server instance: now 0x%x", server);
        a2server_instance = server;
    }
}

// Address: 005051c5
extern "C" __declspec(naked) void remember_a2server() {
    __asm {
        // Pick a2server pointer from the stack.
        mov ecx, DWORD PTR [ebp-0xc38]
        call RememberA2Server

        // Replay original instruction and restore instruction pointer.
        mov eax, DWORD PTR [ebp-0x10]
        mov ecx, 0
        mov cl, BYTE PTR [eax+0xc]
        mov edx, 0x005051cd
        jmp edx
    }
}

template <typename T>
void RemoveLinkedListElement(T_SRV_LINKED_NODE<T>* item, T_LINKEDLIST<T>* list) {
    if (item->next) {
        item->next->prev = item->prev;
    } else {
        list->last_node = item->prev;
    }
    if (item->prev) {
        item->prev->next = item->next;
    } else {
        list->first_node = item->next;
    }
    list->size--;
}

bool IsBookOfBless(T_INVENTORY_ITEM* item) {
    return item
        && item->id == 3589                                 // Astral book,
        && item->effects.size != 0                          // with magic,
        && item->effects.first_node->value->effect_id == 42 // casting spell
        && item->effects.first_node->value->value1 == 20;   // Bless.
}

// Refresh the inventory cached at the client.
// Taken from around address 00503663.
void RefreshPlayerInventory(T_UNIT* unit) {
    // Only ECX is used by the function, but __fastcall passes EDX too.
    typedef void (__fastcall *A2RefreshInventory)(int address, int unused, T_UNIT* unit, T_PLAYER* player, int arg4, int arg5, int arg6, int arg7);
    A2RefreshInventory refresh_inventory = (A2RefreshInventory)0x00519221;

    refresh_inventory(0x006c3a08, 0, unit, unit->player, -1, 0x0ffb, 0, 0);
}

// Sack pickup logic for a solo character.
//
// Iterates through all sacks currently on the map. If the character is staying
// on top of a sack and the character has an astral book with bless, replace
// that book with the treasure(s) from the sack. If the player already has a
// treasure, increase the amount of treasures and don't remove the book.
//
// Known issues:
// 1. Doesn't update the sack money size, so the bag visually retains its size.
//  I didn't yet find where this value is stored.
// 2. Leaks the memory of the item if the player already had a treasure. YOLO!
// 3. Visual issue: the treasure doesn't seem to appear in the player's
//  inventory until they reorder anything within the inventory.
void SoloPickup(T_UNIT* unit, A2Server* server) {
    if (unit == nullptr) {
        Printf("[solo_pickup]: nullptr unit");
        return;
    }

    if (unit->inventory == nullptr) {
        Printf("[solo_pickup]: nullptr inventory");
        return;
    }

    if (server == nullptr) {
        Printf("[solo_pickup]: nullptr a2server");
        return;
    }

    auto* server_struct = server->server_struct;
    if (server_struct == nullptr) {
        Printf("[solo_pickup]: nullptr server_struct");
        return;
    }

    auto* sacks = server_struct->sacks;
    if (sacks == nullptr || !sacks->size) {
        Printf("[solo_pickup]: no sacks");
        return;
    }

    bool picked_up = false;

    auto sack_ptr = sacks->first_node;
    for (auto sack_ptr = sacks->first_node; sack_ptr != nullptr; sack_ptr = sack_ptr->next) {
        const auto& sack = sack_ptr->value;
        if (!sack || unit->position->yx != sack->position->yx || !sack->items) {
            continue;
        }

        for (auto item_ptr = sack->items->first_node; item_ptr != nullptr; item_ptr = item_ptr->next) {
            if (item_ptr->value->id != 3667) {
                continue;
            }

            bool had_treasure_in_inventory = false;

            for (auto inventory_ptr = unit->inventory->list.first_node; inventory_ptr != nullptr; inventory_ptr = inventory_ptr->next) {
                if (inventory_ptr->value->id == item_ptr->value->id) {
                    // Increase the amount of treasures in player's inventory.
                    // Note: here we assume that all treasures are the same and don't have effects.
                    inventory_ptr->value->amount += item_ptr->value->amount;

                    RemoveLinkedListElement(item_ptr, sack->items);
                    // We leak the memory of `item_ptr` here. I don't know how to clean it up :D

                    zxmgr::SendMessage((byte*)unit->player, "You picked up another treasure!");
                    zxmgr::SendMessage((byte*)unit->player, "Use #reborn command to check your status.");
                    had_treasure_in_inventory = true;
                    picked_up = true;
                    break;
                }
            }

            if (!had_treasure_in_inventory) {
                for (auto inventory_ptr = unit->inventory->list.first_node; inventory_ptr != nullptr; inventory_ptr = inventory_ptr->next) {
                    if (IsBookOfBless(inventory_ptr->value)) {
                        // Replace the book with the treasure(s).
                        inventory_ptr->value->id = item_ptr->value->id;
                        inventory_ptr->value->amount = item_ptr->value->amount;
                        inventory_ptr->value->effects.size = 0;
                        inventory_ptr->value->effects.first_node = nullptr;
                        inventory_ptr->value->effects.last_node = nullptr;

                        RemoveLinkedListElement(item_ptr, sack->items);
                        // We leak the memory of `item_ptr` here. I don't know how to clean it up :D

                        zxmgr::SendMessage((byte*)unit->player, "You picked up a treasure!");
                        zxmgr::SendMessage((byte*)unit->player, "Use #reborn command to check your status.");
                        picked_up = true;
                        break;
                    }
                }
            }

            if (sack->items->size == 0) {
                RemoveLinkedListElement(sack_ptr, sacks);
            }
        }
    }

    if (picked_up) {
        RefreshPlayerInventory(unit);
    }
}

// Address: 005a9977
extern "C" __declspec(naked) void solo_pickup_sack() {
    T_UNIT* unit;
    __asm {
        // Pick the unit pointer.
        mov eax, DWORD PTR [ebp+0x8]
        mov unit, eax
    }

    if (!IsSoloPlayer(unit) || Config::ServerID == EASY) {
        // Original logic for regular chars and first server.
        unit->dword50 = 2;
    } else {
        SoloPickup(unit, a2server_instance);
    }

    __asm {
        // Restore original position. Don't replay original instruction,
        // because we're skipping it for solo characters.
        mov edx, 0x005a997e
        jmp edx
    }
}

// Address: 005a99dd
extern "C" __declspec(naked) void solo_pick_all_sacks() {
    T_UNIT* unit;
    __asm {
        // Pick the unit pointer, it's stored in ECX by the instruction at 0x005a99da.
        mov unit, ecx
    }

    if (!IsSoloPlayer(unit) || Config::ServerID == EASY) {
        // Original logic for regular chars and first server.
        unit->dword50 = 26;
    }

    __asm {
        // Restore original position. Don't replay original instruction,
        // because we're skipping it for solo characters.
        mov edx, 0x005a99e4
        jmp edx
    }
}
