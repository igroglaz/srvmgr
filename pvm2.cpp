#include "srvmgrdef.h"
#include "syslib.h"
#include "lib\utils.hpp"

uint32_t vd2_sp1[] = {1, 2, 4, 5, 8, 10, 13, 16, 19, 20, 21, 24, 26, 27}; // level "easy"
uint32_t vd2_sp2[] = {3, 6, 7, 11, 12, 17, 23, 25};                       // level "hard"
uint32_t vd2_sp3[] = {18, 22};                                            // level "horror"

// returns count of spells in book that belong to specified level.
uint32_t vd2_QuerySpells(byte* character, uint32_t level)
{
    if(!character) return 0;
    if(level > 3 || !level) return 0;

    byte* spellbook = *(byte**)(character + 0x140);
    if(!spellbook) return 0;
    uint32_t* checkwith = NULL;
    uint32_t checksize = 0;

    if(level == 1)
    {
        checkwith = vd2_sp1;
        checksize = sizeof(vd2_sp1) / sizeof(uint32_t);
    }
    else if(level == 2)
    {
        checkwith = vd2_sp2;
        checksize = sizeof(vd2_sp2) / sizeof(uint32_t);
    }
    else if(level == 3)
    {
        checkwith = vd2_sp3;
        checksize = sizeof(vd2_sp3) / sizeof(uint32_t);
    }

    if(!checkwith || !checksize) return 0;

    uint32_t retval = 0;

    uint32_t spbsize = *(uint32_t*)(spellbook + 0x0C);

    for(uint32_t i = 0; i < checksize; i++)
    {
        uint32_t spell = checkwith[i];
        if(spell >= 32) continue;
        if(*(uint32_t*)(spellbook + 0x0C) <= spell) continue;
        byte* spel2 = *(byte**)(*(uint32_t*)(spellbook + 8) + 4 * spell);
        if(spel2) retval++;
    }

    return retval;
}

// returns count of scrolls that belong to specified level.
uint32_t vd2_QueryScrolls(byte* character, uint32_t level, bool elven)
{
    if(!character) return 0;
    if(level > 3 || !level) return 0;

    uint32_t* checkwith = NULL;
    uint32_t checksize = 0;

    if(level == 1)
    {
        checkwith = vd2_sp1;
        checksize = sizeof(vd2_sp1) / sizeof(uint32_t);
    }
    else if(level == 2)
    {
        checkwith = vd2_sp2;
        checksize = sizeof(vd2_sp2) / sizeof(uint32_t);
    }
    else if(level == 3)
    {
        checkwith = vd2_sp3;
        checksize = sizeof(vd2_sp3) / sizeof(uint32_t);
    }

    if(!checkwith || !checksize) return 0;

    uint32_t retval = 0;

    byte* pack = *(byte**)(character + 0x7C);
    if(!pack) return 0;
    int itemcnt = 0;
    byte* lp = *(byte**)(pack + 8);
    while(lp)
    {
        byte* item = *(byte**)(lp + 8);
        if(item)
        {
            uint32_t item_class = *(uint8_t*)(item + 0x45);
            uint32_t item_material = *(uint8_t*)(item + 0x46);
            uint32_t item_option = *(uint16_t*)(item + 0x0C);
            uint32_t item_slot = *(uint8_t*)(item + 0x58);

            if(item_slot == 0 && item_class == 0 && item_material == 0 && *(uint16_t*)(item + 0x4A) == 1)
            {
                // that is, slot = 14
                item_slot = 14;
            }

            uint16_t iid = item_material & 0xF;
            iid <<= 4;
            iid |= item_slot & 0xF;
            iid <<= 8;
            iid |= (item_class & 0x7) << 5;
            iid |= item_option;

            uint32_t item_count = *(uint16_t*)(item + 0x42);

            if((iid & 0xF000) == 0 && (iid & 0x0E00) == 0x0E00) // scroll
            {
                bool spell_match = false;
                uint32_t kindof = iid & 0x00FF;
                if(((kindof >= 6 && kindof <= 34) && !elven) || // scroll
                   ((kindof >= 35 && kindof <= 63) && elven)) // superscroll
                {
                    kindof -= 5;
                    if(elven) kindof -= 29;

                    for(uint32_t i = 0; i < checksize; i++)
                    {
                        if(checkwith[i] == kindof)
                        {
                            spell_match = true;
                            break;
                        }
                    }
                }

                if(spell_match && item_count > retval) retval = item_count;
            }
        }
        lp = *(byte**)(lp + 4);
    }

    return retval;
}

bool vd2_CheckItemLevel(byte* item, uint32_t level)
{
    uint32_t item_class = *(uint8_t*)(item + 0x45);
    uint32_t item_material = *(uint8_t*)(item + 0x46);
    uint32_t item_option = *(uint16_t*)(item + 0x0C);
    uint32_t item_slot = *(uint8_t*)(item + 0x58);

    uint32_t item_level = 0;

    // special case: (null)
    if(item_class == 0 && item_material == 0 && item_option == 0 && item_slot == 0) return 0;

    if(item_slot == 0 && item_class == 0 && item_material == 0 && *(uint16_t*)(item + 0x4A) == 1)
    {
        // that is, slot = 14
        item_slot = 14;
    }

    if(item_slot == 14) return false; // don't check scrolls here

    uint32_t item_count = *(uint16_t*)(item + 0x42);

    if(item_material == 7 || item_material == 12) item_level = 2; // hard, meteoric/dragonleather
    else if(item_material == 14) item_level = 3; // horror, crystal
    else item_level = 1; // allowed, easy

    //if(item_class == 3 && item_level < 2) item_level = 2; // very rare

    byte* parms = *(byte**)(item + 0x28);
    while(parms)
    {
        byte* parm = *(byte**)(parms + 8);
        if(parm)
        {
            uint32_t prm1 = *(uint8_t*)(parm + 0x3C);
            uint32_t val1 = *(uint16_t*)(parm + 0x40);
            uint32_t val2 = *(uint16_t*)(parm + 0x42);

            if(prm1 == 2) // body
            {
                if(item_level <= 2) item_level = 2;
                if(val1 >= 3) item_level = 3;
            }
            else if(prm1 >= 3 && prm1 <= 5) // reaction, mind, spirit
            {
                if(val1 >= 2 && item_level < 2) item_level = 2;
            }
            else if(prm1 == 41) // castspell
            {
                if(item_level < 3)
                {
                    for(uint32_t i = 0; i < sizeof(vd2_sp3) / sizeof(uint32_t); i++)
                        if(vd2_sp3[i] == val1)
                            item_level = 3;
                }

                if(item_level < 2)
                {
                    for(uint32_t i = 0; i < sizeof(vd2_sp2) / sizeof(uint32_t); i++)
                        if(vd2_sp2[i] == val1)
                            item_level = 2;
                }

                if(val2 > 75 && item_level < 2) item_level = 2;
                if(val2 > 100 && item_level < 3) item_level = 3;
            }
            else if(prm1 >= 33 && prm1 <= 37) // skillX (mage)
            {
                if(item_level < 3)
                {
                    if(val1 > 18) item_level = 3;
                }

                if(item_level < 2)
                {
                    if(val1 > 10) item_level = 2;
                }
            }
            else if(prm1 >= 27 && prm1 <= 31) // skillX (fighter)
            {
                if(item_level < 3)
                {
                    if(val1 > 18) item_level = 3;
                }

                if(item_level < 2)
                {
                    if(val1 > 10) item_level = 2;
                }
            }
        }
        parms = *(byte**)(parms + 4);
    }

    if(!item_level) return false;
    return (item_level == level);
}

// returns count of items (both on body and in pack) that belong to specified level.
uint32_t vd2_QueryItems(byte* character, uint32_t level)
{
    if(!character) return 0;
    if(level > 3 || !level) return 0;

    uint32_t retval = 0;

    byte* pack = *(byte**)(character + 0x7C);
    if(!pack) return 0;
    int itemcnt = 0;

    // in bag
    byte* lp = *(byte**)(pack + 8);
    while(lp)
    {
        byte* item = *(byte**)(lp + 8);
        if(item && vd2_CheckItemLevel(item, level)) retval += *(uint16_t*)(item + 0x42);
        lp = *(byte**)(lp + 4);
    }

    // on body
    for(uint32_t i = 1; i <= 12; i++)
    {
        byte* item = NULL;
        if(i == 1) item = *(byte**)(character + 0x74);
        else if(i == 2) item = *(byte**)(character + 0x78);
        else item = *(byte**)(character + 4 * i + 0x208);
        if(item && vd2_CheckItemLevel(item, level)) retval++; // count = always 1
    }

    return retval;
}

bool vd2_CheckStrong(byte* p1)
{
    uint16_t p1_body        = *(uint16_t*)(p1 + 0x84);
    uint16_t p1_reaction    = *(uint16_t*)(p1 + 0x86);
    uint16_t p1_mind        = *(uint16_t*)(p1 + 0x88);
    uint16_t p1_spirit      = *(uint16_t*)(p1 + 0x8A);

    uint32_t p1_statsum = p1_body + p1_reaction + p1_mind + p1_spirit;
    uint32_t max_statsum = 140;
    if((p1_statsum > max_statsum) && ((p1_statsum - max_statsum) > 10)) return true;

    if(vd2_QuerySpells(p1, 3)) return true;
    if(vd2_QuerySpells(p1, 2) > 2) return true;

    if(vd2_QueryScrolls(p1, 1, false) > 100) return true; // >100 scrolls of easy
    if(vd2_QueryScrolls(p1, 1, true) > 25) return true; // >25 superscrolls of easy
    if(vd2_QueryScrolls(p1, 2, false) > 50) return true; // >50 scrolls of hard
    if(vd2_QueryScrolls(p1, 2, true) > 10) return true; // >10 superscrolls of hard
    if(vd2_QueryScrolls(p1, 3, false) > 10) return true; // >10 scrolls of horror
    if(vd2_QueryScrolls(p1, 3, true) > 5) return true; // >5 superscrolls of horror

    if(vd2_QueryItems(p1, 3)) return true;
    if(vd2_QueryItems(p1, 2) > 2) return true;

    return false;
}

// p1 = кто бьёт, p2 = кого бьём
// return value: true if p1 > p2, false if p2 > p1
uint32_t _stdcall VerifyDamage2(byte* p1, byte* p2)
{
    if(!p1 || !p2) return 0;
    byte* player1 = *(byte**)(p1 + 0x14);
    byte* player2 = *(byte**)(p2 + 0x14);
    if(!player1 || !player2) return 0;
    uint32_t flags1 = *(uint32_t*)(player1 + 0x2C);
    uint32_t flags2 = *(uint32_t*)(player2 + 0x2C);
    if(flags1 || flags2) return 0;
    byte* main1 = *(byte**)(player1 + 0x38);
    byte* main2 = *(byte**)(player2 + 0x38);
    if(main1 != p1) return 0;
    if(main2 != p2) return 0;

    bool ststrong_1 = vd2_CheckStrong(p1);
    bool ststrong_2 = vd2_CheckStrong(p2);

    if(ststrong_1 && !ststrong_2) return 1; // do not allow strong players attack weak players
    if(!ststrong_1 && ststrong_1) return 1; // do not allow weak players attack strong players
    else return 0;
}
