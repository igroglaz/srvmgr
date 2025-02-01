#include "utils.h"
#include "lib/utils.hpp"

bool IsScroll(T_INVENTORY_ITEM* item) {
    return 3590 <= item->id && item->id <= 3647; // Reference: https://igroglaz.com/allods2/razrabotka/opisanie-predmetov
}

void a2insert(T_INVENTORY_LIST* list, int pos, T_INVENTORY_ITEM* item);
void RefreshPlayerInventory(T_UNIT* unit);

int __fastcall CheckItemWhenCasting(T_INVENTORY_ITEM* item, T_UNIT* unit, int position) {
    if (!item || !unit) { // Impossible, but let's check anyway.
        return 1;
    }

    // If the item is not a scroll, or it's not a scroll with spell (for some
    // reason), or if current unit is a mage --- don't cast anything and put
    // the item back.
    // Note: vanilla logic is `item->effects.size == 0 || item->effects.first_node->value->effect_id != 0x29`.
    if (!IsScroll(item) || item->effects.size == 0 || item->effects.first_node->value->effect_id != 0x29 || !IsWarrior(unit)) {
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
extern "C" __declspec(naked) void fix_scroll_burn() {
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
