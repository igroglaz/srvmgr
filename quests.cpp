#include "config_new.h"

int __declspec(naked) imp_inn_quest_roll_interval()
{ // 0053693C
    __asm
    {
        mov     eax, dword ptr [Config::QuestRollInterval]
        cmp		[edx + 0xD8], eax
		ret
    }
}
/**
Handles a situation when player upgrades an item in the inn, but error happens.
The error happens in two cases:
1. There is no original item on the char. For instance you upgrade a sword, but then exit the inn and take the sword off. Then enters and accept the upgrade.
2. The item-to-be-upgraded is missing a special flag(item->dword54). The flag is set when server picks an item for upgrade.
Originally server would delete both the upgraded and the original items found. But this method only deletes the upgraded item and leaves the original item intact.
**/
int __declspec(naked) imp_inn_item_upgrade_failed()
{ // 0056154E
    __asm
    {
		/*
		Logic for the following code:
		if(orig_item != NULL){
			orig_item->put_on_unit(unit);  // replaces upgraded item
		}else{
			if(upgraded_item != NULL){
				upgraded_item->take_off(unit); // removes upgraded item
			}
		}
		if(upgraded_item != NULL){
			delete upgraded_item;
		}
		*/
		mov		ecx, [ebp-68h]	// orig_item as this
		cmp		ecx, 0
		jz		orig_item_is_null	// if orig_item == NULL, skip putting it back
        mov		edx, [ebp+8]	// unit
		push	edx
		mov		eax, [ecx]
		call	dword ptr [eax + 38h] // call orig_item->put_on_unit(unit)
		jmp		delete_upgraded_item
orig_item_is_null:
		mov     ecx, [ebp-3Ch]	// upgraded_item as this
		cmp		ecx, 0
		jz		exit			// if upgraded_item == null => goto exit
		mov     edx, [ebp+8]	// unit
		push    edx
		mov     edx, [ecx]		// edx = vtable of upgraded_item
		call    dword ptr [edx+3Ch]	// call upgraded_item->take_off(unit)
delete_upgraded_item:
		mov     ecx, [ebp-3Ch]	// upgraded_item as this
		cmp		ecx, 0
		jz		exit			// if upgraded_item == null => goto exit
		push    1
		mov     eax, [ecx]		// eax = vtable of upgraded_item
		call    dword ptr [eax+4]	// delete upgraded_item
exit:
		ret
    }
}