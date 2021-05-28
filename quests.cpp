#include "config_new.h"
#include "utils.h"

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



struct TQuest{
  void *clazz;
  _DWORD inn_id;
  T_ID player_id;
  __declspec(align(8)) _DWORD status;
  __int32 obj;
  _DWORD landmark_id;
  _DWORD dword1C;
  _DWORD dword20;
  __declspec(align(8)) _DWORD dword28;
  _DWORD dword2C;
};

struct TQuestNode{
  TQuestNode *next_node;
  _DWORD dword4;
  _DWORD dword8;
  TQuest *quest;
};

struct TQuestHashMap{
  _BYTE gap0[8];
  TQuestNode** bucket_arr;
  int buckets_size;
  _DWORD size;
  _BYTE gap14[96];
  TQuestNode *current_node;
  TQuest *quest;
  _BYTE gap7C[4];
  _DWORD dword80;
};

class QuestIterator{
private:
	static const TQuestHashMap& GLOBAL_QUEST_MAP;
	TQuestNode* currentNode;
	int currentBucketInd;
	int getBucketsSize(){
		return GLOBAL_QUEST_MAP.buckets_size;
	}
	int getQuestsSize(){
		return GLOBAL_QUEST_MAP.size;
	}
	TQuestNode* getNextNonEmptyBucket(){
		if(GLOBAL_QUEST_MAP.bucket_arr){
			for (;currentBucketInd < GLOBAL_QUEST_MAP.buckets_size; currentBucketInd++){
				TQuestNode* node = GLOBAL_QUEST_MAP.bucket_arr[currentBucketInd];
				if(node){
					currentBucketInd++;
					return node;
				}
			}
		}
		return NULL;
	}
public:
	QuestIterator(){
		currentNode = NULL;
		currentBucketInd = 0;
	}
	TQuestNode* next(){
		if(!currentNode){
			currentNode = getNextNonEmptyBucket();
		}else{
			currentNode = currentNode->next_node;
			if(!currentNode){
				currentNode = getNextNonEmptyBucket();
			}
		}
		return currentNode;
	}
};
const TQuestHashMap& QuestIterator::GLOBAL_QUEST_MAP = *(TQuestHashMap*)(void *)0x6CE4D8;

const void* CLASS_KILL_N_MONSTERS = (void*)0x0060F9C0;
const void* CLASS_KILL_MONSTER = (void*)0x0060F9F8;
const void* CLASS_KILL_GROUP = (void*)0x60F988;

bool player_has_quest_for_n_monsters(T_PLAYER* player, _DWORD quest_monster_type){
	QuestIterator iter = QuestIterator();
	TQuestNode* node;
	while((node = iter.next()) != NULL){
		TQuest* quest = node->quest;
		_DWORD existing_quest_monster_type = quest->obj;
		if( quest->clazz == CLASS_KILL_N_MONSTERS && existing_quest_monster_type == quest_monster_type && player->id_ext.id == quest->player_id.id){
			return true;
		}
	}
    return false;
}
bool player_has_quest_for_monster_id(T_PLAYER* player, __int16 monsterId){
	QuestIterator iter = QuestIterator();
	TQuestNode* node;
	while((node = iter.next()) != NULL){
		TQuest* quest = node->quest;
		_WORD id = (*(T_ID*)&quest->obj).id;
		if( quest->clazz == CLASS_KILL_MONSTER && id == monsterId && player->id_ext.id == quest->player_id.id){
			return true;
		}
	}
    return false;
}

bool player_has_quest_for_monster_group(T_PLAYER* player, _DWORD groupId){
	QuestIterator iter = QuestIterator();
	TQuestNode* node;
	while((node = iter.next()) != NULL){
		TQuest* quest = node->quest;
		_DWORD id = quest->obj;
		if( quest->clazz == CLASS_KILL_GROUP && id == groupId && player->id_ext.id == quest->player_id.id){
			return true;
		}
	}
    return false;
}

void __stdcall filter_out_existing_n_monters_quests(T_PLAYER* player, _DWORD quest_monster_type, unsigned int* matching_monsters_counter_ptr){
	if(Config::AllowOnlyOneQuest_KillNMonsters && player_has_quest_for_n_monsters(player, quest_monster_type)){
		return;
	} else {
		// There is no such a monster in the player's quests, we can add it into the possible quests
		*matching_monsters_counter_ptr = *matching_monsters_counter_ptr + 1;
	}
}

void __stdcall filter_out_existing_monster_quests(T_PLAYER* player, T_UNIT* quest_monster_selected, unsigned int* matching_monsters_counter_ptr){
	if(Config::AllowOnlyOneQuest_KillTheMonster && player_has_quest_for_monster_id(player, quest_monster_selected->id_ext.id)){
		return;
	} else {
		// There is no such a monster in the player's quests, we can add it into the possible quests
		*matching_monsters_counter_ptr = *matching_monsters_counter_ptr + 1;
	}
}

void __stdcall filter_out_existing_group_quests(T_PLAYER* player, T_UNIT* quest_monster_selected, unsigned int* matching_monsters_counter_ptr){
	if(Config::AllowOnlyOneQuest_KillTheGroup && player_has_quest_for_monster_id(player, quest_monster_selected->id_ext.id)){
		return;
	} else {
		// There is no such a monster in the player's quests, we can add it into the possible quests
		*matching_monsters_counter_ptr = *matching_monsters_counter_ptr + 1;
	}
}

#define LOCAL_VAR_QUEST_MONSTER_SELECTED EBP - 0X434
#define LOCAL_VAR_MATCHING_MONSTERS_COUNTER EBP - 0X43C
#define LOCAL_VAR_PLAYER EBP + 0X8
int __declspec(naked) remove_existing_n_monters_quests_wrapper(){
    //0056289E
    __asm{
		lea eax, [LOCAL_VAR_MATCHING_MONSTERS_COUNTER]
		push eax
		push edx
		mov eax, [LOCAL_VAR_PLAYER]
		push eax
		call filter_out_existing_n_monters_quests
		retn
    }
}

int __declspec(naked) remove_existing_monster_quests_wrapper(){
    //00562E10
    __asm{
		lea eax, [LOCAL_VAR_MATCHING_MONSTERS_COUNTER]
		push eax
		mov eax, [LOCAL_VAR_QUEST_MONSTER_SELECTED]
		push eax
		mov eax, [LOCAL_VAR_PLAYER]
		push eax
		call filter_out_existing_monster_quests
		retn
    }
}

int __declspec(naked) remove_existing_group_quests_wrapper(){
    //00563355
    __asm{
		lea eax, [LOCAL_VAR_MATCHING_MONSTERS_COUNTER]
		push eax
		mov eax, [LOCAL_VAR_QUEST_MONSTER_SELECTED]
		push eax
		mov eax, [LOCAL_VAR_PLAYER]
		push eax
		call filter_out_existing_monster_quests
		retn
    }
}

#undef LOCAL_VAR_QUEST_MONSTER_SELECTED
#undef LOCAL_VAR_MATCHING_MONSTERS_COUNTER
#undef LOCAL_VAR_PLAYER