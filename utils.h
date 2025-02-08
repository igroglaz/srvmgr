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
struct  T_PLAYER
{
    _BYTE gap0[4];
    T_ID id_ext;
    _BYTE gap1[16];
    const char* name;
    _BYTE gap2[8];
    _DWORD dword24;
    _DWORD dword28;
    int unitType;
    _BYTE gap3[2];
    _WORD word32;
    _BYTE gap4[4];
    T_UNIT* current_unit;
    int32_t money;
    _BYTE gap5[2];
    _BYTE byte42;
    _BYTE gap6[2565];
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
  uint8_t gap[10];
  uint16_t command_to;
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
  _DWORD dword3C;
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
  _WORD word94;
  _WORD word96;
  _BYTE gap55[30];
  _BYTE byteB6;
  _BYTE gap66[97];
  uint16_t skills[5];
  _BYTE gap12A[14];
  _DWORD exp;
  _BYTE gap77[12];
  void* spellbook;
  _BYTE gap8[8];
  _WORD word14C;
  _BYTE gap9[82];
  _DWORD dword1A0;
  _BYTE gap1A4[32];
  A2UnitEye2* eye2;
  _BYTE gap1c8[64];
};

struct A2Human {
  T_UNIT unit;
  T_INVENTORY_ITEM* dress[13];
  int32_t main_sphere;
  int32_t experience_per_sphere[5];
};

template <typename T>
struct A2ListWrapper {
  void* rtti_pointer;
  T_LINKEDLIST<T> list;
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
