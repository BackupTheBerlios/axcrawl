/*
 *  File:       potions.h
 *  Summary:    Functions for potions
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *               <1>     -/--/--        LRH             Created
 */


#ifndef IT_USE2_H
#define IT_USE2_H


#include "externs.h"


/* ***********************************************************************
 * called from: ability - beam - decks - item_use - misc - religion -
 *              spell - spells - spells1
 * *********************************************************************** */
bool potion_effect(char pot_eff, int pow,int bCursed=0);


#endif
