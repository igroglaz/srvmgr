#include "utils.h"
#include "config_new.h"
#include <vector>
#define _USE_MATH_DEFINES
#include "math.h"
#include "this_call.h"
#include "solo.h"

void* (__cdecl *a2_operator_new)(int) = (void* (*)(int))0x005DDF54;
auto a2_delete = (void (__cdecl *)(void*))0x005ddf90;
auto a2_bag_destructor = (void (__fastcall *)(T_INVENTORY_LIST*))0x00551c7a;


T_INVENTORY_LIST* __stdcall create_new_item_list()
{
    #define FUNC_ITEM_LIST_CONSTRUCTOR 0x00551C0A
    T_INVENTORY_LIST* list = (T_INVENTORY_LIST*)a2_operator_new(0x24);
    return (T_INVENTORY_LIST*)this_call(FUNC_ITEM_LIST_CONSTRUCTOR, list);
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


void a2insert(T_INVENTORY_LIST * list, int pos, T_INVENTORY_ITEM* item)
{
    this_call(0x00551FC3, (void*)list, (void*)pos, (void*)item);
}

void a2insert(T_INVENTORY_LIST * list, T_INVENTORY_ITEM* item)
{
    a2insert(list, list->maxInd, item);
}

T_INVENTORY_ITEM* a2remove(T_INVENTORY_LIST * list, int pos, int n)
{
    return (T_INVENTORY_ITEM*)this_call(0x00552E42, (void*)list, (void*)pos, (void*)n);
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
void __stdcall drop_rnd_items(T_INVENTORY_LIST * item_list_src, T_INVENTORY_LIST * item_list_dst, float probability)
{
    T_SRV_LINKED_NODE<T_INVENTORY_ITEM>* src_current = item_list_src->list.last_node;
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
    for (std::vector<IndNum>::iterator it = to_remove.begin() ; it != to_remove.end(); ++it)
    {
        a2insert(item_list_dst, 0, a2remove(item_list_src, it->ind, it->num));
    }
}

void __stdcall drop_rnd_weared_items(T_UNIT* unit, T_INVENTORY_LIST * item_list_dst, float probability)
{
    if (unit->clazz != A2_HUMAN_CLASS)
        return;        // unit does not support weared items

    for (int i = 1; i < 13; ++i )
    {
        if (getDropNum(1, probability) > 0)
        {
            __asm
            {
                mov        ecx, [i]
                mov        edx, [unit]
                mov        eax, [edx + ecx * 4 + 0x208]
                push    eax
                mov        ecx, [unit]            // this
                mov        edx, [ecx]
                mov        edx, [edx + 0x48]
                call    edx
                push    eax                    // put item on stack
                mov        ecx,    item_list_dst
                mov        edx, 0x551FA3
                call    edx
            }
        }
    }
    // ugly asm code duplication. Need to refactor
    for (int i = 0; i < 2; ++i )
    {
        if (getDropNum(1, probability) > 0)
        {
            __asm
            {
                mov        ecx, [i]
                mov        edx, [unit]
                mov        eax, [edx + ecx * 4 + 0x74]
                push    eax
                mov        ecx, [unit]            // this
                mov        edx, [ecx]
                mov        edx, [edx + 0x48]
                call    edx
                push    eax                    // put item on stack
                mov        ecx,    item_list_dst
                mov        edx, 0x551FA3
                call    edx
            }
        }
    }
}

int CopyInventoryToMap(T_UNIT *unit, T_INVENTORY_LIST *inventory, int a3, int a4)
{
    #define FUNC_COPY_INVENTORY_TO_MAP 0x0052D8D3
    return this_call(FUNC_COPY_INVENTORY_TO_MAP, (void *)unit, (void *)inventory, (void *)a3, (void *)a4);
}

bool isPlayerUnit(T_UNIT* unit)
{
    return unit->player->unitType == 0;
}

void DeleteInventory(T_INVENTORY_LIST* bag) {
    a2_bag_destructor(bag);
    a2_delete(bag);
}

void __stdcall drop_partially(T_UNIT* unit, int a3, int a4)
{
    if (unit && unit->inventory)
    {
        if (isPlayerUnit(unit))
        {
            T_INVENTORY_LIST* bag = create_new_item_list();
            drop_rnd_items(unit->inventory, bag, Config::InventoryDropProbability);
            drop_rnd_weared_items(unit, bag, Config::WearDropProbability);

            if (bag->list.size) {
                PoisonStapleCell(unit->position);
            }

            if (IsGigaPlayer(unit)) {
                DeleteInventory(bag);
            } else {
                CopyInventoryToMap(unit, bag, a3, a4);
            }
        }
        else
        {
            StapleCellOnMobKill(unit);

            // If this is a monster, we drop all items like it's done in original a2
            CopyInventoryToMap(unit, unit->inventory, a3, a4);
            unit->inventory = create_new_item_list();
        }
    }
}

void __declspec(naked) imp_drop_partially()
{ // 0052E264
    __asm
    {
        mov     ecx, [ebp-174h]        // pass a4
        push    ecx
        mov     edx, [ebp-1Ch]        // pass a3
        push    edx
        mov     ecx, [ebp-164h]        // pass unit
        push    ecx    
        call    drop_partially
        mov     [ebp-0C0h], eax        // pass ground bag
        ret        0xC
    }
}

void __stdcall update_unit_ui_wrapper(T_UNIT *unit, int a){
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
