/*
 *  File:       item_use.cc
 *  Summary:    Functions for making use of inventory items.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *   <9>     02.10.01    ALX    .- Added mini_invent() to commands 'w','W','z','P','q','e' 
 *                              .- read_scroll() has a param. to (inicially) browse books instead scrolls
 *                              .- dwarven and orcs, also have a plus when launching with a dwarven/orcish launchers
 *                              .- takeoff_armour now has a param "quiet"
 *                              .- wield_effects() redone to work with new Equipment()  
 *                              .- unwield, unwear and effects from removing rings, moved to remove.cc
 *
 *   <8>     28July2000  GDL    Revised player throwing
 *   <7>     11/23/99    LRH    Horned characters can wear hats/caps
 *   <6>     7/13/99     BWR    Lowered learning rates for
 *                              throwing skills, and other
 *                              balance tweaks
 *   <5>     5/28/99     JDJ    Changed wear_armour to allow Spriggans to
 *                              wear bucklers.
 *   <4>     5/26/99     JDJ    body armour can be removed and worn if an
 *                              uncursed cloak is being worn.
 *                              Removed lots of unnessary mpr string copying.
 *                              Added missing ponderous message.
 *   <3>     5/20/99     BWR    Fixed staff of air bug, output of trial
 *                              identified items, a few you.wield_changes so
 *                              that the weapon gets updated.
 *   <2>     5/08/99     JDJ    Added armour_prompt.
 *   <1>     -/--/--     LRH    Created
 */

#include "AppHdr.h"
#include "item_use.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "externs.h"

#include "beam.h"
#include "debug.h"
#include "delay.h"
#include "describe.h"
#include "direct.h"
#include "effects.h"
#include "fight.h"
#include "food.h"
#include "invent.h"
#include "remove.h"
#include "potions.h"
#include "it_use3.h"
#include "items.h"
#include "itemname.h"
#include "misc.h"
#include "monplace.h"
#include "monstuff.h"
#include "mstuff2.h"
#include "mon-util.h"
#include "ouch.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "skills.h"
#include "skills2.h"
#include "spells1.h"
#include "spells2.h"
#include "spells3.h"
#include "spl-book.h"
#include "spl-cast.h"
#include "stuff.h"
#include "transfor.h"
#include "view.h"
#include "wpn-misc.h"

#include "options.h" //alex:18.02.02
#include "objects.h" //alex:01.03.02

#include "remove.h" //alex:15.03.02

#include "scrolls.h" //alex:15.03.02

bool drink_fountain(void);
static void throw_it(struct bolt &pbolt, int throw_2);
void use_randart(unsigned char item_wield_2);

// unStack(int item_to_unstack)
//When wear/put an item with quqntity > 1, this item "un-stacks.
//Ex: K - 2 ring of magic power
//When put on a ring:
//
//K - ring of magic power (on left hand)
//M - ring of magic power
//M was a free slot before un-stack
//
//The index of item K is the param passed to unStack
//
//If isn't any free slot, the rest of quantity of the item 
//is dropped.
//Alex: 06.10.01
void unStack(int item,bool quiet=false)
 {
  int m;  

  if (you.inv[item].quantity<2) return;
    
  //search a free slot
  for (m = 0; m < ENDOFPACK; m++)
    {
        if (you.inv[m].quantity== 0)
        {             
            //copy the item in the new slot
            you.inv[m].flags = you.inv[item].flags;
            you.inv[m].base_type = you.inv[item].base_type;
            you.inv[m].sub_type = you.inv[item].sub_type;
            you.inv[m].plus = you.inv[item].plus;
            you.inv[m].plus2 = you.inv[item].plus2;
            you.inv[m].special = you.inv[item].special;
            you.inv[m].colour = you.inv[item].colour;
            you.inv[m].luck=you.inv[item].luck;
            strcpy(you.inv[m].inscription,you.inv[item].inscription);
            
            //the quantity is quantity - 1. The "1" is
            //the quantity reamining in the original slot
            //to be weared.             
            you.inv[m].quantity = you.inv[item].quantity-1;
            you.inv[item].quantity=1;
            
            in_name( m, DESC_INVENTORY, str_pass);
            if (!quiet) mpr(str_pass);  
            return;
        }            
    }   
    
  //It isn't any free slot. Drop  
  if (!quiet) mpr("Your pack overflows");       
  if (!copy_item_to_grid( you.inv[item], you.x_pos, you.y_pos,1))
    {
        mpr( "Too many items on this level, not dropping the item." );
        return;
    }
    
    if (!quiet)
      {
        quant_name( you.inv[item], 1, DESC_NOCAP_A, str_pass );
        snprintf( info, INFO_SIZE, "You drop %s.", str_pass );
        mpr(info);
      }   
    dec_inv_item_quantity(item, 1);
    you.turn_is_over = 1;
}    
 
 
// Stack(int item_to_stack)
//When un-wear/remove an item, stack() searchs in the inventory
//another item identical.
//Ex: K - ring of magic power (on left hand)  --> put on
//    M - ring of magic power                 --> in your inventory
//When removing K
//
//M - 2 ring of magic power
//
//The index of item K is the param passed to Stack
//Alex: 06.10.01
void Stack(int item,bool quiet)
 {
  int m,n;  
  const int base_type = you.inv[item].base_type;
  const int sub_type  = you.inv[item].sub_type;
  
  for (m = 0; m < ENDOFPACK; m++)
    {
        
       if (items_stack(you.inv[item],you.inv[m]) && item!=m)       
          {
            inc_inv_item_quantity(m, 1);
            dec_inv_item_quantity(item,1);
            if (!quiet)
              {
                in_name( m, DESC_INVENTORY, info );
                mpr(info);
              } 
            return;
          }	
    
    }                           // end of for m loop. 
}    

bool wield_weapon(bool auto_wield,int wpn_slot=-1)
{
    int item_slot = 0;

    if (inv_count() < 1)
    {
        canned_msg(MSG_NOTHING_CARRIED);
        return false;
    }

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return false;
    }

    if (you.attribute[ATTR_TRANSFORMATION] != TRAN_NONE)
    {
        if (!can_equip(0))
        {
            mpr("You can't wield anything in your present form.");
            return false;
        }
    }

    if (you.equip[EQ_WEAPON] != -1
        && you.inv[you.equip[EQ_WEAPON]].base_type == OBJ_WEAPONS
        && item_cursed( you.inv[you.equip[EQ_WEAPON]] ))
    {
        mpr("You can't unwield your weapon to draw a new one!");
        return false;
    }

    if (you.sure_blade)
    {
        mpr("The bond with your blade fades away.");
        you.sure_blade = 0;
    }
                            
    if (auto_wield)
    {
        if (you.equip[EQ_WEAPON] == 0)  // ie. weapon is currently 'a'
            item_slot = 1;
        else
            item_slot = 0;
    }

    // Prompt if not using the auto swap command, 
    // or if the swap slot is empty.
    if (!auto_wield || !is_valid_item( you.inv[item_slot] ))
    {   
        if (wpn_slot!=-1)
          item_slot=wpn_slot;
        else  
           item_slot = prompt_invent_item( "Wield which item (- for none)?",
                                        OBJ_WEAPONS, true, '-' );

        if (item_slot == PROMPT_ABORT)
        {
            canned_msg( MSG_OK);
            return false;
        }
        else if (item_slot == PROMPT_GOT_SPECIAL)  // '-' or bare hands
        {
            item_slot=you.equip[EQ_WEAPON];	
            if (item_slot != -1)
            {            	            	
                you.equip[EQ_WEAPON]=-1;
                canned_msg( MSG_EMPTY_HANDED );
                you.time_taken *= 3;
                you.time_taken /= 10;
                
                Stack(item_slot);//alex:16.11.01  
                
                unwield_item(item_slot);
                you.turn_is_over = 1;
            }
            else
            {
                mpr( "You are already empty-handed.");
            }
            return true;
        }
    }

    if (item_slot == you.equip[EQ_WEAPON])
    {
        mpr("You are already wielding that!");
        return false;
    }

    for (int i = EQ_CLOAK; i <= EQ_AMULET; i++)
    {
        if (item_slot == you.equip[i])
        {
            mpr("You are wearing that object!");
            return false;
        }
    }
    
    //Take into account the quiver
    //cursed missiles can't go out of the quiver
    //Alex:28.02.02
    if (you.equip[EQ_QUIVER]==item_slot)
      {   
          if (item_cursed( you.inv[item_slot]))
           {
              mpr("The missiles in the quiver are stuck to it.");
              return false;
           } 
          you.equip[EQ_QUIVER]=-1;
          mpr("Your quiver is empty.");
          return true;
      } 
      
    if (you.inv[item_slot].base_type != OBJ_WEAPONS)
    {
        if (you.inv[item_slot].base_type == OBJ_STAVES
            && you.equip[EQ_SHIELD] != -1)
        {
            mpr("You can't wield that with a shield.");
            return false;
        }

    }
    else
    {
        if ((you.species < SP_OGRE || you.species > SP_OGRE_MAGE)
            && mass_item( you.inv[item_slot] ) >= 500)
        {
            mpr("That's too large and heavy for you to wield.");
            return false;
        }

        if ((you.species == SP_HALFLING || you.species == SP_GNOME
             || you.species == SP_KOBOLD || you.species == SP_SPRIGGAN)

            && (you.inv[item_slot].sub_type == WPN_GREAT_SWORD
                || you.inv[item_slot].sub_type == WPN_TRIPLE_SWORD
                || you.inv[item_slot].sub_type == WPN_GREAT_MACE
                || you.inv[item_slot].sub_type == WPN_GREAT_FLAIL
                || you.inv[item_slot].sub_type == WPN_BATTLEAXE
                || you.inv[item_slot].sub_type == WPN_EXECUTIONERS_AXE
                || you.inv[item_slot].sub_type == WPN_HALBERD
                || you.inv[item_slot].sub_type == WPN_GLAIVE
                || you.inv[item_slot].sub_type == WPN_GIANT_CLUB
                || you.inv[item_slot].sub_type == WPN_GIANT_SPIKED_CLUB
                || you.inv[item_slot].sub_type == WPN_SCYTHE))
        {
            mpr("That's too large for you to wield.");
            return false;

        }

        if (hands_reqd_for_weapon( you.inv[item_slot].base_type,
                              you.inv[item_slot].sub_type ) == HANDS_TWO_HANDED
            && you.equip[EQ_SHIELD] != -1)
        {
            mpr("You can't wield that with a shield.");
            return false;
        }

        int weap_brand = you.inv[item_slot].special;

        if (is_random_artefact( you.inv[item_slot] ))
            weap_brand = randart_wpn_property( you.inv[item_slot], RAP_BRAND );

        if ((you.is_undead || you.species == SP_DEMONSPAWN)
            && (!is_fixed_artefact( you.inv[item_slot] )
                && (weap_brand == SPWPN_HOLY_WRATH 
                    || weap_brand == SPWPN_DISRUPTION)))
        {
            mpr("This weapon will not allow you to wield it.");
            you.turn_is_over = 1;
            return false;
        }
     }   

     //the next if was inside the part if and part else of the above if {alex:16.11.01} 
     if (you.equip[EQ_WEAPON] != -1)
            {
              Stack(you.equip[EQ_WEAPON]);//alex:16.11.01            
              unwield_item(you.equip[EQ_WEAPON]);                            
            }
     you.equip[EQ_WEAPON]=item_slot;       
    

    // any oddness on wielding taken care of here
    wield_effects(item_slot,true );
    
    //Alex:12.11.01
    //UnStack weapon when wielded
    if (you.inv[item_slot].quantity>1) unStack(item_slot);
    
    in_name( item_slot, DESC_INVENTORY_EQUIP, str_pass );
    mpr( str_pass );

    // warn player about low str/dex or throwing skill
    wield_warning();

    // time calculations
    you.time_taken *= 5;
    you.time_taken /= 10;

    you.wield_change = true;
    you.turn_is_over = 1;
}

// provide a function for handling initial wielding of 'special'
// weapons,  or those whose function is annoying to reproduce in
// other places *cough* auto-butchering *cough*    {gdl}
void wield_effects(int item_wield_2, bool showMsgs)
{
    unsigned char i_dam = 0;

    // and here we finally get to the special effects of wielding {dlb}
    if (you.inv[item_wield_2].base_type == OBJ_MISCELLANY)
    {
        if (you.inv[item_wield_2].sub_type == MISC_LANTERN_OF_SHADOWS)
        {
            if (showMsgs)
                mpr("The area is filled with flickering shadows.");

            you.special_wield = SPWLD_SHADOW;
        }
    }

    if (you.inv[item_wield_2].base_type == OBJ_STAVES)
    {
        if (you.inv[item_wield_2].sub_type == STAFF_POWER)
        {
            inc_max_mp(13);
            set_ident_flags( you.inv[item_wield_2], ISFLAG_EQ_WEAPON_MASK );
        }
        else
        {
            // Most staves only give curse status when wielded and
            // right now that's always "uncursed". -- bwr
            set_ident_flags( you.inv[item_wield_2], ISFLAG_KNOW_CURSE );
        }
    }

    if (you.inv[item_wield_2].base_type == OBJ_WEAPONS)
    {
        if (is_demonic(you.inv[item_wield_2].sub_type)
            && (you.religion == GOD_ZIN || you.religion == GOD_SHINING_ONE
                || you.religion == GOD_ELYVILON))
        {
            if (showMsgs)
                mpr("You really shouldn't be using a nasty item like this.");
        }

        if (item_cursed( you.inv[item_wield_2] ))
        {
            mpr("It sticks to your hand!");
        }

        set_ident_flags( you.inv[item_wield_2], ISFLAG_EQ_WEAPON_MASK );

        if (is_random_artefact( you.inv[item_wield_2] ))
        {
            i_dam = randart_wpn_property(you.inv[item_wield_2], RAP_BRAND);
            use_randart(item_wield_2);
        }
        else 
        {
            i_dam = you.inv[item_wield_2].special;
        }

        if (i_dam != SPWPN_NORMAL)
        {
            // message first
            if (showMsgs)
            {
                switch (i_dam)
                {
                case SPWPN_FLAMING:
                    mpr("It bursts into flame!");
                    break;

                case SPWPN_FREEZING:
                    mpr("It glows with a cold blue light!");
                    break;

                case SPWPN_HOLY_WRATH:
                    mpr("It softly glows with a divine radiance!");
                    break;

                case SPWPN_ELECTROCUTION:
                    mpr("You hear the crackle of electricity.");
                    break;

                case SPWPN_ORC_SLAYING:
                    mpr((you.species == SP_HILL_ORC)
                            ? "You feel a sudden desire to commit suicide."
                            : "You feel a sudden desire to kill orcs!");
                    break;

                case SPWPN_VENOM:
                    mpr("It begins to drip with poison!");
                    break;

                case SPWPN_PROTECTION:
                    mpr("You feel protected!");
                    break;

                case SPWPN_DRAINING:
                    mpr("You sense an unholy aura.");
                    break;

                case SPWPN_SPEED:
                    mpr("Your hands tingle!");
                    break;

                case SPWPN_FLAME:
                    mpr("It glows red for a moment.");
                    break;

                case SPWPN_FROST:
                    mpr("It is covered in frost.");
                    break;

                case SPWPN_VAMPIRICISM:
                    if (you.species != SP_MUMMY)
                        mpr("You feel a strange hunger.");
                    else
                        mpr("You feel strangely empty.");
                    break;

                case SPWPN_DISRUPTION:
                    mpr("You sense a holy aura.");
                    break;

                case SPWPN_PAIN:
                    mpr("A searing pain shoots up your arm!");
                    break;

                case SPWPN_SINGING_SWORD:
                    mpr("The Singing Sword hums in delight!");
                    break;

                case SPWPN_WRATH_OF_TROG:
                    mpr("You feel bloodthirsty!");
                    break;

                case SPWPN_SCYTHE_OF_CURSES:
                    mpr("A shiver runs down your spine.");
                    break;

                case SPWPN_GLAIVE_OF_PRUNE:
                    mpr("You feel pruney.");
                    break;

                case SPWPN_SCEPTRE_OF_TORMENT:
                    mpr("A terribly searing pain shoots up your arm!");
                    break;

                case SPWPN_SWORD_OF_ZONGULDROK:
                    mpr("You sense an extremely unholy aura.");
                    break;

                case SPWPN_SWORD_OF_POWER:
                    mpr("You sense an aura of extreme power.");
                    break;

                case SPWPN_STAFF_OF_OLGREB:
                    // mummies cannot smell
                    if (you.species != SP_MUMMY)
                        mpr("You smell chlorine.");
                    else
                        mpr("The staff glows slightly green.");
                    break;

                case SPWPN_VAMPIRES_TOOTH:
                    // mummies cannot smell, and do not hunger {dlb}
                    if (you.species != SP_MUMMY)
                        mpr("You feel a strange hunger, and smell blood on the air...");
                    else
                        mpr("You feel strangely empty.");
                    break;

                default:
                    break;
                }
            }

            // effect second
            switch (i_dam)
            {
            case SPWPN_PROTECTION:
                you.redraw_armour_class = 1;
                break;

            case SPWPN_DISTORTION:
                miscast_effect(SPTYP_TRANSLOCATION, 9, 90, 100);
                break;

            case SPWPN_SINGING_SWORD:
                you.special_wield = SPWLD_SING;
                break;

            case SPWPN_WRATH_OF_TROG:
                you.special_wield = SPWLD_TROG;
                break;

            case SPWPN_SCYTHE_OF_CURSES:
                you.special_wield = SPWLD_CURSE;
                break;

            case SPWPN_MACE_OF_VARIABILITY:
                you.special_wield = SPWLD_VARIABLE;
                break;

            case SPWPN_GLAIVE_OF_PRUNE:
                you.special_wield = SPWLD_NONE;
                break;

            case SPWPN_SCEPTRE_OF_TORMENT:
                you.special_wield = SPWLD_TORMENT;
                break;

            case SPWPN_SWORD_OF_ZONGULDROK:
                you.special_wield = SPWLD_ZONGULDROK;
                break;

            case SPWPN_SWORD_OF_POWER:
                you.special_wield = SPWLD_POWER;
                break;

            case SPWPN_STAFF_OF_OLGREB:
                // josh declares mummies cannot smell {dlb}
                you.special_wield = SPWLD_OLGREB;
                break;

            case SPWPN_STAFF_OF_WUCAD_MU:
                miscast_effect(SPTYP_DIVINATION, 9, 90, 100);
                you.special_wield = SPWLD_WUCAD_MU;
                break;
            }
        }
    }
}                               // end wield_weapon()
  


//---------------------------------------------------------------
//
// armour_prompt
//
// Prompt the user for some armour. Returns true if the user picked
// something legit.
//
//---------------------------------------------------------------
bool armour_prompt(const string & mesg, int *index)
{
    ASSERT(index != NULL);

    bool  succeeded = false;
    int   slot;   

    if (inv_count() < 1)
        canned_msg(MSG_NOTHING_CARRIED);
    else if (you.berserker)
        canned_msg(MSG_TOO_BERSERK);
    else
    {
        slot = prompt_invent_item( mesg.c_str(), OBJ_ARMOUR );

        if (slot != PROMPT_ABORT)
        {
            *index = slot;
            succeeded = true;
        }
    }

    return (succeeded);
}                               // end armour_prompt()

bool cloak_is_being_removed( void )
{
    if (current_delay_action() != DELAY_ARMOUR_OFF)
        return (false);

    if (you.delay_queue.front().parm1 != you.equip[ EQ_CLOAK ])
        return (false);

    return (true);
}

//---------------------------------------------------------------
//
// wear_armour
//
//---------------------------------------------------------------
void wear_armour(void)
{
    int armour_wear_2;

    if (!armour_prompt("Wear which item?", &armour_wear_2))
        return;

    do_wear_armour( armour_wear_2, false );
}

//Added bDelay and use of mpr2() with quiet {alex:28.02.02}
bool do_wear_armour( int item, bool quiet ,bool bDelay=true)
{
    char wh_equip = 0;

    if (!is_valid_item( you.inv[item] ))
    {
        mpr("You don't have any such object.",quiet);
        return (false);
    }
   

    
    if (you.inv[item].base_type != OBJ_ARMOUR)
    {
        mpr2("You can't wear that.",quiet);
        return (false);
    }

    if (item == you.equip[EQ_WEAPON])
    {
        mpr2("You are wielding that object!",quiet);
        return (false);
    }

    for (int loopy = EQ_CLOAK; loopy <= EQ_BODY_ARMOUR; loopy++)
    {
        if (item == you.equip[loopy])
        {
            mpr2("You are already wearing that!",quiet);
            return (false);
        }
    }

    // if you're wielding something,
    if (you.equip[EQ_WEAPON] != -1
        // attempting to wear a shield,
        && (you.inv[item].sub_type == ARM_SHIELD
            || you.inv[item].sub_type == ARM_BUCKLER
            || you.inv[item].sub_type == ARM_LARGE_SHIELD)
        // weapon is two-handed
        && hands_reqd_for_weapon(you.inv[you.equip[EQ_WEAPON]].base_type,
                      you.inv[you.equip[EQ_WEAPON]].sub_type) == HANDS_TWO_HANDED)
    {
        mpr2("You'd need three hands to do that!",quiet);
        return (false);
    }

    if (you.inv[item].sub_type == ARM_BOOTS)
    {
        if (you.species != SP_NAGA && you.inv[item].plus2 == TBOOT_NAGA_BARDING)
        {
           mpr2("You can't wear that!",quiet);
           return (false);
        }

        if (you.species != SP_CENTAUR && you.inv[item].plus2 == TBOOT_CENTAUR_BARDING)
        {
            mpr2("You can't wear that!",quiet);
            return (false);
        }

        if (player_is_swimming() && you.species == SP_MERFOLK)
        {
            mpr2("You don't currently have feet!",quiet);
            return (false);
        }
    }

    wh_equip = EQ_BODY_ARMOUR;

    switch (you.inv[item].sub_type)
    {
    case ARM_BUCKLER:
    case ARM_LARGE_SHIELD:
    case ARM_SHIELD:
        wh_equip = EQ_SHIELD;
        break;
    case ARM_CLOAK:
        wh_equip = EQ_CLOAK;
        break;
    case ARM_HELMET:
        wh_equip = EQ_HELMET;
        break;
    case ARM_GLOVES:
        wh_equip = EQ_GLOVES;
        break;
    case ARM_BOOTS:
        wh_equip = EQ_BOOTS;
        break;
    }

    if (you.species == SP_NAGA && you.inv[item].sub_type == ARM_BOOTS
        && you.inv[item].plus2 == TBOOT_NAGA_BARDING
        && !player_is_shapechanged())
    {
        // it fits
    }
    else if (you.species == SP_CENTAUR
             && you.inv[item].sub_type == ARM_BOOTS
             && you.inv[item].plus2 == TBOOT_CENTAUR_BARDING
             && !player_is_shapechanged())
    {
        // it fits
    }
    else if (you.inv[item].sub_type == ARM_HELMET
             && (cmp_helmet_type( you.inv[item], THELM_CAP )
                 || cmp_helmet_type( you.inv[item], THELM_WIZARD_HAT )))
    {
        // caps & wiz hats always fit, unless your head's too big (ogres &c)
    }
    else if (!can_equip(wh_equip))
    {
        mpr2("You can't wear that in your present form.",quiet);
        return (false);
    }

    // Cannot swim in heavy armour
    if (player_is_swimming()
        && wh_equip == EQ_BODY_ARMOUR
        && !is_light_armour( you.inv[item] ))
    {
        mpr2("You can't swim in that!",quiet);
        return (false);
    }

    // Giant races
    if ((you.species >= SP_OGRE && you.species <= SP_OGRE_MAGE)
        || player_genus(GENPC_DRACONIAN))
    {
        if ((you.inv[item].sub_type >= ARM_LEATHER_ARMOUR
                && you.inv[item].sub_type <= ARM_PLATE_MAIL)
            || (you.inv[item].sub_type >= ARM_GLOVES
                && you.inv[item].sub_type <= ARM_BUCKLER)
            || you.inv[item].sub_type == ARM_CRYSTAL_PLATE_MAIL
            || (you.inv[item].sub_type == ARM_HELMET
                && (cmp_helmet_type( you.inv[item], THELM_HELM )
                    || cmp_helmet_type( you.inv[item], THELM_HELMET ))))
        {
            mpr2("This armour doesn't fit on your body.",quiet);
            return (false);
        }
    }

    // Tiny races
    if (you.species == SP_SPRIGGAN)
    {
        if ((you.inv[item].sub_type >= ARM_LEATHER_ARMOUR
                && you.inv[item].sub_type <= ARM_PLATE_MAIL)
            || you.inv[item].sub_type == ARM_GLOVES
            || you.inv[item].sub_type == ARM_BOOTS
            || you.inv[item].sub_type == ARM_SHIELD
            || you.inv[item].sub_type == ARM_LARGE_SHIELD
            || you.inv[item].sub_type == ARM_CRYSTAL_PLATE_MAIL
            || (you.inv[item].sub_type == ARM_HELMET
                && (cmp_helmet_type( you.inv[item], THELM_HELM )
                    || cmp_helmet_type( you.inv[item], THELM_HELMET ))))
        {
            mpr2("This armour doesn't fit on your body.",quiet);
            return (false);
        }
    }

    bool removedCloak = false;
    int  cloak = -1;

    if ((you.inv[item].sub_type < ARM_SHIELD
            || you.inv[item].sub_type > ARM_LARGE_SHIELD)
        && (you.equip[EQ_CLOAK] != -1 && !cloak_is_being_removed()))
    {
        if (item_uncursed( you.inv[you.equip[EQ_CLOAK]] ))
        {
            cloak = you.equip[ EQ_CLOAK ];
            if (!takeoff_armour(you.equip[EQ_CLOAK]),quiet)
                return (false);

            removedCloak = true;
        }
        else
        {
            mpr2("Your cloak prevents you from wearing the armour.",quiet);
            return (false);
        }
    }

    if (you.inv[item].sub_type == ARM_CLOAK && you.equip[EQ_CLOAK] != -1)
    {
        if (!takeoff_armour(you.equip[EQ_CLOAK]),quiet)
            return (false);
    }

    if (you.inv[item].sub_type == ARM_HELMET && you.equip[EQ_HELMET] != -1)
    {
        if (!takeoff_armour(you.equip[EQ_HELMET]),quiet)
            return (false);
    }

    if (you.inv[item].sub_type == ARM_GLOVES && you.equip[EQ_GLOVES] != -1)
    {
        if (!takeoff_armour(you.equip[EQ_GLOVES]),quiet)
            return (false);
    }

    if (you.inv[item].sub_type == ARM_BOOTS && you.equip[EQ_BOOTS] != -1)
    {
        if (!takeoff_armour(you.equip[EQ_BOOTS]),quiet)
            return (false);
    }

    if ((you.inv[item].sub_type == ARM_SHIELD
            || you.inv[item].sub_type == ARM_LARGE_SHIELD
            || you.inv[item].sub_type == ARM_BUCKLER)
        && you.equip[EQ_SHIELD] != -1)
    {
        if (!takeoff_armour(you.equip[EQ_SHIELD]),quiet)
            return (false);
    }

    if ((you.inv[item].sub_type < ARM_SHIELD
            || you.inv[item].sub_type > ARM_LARGE_SHIELD)
        && you.equip[EQ_BODY_ARMOUR] != -1)
    {
        if (!takeoff_armour(you.equip[EQ_BODY_ARMOUR]),quiet)
            return (false);
    }

    you.turn_is_over = 1;

    if (bDelay)
      {
        int delay = property( you.inv[item], PARM_AC );

        if (delay < 1) delay = 1;

        if (delay) start_delay( DELAY_ARMOUR_ON, delay, item );

        if (removedCloak) start_delay( DELAY_ARMOUR_ON, 1, cloak );
      }
    else
      {
         you.equip[aObjs[OBJ_ARMOUR][you.inv[item].sub_type].slot]=item;
         set_ident_flags( you.inv[item], ISFLAG_EQ_ARMOUR_MASK );
      }   
        
    //UnStacks armour when wearing it {alex:12.11.01}
    unStack(item);
    
    return (true);
}                               // do_end wear_armour()

bool takeoff_armour(int item,bool quiet=false)
{
    if (you.inv[item].base_type != OBJ_ARMOUR)
    {
        mpr2("You aren't wearing that!",quiet);
        return false;
    }

    if (item_cursed( you.inv[item] ))
    {
        for (int loopy = EQ_CLOAK; loopy <= EQ_BODY_ARMOUR; loopy++)
        {
            if (item == you.equip[loopy])
            {
                in_name(item, DESC_CAP_YOUR, info);
                strcat(info, " is stuck to your body!");
                mpr2(info,quiet);
                return false;
            }
        }
    }

    bool removedCloak = false;
    int cloak = -1;

    if (you.inv[item].sub_type < ARM_SHIELD
        || you.inv[item].sub_type > ARM_LARGE_SHIELD)
    {
        if (you.equip[EQ_CLOAK] != -1 && !cloak_is_being_removed())
        {
            if (item_uncursed( you.inv[you.equip[EQ_CLOAK]] ))
            {
                cloak = you.equip[ EQ_CLOAK ];
                if (!takeoff_armour(you.equip[EQ_CLOAK]),quiet)
                    return (false);

                removedCloak = true;
            }
            else
            {
                mpr2("Your cloak prevents you from removing the armour.",quiet);
                return false;
            }
        }

        if (item != you.equip[EQ_BODY_ARMOUR])
        {
            mpr2("You aren't wearing that!",quiet);
            return false;
        }

        // you.equip[EQ_BODY_ARMOUR] = -1;
    }
    else
    {
        switch (you.inv[item].sub_type)
        {
        case ARM_BUCKLER:
        case ARM_LARGE_SHIELD:
        case ARM_SHIELD:
            if (item != you.equip[EQ_SHIELD])
            {
                mpr2("You aren't wearing that!",quiet);
                return false;
            }
            break;

        case ARM_CLOAK:
            if (item != you.equip[EQ_CLOAK])
            {
                mpr2("You aren't wearing that!",quiet);
                return false;
            }
            break;

        case ARM_HELMET:
            if (item != you.equip[EQ_HELMET])
            {
                mpr2("You aren't wearing that!",quiet);
                return false;
            }
            break;


        case ARM_GLOVES:
            if (item != you.equip[EQ_GLOVES])
            {
                mpr2("You aren't wearing that!",quiet);
                return false;
            }
            break;

        case ARM_BOOTS:
            if (item != you.equip[EQ_BOOTS])
            {
                mpr2("You aren't wearing that!",quiet);
                return false;
            }
            break;
        }
    }

    you.turn_is_over = 1;

    int delay = property( you.inv[item], PARM_AC );

    if (delay < 1)
        delay = 1;

    start_delay( DELAY_ARMOUR_OFF, delay, item );
    Stack(item); //{alex:12.11.01}
    if (removedCloak)
        start_delay( DELAY_ARMOUR_ON, 1, cloak );
        

    return true;
}                               // end takeoff_armour()

void throw_anything(void)
{
    struct bolt beam;
    int throw_slot;

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }
    else if (inv_count() < 1)
    {
        canned_msg(MSG_NOTHING_CARRIED);
        return;
    }

    throw_slot = prompt_invent_item( "Throw which item?", OBJ_MISSILES );
    if (throw_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    if (throw_slot == you.equip[EQ_WEAPON]
             && (item_cursed( you.inv[you.equip[EQ_WEAPON]] )))
    {
        mpr("That thing is stuck to your hand!");
        return;
    }
    else
    {
        for (int loopy = EQ_CLOAK; loopy <= EQ_AMULET; loopy++)
        {
            if (throw_slot == you.equip[loopy])
            {
                mpr("You are wearing that object!");
                return;
            }
        }
    }

    throw_it(beam, throw_slot);
}                               // end throw_anything()

void shoot_thing(void)
{
    int i = -1; //alex:28.02.02
    struct bolt beam;

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    if (you.duration[DUR_INMATERIALITY])
    {
      sprintf(str_pass,"In your current form, you can shoot anything");
      mpr(str_pass);
      return;
    }
    
    const int weapon = you.equip[ EQ_WEAPON ];
    int type_wanted = MI_DART;

    if (weapon != -1 && you.inv[ weapon ].base_type == OBJ_WEAPONS
        && launches_things( you.inv[ weapon ].sub_type ))
    {
        type_wanted = launched_by( you.inv[ weapon ].sub_type );
    }

    //Ax-Crawl    
    //First, see if the quiver
    
    
            i=you.equip[EQ_QUIVER];       
            if (i!=NON_ITEM)
              {
                
                if (you.inv[i].quantity<=0)
                  {  
                    mpr("You quiver is empty.");
                    if (xtraOps.ax_quiver) i=-1;else return;
                  }  
                else if (you.inv[i].sub_type != type_wanted)  
                  { 
                    mpr("No suitable missiles.");
                    if (xtraOps.ax_quiver) i=-1;else return;
                  }  
              }  
                
    //if must autoquiver {alex:28.02.02}
    if (i==-1)
       {    
           for (i = 0; i < (ENDOFPACK + 1); i++)
           {
               if (i == ENDOFPACK)
               {
                   mpr("No suitable missiles.");
                   return;
               }
       
               if (you.inv[i].quantity && you.inv[i].base_type == OBJ_MISSILES
                   && you.inv[i].sub_type == type_wanted)
               {
                   break;
               }
           }
           you.equip[EQ_QUIVER]=i;
      }
          
    throw_it(beam, i);
}                               // end shoot_thing()

// throw_it - currently handles player throwing only.  Monster
// throwing is handled in mstuff2:mons_throw()
static void throw_it(struct bolt &pbolt, int throw_2)
{
    struct dist thr;
    char shoot_skill = 0;

    char wepClass, wepType;     // ammo class and type
    char lnchClass, lnchType;   // launcher class and type

    int baseHit = 0, baseDam = 0;       // from thrown or ammo
    int ammoHitBonus = 0, ammoDamBonus = 0;     // from thrown or ammo
    int lnchHitBonus = 0, lnchDamBonus = 0;     // special add from launcher
    int exHitBonus = 0, exDamBonus = 0; // 'extra' bonus from skill/dex/str
    int effSkill = 0;           // effective launcher skill
    bool launched = false;      // item is launched
    bool thrown = false;        // item is sensible thrown item

   if (you.duration[DUR_INMATERIALITY])
    {
      sprintf(str_pass,"In your current form, you can throw anything");
      mpr(str_pass);
      return;
    }
    mpr("Which direction? (*/+/- to target)", MSGCH_PROMPT);

    message_current_target();

    direction(thr);

    if (!thr.isValid)
    {
        if (thr.isCancel)
            canned_msg(MSG_OK);

        return;
    }

    if (you.conf)
    {
        thr.isTarget = true;
        thr.tx = you.x_pos + random2(13) - 6;
        thr.ty = you.y_pos + random2(13) - 6;
    }

    // even though direction is allowed,  we're throwing so we
    // want to use tx, ty to make the missile fly to map edge.
    pbolt.target_x = thr.tx;
    pbolt.target_y = thr.ty;
   
    pbolt.flavour = BEAM_MISSILE; // pbolt.range is set below
    
    //alex:25.05.05
    pbolt.type_slaying=-1; 
    pbolt.second_effect=BEAM_NONE;
    
       
    

    switch (you.inv[throw_2].base_type)
    {
    case OBJ_WEAPONS:    pbolt.type = SYM_WEAPON;  break;
    case OBJ_MISSILES:   pbolt.type = SYM_MISSILE; break;
    case OBJ_ARMOUR:     pbolt.type = SYM_ARMOUR;  break;
    case OBJ_WANDS:      pbolt.type = SYM_STICK;   break;
    case OBJ_FOOD:       pbolt.type = SYM_CHUNK;   break;
    case OBJ_SCROLLS:    pbolt.type = SYM_SCROLL;  break;
    case OBJ_JEWELLERY:  pbolt.type = SYM_TRINKET; break;
    case OBJ_POTIONS:    pbolt.type = SYM_FLASK;   break;    
    case OBJ_BOOKS:      pbolt.type = SYM_OBJECT;  break;
        // this does not seem right, but value was 11 {dlb}
        // notice how the .type does not match the class -- hmmm... {dlb}
    case OBJ_STAVES:      pbolt.type = SYM_CHUNK;  break;
    }

    pbolt.source_x = you.x_pos;
    pbolt.source_y = you.y_pos;
    pbolt.colour = you.inv[throw_2].colour;

    quant_name( you.inv[throw_2], 1, DESC_PLAIN, str_pass );
    strcpy(pbolt.beam_name, str_pass);

    pbolt.thrower = KILL_YOU_MISSILE;

    // get the ammo/weapon type.  Convenience.
    wepClass = you.inv[throw_2].base_type;
    wepType = you.inv[throw_2].sub_type;

    // get the launcher class,type.  Convenience.
    if (you.equip[EQ_WEAPON] < 0)
    {
        lnchClass = -1;
        // set lnchType to 0 so the 'figure out if launched'
        // code doesn't break
        lnchType = 0;
    }
    else
    {
        lnchClass = you.inv[you.equip[EQ_WEAPON]].base_type;
        lnchType = you.inv[you.equip[EQ_WEAPON]].sub_type;
    }

    // baseHit and damage for generic objects
    baseHit = you.strength - mass_item( you.inv[throw_2] ) / 10;
    if (baseHit > 0)
        baseHit = 0;

    baseDam = mass_item( you.inv[throw_2] ) / 100;

    // special: might be throwing generic weapon;
    // use base wep. damage, w/ penalty
    if (wepClass == OBJ_WEAPONS)
    {
        baseDam = property( you.inv[throw_2], PWPN_DAMAGE ) - 4;
        if (baseDam < 0)
            baseDam = 0;
    }

    // figure out if we're thrown or launched
    throw_type(lnchClass, lnchType, wepClass, wepType, launched, thrown);

    // extract launcher bonuses due to magic
    if (launched)
    {
        lnchHitBonus = you.inv[you.equip[EQ_WEAPON]].plus;
        lnchDamBonus = you.inv[you.equip[EQ_WEAPON]].plus2;
    }

    // extract weapon/ammo bonuses due to magic
    ammoHitBonus = you.inv[throw_2].plus;
    ammoDamBonus = you.inv[throw_2].plus2;
    
    //move here from section if (launched) {alex:18.02.02}
    int ammo_brand = you.inv[throw_2].special;
    bool poisoned = (ammo_brand == SPMSL_POISONED || ammo_brand == SPMSL_POISONED_II);
    
    // CALCULATIONS FOR LAUNCHED WEAPONS
    if (launched)
    {
        // this is deliberately confusing: the 'hit' value for
        // ammo is the _damage_ when used with a launcher.  Geez.
        baseHit = 0;
        baseDam = property( you.inv[throw_2], PWPN_HIT );

        // fix ammo damage bonus, since missiles only use inv_plus
        ammoDamBonus = ammoHitBonus;

        // check for matches;  dwarven,elven,orcish
        if (!cmp_equip_race( you.inv[you.equip[EQ_WEAPON]], 0 ))
        {
            if (get_equip_race( you.inv[you.equip[EQ_WEAPON]] ) 
                        == get_equip_race( you.inv[throw_2] ))
            {
                baseHit += 1;
                baseDam += 1;

                // elves with elven bows                
                if (cmp_equip_race(you.inv[you.equip[EQ_WEAPON]], ISFLAG_ELVEN)
                    && player_genus(GENPC_ELVEN))
                {
                    baseHit += 1;
                }
                // also, dwarfven and orcs {alex:18.02.02}
                else if (xtraOps.ax_firing)
                      if  (
                                (cmp_equip_race(you.inv[you.equip[EQ_WEAPON]], ISFLAG_DWARVEN) && player_genus(GENPC_DWARVEN)) 
                            ||  (cmp_equip_race(you.inv[you.equip[EQ_WEAPON]], ISFLAG_ORCISH) && you.species==SP_HILL_ORC) 
                          )                            
                {
                   baseHit+=1;	
                }	   
                
                
                  
            }
        }

        if (you.inv[you.equip[EQ_WEAPON]].sub_type == WPN_CROSSBOW)
        {
            // extra time taken, as a percentage.  range from 30 -> 12
            int extraTime = 30 - ((you.skills[SK_CROSSBOWS] * 2) / 3);

            you.time_taken = (100 + extraTime) * you.time_taken;
            you.time_taken /= 100;
        }

        // for all launched weapons,  maximum effective specific skill
        // is twice throwing skill.  This models the fact that no matter
        // how 'good' you are with a bow,  if you know nothing about
        // trajectories you're going to be a damn poor bowman.  Ditto
        // for crossbows and slings.
        switch (lnchType)
        {
        case WPN_SLING:
            shoot_skill = you.skills[SK_SLINGS];
            break;
        case WPN_BOW:
            shoot_skill = you.skills[SK_BOWS];
            break;
        case WPN_BLOWGUN:
            shoot_skill = you.skills[SK_DARTS];
            break;
        case WPN_CROSSBOW:
        case WPN_HAND_CROSSBOW:
            shoot_skill = you.skills[SK_CROSSBOWS];
            break;
        default:
            shoot_skill = 0;
            break;
        }

        effSkill = you.skills[SK_THROWING] * 2 + 1;
        effSkill = (shoot_skill > effSkill) ? effSkill : shoot_skill;

        // removed 2 random2(2)s from each of the learning curves, but
        // left slings because they're hard enough to develop without
        // a good source of shot in the dungeon.
        switch (lnchType)
        {
        case WPN_SLING:
            // Slings are really easy to learn because they're not
            // really all that good, and its harder to get ammo anyways.
            exercise(SK_SLINGS, 1 + random2avg(3, 2));
            baseHit += 0;
            exHitBonus = (effSkill * 3) / 2;

            // strength is good if you're using a nice sling.
            exDamBonus = (10 * (you.strength - 10)) / 9;
            exDamBonus = (exDamBonus * (2 * baseDam + ammoDamBonus)) / 20;

            // cap
            if (exDamBonus > lnchDamBonus + 1)
                exDamBonus = lnchDamBonus + 1;

            // add skill for slings.. helps to find those vulnerable spots
            exDamBonus += effSkill / 2;

            // now kill the launcher damage bonus            
            
            //Alex: 18.02.02
            //Option to leave the launcher bonus. I don't understand why it 
            //was taken away in vainilla crawl.
            if (!xtraOps.ax_firing) 
               if (lnchDamBonus > 0) lnchDamBonus = 0;
            break;

            // blowguns take a _very_ steady hand;  a lot of the bonus
            // comes from dexterity.  (Dex bonus here as well as below)
        case WPN_BLOWGUN:
            exercise(SK_DARTS, (coinflip()? 2 : 1));
            baseHit -= 2;
            exHitBonus = (effSkill * 3) / 2 + you.dex / 2;

            // no extra damage for blowguns
            exDamBonus = 0;

            // now kill the launcher damage and ammo bonuses
            //Alex: 18.02.02
            //Option to leave the launcher/ammno bonus. I don't understand why
            //they were taken away in vainilla crawl.
            if (!xtraOps.ax_firing) 
               {
               	  if (lnchDamBonus > 0) lnchDamBonus = 0;
                  if (ammoDamBonus > 0) ammoDamBonus = 0;
               }   
            break;


        case WPN_BOW:
            exercise(SK_BOWS, (coinflip()? 2 : 1));
            baseHit -= 4;
            exHitBonus = (effSkill * 2);

            // strength is good if you're using a nice bow
            exDamBonus = (10 * (you.strength - 10)) / 4;
            exDamBonus = (exDamBonus * (2 * baseDam + ammoDamBonus)) / 20;

            // cap
            if (exDamBonus > (lnchDamBonus + 1) * 3)
                exDamBonus = (lnchDamBonus + 1) * 3;

            // add in skill for bows.. help you to find those vulnerable spots.
            exDamBonus += effSkill;

            // now kill the launcher damage bonus
            //Alex: 18.02.02
            //Option to leave the launcher bonus. I don't understand why
            //was taken away in vainilla crawl.
            if (!xtraOps.ax_firing) 
               if (lnchDamBonus > 0) lnchDamBonus = 0;
            break;
            

        case WPN_CROSSBOW:
            // Crossbows are easy for unskilled people.
            exercise(SK_CROSSBOWS, (coinflip()? 2 : 1));
            baseHit = 2;
            exHitBonus = (3 * effSkill) / 2 + 6;
            exDamBonus = effSkill / 2 + 4;
            break;

        case WPN_HAND_CROSSBOW:
            exercise(SK_CROSSBOWS, (coinflip()? 2 : 1));
            baseHit = 1;
            exHitBonus = (3 * effSkill) / 2 + 4;
            exDamBonus = effSkill / 2 + 2;
            break;
        }

        // all launched weapons have a slight chance of improving
        // throwing skill
        if (coinflip())
            exercise(SK_THROWING, 1);

        // all launched weapons get a tohit boost from throwing skill.
        exHitBonus += (3 * you.skills[SK_THROWING]) / 4;

        // special cases for flame, frost, poison, etc.
        int bow_brand  = you.inv[ you.equip[EQ_WEAPON] ].special;

        if (is_random_artefact( you.inv[ you.equip[EQ_WEAPON] ] ))
        {
            bow_brand = randart_wpn_property( you.inv[ you.equip[EQ_WEAPON] ],
                                                RAP_BRAND );
        }

        //moved above to be used also when throwing {alex:18.02.02} 
        //const int ammo_brand = you.inv[throw_2].special;
        //const bool poisoned = (ammo_brand == SPMSL_POISONED || ammo_brand == SPMSL_POISONED_II);
       
        
        
        // check for venom brand (usually only available for blowguns)
        if (!xtraOps.ax_firing)
        {
              if (bow_brand == SPWPN_VENOM 
                  && !(ammo_brand == SPMSL_FLAME 
                      || ammo_brand == SPMSL_ICE 
                      || poisoned))
              {
                  // poison brand the ammo
                  you.inv[throw_2].special = SPMSL_POISONED;
      
                  quant_name( you.inv[throw_2], 1, DESC_PLAIN, str_pass );
                  strcpy( pbolt.beam_name, str_pass );
              }
              
              // Note that bow_brand is known since the bow is equiped.
              if ((bow_brand == SPWPN_FLAME || ammo_brand == SPMSL_FLAME)
                  && ammo_brand != SPMSL_ICE && bow_brand != SPWPN_FROST)
              {
                  baseDam += 1 + random2(5);
                  pbolt.flavour = BEAM_FIRE;
                  strcpy(pbolt.beam_name, "bolt of ");
      
                  if (poisoned)
                      strcat(pbolt.beam_name, "poison ");
      
                  strcat(pbolt.beam_name, "flame");
                  pbolt.colour = RED;
                  pbolt.type = SYM_BOLT;
                  pbolt.thrower = KILL_YOU_MISSILE;
      
                  // ammo known if we can't attribute it to the bow
                  if (bow_brand != SPWPN_FLAME && !poisoned)
                      set_ident_flags( you.inv[throw_2], ISFLAG_KNOW_TYPE );
              }
      
              if ((bow_brand == SPWPN_FROST || ammo_brand == SPMSL_ICE)
                  && ammo_brand != SPMSL_FLAME && bow_brand != SPWPN_FLAME)
              {
                  baseDam += 1 + random2(5);
                  pbolt.flavour = BEAM_COLD;
                  strcpy(pbolt.beam_name, "bolt of ");
      
                  if (poisoned)
                      strcat(pbolt.beam_name, "poison ");
      
                  strcat(pbolt.beam_name, "frost");
                  pbolt.colour = WHITE;
                  pbolt.type = SYM_BOLT;
                  pbolt.thrower = KILL_YOU_MISSILE;
      
                  // ammo known if we can't attribute it to the bow
                  if (bow_brand != SPWPN_FROST && !poisoned)
                      set_ident_flags( you.inv[throw_2], ISFLAG_KNOW_TYPE );
              }
      
              // ammo known if it cancels the effect of the bow
              if ((bow_brand == SPWPN_FLAME && ammo_brand == SPMSL_ICE)
                  || (bow_brand == SPWPN_FROST && ammo_brand == SPMSL_FLAME))
              {
                  set_ident_flags( you.inv[throw_2], ISFLAG_KNOW_TYPE );
              }
        
        }
        
        //-----------------------------------------------------------
        // AX-Firing {alex:21.02.02}
        //-----------------------------------------------------------         
        else
        {         
              // If bow of XXX, the ammo must be XXX or normal
              if (bow_brand == SPWPN_VENOM ||poisoned) 
               {
                  if (ammo_brand == SPMSL_NONE || poisoned)
                  {     
                        //if the bow is normal, identify the ammo
                        if (bow_brand == SPWPN_NORMAL) set_ident_flags( you.inv[throw_2], ISFLAG_KNOW_TYPE);
                        ammo_brand = SPMSL_POISONED;
                        pbolt.flavour = BEAM_POISON;                        
                        strcpy(pbolt.beam_name, "bolt of poison");
                        pbolt.colour = GREEN;
                        baseDam += 1 + random2(5);  
                      }  
                  else
                     ammo_brand = SPMSL_NONE;            
               }
              else if (bow_brand == SPWPN_FLAME || ammo_brand == SPMSL_FLAME)
               { 
                  if  (ammo_brand == SPMSL_FLAME || ammo_brand == SPMSL_NONE)
                      {                         
                        if (bow_brand == SPWPN_NORMAL) set_ident_flags( you.inv[throw_2], ISFLAG_KNOW_TYPE);
                        ammo_brand = SPMSL_FLAME;
                        pbolt.flavour = BEAM_FIRE;                        
                        if (poisoned)
                           strcpy(pbolt.beam_name, "poisoned bolt of fame");
                        else
                           strcpy(pbolt.beam_name, "poisoned bolt of fame");
                        pbolt.colour = RED;
                        baseDam += 1 + random2(5);  
                      }  
                  else
                      ammo_brand = SPMSL_NONE;      
               } 
              else if (bow_brand == SPWPN_FROST || ammo_brand == SPMSL_ICE)
               { 
                  if (ammo_brand == SPMSL_ICE || ammo_brand == SPMSL_NONE)
                      {
                        if (bow_brand == SPWPN_NORMAL) set_ident_flags( you.inv[throw_2], ISFLAG_KNOW_TYPE);
                        ammo_brand = SPMSL_ICE;
                        pbolt.flavour = BEAM_COLD;
                        if (poisoned)
                           strcpy(pbolt.beam_name, "poisoned bolt of cold");
                        else
                           strcpy(pbolt.beam_name, "poisoned bolt of cold");
                        pbolt.colour = WHITE;
                        baseDam += 1 + random2(5);  
                      }  
                  else
                     ammo_brand = SPMSL_NONE;            
               } 
              else if (bow_brand > SPWPN_SLAYING && bow_brand<SPWPN_LAST_SLAYING)
               { 
               	  pbolt.type_slaying=bow_brand;
               	  if (poisoned)
                     strcpy(pbolt.beam_name, "poisoned bolt");
                  else
                     strcpy(pbolt.beam_name, "bolt");
                  pbolt.colour = WHITE;
                  baseDam += 1 + random2(5);  
               } 
              
              //second effects (enchantments) of the missil
              
                  if (ammo_brand != SPMSL_NONE) 
                    {                     
                       switch(ammo_brand)
                         {
                            case SPMSL_CONFUSE:
                               pbolt.second_effect=BEAM_CONFUSION;break;   
                            case SPMSL_SLOW:
                               pbolt.second_effect=BEAM_SLOW;break;      
                            case SPMSL_PARALYSE:
                               pbolt.second_effect=BEAM_PARALYSIS;break;         
                            case SPMSL_BACKLITE:
                               pbolt.second_effect=BEAM_BACKLIGHT;break;             
                                                           
                         }   	                         
                       pbolt.thrower = KILL_YOU_MISSILE;                       
                       
                     }
                
          }     
              
        /* the chief advantage here is the extra damage this does
         * against susceptible creatures */

        /* Note: weapons & ammo of eg fire are not cumulative
         * ammo of fire and weapons of frost don't work together,
         * and vice versa */

        // ID check
        if (item_not_ident( you.inv[you.equip[EQ_WEAPON]], ISFLAG_KNOW_PLUSES )
            && random2(100) < shoot_skill)
        {
            set_ident_flags(you.inv[you.equip[EQ_WEAPON]], ISFLAG_KNOW_PLUSES);

            strcpy(info, "You are wielding ");
            in_name(you.equip[EQ_WEAPON], DESC_NOCAP_A, str_pass);
            strcat(info, str_pass);
            strcat(info, ".");
            mpr(info);

            more();
            you.wield_change = true;
        }
    }

    // CALCULATIONS FOR THROWN WEAPONS
    if (thrown)
    {
        baseHit = 0;

        // since darts/rocks are missiles, they only use inv_plus
        if (wepClass == OBJ_MISSILES)
            {
            	ammoDamBonus = ammoHitBonus;
            	if (wepType==MI_MAGIC_BALL)
            	   {
            	   	int power;
            	   	switch(you.inv[throw_2].special)
            	   	  {
            	   	    case SPMSL_FIREBALL:
            	   	      power=ZAP_FIREBALL;break;
            	   	    case SPMSL_ICEBALL:
            	   	      power=ZAP_ICEBALL;break;
            	   	    case SPMSL_CONFUSE:  
            	   	       power=ZAP_BALLCONFUSION;break;
            	   	    case SPMSL_PARALYSE:
            	   	       power=ZAP_BALLPARALYSIS;break;   
            	   	    case SPMSL_SLOW:
            	   	       power=ZAP_BALLSLOWING;break;      
            	   	    case SPMSL_POISONED_II:
            	   	       power=ZAP_BALLPOISONING;break;         
            	   	    default:
            	   	      power=-1;
            	   	   }    
            	   	
            	   	set_ident_flags(you.inv[throw_2],ISFLAG_KNOW_TYPE);  
            	   	if (power>-1) zapping(power,40,pbolt);            	   	
            	   }	
            } 	

        // all weapons that use 'throwing' go here..
        if (wepClass == OBJ_WEAPONS
            || (wepClass == OBJ_MISSILES && wepType == MI_STONE))
        {
            // elves with elven weapons
            if (cmp_equip_race( you.inv[throw_2], ISFLAG_ELVEN )
                && player_genus(GENPC_ELVEN))                
            {
                baseHit += 1;
            }
            // also, dwarfven and orcs {alex:18.02.02}
            else if (xtraOps.ax_firing)
                      if  (
                                (cmp_equip_race(you.inv[throw_2], ISFLAG_DWARVEN) && player_genus(GENPC_DWARVEN)) 
                            ||  (cmp_equip_race(you.inv[throw_2], ISFLAG_ORCISH) && you.species==SP_HILL_ORC) 
                          )                            
                {
                   baseHit+=1;	
                }	   

            // give an appropriate 'tohit' -
            // axes are -5
            // daggers are +1
            // spears are -1
            // rocks are 0
            if (wepClass == OBJ_WEAPONS)
            {
            	//new 'to_hit' in AX-Crawl
            	if (xtraOps.ax_firing)
            	    switch (wepType)
                    {
                        case WPN_DAGGER:
                            baseHit += 2;
                            break;
                        case WPN_SPEAR:
                            baseHit = 1;
                            break;
                        case WPN_HAND_AXE: //hand axes also are to be thrown  
                            if (one_chance_in(3)) exercise(SK_AXES, (coinflip()? 2 : 1));
                            baseHit = 0;  
                        default:
                            baseHit -= 5;
                            break;
                    }
            	else
                    switch (wepType)
                    {
                        case WPN_DAGGER:
                            baseHit += 1;
                            break;
                        case WPN_SPEAR:
                            baseHit -= 1;
                            break;
                        default:
                            baseHit -= 5;
                            break;
                    }
            }

            exHitBonus = you.skills[SK_THROWING] * 2;

            baseDam = property( you.inv[throw_2], PWPN_DAMAGE );
            exDamBonus =
                (10 * (you.skills[SK_THROWING] / 2 + you.strength - 10)) / 12;

            // now, exDamBonus is a multiplier.  The full multiplier
            // is applied to base damage,  but only a third is applied
            // to the magical modifier.
            exDamBonus = (exDamBonus * (3 * baseDam + ammoDamBonus)) / 30;
        }

        if (wepClass == OBJ_MISSILES && wepType == MI_DART)
        {
            // give an appropriate 'tohit' & damage
            baseHit = 2;
            baseDam = property( you.inv[throw_2], PWPN_DAMAGE );

            exHitBonus = you.skills[SK_DARTS] * 2;
            exHitBonus += (you.skills[SK_THROWING] * 2) / 3;
            exDamBonus = you.skills[SK_DARTS] / 4;

            // exercise skills
            exercise(SK_DARTS, 1 + random2avg(3, 2));
        }

        // exercise skill
        if (coinflip())
            exercise(SK_THROWING, 1);
    }

    // range, dexterity bonus, possible skill increase for silly throwing
    if (thrown || launched)
    {
        if (wepType == MI_LARGE_ROCK)
        {
            pbolt.range = 1 + random2( you.strength / 5 );
            if (pbolt.range > 9)
                pbolt.range = 9;

            pbolt.rangeMax = pbolt.range;
        }
        else
        {
            pbolt.range = 9;
            pbolt.rangeMax = 9;

            exHitBonus += you.dex / 2;

            // slaying bonuses
            if (!(launched && wepType == MI_NEEDLE))
                exDamBonus += slaying_bonus(PWPN_DAMAGE);

            exHitBonus += slaying_bonus(PWPN_HIT);
        }
    }
    else
    {
        // range based on mass & strength, between 1 and 9
        pbolt.range = you.strength - mass_item( you.inv[throw_2] ) / 10 + 3;
        if (pbolt.range < 1)
            pbolt.range = 1;

        if (pbolt.range > 9)
            pbolt.range = 9;

        // set max range equal to range for this
        pbolt.rangeMax = pbolt.range;

        if (one_chance_in(20))
            exercise(SK_THROWING, 1);

        exHitBonus = you.dex / 4;
    }

    // FINALIZE tohit and damage
    if (exHitBonus >= 0)
        pbolt.hit = baseHit + random2avg(exHitBonus + 1, 2);
    else
        pbolt.hit = baseHit - random2avg(0 - (exHitBonus - 1), 2);

    if (exDamBonus >= 0)
        pbolt.damage = dice_def( 1, baseDam + random2(exDamBonus + 1) );
    else
        pbolt.damage = dice_def( 1, baseDam - random2(0 - (exDamBonus - 1)) );

    // only add bonuses if we're throwing something sensible
    if (thrown || launched || wepClass == OBJ_WEAPONS)
    {
        pbolt.hit += ammoHitBonus + lnchHitBonus;
        pbolt.damage.size += ammoDamBonus + lnchDamBonus;
    }

    // don't do negative damage
    if (pbolt.damage.size < 0)
        pbolt.damage.size = 0;

#if DEBUG_DIAGNOSTICS
    snprintf( info, INFO_SIZE, "H:%d+%d;a%dl%d.  D:%d+%d;a%dl%d -> %d,%d",
        baseHit, exHitBonus, ammoHitBonus, lnchHitBonus,
        baseDam, exDamBonus, ammoDamBonus, lnchDamBonus,
        pbolt.hit, pbolt.damage);

    mpr(info);
#endif

    // create message
    if (launched)
        strcpy(info, "You shoot ");
    else
        strcpy(info, "You throw ");

    quant_name( you.inv[throw_2], 1, DESC_NOCAP_A, str_pass );

    strcat(info, str_pass);
    strcat(info, ".");
    mpr(info);

    // ensure we're firing a 'missile'-type beam
    pbolt.isBeam = false;
    pbolt.isTracer = false;
    pbolt.type=SYM_MISSILE; 
    beam(pbolt, throw_2);

    dec_inv_item_quantity( throw_2, 1 );

    // throwing and blowguns are silent
    if (!launched || you.inv[you.equip[EQ_WEAPON]].base_type == WPN_BLOWGUN)
        alert();

    you.turn_is_over = 1;
}                               // end throw_it()

void puton_ring(void)
{
    bool is_amulet = false;
    int item_slot;

    if (inv_count() < 1)
    {
        canned_msg(MSG_NOTHING_CARRIED);
        return;
    }

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    item_slot = prompt_invent_item( "Put on which piece of jewellery?",
                                    OBJ_JEWELLERY );

    if (item_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    if (item_slot == you.equip[EQ_LEFT_RING]
        || item_slot == you.equip[EQ_RIGHT_RING]
        || item_slot == you.equip[EQ_AMULET])
    {
        mpr("You've already put that on!");
        return;
    }

    if (item_slot == you.equip[EQ_WEAPON])
    {
        mpr("You are wielding that object.");
        return;
    }

    if (you.inv[item_slot].base_type != OBJ_JEWELLERY)
    {
        //jmf: let's not take our inferiority complex out on players, eh? :-p
        //mpr("You're sadly mistaken if you consider that jewellery.")
        mpr("You can only put on jewellery.");
        return;
    }

    is_amulet = (you.inv[item_slot].sub_type >= AMU_RAGE);

    if (!is_amulet)     // ie it's a ring
    {
        if (you.equip[EQ_GLOVES] != -1
            && item_cursed( you.inv[you.equip[EQ_GLOVES]] ))
        {
            mpr("You can't take your gloves off to put on a ring!");
            return;
        }

        if (you.inv[item_slot].base_type == OBJ_JEWELLERY
            && you.equip[EQ_LEFT_RING] != -1
            && you.equip[EQ_RIGHT_RING] != -1)
        {
            // and you are trying to wear body you.equip.
            mpr("You've already put a ring on each hand.");
            return;
        }
    }
    else if (you.equip[EQ_AMULET] != -1)
    {
        strcpy(info, "You are already wearing an amulet.");

        if (one_chance_in(20))
        {
            strcat(info, " And I must say it looks quite fetching.");
        }

        mpr(info);
        return;
    }

    int hand_used = 0;

    if (you.equip[EQ_LEFT_RING] != -1)
        hand_used = 1;

    if (you.equip[EQ_RIGHT_RING] != -1)
        hand_used = 0;

    if (is_amulet)
        hand_used = 2;
    else if (you.equip[EQ_LEFT_RING] == -1 && you.equip[EQ_RIGHT_RING] == -1)
    {
        mpr("Put on which hand (l or r)?", MSGCH_PROMPT);

        int keyin = get_ch();

        if (keyin == 'l')
            hand_used = 0;
        else if (keyin == 'r')
            hand_used = 1;
        else if (keyin == ESCAPE)
            return;
        else
        {
            mpr("You don't have such a hand!");
            return;
        }
    }

    you.equip[ EQ_LEFT_RING + hand_used ] = item_slot;        

    int ident = ID_TRIED_TYPE;

    switch (you.inv[item_slot].sub_type)
    {
    case RING_FIRE:
    case RING_HUNGER:
    case RING_ICE:
    case RING_LIFE_PROTECTION:
    case RING_POISON_RESISTANCE:
    case RING_PROTECTION_FROM_COLD:
    case RING_PROTECTION_FROM_FIRE:
    case RING_PROTECTION_FROM_MAGIC:
    case RING_SUSTAIN_ABILITIES:
    case RING_SUSTENANCE:
    case RING_SLAYING:
    case RING_SEE_INVISIBLE:
    case RING_TELEPORTATION:
    case RING_WIZARDRY:
    case RING_REGENERATION:
        break;

    case RING_PROTECTION:
        you.redraw_armour_class = 1;

        if (you.inv[item_slot].plus != 0)
        {
            ident = ID_KNOWN_TYPE;
        }
        break;

    case RING_INVISIBILITY:
        if (!you.invis)
        {
            mpr("You become transparent for a moment.");
            ident = ID_KNOWN_TYPE;
        }
        break;

    case RING_EVASION:
        you.redraw_evasion = 1;
        if (you.inv[item_slot].plus != 0)
        {
            ident = ID_KNOWN_TYPE;
        }
        break;

    case RING_STRENGTH:
        modify_stat(STAT_STRENGTH, you.inv[item_slot].plus, true);

        if (you.inv[item_slot].plus != 0)
        {
            ident = ID_KNOWN_TYPE;
        }
        break;

    case RING_DEXTERITY:
        modify_stat(STAT_DEXTERITY, you.inv[item_slot].plus, true);

        if (you.inv[item_slot].plus != 0)
        {
            ident = ID_KNOWN_TYPE;
        }
        break;

    case RING_INTELLIGENCE:
        modify_stat(STAT_INTELLIGENCE, you.inv[item_slot].plus, true);

        if (you.inv[item_slot].plus != 0)
        {
            ident = ID_KNOWN_TYPE;
        }
        break;

    case RING_MAGICAL_POWER:
        inc_max_mp(9);
        ident = ID_KNOWN_TYPE;
        break;

    case RING_LEVITATION:
        mpr("You feel buoyant.");

        ident = ID_KNOWN_TYPE;
        break;

    case RING_TELEPORT_CONTROL:
        // XXX: is this safe or should we make it a function -- bwr
        you.attribute[ATTR_CONTROL_TELEPORT]++;
        break;

    case AMU_RAGE:
        mpr("You feel a brief urge to hack something to bits.");

        ident = ID_KNOWN_TYPE;
        break;
    }

    you.turn_is_over = 1;

    if (is_random_artefact( you.inv[item_slot] ))
        use_randart(item_slot);
    else
    {
        // Artefacts have completely different appearance than base types
        // so we don't allow them to make the base types known
        set_ident_type( you.inv[item_slot].base_type, 
                        you.inv[item_slot].sub_type, ident );
    }

    if (ident == ID_KNOWN_TYPE)
        set_ident_flags( you.inv[item_slot], ISFLAG_EQ_JEWELLERY_MASK );

    if (item_cursed( you.inv[item_slot] ))
    {
        snprintf( info, INFO_SIZE, 
                  "Oops, that %s feels deathly cold.", (is_amulet) ? "amulet" 
                                                                   : "ring" );
        mpr(info);
    }

    // cursed or not, we know that since we've put the ring on
    set_ident_flags( you.inv[item_slot], ISFLAG_KNOW_CURSE );

    in_name( item_slot, DESC_INVENTORY_EQUIP, str_pass );
    mpr( str_pass );
    if (you.inv[item_slot].quantity>1) unStack(item_slot);//{alex:12.11.01}   
}                               // end puton_ring()

//If index==-1, prompt the jewellery to remove
//Else remove the item slot "index"
//alex:12.11.01
void remove_ring(int index=-1)
{
    int hand_used = 10;
    int ring_wear_2;
    int equipn;

    if (you.equip[EQ_LEFT_RING] == -1 && you.equip[EQ_RIGHT_RING] == -1
        && you.equip[EQ_AMULET] == -1)
    {
        mpr("You aren't wearing any rings or amulets.");
        return;
    }

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    if (you.equip[EQ_GLOVES] != -1 
        && item_cursed( you.inv[you.equip[EQ_GLOVES]] )
        && you.equip[EQ_AMULET] == -1)
    {
        mpr("You can't take your gloves off to remove any rings!");
        return;
    }

    if (you.equip[EQ_LEFT_RING] != -1 && you.equip[EQ_RIGHT_RING] == -1
        && you.equip[EQ_AMULET] == -1)
    {
        hand_used = 0;
    }

    if (you.equip[EQ_LEFT_RING] == -1 && you.equip[EQ_RIGHT_RING] != -1
        && you.equip[EQ_AMULET] == -1)
    {
        hand_used = 1;
    }

    if (you.equip[EQ_LEFT_RING] == -1 && you.equip[EQ_RIGHT_RING] == -1
        && you.equip[EQ_AMULET] != -1)
    {
        hand_used = 2;
    }

    //if index!=-1 enter to verify that the index is correct
    //The verification is made inside this "if"
    //Alex: 05.10.01
    if (hand_used == 10 ||index==-1)
    {
        equipn = prompt_invent_item( "Remove which piece of jewellery?",
                                         OBJ_JEWELLERY ); 

        if (equipn == PROMPT_ABORT)
        {
            canned_msg( MSG_OK );
            return;
        }

        if (you.inv[equipn].base_type != OBJ_JEWELLERY)
        {
            mpr("That isn't a piece of jewellery.");
            return;
        }

        if (you.equip[EQ_LEFT_RING] == equipn)
            hand_used = 0;
        else if (you.equip[EQ_RIGHT_RING] == equipn)
            hand_used = 1;
        else if (you.equip[EQ_AMULET] == equipn)
            hand_used = 2;
        else
        {
            mpr("You aren't wearing that.");
            return;
        }        
    }
   else
     equipn=index; 

    if (you.equip[EQ_GLOVES] != -1 
        && item_cursed( you.inv[you.equip[EQ_GLOVES]] )
        && (hand_used == 0 || hand_used == 1))
    {
        mpr("You can't take your gloves off to remove any rings!");
        return;
    }

    if (you.equip[hand_used + 7] == -1)
    {
        mpr("I don't think you really meant that.");
        return;
    }

    if (item_cursed( you.inv[you.equip[hand_used + 7]] ))
    {
        mpr("It's stuck to you!");

        set_ident_flags( you.inv[you.equip[hand_used + 7]], ISFLAG_KNOW_CURSE );
        return;
    }

    strcpy(info, "You remove ");
    in_name(you.equip[hand_used + 7], DESC_NOCAP_YOUR, str_pass);
    Stack(you.equip[hand_used + 7]); //{alex:12.11.01}

    strcat(info, str_pass);
    strcat(info, ".");
    mpr(info);

    // I'll still use ring_wear_2 here.
    ring_wear_2 = you.equip[hand_used + 7];
    
    effects_remove_ring(ring_wear_2); //alex:15.03.02

    you.equip[hand_used + 7] = -1;

    you.turn_is_over = 1;
}                               // end remove_ring()

void zap_wand(void)
{
    struct bolt beam;
    struct dist zap_wand;

    int item_slot;

    beam.obviousEffect = false;

    if (inv_count() < 1)
    {
        canned_msg(MSG_NOTHING_CARRIED);
        return;
    }

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    item_slot = prompt_invent_item( "Zap which item?", OBJ_WANDS );    
    if (item_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    if (you.inv[item_slot].base_type != OBJ_WANDS
        || you.inv[item_slot].plus < 1)
    {
        canned_msg(MSG_NOTHING_HAPPENS);
        you.inv[item_slot].flags |= ISFLAG_KNOW_PROPERTIES;
        you.turn_is_over = 1;
        return;
    }

    mpr("Which direction? (*/+ to target)", MSGCH_PROMPT);
    message_current_target();
    direction(zap_wand);
    if (!zap_wand.isValid)
    {
        if (zap_wand.isCancel)
            canned_msg(MSG_OK);
        return;
    }

    if (you.conf)
    {
        zap_wand.tx = you.x_pos + random2(13) - 6;
        zap_wand.ty = you.y_pos + random2(13) - 6;
    }

    // blargh! blech! this is just begging to be a problem ...
    // not to mention work-around after work-around as wands are
    // added, removed, or altered {dlb}:
    char type_zapped = you.inv[item_slot].sub_type;

    if (type_zapped == WAND_ENSLAVEMENT)
        type_zapped = ZAP_ENSLAVEMENT;

    if (type_zapped == WAND_DRAINING)
        type_zapped = ZAP_NEGATIVE_ENERGY;

    if (type_zapped == WAND_DISINTEGRATION)
        type_zapped = ZAP_DISINTEGRATION;

    if (type_zapped == WAND_RANDOM_EFFECTS)
    {
        type_zapped = random2(16);
        if (one_chance_in(20))
            type_zapped = ZAP_NEGATIVE_ENERGY;
        if (one_chance_in(17))
            type_zapped = ZAP_ENSLAVEMENT;
    }

    beam.source_x = you.x_pos;
    beam.source_y = you.y_pos;
    beam.target_x = zap_wand.tx;
    beam.target_y = zap_wand.ty;

    zapping(type_zapped, 40, beam);

    if (beam.obviousEffect == 1 || you.inv[item_slot].sub_type == WAND_FIREBALL)
    {
        if (get_ident_type( you.inv[item_slot].base_type, 
                            you.inv[item_slot].sub_type ) != ID_KNOWN_TYPE)
        {
            set_ident_type( you.inv[item_slot].base_type, 
                            you.inv[item_slot].sub_type, ID_KNOWN_TYPE );

            in_name(item_slot, DESC_INVENTORY_EQUIP, str_pass);
            mpr( str_pass );

            // update if wielding
            if (you.equip[EQ_WEAPON] == item_slot)
                you.wield_change = true;
        }
    }
    else
    {
        set_ident_type( you.inv[item_slot].base_type, 
                        you.inv[item_slot].sub_type, ID_TRIED_TYPE );
    }

    you.inv[item_slot].plus--;

    if (get_ident_type( you.inv[item_slot].base_type, 
                        you.inv[item_slot].sub_type ) == ID_KNOWN_TYPE  
        && (item_ident( you.inv[item_slot], ISFLAG_KNOW_PLUSES )
            || you.skills[SK_ENCHANTMENTS] > 5 + random2(15)))
    {
        if (item_not_ident( you.inv[item_slot], ISFLAG_KNOW_PLUSES ))
        {
            mpr("Your skill with enchantments lets you calculate the power of this device...");
        }

        snprintf( info, INFO_SIZE, "This wand has %d charge%s left.",
                 you.inv[item_slot].plus, 
                 (you.inv[item_slot].plus == 1) ? "" : "s" );

        mpr(info);
        set_ident_flags( you.inv[item_slot], ISFLAG_KNOW_PLUSES );
    }

    you.turn_is_over = 1;
    alert();
}                               // end zap_wand()

void drink(void)
{
    int item_slot;

    if (you.is_undead == US_UNDEAD)
    {
        mpr("You can't drink.");
        return;
    }

    if (grd[you.x_pos][you.y_pos] == DNGN_BLUE_FOUNTAIN
        || grd[you.x_pos][you.y_pos] == DNGN_SPARKLING_FOUNTAIN)
    {
        if (drink_fountain())
            return;
    }

    if (inv_count() < 1)
    {
        canned_msg(MSG_NOTHING_CARRIED);
        return;
    }

    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    item_slot = prompt_invent_item( "Drink which item?", OBJ_POTIONS );
    if (item_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    if (you.inv[item_slot].base_type != OBJ_POTIONS)
    {
        mpr("You can't drink that!");
        return;
    }

    if (potion_effect( you.inv[item_slot].sub_type, 40 ))
    {
        set_ident_flags( you.inv[item_slot], ISFLAG_IDENT_MASK );

        set_ident_type( you.inv[item_slot].base_type, 
                        you.inv[item_slot].sub_type, ID_KNOWN_TYPE );
    }
    else
    {
        set_ident_type( you.inv[item_slot].base_type, 
                        you.inv[item_slot].sub_type, ID_TRIED_TYPE );
    }

    dec_inv_item_quantity( item_slot, 1 );
    you.turn_is_over = 1;

    lessen_hunger(40, true);
}                               // end drink()

bool drink_fountain(void)
{
    bool gone_dry = false;
    int temp_rand;              // for probability determinations {dlb}
    int fountain_effect = POT_WATER;    // for fountain effects {dlb}

    switch (grd[you.x_pos][you.y_pos])
    {
    case DNGN_BLUE_FOUNTAIN:
        if (!yesno("Drink from the fountain?"))
            return false;

        mpr("You drink the pure, clear water.");
        break;

    case DNGN_SPARKLING_FOUNTAIN:
        if (!yesno("Drink from the sparkling fountain?"))
            return false;

        mpr("You drink the sparkling water.");
        break;
    }

    if (grd[you.x_pos][you.y_pos] == DNGN_SPARKLING_FOUNTAIN)
    {
        temp_rand = random2(4500);

        fountain_effect = ((temp_rand > 2399) ? POT_WATER :     // 46.7%
                           (temp_rand > 2183) ? POT_DECAY :     //  4.8%
                           (temp_rand > 2003) ? POT_MUTATION :  //  4.0%
                           (temp_rand > 1823) ? POT_HEALING :   //  4.0%
                           (temp_rand > 1643) ? POT_HEAL_WOUNDS :// 4.0%
                           (temp_rand > 1463) ? POT_SPEED :     //  4.0%
                           (temp_rand > 1283) ? POT_MIGHT :     //  4.0%
                           (temp_rand > 1139) ? POT_DEGENERATION ://3.2%
                           (temp_rand > 1019) ? POT_LEVITATION ://  2.7%
                           (temp_rand > 899) ? POT_POISON :     //  2.7%
                           (temp_rand > 779) ? POT_SLOWING :    //  2.7%
                           (temp_rand > 659) ? POT_PARALYSIS :  //  2.7%
                           (temp_rand > 539) ? POT_CONFUSION :  //  2.7%
                           (temp_rand > 419) ? POT_INVISIBILITY :// 2.7%
                           (temp_rand > 329) ? POT_MAGIC :      //  2.0%
                           (temp_rand > 239) ? POT_RESTORE_ABILITIES ://  2.0%
                           (temp_rand > 149) ? POT_STRONG_POISON ://2.0%
                           (temp_rand > 59) ? POT_BERSERK_RAGE :  //2.0%
                           (temp_rand > 39) ? POT_GAIN_STRENGTH : //0.4%
                           (temp_rand > 19) ? POT_GAIN_DEXTERITY  //0.4%
                                            : POT_GAIN_INTELLIGENCE);//0.4%
    }

    potion_effect(fountain_effect, 100);

    switch (grd[you.x_pos][you.y_pos])
    {
    case DNGN_BLUE_FOUNTAIN:
        if (one_chance_in(20))
            gone_dry = true;
        break;

    case DNGN_SPARKLING_FOUNTAIN:
        if (one_chance_in(10))
        {
            gone_dry = true;
            break;
        }
        else
        {
            temp_rand = random2(50);

            // you won't know it (yet)
            if (temp_rand > 40) // 18% probability
                grd[you.x_pos][you.y_pos] = DNGN_BLUE_FOUNTAIN;
        }
        break;
    }

    if (gone_dry)
    {
        mpr("The fountain dries up!");
        if (grd[you.x_pos][you.y_pos] == DNGN_BLUE_FOUNTAIN)
            grd[you.x_pos][you.y_pos] = DNGN_DRY_FOUNTAIN_I;
        else if (grd[you.x_pos][you.y_pos] == DNGN_SPARKLING_FOUNTAIN)
            grd[you.x_pos][you.y_pos] = DNGN_DRY_FOUNTAIN_II;
    }

    you.turn_is_over = 1;
    return true;
}                               // end drink_fountain()

static void handle_read_book( int item_slot )
{
    int spell, spell_index, nthing;       
        
    if (you.inv[item_slot].sub_type == BOOK_DESTRUCTION)
    {
        if (silenced(you.x_pos, you.y_pos))
        {
            mpr("This book does not work if you cannot read it aloud!");
            return;
        }

        tome_of_power(item_slot);
        return;
    }
    else if (you.inv[item_slot].sub_type == BOOK_MANUAL)
    {
        skill_manual(item_slot);
        return;
    }
    else
    {
        // Little experimental restriction here... eventually, 
        // individual books/spells might have skill requirements 
        // before they can even be read.  Right now it gives a
        // clue as to how to get Spellcasting.  -- bwr
        if (you.skills[SK_SPELLCASTING] == 0)
        {
            mpr( "This text is beyond your capability to read." );
            mpr( "You should start by reading simpler magical texts." );
            return;
        }

        spell = read_book( you.inv[item_slot], RBOOK_READ_SPELL );
    }

    if (spell < 'a' || spell > 'h')     //jmf: was 'g', but 8=h
    {
        mesclr();
        return;
    }

    spell_index = letter_to_index( spell );
#if 0
    // the same as the check below
    if (!is_valid_spell_in_book( item_slot, spell_index ))
    {
        mesclr();
        return;
    }
#endif

    nthing = which_spell_in_book(you.inv[item_slot].sub_type, spell_index);
    if (nthing == SPELL_NO_SPELL)
    {
        mesclr();
        return;
    }

    describe_spell( nthing );
    redraw_screen();

    mesclr();
    return;
}

  	

//Read a scroll or book.
//If bBook==0, read a scrool, else a book
//Before, read_scroll() hadn't any parameter.
//Alex: 03.10.01
void read_scroll(int bBook=0)

{
    int id_the_scroll,n;
     
    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    if (inv_count() < 1)
    {
        canned_msg(MSG_NOTHING_CARRIED);
        return;
    }

    int item_slot;
    
    //alex:12.11.01
    //Distinct between reading books and scrolls
    if (bBook)
        item_slot=prompt_invent_item( "Read which book?", OBJ_BOOKS );
    else
        item_slot=prompt_invent_item( "Read which scroll?", OBJ_SCROLLS );
     
    if (item_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    if (!(you.inv[item_slot].base_type == OBJ_BOOKS
         || you.inv[item_slot].base_type == OBJ_SCROLLS))
    {
        mpr("You can't read that!");
        return;
    }

    // here we try to read a book {dlb}:
    if (you.inv[item_slot].base_type == OBJ_BOOKS)
    {
        handle_read_book( item_slot );
        return;
    }

    if (silenced(you.x_pos, you.y_pos))
    {
        mpr("Magic scrolls do not work when you're silenced!");
        return;
    }
 
    // imperfect vision prevents players from reading actual content {dlb}:
    if (you.mutation[MUT_BLURRY_VISION]
        && random2(5) < you.mutation[MUT_BLURRY_VISION])
    {
        mpr((you.mutation[MUT_BLURRY_VISION] == 3 && one_chance_in(3))
                        ? "This scroll appears to be blank."
                        : "The writing blurs in front of your eyes.");
        return;
    }

    // decrement and handle inventory if any scroll other than paper {dlb}:
    const int scroll_type=you.inv[item_slot].sub_type;
    int effect = aObjs[OBJ_SCROLLS][scroll_type].power;
    if (effect != EFE_PAPER)
    {    	
        mpr("As you read the scroll, it crumbles to dust.");
    }

    
    // scrolls of paper are also exempted from this handling {dlb}:
    if (effect != EFE_PAPER)
    {
    	
    	//AX-crawl has special effects fo reading when confused
        if (you.conf && !xtraOps.ax_confuse_rules)
        {
            random_uselessness(random2(9));
            return;
        }
        
        if (!you.skills[SK_SPELLCASTING])
            exercise(SK_SPELLCASTING, (coinflip()? 2 : 1));
    }
    
    switch(effect)
      {
      	//identify the scroll to avoid identify() show the identify scroll
      	//as identified
      	case EFE_IDENTIFY:
      	  if (!you.conf) 
      	    {
      	      set_ident_type( OBJ_SCROLLS, SCR_IDENTIFY, ID_KNOWN_TYPE );
      	    }
      	  break;    
      }	
    
    if (you.conf)
      {
      	//if confused, you can cast something uselessness      	
      	if (one_chance_in(5))
      	   {
      	     random_uselessness(random2(9));
      	     return;
      	   }	
      	
      	//When confused, you can cast a different scroll      	      	
      	if (one_chance_in(3))
      	  {
      	     handle_scroll(random2(EFE_LAST_SCROLL)); 
      	     return;      	     
      	  }
      	//When confused, you can say what scroll was
        id_the_scroll=false;      	           
                
        effect=aObjs[OBJ_SCROLLS][scroll_type].effect_confused;  
      }	      
    
    n=handle_scroll(effect);
    if (!you.conf) id_the_scroll=n;
    
    set_ident_type( OBJ_SCROLLS, scroll_type, 
                    (id_the_scroll) ? ID_KNOWN_TYPE : ID_TRIED_TYPE );
    dec_inv_item_quantity( item_slot, 1 );                
    
}                               // end read_scroll()


void original_name(void)
{
    int item_slot = prompt_invent_item( "Examine which item?", -1 );
    if (item_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    describe_item( you.inv[item_slot] );
    redraw_screen();
}                               // end original_name()

void use_randart(unsigned char item_wield_2)
{
    ASSERT( is_random_artefact( you.inv[ item_wield_2 ] ) );

    FixedVector< char, RA_PROPERTIES >  proprt;
    randart_wpn_properties( you.inv[item_wield_2], proprt );

    if (proprt[RAP_AC])
        you.redraw_armour_class = 1;

    if (proprt[RAP_EVASION])
        you.redraw_evasion = 1;

    // modify ability scores
    modify_stat( STAT_STRENGTH,     proprt[RAP_STRENGTH],     true );
    modify_stat( STAT_INTELLIGENCE, proprt[RAP_INTELLIGENCE], true );
    modify_stat( STAT_DEXTERITY,    proprt[RAP_DEXTERITY],    true );

    if (proprt[RAP_NOISES])
        you.special_wield = 50 + proprt[RAP_NOISES];
}


/*
   remove_item()
   
   Removes a piece of armour or a ring or a amulet
   that is weared
   
   Alex: 03.10.01
*/   
void remove_item()
  {
   unsigned char keyin; 	
   int item;
   
   keyin=mini_remove_invent("Take off which item?");
   
   if (keyin==27) return;
   item=letter_to_index(keyin);
   if (you.inv[item].base_type==OBJ_ARMOUR)
       {
          
          takeoff_armour(item);
       }   
   else
       remove_ring(item);    
  }	
  
  
  
