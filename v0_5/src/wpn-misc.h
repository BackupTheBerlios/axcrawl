/*
 *********************************************************************
 *  File:       wpn-misc.h                                           *
 *  Summary:    temporary home for weapon f(x) until struct'ed       *
 *  Written by: don brodale <dbrodale@bigfootinteractive.com>        *
 *                                                                   *
 *  Changelog(most recent first):                                    *
 *                                                                   *
 *  <00>     12jun2000     dlb     created after little thought      *
 *********************************************************************
*/


#ifndef WPNMISC_H
#define WPNMISC_H

#include "externs.h"


/* ***********************************************************************
 * called from: food.h 
 * *********************************************************************** */
bool can_cut_meat(unsigned char wclass, unsigned char wtype);

/* ***********************************************************************
 * called from: acr - fight - food - item_use - itemname - spells2
 * *********************************************************************** */
char damage_type(unsigned char wclass, unsigned char wtype);


// last updated: 10jun2000 {dlb}
/* ***********************************************************************
 * called from: describe - fight - item_use
 * *********************************************************************** */
int hands_reqd_for_weapon(unsigned char wclass, unsigned char wtype);


// last updated: 10jun2000 {dlb}
/* ***********************************************************************
 * called from: dungeon - fight - item_use - randart
 * *********************************************************************** */
bool is_demonic(unsigned char weapon_subtype);


// last updated: 10jun2000 {dlb}
/* ***********************************************************************
 * called from: dungeon - item_use - mstuff2
 * *********************************************************************** */
unsigned char launched_by(unsigned char weapon_subtype);


// last updated: 10jun2000 {dlb}
/* ***********************************************************************
 * called from: describe - dungeon - fight - item_use - mstuff2 - randart -
 *              spells2 - spells3
 * *********************************************************************** */
bool launches_things( unsigned char weapon_subtype );


// last updated: 10jun2000 {dlb}
/* ***********************************************************************
 * called from: describe - fight - files - it_use3 - newgame - spells1
 * *********************************************************************** */
char weapon_skill(unsigned char wclass, unsigned char wtype);

// last updated: 03.12.01 {Alex}
//Shows a detailed description of a weapon
/* ***********************************************************************
 * called from: acr.cc
 * *********************************************************************** */
void cmd_desc_wep();

//if bOnlyTotal=1, then returns only the number that represents
//the damage of the hit
//Detailed info of the accuracity
string desc_hit(const item_def &item,int bOnlyTotal=0);
//Detailed info of the damage of a weapon
string desc_damage( const item_def &item,int bOnlyTotal=0);
//Detailed info of the accuracity when unarmed
string desc_unarmed(int bOnlyTotal=0);
//Shows info about damage when unarmed
string desc_dam_unarmed(int bOnlyTotal=0);
//Detailed info of the accuracity of the second attack
string desc_hit_2_attack(int bOnlyTotal=0);
//Shows info about damage of seccond attack
static string desc_dam_2_attack(int bOnlyTotal=0);

void spoiler_gen_prop_weap(); 
void spoiler_gen_prop_armour();
 void spoiler_gen_prop_missil();
void spoilers_gen_unrands();
void spoiler_gen_prop_potions();
void spoiler_gen_prop_wands();
void spoiler_gen_prop_jew();
  void spoiler_gen_prop_its();
#endif
