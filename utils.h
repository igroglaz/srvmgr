#pragma once

#include <cstdint>

#pragma pack(1)
#define _DWORD __int32
#define _WORD __int16
#define _BYTE char

const void* const A2_HUMAN_CLASS = reinterpret_cast<void*>(0x0060F0C8);

struct T_ID
{
  _WORD id;
  _WORD type;
};

template <typename T>
struct __declspec(align(4)) T_SRV_LINKED_NODE
{
  T_SRV_LINKED_NODE *next;
  T_SRV_LINKED_NODE *prev;
  T* value;
};

template <typename T>
struct __declspec(align(4)) T_LINKEDLIST
{
  _BYTE gap0[4];
  T_SRV_LINKED_NODE<T> *first_node;
  T_SRV_LINKED_NODE<T> *last_node;
  __int32 size;
  _DWORD dword10;
  struct CPlex *pcplex14;
  _DWORD dword18;
};

template <typename T>
struct A2ListWrapper {
  void* rtti_pointer;
  T_LINKEDLIST<T> list;
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

struct T_INVENTORY_ITEM
{
  _DWORD dword0;
  _BYTE gap4[8];
  _WORD wordC;
  _BYTE gapE[14];
  uint32_t price;
  T_LINKEDLIST<A2Effect> effects;
  A2WorldEquip* world_equip;
  unsigned __int16 id;
  _WORD amount;
  _BYTE byte44;
  _BYTE byte45;
  _BYTE byte46;
  _WORD word48;
  _WORD word4A;
  _BYTE byte4C;
  _BYTE byte4D;
  _BYTE gap4E[6];
  _DWORD dword54;
};

struct T_INVENTORY_LIST
{
  T_LINKEDLIST<T_INVENTORY_ITEM> list;
  int maxInd;
  _DWORD dword20;
};

struct A2Position {
  uint8_t x;
  uint8_t y;
  uint16_t yx;
  uint32_t f4;
  void* instance;
};

struct T_UNIT;
struct T_PLAYER;
struct A2Group {
  T_LINKEDLIST<T_UNIT> units;
  int group_id;
  T_LINKEDLIST<void*> some_list;
  void* some_ptr;
  void* some_addr;
  T_PLAYER* player;
  void* some_addr2;
  void* some_addr3;
};

struct  T_PLAYER
{
    _BYTE gap0[4];
    T_ID id_ext;
    _BYTE gap1[12];
    uint32_t flags; // 0x14
    const char* name;
    _BYTE gap2[8];
    A2ListWrapper<T_UNIT>* unit_list;
    T_LINKEDLIST<A2Group>* groups;
    int unitType;
    _BYTE gap3[2];
    _WORD word32;
    _BYTE gap4[4];
    T_UNIT* current_unit; // 0x38
    int32_t money;
    _BYTE gap5[2];
    _BYTE byte42;
    _BYTE byte43;
    uint8_t monster_kills_by_server_id[2560];
    uint8_t gapA44[4];
    uint32_t monster_kills;
    uint32_t player_kills;
    _BYTE gap7[4];
    uint32_t deaths;
    _DWORD dwordA58;
    _BYTE gap8[4];
    _DWORD dwordA60;
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

struct __declspec(align(4)) T_UNIT
{
  void *clazz;
  T_ID id_ext;
  _BYTE gap0[6];
  _BYTE type_id;
  _BYTE byteF;
  A2Position* position;
  T_PLAYER *player;
  _BYTE gap1[12];
  int *pint24;
  _DWORD dword28;
  _DWORD dword2C;
  _DWORD dword30;
  CPlex *pcplex34;
  _BYTE gap2[4];
  void* monster_info; // This is `MonsterInfo`, defined in `inn.cpp`. TODO: gather all types in one file.
  T_UNIT* last_hit_by;
  _BYTE gap4[6];
  _BYTE byte4A;
  _BYTE face;
  _BYTE unit_attrs;
  _BYTE gap5[3];
  _DWORD state;
  _DWORD unit_state;
  void *pcobject58;
  void *pcobject5C;
  _BYTE byte60;
  _BYTE byte61;
  _BYTE gap6[2];
  void *piunknownvtbl64;
  void *piunknownvtbl68;
  _BYTE byte6C;
  _BYTE gap7[3];
  _DWORD dword70;
  T_INVENTORY_ITEM *weapon;
  T_INVENTORY_ITEM *shield;
  T_INVENTORY_LIST *inventory;
  const char* name;
  uint16_t body;
  uint16_t reaction;
  uint16_t mind;
  uint16_t spirit;
  uint16_t speed;
  _BYTE gap8e[6];
  _WORD hp;
  _WORD hp_max;
  _WORD hp_regen;
  _WORD mp;
  _WORD mp_max;
  _WORD mp_regen;
  _BYTE gapA0[5];
  uint8_t scan_range;
  uint16_t attack;
  _BYTE gapA6[12];
  uint8_t hand_damage_min;
  uint8_t hand_damage_spread;
  uint8_t physical_damage_type;
  _BYTE gapBA[7];
  uint16_t defence;
  uint16_t absorption;
  uint16_t protection_magic[6];
  uint8_t protection_physical[6];
  _BYTE gap66[68];
  uint16_t skills[5];
  _BYTE gap12A[14];
  _DWORD exp;
  _BYTE gap77[12];
  void* spellbook;
  _BYTE gap8[8];
  uint16_t server_id;
  _BYTE gap9[82];
  uint32_t summoned;
  _BYTE gap1A4[28];
  A2UnitEye* eye;
  A2UnitEye2* eye2;
  _BYTE gap1c8[64];
};

struct A2Human {
  T_UNIT unit;
  T_INVENTORY_ITEM* dress[13];
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
  T_LINKEDLIST<T_INVENTORY_ITEM>* items;
};

struct A2ServerStruct {
  T_LINKEDLIST<void>* buildings;
  A2ListWrapper<void>* some_things;
  T_LINKEDLIST<A2Bag>* sacks;
  A2ListWrapper<void>* units;
  T_LINKEDLIST<void> virtual_casters;
  T_LINKEDLIST<T_UNIT> some_units;
};

struct A2Server {
  uint8_t gap[0x7c];
  A2ServerStruct* server_struct;
  // ...
};

#pragma pack()

bool IsWarrior(T_UNIT* unit);
bool IsFemale(T_UNIT* unit);
