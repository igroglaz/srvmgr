#include "solo.h"

#include "config_new.h"
#include "lib/utils.hpp"
#include "utils.h"
#include "zxmgr.h"

// Solo-characters are allowed to pick up bags only from the stapled cells (from mobs they've killed).
// When a monster leaves a new bag, this cell is stapled with the ID of last-hit player.
// If several different players killed mobs in the same cell without picking up a bag, the staple is poisoned (set to -1).
// If a player adds an item to a staple bag (by dying or moving an item), the staple is also poisoned.
std::unordered_map<uint16_t, int8_t> staple_cells;
// Names of characters that have stapled cells. If the name changes, that's due to relogin, so we remove all staples.
std::unordered_map<int8_t, std::string> staple_char_names;

// Characters with `@` as the first letter of the name are in the solo mode.
bool IsPureSoloPlayer(T_UNIT* unit) {
    return unit->name[0] == '@';
}

// Giga-players --- solo players which get reverted to checkpoint if they die.
bool IsGigaPlayer(T_UNIT* unit) {
    return unit->name[0] == '_';
}

// All solo characters.
bool IsSoloPlayer(T_UNIT* unit) {
    return IsPureSoloPlayer(unit) || IsGigaPlayer(unit);
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

void ClearStapleCells() {
    staple_cells.clear();
    staple_char_names.clear();
}

// Address: 00504a99
extern "C" __declspec(naked) void remember_a2server() {
    __asm {
        // a2server pointer is in ECX, input parameter.
        call RememberA2Server

        // Replay original instruction and restore instruction pointer.
        push -1
        push 0x601976
        mov edx, 0x00504aa0
        jmp edx
    }
}

T_INVENTORY_ITEM* a2remove(T_INVENTORY_LIST* list, int pos, int n); // defined in partial_drop.cpp

bool GigaAllowedToPickup(T_INVENTORY_ITEM* item, ServerIDType server_id) {
    if (item->id == 3667) { // Treasures can be picked up always.
        return true;
    }

    if (server_id >= QUEST_T1) {
        // At quest maps we allow picking up quest items.
        return item->price == 2 && item->effects.size;
    }

    return false;
}

// Refresh the inventory cached at the client.
// Taken from around address 00503663.
void RefreshPlayerInventory(T_UNIT* unit) {
    Printf("[solo] RefreshPlayerInventory: unit=0x%x (%s), player=0x%x (id=%d)", unit, unit ? unit->name : "?", unit ? unit->player : nullptr, unit && unit->player ? unit->player->id_ext.id : -1);

    // Only ECX is used by the function, but __fastcall passes EDX too.
    typedef void (__fastcall *A2RefreshInventory)(int address, int unused, T_UNIT* unit, T_PLAYER* player, int arg4, int arg5, int arg6, int arg7);
    A2RefreshInventory refresh_inventory = (A2RefreshInventory)0x00519221;

    refresh_inventory(0x006c3a08, 0, unit, unit->player, -1, 0x0ffb, 0, 0);
}

// If the player's name has changed, it means that the player has logged out and another one logged in.
// The player ID is reused, but we should clear all staples for the logged-out player.
void CheckStaplesForReloggedCharacter(T_UNIT* unit) {
    Printf("[solo] CheckStaplesForReloggedCharacter: unit=0x%x (%s, player_id=%d)", unit, unit ? unit->name : "?", unit && unit->player ? unit->player->id_ext.id : -1);

    if (!unit || !unit->player) {
        return;
    }

    const int8_t player_id = static_cast<int8_t>(unit->player->id_ext.id);
    auto stapled_name_it = staple_char_names.find(player_id);
    if (stapled_name_it == staple_char_names.end()) {
        Printf("[solo] CheckStaplesForReloggedCharacter: saved character name %d -> %s", player_id, unit->name);
        staple_char_names[player_id] = unit->name;
        return;
    }
    
    if (stapled_name_it->second != unit->name) {
        staple_char_names.erase(stapled_name_it);
        staple_char_names[player_id] = unit->name;

        int removed = 0;

        for (auto it = staple_cells.begin(); it != staple_cells.end(); ++it) {
            if (it->second == player_id) {
                ++removed;
                staple_cells.erase(it);
            }
        }

        Printf("[solo] CheckStaplesForReloggedCharacter: player %d relogged, removed %d staples", player_id, removed);
    }
}

// Giga-players are allowed to pick up only certain items.
// This function is a pre-pickup --- it removes all forbidden items from the bag the unit is standing on.
bool GigaPickup(T_UNIT* unit) {
    if (!IsGigaPlayer(unit)) {
        return true;
    }

    if (EASY < Config::ServerID) {
        bool allowed = false;

        auto sack = FindSack(unit->position->yx);
        if (!sack) {
            return false;
        }

        auto items = sack->value->items;
        for (auto item_it = items->first_node; item_it; item_it = item_it->next) {
            if (GigaAllowedToPickup(item_it->value, Config::ServerID)) {
                allowed = true;
                break;
            }
        }

        if (!allowed) {
            // Don't do anything if we're not allowed to pick the bag up.
            return false;
        }

        // Can pickup? Remove all forbidden items before the pickup.
        Printf("[giga] character %s picks up bag at %d. It had %d items and %d money", unit->name, unit->position->yx, items->size, sack->value->money);
        int position = 0;
        auto item_it = items->first_node;
        while (item_it) {
            auto item = item_it->value;

            if (!GigaAllowedToPickup(item, Config::ServerID)) {
                Printf("[giga] deleting item in sack at position %d: id=%d, amount=%d, price=%d, effects total=%d", position, item->id, item->amount, item->price, item->effects.size);
                auto next = item_it->next;
                // Reinterpret cast is kinda sketchy, but `T_INVENTORY_LIST*` has the first value field `T_LINKED_LIST<T_INVENTORY_ITEM>`, so it should still work.
                a2remove(reinterpret_cast<T_INVENTORY_LIST*>(items), position, item->amount);
                item_it = next;
            } else {
                Printf("[giga] leaving item in sack at position %d: id=%d, amount=%d, price=%d, effects total=%d", position, item->id, item->amount, item->price, item->effects.size);
                item_it = item_it->next;
                ++position;
            }
        }

        sack->value->money = 0;
    }

    return true;
}

// Sack pickup logic for a solo character.
//
// If the chosen sack is stapled with player ID, allow the player to pick it up.
void __cdecl SoloPickup(T_UNIT* unit, int y, int x) {
    Printf("[solo] SoloPickup: unit=0x%x (%s), y=%d, x=%d", unit, unit ? unit->name : "?", y, x);

    if (!IsSoloPlayer(unit)) {
        // Original logic for regular chars.
        unit->state = 2;
        return;
    }

    if (unit == nullptr || unit->position == nullptr) {
        Printf("[solo_pickup]: nullptr unit");
        return;
    }

    uint16_t yx = (((y & 0xFF) << 8) | (x & 0xFF)) & 0xFFFF;

    CheckStaplesForReloggedCharacter(unit);
    
    auto staple_it = staple_cells.find(yx);
    Printf("[solo_pickup]: staple at %d: %d", yx, (staple_it != staple_cells.end() ? staple_it->second : -1));
    if (staple_it != staple_cells.end() && staple_it->second == unit->player->id_ext.id) {
        // If the player is currently staying on the sack, let them pick it up. Otherwise let them only go to the sack.
        // This is to prevent races with other players poisoning the cell.
        if (unit->position->yx == yx) {
            Printf("[solo] SoloPickup: unit=0x%x (%s) picks up bag at %d", unit, unit->name, yx);

            if (!GigaPickup(unit)) {
                return;
            }

            unit->state = 2;
        } else {
            Printf("[solo] SoloPickup: unit=0x%x (%s) moves to %d", unit, unit->name, yx);
            unit->state = 1;
        }
    }
}

// Address: 005a9977
extern "C" __declspec(naked) void solo_pickup_sack() {
    __asm {
        push DWORD PTR [ebp+0xc]    // x coordinate of the user click
        push DWORD PTR [ebp+0x10]   // y coordinate of the user click
        push ecx                    // Unit
        call SoloPickup

        // Restore original position. Don't replay original instruction,
        // because we did it ourselves.
        mov edx, 0x005a997e
        jmp edx
    }
}

void __fastcall SoloPickupAll(T_UNIT* unit) {
    if (!unit || !unit->position || !unit->player) {
        Printf("[solo_pickup_all] null unit");
    }

    Printf("[solo] SoloPickupAll: unit=0x%x (%s)", unit, unit->name);

    if (!IsSoloPlayer(unit)) {
        // Original logic for regular chars.
        unit->state = 26;
        return;
    }

    CheckStaplesForReloggedCharacter(unit);

    // If the player stands on top of a stapled cell, let them pick up only that bag.
    auto staple_it = staple_cells.find(unit->position->yx);
    if (staple_it != staple_cells.end() && staple_it->second == unit->player->id_ext.id) {
        Printf("[solo_pickup_all] player %s picks up the bag at %d", unit->name, unit->position->yx);

        if (!GigaPickup(unit)) {
            return;
        }

        unit->state = 2;
        unit->eye2->command_to = unit->position->yx;
        // The "pickup bag" function also sets two other parameters to 0, and the
        // "pickup all bags" does the same.
    }
}

// Address: 005a99dd
extern "C" __declspec(naked) void solo_pick_all_sacks() {
    __asm {
        // The unit pointer is in ECX after the instruction at 0x005a99da.
        call SoloPickupAll

        // Restore original position. Don't replay original instruction,
        // because we did it ourselves.
        mov edx, 0x005a99e4
        jmp edx
    }
}

extern "C" void __fastcall PoisonStapleCell(A2Position* pos) {
    Printf("[staple]: PoisonStapleCell: 0x%x -> %d", pos, pos ? pos->yx : 0);
    staple_cells[pos->yx] = -1;
    Printf("[staple]: poisoned cell %d, there are %d staple cells now", pos->yx, staple_cells.size());
}

T_SRV_LINKED_NODE<A2Bag>* FindSack(uint16_t pos_yx) {
    Printf("[solo] FindSack at %d", pos_yx);

    A2Server* server = a2server_instance;

    if (server == nullptr) {
        Printf("[staple]: nullptr a2server");
        return nullptr; // Likely there are no sacks yet.
    }

    auto* server_struct = server->server_struct;
    if (server_struct == nullptr) {
        Printf("[staple]: nullptr server_struct");
        return nullptr; // Likely there are no sacks yet.
    }

    auto* sacks = server_struct->sacks;
    if (sacks == nullptr || !sacks->size) {
        Printf("[solo] FindSack: sacks=0x%x, size=%d", sacks, sacks ? sacks->size : -1);
        return nullptr;
    }

    auto sack_ptr = sacks->first_node;
    for (auto sack_ptr = sacks->first_node; sack_ptr != nullptr; sack_ptr = sack_ptr->next) {
        if (sack_ptr->value->position->yx == pos_yx) {
            Printf("[solo] FindSack: found sack=0x%x, size=%d", sack_ptr->value, sack_ptr->value->items ? sack_ptr->value->items->size : -1);
            return sack_ptr;
        }
    }
        
    Printf("[solo] FindSack: found no sack at %d", pos_yx);

    return nullptr;
}

extern "C" void __fastcall StapleCellOnMobKill(T_UNIT* killed_unit) {
    Printf(
        "[solo] StapleCellOnMobKill: unit=0x%x (name=%s, id=%d), killed_by=0x%x (name=%s, id=%d)",
        killed_unit,
        killed_unit ? killed_unit->name : "?",
        killed_unit ? killed_unit->id_ext.id : 0,
        killed_unit ? killed_unit->last_hit_by : nullptr,
        killed_unit && killed_unit->last_hit_by && killed_unit->last_hit_by->player ? killed_unit->last_hit_by->player->name : "?",
        killed_unit && killed_unit->last_hit_by && killed_unit->last_hit_by->player ? killed_unit->last_hit_by->player->id_ext.id : -1
    );

    if (!killed_unit || !killed_unit->last_hit_by || !killed_unit->last_hit_by->player || killed_unit->last_hit_by->player->id_ext.id < 16) {
        return;
    }

    int8_t killer_player_id = static_cast<int8_t>(killed_unit->last_hit_by->player->id_ext.id);
    auto position = killed_unit->position->yx;

    bool sack_exists_here = FindSack(position) != nullptr;

    int8_t staple_with = killer_player_id;

    // If there's no sack, staple the cell with the killer player. Even if the cell was previously stapled with other players.
    if (sack_exists_here) {
        // There is a sack. Is it stapled?
        auto staple_it = staple_cells.find(position);
        if (staple_it != staple_cells.end()) {
            // Is it stapled by another player? Yes --- poison it.
            if (staple_it->second != killer_player_id) {
                staple_with = -1;
            }
        }
    }

    CheckStaplesForReloggedCharacter(killed_unit->last_hit_by);

    staple_cells[position] = staple_with;

    Printf("[staple]: stapled cell %d with %d, there are %d staple cells now", position, staple_with, staple_cells.size());
}

// Address: 00505e9c
extern "C" __declspec(naked) void drop_item_under() {
    // EAX holds the address to the drop cell.
    __asm {
        mov ecx, eax
        call PoisonStapleCell

        // Restore original instruction and position.
        mov eax, DWORD PTR [ebp-0xc38]
        mov edx, 0x00505ea2
        jmp edx
    }
}

// Address: 00505ebe
extern "C" __declspec(naked) void drop_item_to_another() {
    Printf("[solo] drop_item_to_another");

    // ECX holds the address to the drop cell.
    __asm {
        call PoisonStapleCell

        // Restore original instruction and position.
        mov edx, DWORD PTR [ebp-0xc38]
        mov eax, 0x00505ec4
        jmp eax
    }
}

int __fastcall ChooseDropItem(uint8_t* packet, T_UNIT* unit) {
    uint8_t from = packet[0xc], to = packet[0xd];
    
    if (to == 3 && (from == 1 || from == 2)) {
        if (IsGigaPlayer(unit)) {
            Printf("[giga-drop] %d->%d for player %s is not allowed", from, to, unit->name);

            // Refresh player's inventory so that client knows that nothing was dropped.
            RefreshPlayerInventory(unit);

            return 1;
        }
    }

    return 0;
}

// Address: 0050597c
// Prevent giga-players from dropping items on the map.
extern "C" __declspec(naked) void choose_drop_item() {
    __asm {
        // Original instruction can be replayed immediately.
        mov DWORD PTR [ebp-0x60], 0

        mov ecx, DWORD PTR [ebp-0x5c] // Packet.
        mov edx, DWORD PTR [ebp-0x54] // Unit.
        call ChooseDropItem

        cmp eax, 0 // Did `ChooseDropItem` return 0?
        jz original // Yes -> proceed with original logic.

        // Don't do anything, immediately return from the function.
        mov edx, 0x0050864e
        jmp edx

    original:
        // Restore original position.
        mov edx, 0x00505983
        jmp edx
    }
}

// Address: 005060a4
// Prevent giga-players from dropping gold on the map.
// If the regular player drops gold, poison the cell.
extern "C" __declspec(naked) void drop_gold() {
    T_PLAYER* player;
    __asm {
        mov player, eax    // Player is in EAX
    }

    Printf("[solo] drop_gold: player=0x%x (%s)", player, player ? player->name : "?");

    if (player && player->current_unit) {
        if (IsGigaPlayer(player->current_unit)) {
            Printf("[giga-drop] dropping gold for player %s is not allowed", player->current_unit->name);

            // Refresh player's inventory so that client knows that nothing was dropped.
            RefreshPlayerInventory(player->current_unit);

            __asm {
                // Prevent changes, immediately return from the function.
                __asm {
                    mov edx, 0x0050864e
                    jmp edx
                }
            }
        } else {
            PoisonStapleCell(player->current_unit->position);
        }
    }

    __asm {
        // Restore original instruction and position.
        mov edx, DWORD PTR [ebp-0x8c]
        mov eax, 0x005060aa
        jmp eax
    }
}
