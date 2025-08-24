#ifndef PVM2_HPP_INCLUDED
#define PVM2_HPP_INCLUDED

#include "a2types.h"

extern uint32_t* vd2_sp1;
extern uint32_t* vd2_sp2;
extern uint32_t* vd2_sp3;

uint32_t vd2_QuerySpells(A2Unit* character, uint32_t level);
uint32_t vd2_QueryScrolls(A2Unit* character, uint32_t level, bool elven);
bool vd2_CheckItemLevel(A2InventoryItem* item, uint32_t level);
uint32_t vd2_QueryItems(A2Unit* character, uint32_t level);
bool vd2_CheckStrong(A2Unit* character);
uint32_t _stdcall VerifyDamage2(A2Unit* p1, A2Unit* p2);

#endif // PVM2_HPP_INCLUDED
