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
