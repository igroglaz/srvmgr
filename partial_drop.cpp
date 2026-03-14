#include "a2types.h"
#include "config_new.h"
#define _USE_MATH_DEFINES
#include "math.h"
#include "this_call.h"
#include "solo.h"

void* (__cdecl *a2_operator_new)(int) = (void* (*)(int))0x005DDF54;
auto a2_delete = (void (__cdecl *)(void*))0x005ddf90;
auto a2_bag_destructor = (void (__fastcall *)(A2InventoryList*))0x00551c7a;

A2InventoryList* __stdcall create_new_item_list()
{
    #define FUNC_ITEM_LIST_CONSTRUCTOR 0x00551C0A
    A2InventoryList* list = (A2InventoryList*)a2_operator_new(0x24);
    return (A2InventoryList*)this_call(FUNC_ITEM_LIST_CONSTRUCTOR, list);
}

float rnd()
{
    return (float)rand() / (RAND_MAX + 1);
}

float rnd_gaussian(float mu, float sigma)
{
    float u1, u2;
    float two_pi = static_cast<float>(2.0 * M_PI);
    u1 = rnd();
    u2 = rnd();
    float z0 = static_cast<float>(sqrt(-2.0 * log(u1)) * cos(two_pi * u2));
    return z0 * sigma + mu;
}


void a2insert(A2InventoryList * list, int pos, A2InventoryItem* item)
{
    this_call(0x00551FC3, (void*)list, (void*)pos, (void*)item);
}

void a2insert(A2InventoryList * list, A2InventoryItem* item)
{
    a2insert(list, list->maxInd, item);
}

A2InventoryItem* a2remove(A2InventoryList * list, int pos, int n)
{
    return (A2InventoryItem*)this_call(0x00552E42, (void*)list, (void*)pos, (void*)n);
}

struct IndNum
{
    __int16 ind;
    __int16 num;
};

float limit(float n, float lo, float hi)
{
    if (n < lo)
        return lo;
    if (n > hi)
        return hi;
    return n;
}
int round_num(float a)
{
    return (int)(a+0.5f);
}
int getDropNum(int num, float probability)
{
        int dropN = 0;
        if (probability == 0)
            dropN = 0;
        else if (probability == 1)
            dropN = num;
        else
        {
            if (num == 1)
            {
                if (rnd() < probability)
                    dropN = 1;
            }
            else
            {
                float odds = limit(rnd_gaussian(probability, 0.1f), 0, 1);
                dropN = round_num(odds * num);
            }
        }
        return dropN;
}

void __stdcall drop_rnd_items(A2InventoryList* item_list_src, A2InventoryList* item_list_dst, float probability) {
    A2Node<A2InventoryItem>* src_current = item_list_src->list.last_node;
    int ind = item_list_src->list.size - 1;
    std::vector<IndNum> to_remove;
    while (src_current != NULL)
    {
        if (src_current->value->id == 3667) {
            ind--;
            src_current = src_current->prev;
            continue;
        }

        int dropN = getDropNum(src_current->value->amount, probability);

        if (dropN > 0)
        {
            IndNum item;
            item.ind = ind;
            item.num = dropN;
            to_remove.push_back(item);
        }

        ind--;
        src_current = src_current->prev;
    }

    for (std::vector<IndNum>::iterator it = to_remove.begin() ; it != to_remove.end(); ++it) {
        a2insert(item_list_dst, 0, a2remove(item_list_src, it->ind, it->num));
    }
}

void __stdcall drop_rnd_weared_items(A2Unit* unit, A2InventoryList* item_list_dst, float probability) {
    if (unit->clazz != A2_CLASS_HUMAN) {
        return; // unit does not support worn items
    }

    const A2Human* human = reinterpret_cast<A2Human*>(unit);

    for (int i = -2; i < 13; ++i ) {
        A2InventoryItem* item = (i == -2 ? unit->weapon : i == -1 ? unit->shield : human->dress[i]);

        if (i == -2 && IsPurePlayer(unit)) {
            continue; // Pure players keep their weapon on death
        }

        if (item && getDropNum(1, probability) > 0) {
            a2insert(item_list_dst, unit->clazz->UnitUnequip(unit, item));
        }
    }
}

void CopyInventoryToMap(A2Unit *unit, A2InventoryList *inventory, int money, int main_unit) {
    if (inventory->list.size || money) {
        this_call(0x0052D8D3, (void *)unit, (void *)inventory, (void *)money, (void *)main_unit);
    }
}

bool isPlayerUnit(A2Unit* unit)
{
    return unit->player->is_ai == 0;
}

void DeleteInventory(A2InventoryList* bag) {
    a2_bag_destructor(bag);
    a2_delete(bag);
}

void __fastcall drop_partially(A2Unit* unit, int unused_edx, A2InventoryList* unused, int money, int main_unit) {
    Printf("drop_partially: unit=0x%x, money=%d, main_unit=%d", unit, money, main_unit);

    if (!unit) {
        return;
    }

    if (isPlayerUnit(unit)) {
        A2InventoryList* bag = create_new_item_list();

        drop_rnd_items(unit->inventory, bag, Config::InventoryDropProbability);
        drop_rnd_weared_items(unit, bag, Config::WearDropProbability);

        if (bag->list.size) {
            PoisonStapleCell(unit->position);
        }

        if (IsGigaPlayer(unit) || IsPurePlayer(unit)) {
            DeleteInventory(bag);
        } else {
            CopyInventoryToMap(unit, bag, money, main_unit);
        }
    } else if (unit->inventory) {
        StapleCellOnMobKill(unit);

        // If this is a monster, we drop all items like it's done in original a2
        CopyInventoryToMap(unit, unit->inventory, money, main_unit);
        unit->inventory = create_new_item_list();
    }
}

void __declspec(naked) imp_drop_partially()
{ // 0052E264
    __asm
    {
        // All parameters are taken from the stack and ECX.
        call    drop_partially
        mov edx, 0x0052e2c8
        jmp edx
    }
}

void __stdcall update_unit_ui_wrapper(A2Unit *unit, int a){
    __asm
    {
        mov     edx, a
        push    edx
        mov     edx, unit
        push    edx
        mov     ecx, 0x006C3A08
        mov     edx, 0x51C601
        call    edx
    }
}

// Address: 0052da11
void __declspec(naked) partial_drop_skip_undress() {
    __asm {
        // Skip putting weapon, shield and all equipped items into the inventory.
        mov eax, 0x0052daab
        jmp eax
    }
}