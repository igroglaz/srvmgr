#include "a2types.h"
#include "srvmgrdef.h"
#include "syslib.h"
#include "lib\utils.hpp"

uint32_t vd2_sp1[] = {1, 2, 4, 5, 8, 10, 13, 16, 19, 20, 21, 24, 26, 27}; // level "easy"
uint32_t vd2_sp2[] = {3, 6, 7, 11, 12, 17, 23, 25};                       // level "hard"
uint32_t vd2_sp3[] = {18, 22};                                            // level "horror"

// returns count of spells in book that belong to specified level.
uint32_t vd2_QuerySpells(A2Unit* character, uint32_t level)
{
    if(!character) return 0;
    if(level > 3 || !level) return 0;

    A2SpellBook* spellbook = character->spellbook;
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

    uint32_t spbsize = spellbook->spells.size;

    for(uint32_t i = 0; i < checksize; i++)
    {
        uint32_t spell = checkwith[i];
        if(spell >= 32) continue;
        if(spellbook->spells.size <= static_cast<int32_t>(spell)) continue;
        A2Spell* spel2 = spellbook->spells.data[spell];
        if(spel2) retval++;
    }

    return retval;
}

// returns count of scrolls that belong to specified level.
uint32_t vd2_QueryScrolls(A2Unit* character, uint32_t level, bool elven)
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

    A2InventoryList* pack = character->inventory;
    if(!pack) return 0;
    int itemcnt = 0;
    A2Node<A2InventoryItem>* lp = pack->list.first_node;
    while(lp)
    {
        A2InventoryItem* item = lp->value;
        if(item)
        {
            uint8_t item_class = item->shape;
            uint8_t item_material = item->material;
            uint16_t item_option = item->option;
            uint8_t item_slot = reinterpret_cast<A2Armor*>(item)->slot;

            if(item_slot == 0 && item_class == 0 && item_material == 0 && item->weight == 1)
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

            uint32_t item_count = item->amount;

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
        lp = lp->next;
    }

    return retval;
}

bool vd2_CheckItemLevel(A2InventoryItem* item, uint32_t level)
{
    uint8_t item_class = item->shape;
    uint8_t item_material = item->material;
    uint16_t item_option = item->option;
    uint8_t item_slot = reinterpret_cast<A2Armor*>(item)->slot;

    uint32_t item_level = 0;

    // special case: (null)
    if(item_class == 0 && item_material == 0 && item_option == 0 && item_slot == 0) return 0;

    if(item_slot == 0 && item_class == 0 && item_material == 0 && item->weight == 1)
    {
        // that is, slot = 14
        item_slot = 14;
    }

    if(item_slot == 14) return false; // don't check scrolls here

    uint32_t item_count = item->amount;

    if(item_material == 7 || item_material == 12) item_level = 2; // hard, meteoric/dragonleather
    else if(item_material == 14) item_level = 3; // horror, crystal
    else item_level = 1; // allowed, easy

    //if(item_class == 3 && item_level < 2) item_level = 2; // very rare

    A2Node<A2Effect>* parms = item->effects.first_node;
    while(parms)
    {
        A2Effect* parm = parms->value;
        if(parm)
        {
            uint8_t prm1 = parm->effect_id;
            uint8_t val1 = parm->value1;
            uint8_t val2 = parm->value2;

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
        parms = parms->next;
    }

    if(!item_level) return false;
    return (item_level == level);
}

// returns count of items (both on body and in pack) that belong to specified level.
uint32_t vd2_QueryItems(A2Unit* character, uint32_t level)
{
    if(!character) return 0;
    if(level > 3 || !level) return 0;

    uint32_t retval = 0;

    A2InventoryList* pack = character->inventory;
    if(!pack) return 0;
    int itemcnt = 0;

    // in bag
    A2Node<A2InventoryItem>* lp = pack->list.first_node;
    while(lp)
    {
        A2InventoryItem* item = lp->value;
        if(item && vd2_CheckItemLevel(item, level)) retval += item->amount;
        lp = lp->next;
    }

    // on body
    for(uint32_t i = 1; i <= 12; i++)
    {
        A2InventoryItem* item = NULL;
        if(i == 1) item = character->weapon;
        else if(i == 2) item = character->shield;
        else item = reinterpret_cast<A2Human*>(character)->dress[i];
        if(item && vd2_CheckItemLevel(item, level)) retval++; // count = always 1
    }

    return retval;
}

bool vd2_CheckStrong(A2Unit* p1)
{
    uint16_t p1_body        = p1->body;
    uint16_t p1_reaction    = p1->reaction;
    uint16_t p1_mind        = p1->mind;
    uint16_t p1_spirit      = p1->spirit;

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
uint32_t _stdcall VerifyDamage2(A2Unit* p1, A2Unit* p2)
{
    if(!p1 || !p2) return 0;
    A2Player* player1 = p1->player;
    A2Player* player2 = p2->player;
    if(!player1 || !player2) return 0;
    uint32_t flags1 = player1->unitType;
    uint32_t flags2 = player2->unitType;
    if(flags1 || flags2) return 0;
    A2Unit* main1 = player1->current_unit;
    A2Unit* main2 = player2->current_unit;
    if(main1 != p1) return 0;
    if(main2 != p2) return 0;

    bool ststrong_1 = vd2_CheckStrong(p1);
    bool ststrong_2 = vd2_CheckStrong(p2);

    if(ststrong_1 && !ststrong_2) return 1; // do not allow strong players attack weak players
    if(!ststrong_1 && ststrong_1) return 1; // do not allow weak players attack strong players
    else return 0;
}
