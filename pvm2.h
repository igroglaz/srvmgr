#ifndef PVM2_HPP_INCLUDED
#define PVM2_HPP_INCLUDED

extern uint32_t* vd2_sp1;
extern uint32_t* vd2_sp2;
extern uint32_t* vd2_sp3;

uint32_t vd2_QuerySpells(byte* character, uint32_t level);
uint32_t vd2_QueryScrolls(byte* character, uint32_t level, bool elven);
bool vd2_CheckItemLevel(byte* item, uint32_t level);
uint32_t vd2_QueryItems(byte* character, uint32_t level);
bool vd2_CheckStrong(byte* character);
uint32_t _stdcall VerifyDamage2(byte* p1, byte* p2);

#endif // PVM2_HPP_INCLUDED
