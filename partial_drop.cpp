#include "utils.h"
#include "config_new.h"
#include <vector>
#define _USE_MATH_DEFINES
#include "math.h"
#include "this_call.h"

void* (__cdecl *a2_operator_new)(int) = (void* (*)(int))0x005DDF54;

T_LINKEDLIST* __stdcall create_new_item_list()
{
	#define FUNC_ITEM_LIST_CONSTRUCTOR 0x00551C0A
	T_LINKEDLIST* list = (T_LINKEDLIST*)a2_operator_new(0x24);
	return (T_LINKEDLIST*)this_call(FUNC_ITEM_LIST_CONSTRUCTOR, list);
}

float rnd(){
	return (float)rand() / (RAND_MAX + 1);
}

float rnd_gaussian(float mu, float sigma){
	float u1, u2;
	float two_pi = 2.0 * M_PI;
	u1 = rnd();
	u2 = rnd();
	float z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
	return z0 * sigma + mu;
}


void a2insert(T_LINKEDLIST * list, int pos, T_INVENTORY_ITEM* item){
	this_call(0x00551FC3, (void*)list, (void*)pos, (void*)item);
}

void a2insert(T_LINKEDLIST * list, T_INVENTORY_ITEM* item){
    a2insert(list, list->maxInd, item);
}

T_INVENTORY_ITEM* a2remove(T_LINKEDLIST * list, int pos, int n){
	return (T_INVENTORY_ITEM*)this_call(0x00552E42, (void*)list, (void*)pos, (void*)n);
	//__asm{
	//	mov		ecx, n
	//	push	ecx
	//	mov		ecx, pos
	//	push	ecx
	//	mov		ecx, list
	//	mov		edx, 0x00552E42
	//	call	edx
	//}
}

T_INVENTORY_ITEM* unit_unwear_item(T_UNIT * unit, T_INVENTORY_ITEM* item){
	return 0;
}

struct IndNum{
	__int16 ind;
	__int16 num;
};

float limit(float n, float lo, float hi){
	if(n < lo)
		return lo;
	if(n > hi)
		return hi;
	return n;
}
int round(float a){
	return (int)(a+0.5f);
}
int getDropNum(int num, float probability){
		int dropN = 0;
		if(probability == 0){
			dropN = 0;
		} else if(probability == 1){
			dropN = num;
		}else{
			if(num == 1){
				if(rnd() < probability){
					dropN = 1;
				}
			}else{
				float odds = limit(rnd_gaussian(probability, 0.1), 0, 1);
				dropN = round(odds * num);
			}
		}
		return dropN;
}
void __stdcall drop_rnd_items(T_LINKEDLIST * item_list_src, T_LINKEDLIST * item_list_dst, float probability)
{
	T_SRV_LINKED_NODE* src_current = item_list_src->last_node;
	int ind = item_list_src->size-1;
	std::vector<IndNum> to_remove;
	while(src_current != NULL){
		int dropN = getDropNum(src_current->value->amount, probability);
		if(dropN > 0){
			IndNum item;
			item.ind = ind;
			item.num = dropN;
			to_remove.push_back(item);
		}
		ind--;
		src_current = src_current->prev;
	}
	for (std::vector<IndNum>::iterator it = to_remove.begin() ; it != to_remove.end(); ++it){
		a2insert(item_list_dst, 0, a2remove(item_list_src, it->ind, it->num));
	}
}

bool __stdcall unit_has_weared_items(T_UNIT* unit)
{
	if(unit->weapon || unit->shield){
		return true;
	}
	for (int i = 1; i < 13; ++i )
	{
		T_INVENTORY_ITEM* item = (T_INVENTORY_ITEM*)((char*)unit+0x208)+i;
		if(item){
			return true;
		}
	}
	return false;
}
void __stdcall drop_rnd_weared_items(T_UNIT* unit, T_LINKEDLIST * item_list_dst, float probability)
{
	if(unit->clazz != (void *)0x0060F0C8){
		return;		// unit does not support weared items
	}
	for (int i = 1; i < 13; ++i )
	{
		if(getDropNum(1, probability) > 0){
			__asm{
				mov		ecx, [i]
				mov		edx, [unit]
				mov		eax, [edx + ecx * 4 + 0x208]
				push	eax
				mov		ecx, [unit]			// this
				mov		edx, [ecx]
				mov		edx, [edx + 0x48]
				call	edx
				push	eax					// put item on stack
				mov		ecx,	item_list_dst
				mov		edx, 0x551FA3
				call	edx
			}
		}
	}
	// ugly asm code duplication. Need to refactor
	for (int i = 0; i < 2; ++i )
	{
		if(getDropNum(1, probability) > 0){
			__asm{
				mov		ecx, [i]
				mov		edx, [unit]
				mov		eax, [edx + ecx * 4 + 0x74]
				push	eax
				mov		ecx, [unit]			// this
				mov		edx, [ecx]
				mov		edx, [edx + 0x48]
				call	edx
				push	eax					// put item on stack
				mov		ecx,	item_list_dst
				mov		edx, 0x551FA3
				call	edx
			}
		}
	}
}

int CopyInventoryToMap(T_UNIT *unit, T_LINKEDLIST *inventory, int a3, int a4){
#define FUNC_COPY_INVENTORY_TO_MAP 0x0052D8D3
	return this_call(FUNC_COPY_INVENTORY_TO_MAP, (void *)unit, (void *)inventory, (void *)a3, (void *)a4);
	//__asm{
	//	mov		ecx, a4
	//	push	ecx
	//	mov		ecx, a3
	//	push	ecx
	//	mov		ecx, inventory
	//	push	ecx
	//	mov		ecx, unit	// this
	//	mov		edx, FUNC_COPY_INVENTORY_TO_MAP
	//	call    edx 
	//}
}
const int T_UNIT_SKIP_MAN = 0x1102;
const int T_UNIT_SKIP_DROP = 0xBA18;
const int T_UNIT_SKIP_DMG = 0xA203;
bool __stdcall nonStandardUnit(T_UNIT* unit, unsigned __int16 spec){
	if(unit && unit->inventory && unit->inventory->size >= 1){
		T_SRV_LINKED_NODE* node = unit->inventory->first_node;
		for(int i = 0; i < 3; i++){
			if(!node){
				break;
			}
			if(node->value->id == spec){
				return true;
			}
			node=node->next;
		}
	}
	return false;
}
#define uint32 unsigned __int32
bool isPlayerUnit(T_UNIT* unit){
	return unit->unknown_struct->unitType == 0;
}
void __stdcall drop_partially(T_UNIT* unit, int a3, int a4)
{
	if(unit && unit->inventory && (unit->inventory->size > 0 || unit_has_weared_items(unit))){
		if(nonStandardUnit(unit, T_UNIT_SKIP_DROP)){
			return;
		}
		if(isPlayerUnit(unit)){
			T_LINKEDLIST* bag = create_new_item_list();
			drop_rnd_items(unit->inventory, bag, Config::InventoryDropPropapility);
			drop_rnd_weared_items(unit, bag, Config::WearDropPropapility);
			CopyInventoryToMap(unit, bag, a3, a4);
		}else{
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
		mov     ecx, [ebp-174h]		// pass a4
		push    ecx
		mov     edx, [ebp-1Ch]		// pass a3
		push    edx
		mov     ecx, [ebp-164h]		// pass unit
		push	ecx	
		call	drop_partially
		mov     [ebp-0C0h], eax		// pass ground bag

ret_point:
		ret		0xC
    }
}
int __stdcall check_unit_dmg(T_UNIT *unit, int a2, int a3){
	if(nonStandardUnit(unit, T_UNIT_SKIP_DMG)){
		return 0;
	}
	int res;
	__asm{
		mov		edx, a3
		push	edx
		mov		edx, a2
		push	edx
		mov		ecx, unit
		mov		edx, unit
		mov		edx, [edx]
		mov		edx, [edx + 0x54]
		call	edx
		mov		res, eax
	}
	return res;
}
int __stdcall imp_check_unit_man(){
	__asm{
		push	eax	// store for future
		push	ecx // store for future
		push	T_UNIT_SKIP_MAN
		push	ecx
		call	nonStandardUnit
		cmp		al, 1
		pop		ecx
		pop		eax
		je		special_case
std_case:
		mov		[ecx+0x9A], ax
special_case:
	}
}

int __declspec(naked) imp_check_unit_dmg()
{ // 0053693C
    __asm
    {
        mov     eax, [ebp+8]
        push    eax
        mov     ecx, [ebp-4]
        push    ecx
        mov     ecx, [ebp+0Ch]
		push	ecx
		call	check_unit_dmg
		mov     [ebp-0xC], eax
ret_point:
		ret		0x8
    }
}