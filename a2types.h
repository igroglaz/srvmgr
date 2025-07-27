#pragma once

#include <cstdint>

const void* const A2_CLASS_HUMAN = (void*)0x0060F0C8;
const void* const A2_CLASS_KILL_N_MONSTERS = (void*)0x0060F9C0;
const void* const A2_CLASS_KILL_MONSTER = (void*)0x0060F9F8;
const void* const A2_CLASS_KILL_GROUP = (void*)0x60F988;

struct A2ID {
    int16_t id;
    int16_t type;
};

template <typename T>
struct A2Node {
    A2Node *next;
    A2Node *prev;
    T* value;
};

template <typename T>
struct A2LinkedList {
    int8_t gap0[4];
    A2Node<T> *first_node;
    A2Node<T> *last_node;
    int32_t size;
    int32_t dword10;
    struct CPlex *pcplex14;
    int32_t dword18;
};

template <typename T>
struct A2ListWrapper {
    void* rtti_pointer;
    A2LinkedList<T> list;
};

template <typename T>
struct A2Array {
    int whatever0;
    T *data;
    int size;
    int whatever1[2];
};

struct A2MonsterInfoData {
    int body;
    int reaction;
    int mind;
    int spirit;
    int healthMax;
    int healthRegeneration;
    int manaMax;
    int manaRegeneration;
    int speed;
    int rotationSpeed;
    int scanRange;
    int physicalMin;
    int physicalMax;
    int attackType;
    int toHit;
    int defence;
    int absorbtion;
    int charge;
    int relax;
    int protectionFire;
    int protectionWater;
    int protectionAir;
    int protectionEarth;
    int protectionAstral;
    int protectionBlade;
    int protectionAxe;
    int protectionBludgeon;
    int protectionPike;
    int protectionShooting;
    int typeId;
    int face;
    int tokenSize;
    int movementType;
    int dyingTime;
    int withdraw;
    int wimpy;
    int seeInvisible;
    int experience;
    int treasureGold;
    int treasureGoldMin;
    int treasureGoldMax;
    int treasureItem;
    int treasureItemMin;
    int treasureItemMax;
    int treasureItemMask;
    int notUsed;
    int notUsed1;
    int power;
    int spell1ID;
    int spellProbability1;
    int spell2ID;
    int spellProbability2;
    int spell3ID;
    int spellProbability3;
    int spellPower;
    int serverId;
    int knownSpellsMask;
    int skillFire;
    int skillWater;
    int skillAir;
    int skillEarth;
    int skillAstral;
};

struct A2MonsterInfo {
    int whatever0;
    const char* name;
    A2Array<A2MonsterInfoData> monsterData;
    A2Array<char*> equipment;
};

struct A2Effect {
    uint8_t gap0[0x3c];
    uint8_t effect_id; // Effect ID, same as in the editor.
    uint8_t usage_type;
    uint8_t gap3E[2];
    uint8_t value1; // The main effect value.
    uint8_t value2; // Optional second value. Used for damage, like "astral damage 3--5".
};

struct A2WorldEquip {
    void* clazz;
    const char* name;
    // ...
};

struct A2InventoryItem {
    int32_t dword0;
    int8_t gap4[8];
    int16_t wordC;
    int8_t gapE[14];
    uint32_t price;
    A2LinkedList<A2Effect> effects;
    A2WorldEquip* world_equip;
    uint16_t id;
    int16_t amount;
    int8_t byte44;
    int8_t byte45;
    int8_t byte46;
    int8_t byte47;
    int16_t word48;
    int16_t word4A;
    int8_t byte4C;
    int8_t byte4D;
    int8_t gap4E[6];
    int32_t dword54;
};

struct A2InventoryList {
    A2LinkedList<A2InventoryItem> list;
    int32_t maxInd;
    int32_t dword20;
};

struct A2Position {
    uint8_t x;
    uint8_t y;
    uint16_t yx;
    uint32_t f4;
    void* instance;
};

struct A2Unit;
struct A2Player;
struct A2Group {
    A2LinkedList<A2Unit> units;
    int32_t group_id;
    A2LinkedList<void*> some_list;
    void* some_ptr;
    void* some_addr;
    A2Player* player;
    void* some_addr2;
    void* some_addr3;
};

struct A2Player {
    int8_t gap0[4];
    A2ID id_ext;
    int8_t gap1[12];
    uint32_t flags; // 0x14
    const char* name;
    int8_t gap2[8];
    A2ListWrapper<A2Unit>* unit_list;
    A2LinkedList<A2Group>* groups;
    int32_t unitType;
    int8_t gap3[2];
    int16_t word32;
    int8_t gap4[4];
    A2Unit* current_unit; // 0x38
    int32_t money;
    int8_t gap5[2];
    int8_t byte42;
    int8_t byte43;
    uint8_t monster_kills_by_server_id[2560];
    uint8_t gapA44[4];
    uint32_t monster_kills;
    uint32_t player_kills;
    int8_t gap7[4];
    uint32_t deaths;
    int32_t dwordA58;
    int8_t gap8[4];
    int32_t dwordA60;
    char gapA62[20];
    const char* account_name;
};

struct A2UnitEye2 {
    uint8_t gap0[10];
    uint16_t command_to;
    uint8_t gapB[56];
    uint32_t wimpy;
    uint8_t gap48[41];
    uint8_t see_invisible;
};

struct A2UnitEye {
    uint8_t gap0[10];
    uint8_t rotation_speed;
    uint8_t gapB[169];
};

struct A2Unit {
    void *clazz;
    A2ID id_ext;
    int8_t gap0[6];
    int8_t type_id;
    int8_t byteF;
    A2Position* position;
    A2Player *player;
    int8_t gap1[12];
    int* pint24;
    int32_t dword28;
    int32_t dword2C;
    int32_t dword30;
    CPlex *pcplex34;
    int8_t gap2[4];
    A2MonsterInfo* monster_info;
    A2Unit* last_hit_by;
    int8_t gap4[6];
    int8_t byte4A;
    int8_t face;
    int8_t unit_attrs;
    int8_t gap5[3];
    int32_t state;
    int32_t unit_state;
    void *pcobject58;
    void *pcobject5C;
    int8_t byte60;
    int8_t byte61;
    int8_t gap6[2];
    void *piunknownvtbl64;
    void *piunknownvtbl68;
    int8_t byte6C;
    int8_t gap7[3];
    int32_t dword70;
    A2InventoryItem *weapon;
    A2InventoryItem *shield;
    A2InventoryList *inventory;
    const char* name;
    uint16_t body;
    uint16_t reaction;
    uint16_t mind;
    uint16_t spirit;
    uint16_t speed;
    int8_t gap8e[6];
    int16_t hp;
    int16_t hp_max;
    int16_t hp_regen;
    int16_t mp;
    int16_t mp_max;
    int16_t mp_regen;
    int8_t gapA0[5];
    uint8_t scan_range;
    uint16_t attack;
    int8_t gapA6[12];
    uint8_t hand_damage_min;
    uint8_t hand_damage_spread;
    uint8_t physical_damage_type;
    int8_t gapBA[7];
    uint16_t defence;
    uint16_t absorption;
    uint16_t protection_magic[6];
    uint8_t protection_physical[6];
    int8_t gap66[68];
    uint16_t skills[5];
    int8_t gap12A[14];
    int32_t exp;
    int8_t gap77[12];
    void* spellbook;
    int8_t gap8[8];
    uint16_t server_id;
    int8_t gap9[82];
    uint32_t summoned;
    int8_t gap1A4[28];
    A2UnitEye* eye;
    A2UnitEye2* eye2;
    int8_t gap1c8[64];
};

struct A2Human {
    A2Unit unit;
    A2InventoryItem* dress[13];
    int32_t main_sphere;
    int32_t experience_per_sphere[5];
};

struct A2Bag {
    void* vtbl;
    int32_t f4;
    int32_t token_id;
    int16_t item_data_id;
    int16_t type_id;
    A2Position* position;
    void* owner;
    int8_t gap[36];
    int32_t money;
    A2LinkedList<A2InventoryItem>* items;
};

struct A2ServerStruct {
    A2LinkedList<void>* buildings;
    A2ListWrapper<void>* some_things;
    A2LinkedList<A2Bag>* sacks;
    A2ListWrapper<void>* units;
    A2LinkedList<void> virtual_casters;
    A2LinkedList<A2Unit> some_units;
};

struct A2Server {
    uint8_t gap[0x7c];
    A2ServerStruct* server_struct;
    // ...
};

struct A2Quest {
    void *clazz;
    int32_t inn_id;
    A2ID player_id;
    int32_t gap0C;
    int32_t status;
    int32_t obj;
    int32_t landmark_id;
    int32_t target;
    int32_t current;
    int32_t dword24;
    int32_t dword28;
    int32_t dword2C;
};

struct A2QuestNode {
    A2QuestNode *next_node;
    int32_t dword4;
    int32_t dword8;
    A2Quest *quest;
};

struct A2QuestHashMap {
    int8_t gap0[8];
    A2QuestNode** bucket_arr;
    int buckets_size;
    int32_t size;
    int8_t gap14[96];
    A2QuestNode *current_node;
    A2Quest *quest;
    int8_t gap7C[4];
    int32_t dword80;
};

struct A2GameDataRes {
    char whatever[0x8c];
    A2Array<A2MonsterInfo> monsters;
};


bool IsWarrior(const A2Unit* unit);
bool IsFemale(const A2Unit* unit);
