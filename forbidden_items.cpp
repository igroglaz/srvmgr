#include "config_new.h"
#include "zxmgr.h"
#include "srvmgr.h"
#include "lib\utils.hpp"

bool ItemRemover_CheckForbidden(A2InventoryItem* item)
{
    return false;
    /*
    std::string item_name = *(const char**)(*(byte**)(item + 0x3C) + 4);
    uint16_t item_count = *(uint16_t*)(item + 0x42);
    if(item_name == "Potion Big Healing" ||
        item_name == "Potion Medium Healing") return true;
    return false;*/
}

std::vector<A2InventoryItem*> ItemRemover_Process(A2Unit* unit)
{
    std::vector<A2InventoryItem*> saved_items;

    A2InventoryList* pack = unit->inventory;
    if(!pack) return saved_items;

    uint32_t index = 0;
    A2Node<A2InventoryItem>* lp = pack->list.first_node;
    A2Node<A2InventoryItem>* last_lp = NULL;
    while(lp)
    {
        A2InventoryItem* item = lp->value;
        uint16_t item_count = item->amount;
        if(item_count && ItemRemover_CheckForbidden(item))
        {
            A2InventoryItem* p_item = zxmgr::GetItemFromPack(pack, index, item_count);
            if(p_item)
            {
                saved_items.push_back(p_item);
                if(last_lp) lp = last_lp;
                else lp = pack->list.first_node;
            }
        }

        last_lp = lp;
        lp = lp->next;
        index++;
    }

    return saved_items;
}
