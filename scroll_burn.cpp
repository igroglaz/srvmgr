#include <set>

#include "a2types.h"
#include "lib/utils.hpp"

bool IsScroll(A2InventoryItem* item) {
    return 3590 <= item->id && item->id <= 3647; // Reference: https://igroglaz.com/allods2/razrabotka/opisanie-predmetov
}

void a2insert(A2InventoryList* list, int pos, A2InventoryItem* item);
void RefreshPlayerInventory(A2Unit* unit);

int __fastcall CheckItemWhenCasting(A2InventoryItem* item, A2Unit* unit, int position) {
    Printf("[scroll_burn] CheckItemWhenCasting: item=0x%x (id=%d), unit=0x%x (%s), position=%d", item, item ? item->id : 0, unit, unit ? unit->name : "?", position);

    if (!item || !unit) { // Impossible, but let's check anyway.
        return 1;
    }

    // If the item is not a scroll, or it's not a scroll with spell (for some
    // reason), or if current unit is a mage --- don't cast anything and put
    // the item back.
    // Note: vanilla logic is `item->effects.size == 0 || item->effects.first_node->value->effect_id != 0x29`.
    if (!IsScroll(item) || item->effects.size == 0 || item->effects.first_node->value->effect_id != 0x29 || !IsWarrior(unit)) {
        Printf(
            "[scroll_burn] prevent burning item: id=%d, effects=%d, first=%d, warrior=%d",
            item->id,
            item->effects.size,
            item->effects.size && item->effects.first_node && item->effects.first_node->value ? item->effects.first_node->value->effect_id : 0,
            IsWarrior(unit)
        );
        if (unit->inventory) {
            Printf("[scroll_burn] item id=%d should be put into inventory of unit '%s' at position %d", item->id, unit->name, position);
            a2insert(unit->inventory, position, item);
            RefreshPlayerInventory(unit);
        }
        return 1;
    }

    return 0;
}

// Address: 005056e1
void __declspec(naked) fix_scroll_burn() {
    __asm {
        mov edx, DWORD PTR [ebp-0x10] // Packet.
        mov eax, 0
        mov ax, WORD PTR [edx+0x10] // 2-byte position in the inventory.
        push eax
        mov ecx, DWORD PTR [ebp-0x40] // Item.
        mov edx, DWORD PTR [ebp-0x18] // Unit.
        call CheckItemWhenCasting
        cmp eax, 0
        jz item_allowed  // If casting is not allowed, bail.
        mov eax, 0x005056f4
        jmp eax

    item_allowed:
        // Proceed with original logic.
        mov eax, 0x005056f9
        jmp eax
    }
}

// Following two sets are all rings and amulets. All custom items are supposed to be worn exlusively by wizards or fighters.
std::set<uint16_t> forbidden_wizard_items{
    54337, 54594, // Black Pearl
    54273, 54530, // Black Widow
    54465, 54722, // Crimson Red Crystal
    54305, 54562, // Flamer
    17473, 17730, // Golden Wrath
    54433, 54690, // Valkyr
};
std::set<uint16_t> forbidden_warrior_items{
    58561, 58818, // Aizen's Hogyoku
    54401, 54658, // Elf
    5249, 5506,   // Light Fantastic
    1153, 1410,   // Sanctuary Red
    50305, 50562, // Sea Witcher
    37985, 38242, // Shaman
    54369, 54626, // Vampic
};

bool CanEat(A2Unit* unit, A2InventoryItem* item) {
    if (item->id == 3667) {  // Treasure.
        return false;
    }

    if (IsWarrior(unit)) {
        if (forbidden_warrior_items.count(item->id)) {
            return false;
        }
    } else {
        if (forbidden_wizard_items.count(item->id)) {
            return false;
        }
    }

    return true;
}

struct MovePacket {
    uint8_t whatever[12];
    uint8_t from;
    uint8_t to;
    uint16_t position;
};

bool __fastcall DontEatBadFood(A2InventoryItem* item, A2Unit* unit, MovePacket* packet) {
    if (item == nullptr) { // The item might not exist.
        return false;
    }

    if (!unit) { // Impossible, but let's check anyway.
        return false;
    }

    // "Eating" the item --- moving from the bag onto the character. 1: character, 2: bag, 3: map, 4: store.
    if (packet->from == 2 && packet->to == 1) {
        // Check if the item can be consumed/put on.
        if (!CanEat(unit, item)) {
            Printf("[bad_food] prevent eating bad food: item id=%d, unit='%s', warrior=%d", item->id, unit->name, IsWarrior(unit));
            if (unit->inventory) {
                Printf("[bad_food] item id=%d should be put into inventory of unit '%s' at position %d", item->id, unit->name, packet->position);
                a2insert(unit->inventory, packet->position, item);
                RefreshPlayerInventory(unit);
            }
            return false;
        }
    }

    return true;
}

// Address: 00505bdf
void __declspec(naked) dont_eat_bad_food() {
    __asm {
        push DWORD PTR [ebp+0x8] // Packet.
        mov ecx, DWORD PTR [ebp-0x60] // Item.
        mov edx, DWORD PTR [ebp-0x54] // Unit.
        call DontEatBadFood

        cmp eax, 0
        jnz item_allowed  // If eating is not allowed, bail.
        mov eax, 0x0050864e
        jmp eax

    item_allowed:
        // Proceed with original logic.
        mov eax, 0x00505bea
        jmp eax
    }
}
