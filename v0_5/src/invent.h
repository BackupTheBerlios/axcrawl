/*
 *  File:       invent.cc
 *  Summary:    Functions for inventory related commands.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *               <1>     -/--/--        LRH             Created
 */


#ifndef INVENT_H
#define INVENT_H

#define PROMPT_ABORT        -1
#define PROMPT_GOT_SPECIAL  -2

int prompt_invent_item( const char *prompt, int type_expect,
                        bool must_exist = true, 
                        const char other_valid_char = '\0',
                        int *const count = NULL );

// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: invent - ouch - shopping
 * *********************************************************************** */
unsigned char invent(int item_class_inv, bool show_price);


// last updated 24may2000 {dlb}
/* ***********************************************************************
 * called from: acr - command - food - item_use - items - spl-book - spells1
 * *********************************************************************** */
unsigned char get_invent(int invent_type);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void list_commands(bool wizard);

//Show a inventory of objects of "object_class"
//in the message area. 
//For 'z','W','w','P','R','T' commands
//Alex: 02.10.01
void mini_invent(int object_class);

//From remove_item
//Show a list of the objects worn (including rings and amulets)
//in the message area just after the prompt "Wield wich weapon...", "Wear which..."
//
//This function is to allow unify commands 'T' and 'R'
//Alex:03.10.01
int mini_remove_invent(char *prompt=NULL);
#endif
