/*
 *  File:       invent.cc
 *  Summary:    Functions for inventory related commands.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <6>     14/11/01    ALX     .- command_string moved to new help system
 *                                     
 *                                  .- prompt_invent calls to mini_invent that 
 *                                     shows below the prompt a list 
 *                                     of items of the class specified when calling prompt_invent
 *                                  .- in mini_invent, asking for obj_weapons, shows also
 *                                     staves. 
 *                                  .- Asking for missiles, show also axes, daggers and spears
 *
 *      <5>     10/9/99     BCR     Added wizard help screen
 *      <4>     10/1/99     BCR     Clarified help screen
 *      <3>     6/9/99      DML     Autopickup
 *      <2>     5/20/99     BWR     Extended screen lines support
 *      <1>     -/--/--     LRH     Created
 */

#define INV_OBJECTS "*)([/%?=!+0}&$¿"

#include "AppHdr.h"
#include "invent.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "itemname.h"
#include "items.h"
#include "shopping.h"
#include "stuff.h"
#include "view.h"

#ifdef MACROS
#include "macro.h"
#endif


int letter_to_object(unsigned char keyin,int type_default,int orig_type)
  {
      switch(keyin)
                  {
                     case '*':
                        return -1;
                        break;
                     case '=':
                        return OBJ_JEWELLERY;
                        break;
                     case '!':
                        return OBJ_POTIONS;
                        break;   
                     case '+':
                        return OBJ_BOOKS;
                        break;
                     case ')':
                        return OBJ_WEAPONS;
                        break;   
                      case '}':
                        return OBJ_MISCELLANY;
                        break;  
                      case '¿':
                        return OBJ_SCROLLS;
                        break;  
                      case '%':
                        return OBJ_FOOD;
                        break;  
                      case '&':
                        return OBJ_CORPSES;
                        break;  
                      case '?':
                        return orig_type;
                        break; 
                      case '0':
                        return OBJ_ORBS;
                        break;
                      case '(':
                        return OBJ_MISSILES;
                        break;  
                      case '/':
                        return OBJ_WANDS;
                        break;
                      case '\\':
                        return OBJ_STAVES;
                        break;
                      case '[': 
                        return OBJ_ARMOUR;
                        break;                          
                      default:
                        return type_default;                          
                  }      
  }


unsigned char invent(int item_class_inv, bool show_price)
{
    char st_pass[60];

    int i, j;
    char lines = 0;
    unsigned char anything = 0;
    char strng[10] = "";
    char yps = 0;
    char temp_id[4][50];

    const int num_lines = get_number_of_lines();

    FixedVector< int, NUM_OBJECT_CLASSES >  inv_class2;
    int inv_count = 0;
    unsigned char ki = 0;

#ifdef DOS_TERM
    char buffer[4600];

    gettext(1, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 50; j++)
        {
            temp_id[i][j] = 1;
        }
    }

    strcpy(st_pass, "");
    clrscr();

    for (i = 0; i < NUM_OBJECT_CLASSES; i++)
        inv_class2[i] = 0;

    for (i = 0; i < ENDOFPACK; i++)
    {
        if (you.inv[i].quantity)
        {
            inv_class2[ you.inv[i].base_type ]++;
            inv_count++;
        }
    }

    if (!inv_count)
    {
        cprintf("You aren't carrying anything.");

        if (getch() == 0)
            getch();

        goto putty;
    }

    if (item_class_inv != -1)
    {
        for (i = 0; i < NUM_OBJECT_CLASSES; i++)
        {
            if (item_class_inv == OBJ_MISSILES && i == OBJ_WEAPONS)
                i++;

            if (item_class_inv == OBJ_WEAPONS 
                && (i == OBJ_STAVES || i == OBJ_MISCELLANY))
            {
                i++;
            }   

            if (item_class_inv == OBJ_SCROLLS && i == OBJ_BOOKS)
                i++;

            if (i < NUM_OBJECT_CLASSES && item_class_inv != i)
                inv_class2[i] = 0;
        }
    }

    if ((item_class_inv == -1 && inv_count > 0)
        || (item_class_inv != -1 && inv_class2[item_class_inv] > 0)
        || (item_class_inv == OBJ_MISSILES && inv_class2[OBJ_WEAPONS] > 0)
        || (item_class_inv == OBJ_WEAPONS 
            && (inv_class2[OBJ_STAVES] > 0 || inv_class2[OBJ_MISCELLANY] > 0))
        || (item_class_inv == OBJ_SCROLLS && inv_class2[OBJ_BOOKS] > 0))
    {
        cprintf("  Inventory");
        lines++;

        for (i = 0; i < 15; i++)
        {
            if (inv_class2[i] != 0)
            {
                if (lines > num_lines - 3)
                {
                    gotoxy(1, num_lines);
                    cprintf("-more-");

                    ki = getch();

                    if (ki == ESCAPE)
                    {
#ifdef DOS_TERM
                        puttext(1, 1, 80, 25, buffer);
#endif
                        return (ESCAPE);
                    }
                    else if (isalpha(ki) || strchr(INV_OBJECTS,ki))
                    {
#ifdef DOS_TERM
                        puttext(1, 1, 80, 25, buffer);
#endif
                        return (ki);
                    }

                    if (ki == 0)
                        ki = getch();

                    lines = 0;
                    clrscr();
                    gotoxy(1, 1);
                    anything = 0;

                }

                if (lines > 0)
                    cprintf(EOL " ");

                textcolor(BLUE);
                
                PrintNameObjectClass(i);
                

                textcolor(LIGHTGREY);
                lines++;

                for (j = 0; j < ENDOFPACK; j++)
                {
                    if (lines > num_lines - 2 && inv_count > 0)
                    {
                        gotoxy(1, num_lines);
                        cprintf("-more-");
                        ki = getch();

                        if (ki == ESCAPE)
                        {
#ifdef DOS_TERM
                            puttext(1, 1, 80, 25, buffer);
#endif
                            return (ESCAPE);
                        }
                        else if (isalpha(ki) || strchr(INV_OBJECTS,ki))
                        {
#ifdef DOS_TERM
                            puttext(1, 1, 80, 25, buffer);
#endif
                            return (ki);
                        }

                        if (ki == 0)
                            ki = getch();

                        lines = 0;
                        clrscr();
                        gotoxy(1, 1);
                        anything = 0;
                    }

                    if (is_valid_item(you.inv[j]) && you.inv[j].base_type==i)
                    {
                        anything++;

                        if (lines > 0)
                            cprintf(EOL);

                        lines++;

                        yps = wherey();

                        in_name( j, DESC_INVENTORY_EQUIP, st_pass );
                        cprintf( st_pass );

                        inv_count--;


                        if (show_price)
                        {
                            cprintf(" (");

                            //itoa( item_value( you.inv[j], temp_id, true ), strng, 10 );
                            
                            //Don't show the real value, show the value a shopkeeper will pay you
                            //alex:07.02.02
                            itoa( item_value( you.inv[j], temp_id, false,true ), strng, 10 );

                            cprintf(strng);
                            cprintf("gold)");
                        }

                        if (wherey() != yps)
                            lines++;
                    }
                }               // end of j loop
            }                   // end of if inv_class2
        }                       // end of i loop.
    }
    else
    {
        if (item_class_inv == -1)
            cprintf("You aren't carrying anything.");
        else
        {
            if (item_class_inv == OBJ_WEAPONS)
                cprintf("You aren't carrying any weapons.");
            else if (item_class_inv == OBJ_MISSILES)
                cprintf("You aren't carrying any ammunition.");
            else
                cprintf("You aren't carrying any such object.");

            anything++;
        }
    }

    if (anything > 0)
    {
        ki = getch();

        if (isalpha(ki) || strchr(INV_OBJECTS,ki))
        {
#ifdef DOS_TERM
            puttext(1, 1, 80, 25, buffer);
#endif
            return (ki);
        }

        if (ki == 0)
            ki = getch();
    }

  putty:
#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif

    return (ki);
}                               // end invent()


// Reads in digits for a count and apprends then to val, the
// return value is the character that stopped the reading.
static unsigned char get_invent_quant( unsigned char keyin, int &quant )
{
    quant = keyin - '0';

    for(;;)
    {
        keyin = get_ch();

        if (!isdigit( keyin ))
            break;

        quant *= 10;
        quant += (keyin - '0');

        if (quant > 9999999)
        {
            quant = 9999999;
            keyin = '\0';
            break;
        }
    }

    return (keyin);
}



// This function prompts the user for an item, handles the '?' and '*'
// listings, and returns the inventory slot to the caller (which if
// must_exist is true (the default) will be an assigned item, with
// a positive quantity.
//
// It returns PROMPT_ABORT       if the player hits escape.
// It returns PROMPT_GOT_SPECIAL if the player hits the "other_valid_char".
//
// Note: This function never checks if the item is appropriate.
int prompt_invent_item( const char *prompt, int type_expect,
                        bool must_exist = true, 
                        const char other_valid_char = '\0',
                        int *const count = NULL )
{
    char           keyin = '*';
    int            ret = -1;

    bool           need_redraw = false;
    bool           need_prompt = prompt!=NULL;
    bool           need_getch  = prompt!=NULL;
    
    int            orig_type=type_expect; //alex:09.12.01 

    //Show mini-invent {alex:12.11.01}
    
    if (type_expect!=-1) mini_invent(type_expect);
    for (;;)
    {
        if (need_redraw)
        {
            mesclr();
            redraw_screen();
        }

        if (need_prompt)
            mpr( prompt, MSGCH_PROMPT );        

        if (need_getch)
            {
               keyin = get_ch();
               type_expect=letter_to_object(keyin,type_expect,orig_type);
             }  

        need_redraw = false;
        need_prompt = true;
        need_getch  = true;

        // Note:  We handle any "special" character first, so that
        //        it can be used to override the others.
        if (other_valid_char != '\0' && keyin == other_valid_char)
        {
            ret = PROMPT_GOT_SPECIAL;
            break;
        }
        //else if (!isalpha(keyin))  == '?' || keyin == '*')
        else if (strchr(INV_OBJECTS,keyin))
        {   
            type_expect=letter_to_object(keyin,type_expect,orig_type);
            keyin = invent( type_expect, false);
            need_getch  = false;

            // Don't redraw if we're just going to display another listing
            //need_redraw = (keyin != '?' && keyin != '*');
            need_redraw=(strchr(INV_OBJECTS,keyin)==NULL);
            // A prompt is nice for when we're moving to "count" mode.
            need_prompt = (count != NULL && isdigit( keyin ));
         }   
        else if (count != NULL && isdigit( keyin ))
        {
            // The "read in quantity" mode
            keyin = get_invent_quant( keyin, *count );

            need_prompt = false;
            need_getch  = false;
        }
        else if (keyin == ESCAPE || keyin == ' '
                || keyin == '\r' || keyin == '\n')
        {
            ret = PROMPT_ABORT;
            break;
        }
        else if (isalpha( keyin )) 
        {
            ret = letter_to_index( keyin );

            if (must_exist && !is_valid_item( you.inv[ret] ))
                mpr( "You do not have any such object." );
            else
                break;
        }
        else    // we've got a character we don't understand...
        {
            canned_msg( MSG_HUH );
        }
    }
    mesclr(); //alex:12.11.01
    return (ret);
}




//--------------------------------------------------------------------------------------------------------
//For wield, zap, wear, etc commands.
//Show a list (up to 14) of the objects of the class "object_class"
//in the message area just after the prompt "Wield wich weapon...", "Wear which..."
//
//Alex:02.10.01
void mini_invent(int object_class) 
  {
   
        int j,bOK;
        int n=0;
        const int NUMBER_OF_LINES = get_number_of_lines();
        int nrow=NUMBER_OF_LINES-6;
        mesclr();
        textcolor(WHITE);        
        for (j = 0; j < ENDOFPACK && n<14; j++)
           {    
             bOK=0;	
             if (you.inv[j].quantity>0)
               {

               	 if (you.inv[j].base_type == object_class) 
               	   bOK=1;
               	 else  
               	    switch(object_class)
               	      {
               	         case OBJ_WEAPONS:
               	             //staves are weapons
               	             if (you.inv[j].base_type==OBJ_STAVES) bOK=1;
               	             break;
               	         case OBJ_MISSILES:
               	             //some weapons are missiles
               	             if (you.inv[j].base_type==OBJ_WEAPONS)
               	                if (you.inv[j].sub_type==WPN_HAND_AXE  || you.inv[j].sub_type==WPN_AXE || you.inv[j].sub_type==WPN_SPEAR || you.inv[j].sub_type==WPN_DAGGER)
               	                    bOK=1;
               	             break;
               	       }//switch
               	   	
                 } //you.inv[j].quantity>0	
                   
               	         
             if (bOK)  
               {                                  
                 in_name( j, DESC_INVENTORY_EQUIP, str_pass );
                 if (n<7)
                     gotoxy(1,nrow++);                      
                 else
                     gotoxy(40,nrow++);                        
                     
                 str_pass[39]=0;
                 cprintf(str_pass);
                 n++;
                 if (n==7) nrow=NUMBER_OF_LINES-6;
                 clreol();
               }
           }        
  }  
    
//--------------------------------------------------------------------------------------------------------
  
  
  
//--------------------------------------------------------------------------------------------------------
//From remove_item
//Show a list of the objects worn (including rings and amulets)
//in the message area just after the prompt "Wield wich weapon...", "Wear which..."
//
//This function is to allow unify commands 'T' and 'R'
//Alex:03.10.01
int mini_remove_invent(char *prompt=NULL) 
  {
   
        int j;
        int n=0,m;
        int worn=0;        //weapons can't be removed. They are unwielded
        const int NUMBER_OF_LINES = get_number_of_lines();
        int nrow=NUMBER_OF_LINES-6;
        textcolor(WHITE);
        for (j = EQ_WEAPON+1; j <= EQ_AMULET && n<14; j++)
           {             
               if (you.equip[j]>=0)
                 {
                   if (n<7)
                      gotoxy(1,nrow++);
                   else
                      gotoxy(45,nrow++);                        
                      
                   in_name( you.equip[j], DESC_INVENTORY_EQUIP, str_pass );
                   cprintf(str_pass);n++;  
                   if (n==7) nrow=NUMBER_OF_LINES-6;
                   clreol();
                 }  
           }        
        mesclr(); 
        if (prompt!=NULL) mpr(prompt, MSGCH_PROMPT);
        int keyin = get_ch();        
        mesclr(); 
        return keyin;                
  }