/*
 *  File:       item_use.cc
 *  Summary:    Functions for making use of inventory items.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *              <2>             5/26/99         JDJ             Exposed armour_prompt. takeoff_armour takes an index argument.
 *              <1>             -/--/--         LRH             Created
 */


#ifndef ITEM_USE_H
#define ITEM_USE_H


#include <string>


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr - item_use
 * *********************************************************************** */
bool armour_prompt(const string & mesg, int *index);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr - item_use - items
 * *********************************************************************** */
 
//Added param quiet
//Alex:01.03.02
bool takeoff_armour(int index,bool quiet=false);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void drink(void);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void original_name(void);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void puton_ring(void);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
//Read a scroll or book.
//If bBook==0, read a scrool, else a book
//Before, read_scroll() hadn't any parameter.
//Alex: 03.10.01
void read_scroll(int bBook=0);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
//If index==-1, prompt the jewellery to remove
//Else remove the item slot "index"
//alex:12.11.01
void remove_ring(int index=-1);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void shoot_thing(void);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void throw_anything(void);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void wear_armour( void );

// last updated 10Sept2001 {bwr}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
bool do_wear_armour( int item, bool quiet,bool bDelay=true );


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
bool wield_weapon(bool auto_wield,int wpn_slot=-1);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void zap_wand(void);


// last updated 15jan2001 {gdl}
/* ***********************************************************************
 * called from: item_use food
 * *********************************************************************** */
void wield_effects(int item_wield_2, bool showMsgs);

// last updated 10sept2001 {bwr}
/* ***********************************************************************
 * called from: delay.cc item_use.cc it_use2.cc
 * *********************************************************************** */
void use_randart( unsigned char item_wield_2 );

/*
   remove_item()
   
   Removes a piece of armour or a ring or a amulet
   that is weared
   
   Alex: 03.10.01
*/   
void remove_item();

//Un-stacks a item when wielded, worn or putting on
//Called from item_use.cc
//Alex: 16.1.01
void unStack(int item,bool quiet=false);

//Stacks a item when unwielded or removed.
//Called from item_use.cc, it_use2.cc
//Alex: 16.1.01
void Stack(int item,bool quiet=false);

#endif
