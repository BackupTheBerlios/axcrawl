/*============================================================================

scrolls.cc

Effects for reading scrolls

Alex: 04.03.02
============================================================================*/
#include "AppHdr.h"
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

#include "options.h" 
#include "objects.h" 
#include "scrolls.h"

bool affix_weapon_enchantment( void )
{
    const int wpn = you.equip[ EQ_WEAPON ];
    bool success = true;

    struct bolt beam;

    if (wpn == -1 || !you.duration[ DUR_WEAPON_BRAND ])
        return (false);

    switch (you.inv[you.equip[EQ_WEAPON]].special)
    {
    case SPWPN_VORPAL:
        if (damage_type( you.inv[ wpn ].base_type,
                         you.inv[ wpn ].sub_type ) != DVORP_CRUSHING)
        {
            strcat(info, "'s sharpness seems more permanent.");
        }
        else
        {
            strcat(info, "'s heaviness feels very stable.");
        }
        mpr(info);
        break;

    case SPWPN_FLAMING:
        strcat(info," is engulfed in an explosion of flames!");
        mpr(info);

        beam.type = SYM_BURST;
        //beam.damage = 110;
        beam.damage = dice_def( 3, 10 );
        beam.flavour = 2;
        beam.target_x = you.x_pos;
        beam.target_y = you.y_pos;
        strcpy(beam.beam_name, "fiery explosion");
        beam.colour = RED;
        beam.thrower = KILL_YOU;
        beam.ex_size = 2;
        beam.isTracer = false;

        explosion(beam);
        break;

    case SPWPN_FREEZING:
        strcat(info," glows brilliantly blue for a moment.");
        mpr(info);
        cast_refrigeration(60);
        break;

    case SPWPN_DRAINING:
        strcat(info," thirsts for the lives of mortals!");
        mpr(info);
        drain_exp();
        break;

    case SPWPN_VENOM:
        strcat(info, " seems more permanently poisoned.");
        mpr(info);
        cast_toxic_radiance();
        break;

    case SPWPN_DISTORTION:
        strcat(info, " twongs alarmingly.");
        mpr(info);

        // from unwield_item
        miscast_effect(SPTYP_TRANSLOCATION, 9, 90, 100);
        break;

    default:
        success = false;
        break;
    }

    if (success)
        you.duration[DUR_WEAPON_BRAND] = 0;

    return (success);
} 
bool enchant_weapon( int which_stat, bool quiet=false )
{
    const int wpn = you.equip[ EQ_WEAPON ];
    bool affected = true;
    int enchant_level;

    if (wpn == -1 
        || (you.inv[ wpn ].base_type != OBJ_WEAPONS
            && you.inv[ wpn ].base_type != OBJ_MISSILES))
    {
        if (!quiet)
            canned_msg(MSG_NOTHING_HAPPENS);

        return (false);
    }

    you.wield_change = true;

    // missiles only have one stat
    if (you.inv[ wpn ].base_type == OBJ_MISSILES)
        which_stat = ENCHANT_TO_HIT;

    if (which_stat == ENCHANT_TO_HIT)
        enchant_level = you.inv[ wpn ].plus;
    else
        enchant_level = you.inv[ wpn ].plus2;

    // artefacts can't be enchanted, but scrolls still remove curses
    if (you.inv[ wpn ].base_type == OBJ_WEAPONS
        && (is_fixed_artefact( you.inv[wpn] )
            || is_random_artefact( you.inv[wpn] )))
    {
        affected = false;
    }

    if (enchant_level >= 4 && random2(9) < enchant_level)
    {
        affected = false;
    }

    // if it isn't affected by the enchantment, it will still
    // be uncursed:
    if (!affected)
    {
        if (item_cursed( you.inv[you.equip[EQ_WEAPON]] ))
        {
            if (!quiet)
            {
                in_name(you.equip[EQ_WEAPON], DESC_CAP_YOUR, str_pass);
                strcpy(info, str_pass);
                strcat(info, " glows silver for a moment.");
                mpr(info);
            }

            do_uncurse_item( you.inv[you.equip[EQ_WEAPON]] );

            return (true);
        }
        else
        {
            if (!quiet)
                canned_msg(MSG_NOTHING_HAPPENS);

            return (false);
        }
    }

    // vVvVv    This is *here* (as opposed to lower down) for a reason!
    in_name( wpn, DESC_CAP_YOUR, str_pass );
    strcpy( info, str_pass );

    do_uncurse_item( you.inv[ wpn ] );

    if (you.inv[ wpn ].base_type == OBJ_WEAPONS)
    {
        if (which_stat == ENCHANT_TO_DAM)
        {
            you.inv[ wpn ].plus2++;

            if (!quiet)
            {
                strcat(info, " glows red for a moment.");
                mpr(info);
            }
        }
        else if (which_stat == ENCHANT_TO_HIT)
        {
            you.inv[ wpn ].plus++;

            if (!quiet)
            {
                strcat(info, " glows green for a moment.");
                mpr(info);
            }
        }
    }
    else if (you.inv[ wpn ].base_type == OBJ_MISSILES)
    {
        strcat( info, (you.inv[ wpn ].quantity > 1) ? " glow"
                                                    : " glows" );

        strcat(info, " red for a moment.");

        you.inv[ wpn ].plus++;
    }

    return (true);
}


//Curses your weapon
//Returns true if all ok
bool curse_weapon()
  {
        int nthing = you.equip[EQ_WEAPON];

        if (nthing == -1
            || you.inv[nthing].base_type != OBJ_WEAPONS
            || item_cursed( you.inv[nthing] ))
        {
            canned_msg(MSG_NOTHING_HAPPENS);
            return false;
        }
        else
        {
            in_name( nthing, DESC_CAP_YOUR, str_pass );
            strcpy(info, str_pass);
            strcat(info, " glows black for a moment.");
            mpr(info);

            do_curse_item( you.inv[nthing] );
            you.wield_change = true;
            return true;
        }  	
  }	  
 

//Scroll of enchant weapon III
//Returns true if all ok 
bool enchant_weapon_III()
  {
        if (you.equip[ EQ_WEAPON ] != -1) 
        {
            if (!affix_weapon_enchantment())
            {
                in_name( you.equip[EQ_WEAPON], DESC_CAP_YOUR, str_pass );
                strcpy( info, str_pass );
                strcat( info, " glows bright yellow for a while." );
                mpr( info );

                enchant_weapon( ENCHANT_TO_HIT, true );

                if (coinflip())
                    enchant_weapon( ENCHANT_TO_HIT, true );

                enchant_weapon( ENCHANT_TO_DAM, true );

                if (coinflip())
                    enchant_weapon( ENCHANT_TO_DAM, true );

                do_uncurse_item( you.inv[you.equip[EQ_WEAPON]] );
                return true;
            }
        }
        else
        {
            canned_msg(MSG_NOTHING_HAPPENS);
            return false;
        }  	
  }	



//Curses a piece of armour
//Returns true if all ok
bool curse_armour()
{
// make sure there's something to curse first
        int count = 0;
        int affected = EQ_WEAPON;
        int i;
        
        for (i = EQ_CLOAK; i <= EQ_BODY_ARMOUR; i++)
        {
            if (you.equip[i] != -1 && item_uncursed( you.inv[you.equip[i]] ))
            {
                count++;
                if (one_chance_in( count ))
                    affected = i;
            }
        }

        if (affected == EQ_WEAPON)
        {
            canned_msg(MSG_NOTHING_HAPPENS);
            return false;
        }

        // make the name _before_ we curse it
        in_name( you.equip[affected], DESC_CAP_YOUR, str_pass );
        do_curse_item( you.inv[you.equip[affected]] );

        strcpy(info, str_pass);
        strcat(info, " glows black for a moment.");
        mpr(info);	
        return true;
}

void scr_paper()
  {
    mpr("This scroll appears to be blank."); 	
  }	
  

void scr_random_uselessness()
  {
    random_uselessness(random2(9));	
  }	  

void scr_summoning()
  {
    if (create_monster(MONS_ABOMINATION_SMALL, ENCH_ABJ_VI, BEH_FRIENDLY,
                           you.x_pos, you.y_pos, MHITNOT, 250) != -1)
        {
            mpr("A horrible Thing appears!");
        }  	  
  }
          
void scr_noise()
  {
    mpr("You hear a loud clanging noise!");
    noisy(25, you.x_pos, you.y_pos);  	
  }	
  
  
void scr_forgetfulness()
  {
     mpr("You feel momentarily disoriented.");
     if (!wearing_amulet(AMU_CLARITY)) forget_map(50 + random2(50));  	
  }	
  
  
bool scr_magic_mapping()
  {
     if (you.level_type == LEVEL_LABYRINTH
            || you.level_type == LEVEL_ABYSS)
        {
            mpr("You feel momentarily disoriented.");
            return false;
        }
        else
        {
            mpr("You feel aware of your surroundings.");
            magic_mapping(21, 95 + random2(10));
            return true;
        }	
  }	   

void scr_torment()
  {
       torment( you.x_pos, you.y_pos );

        // is only naughty if you know you're doing it
        if (get_ident_type( OBJ_SCROLLS, SCR_TORMENT ) == ID_KNOWN_TYPE)
        {
            naughty(NAUGHTY_UNHOLY, 10);
        } 	
  }	
  
void scr_immolation()
  {
  	struct bolt beam;

        // added: scroll effects are never tracers.
        beam.isTracer = false;
        mpr("It explodes in your hands!");

        beam.type = SYM_BURST;
        //beam.damage = 110;
        beam.damage = dice_def( 3, 10 );
        
        // unsure about this    // BEAM_EXPLOSION instead? {dlb}
        beam.flavour = BEAM_FIRE;
        beam.target_x = you.x_pos;
        beam.target_y = you.y_pos;
        strcpy(beam.beam_name, "fiery explosion");
        beam.colour = RED;
        // your explosion, (not someone else's explosion)
        beam.thrower = KILLED_BY_SCR_IMMOLATION;
        beam.ex_size = 2;

        explosion(beam);  	
  }	  
       
       
void scr_vorpal_weapon(int confuse_effect)
  {
        int nthing = you.equip[EQ_WEAPON];
        if (nthing == -1
            || you.inv[ nthing ].base_type != OBJ_WEAPONS
            || (you.inv[ nthing ].base_type == OBJ_WEAPONS
                && (is_fixed_artefact( you.inv[ nthing ] )
                    || is_random_artefact( you.inv[ nthing ] )
                    || launches_things( you.inv[ nthing ].sub_type ))))
        {
            canned_msg(MSG_NOTHING_HAPPENS);
            return;
        }

        in_name(nthing, DESC_CAP_YOUR, str_pass);

        strcpy(info, str_pass);
        strcat(info, " emits a brilliant flash of light!");
        mpr(info);
        alert();

        //if confused, the scroll affects special weapons
        if (you.inv[you.equip[EQ_WEAPON]].special != SPWPN_NORMAL && !confuse_effect)
        {
            mpr("You feel strangely frustrated.");
            return;
        }

        you.wield_change = true;
        you.inv[nthing].special = SPWPN_VORPAL;  	
  }	
  
  
bool scr_recharging()
 {
        int nthing = you.equip[EQ_WEAPON];

        if (nthing != -1
            && you.inv[ nthing ].base_type == OBJ_WEAPONS
            && you.inv[ nthing ].special == SPWPN_ELECTROCUTION)
        {
            return enchant_weapon( ENCHANT_TO_DAM );
        }

        if (!recharge_wand())
        {
            canned_msg(MSG_NOTHING_HAPPENS);
            return false;
        }	
 } 	          
 
void scr_invulnerability()
 {
    mpr("You can't be harmmed");
    you.duration[DUR_INVULNERABILTY]+=5+random2(10);	    	
 } 
 	 
 	 
void scr_detect()
 {
   your_spells(SPELL_DETECT_TRAPS,50,0);
   your_spells(SPELL_DETECT_CREATURES,50,0); 	
 } 
 
 

bool enchant_armour( void )
{
    // NOTE: It is assumed that armour which changes in this way does
    // not change into a form of armour with a different evasion modifier.
    int nthing = you.equip[EQ_BODY_ARMOUR];

    if (nthing != -1
        && (you.inv[nthing].sub_type == ARM_DRAGON_HIDE
            || you.inv[nthing].sub_type == ARM_ICE_DRAGON_HIDE
            || you.inv[nthing].sub_type == ARM_STEAM_DRAGON_HIDE
            || you.inv[nthing].sub_type == ARM_MOTTLED_DRAGON_HIDE
            || you.inv[nthing].sub_type == ARM_STORM_DRAGON_HIDE
            || you.inv[nthing].sub_type == ARM_GOLD_DRAGON_HIDE
            || you.inv[nthing].sub_type == ARM_SWAMP_DRAGON_HIDE
            || you.inv[nthing].sub_type == ARM_TROLL_HIDE))
    {
        in_name( you.equip[EQ_BODY_ARMOUR], DESC_CAP_YOUR, str_pass );
        strcpy(info, str_pass);
        strcat(info, " glows purple and changes!");
        mpr(info);

        you.redraw_armour_class = 1;

        hide2armour( &(you.inv[nthing].sub_type) );
        return (true);
    }

    // pick random piece of armour
    int count = 0;
    int affected_slot = EQ_WEAPON;

    for (int i = EQ_CLOAK; i <= EQ_BODY_ARMOUR; i++) 
    {
        if (you.equip[i] != -1)
        {
            count++;
            if (one_chance_in( count ))
                affected_slot = i;
        }
    } 

    // no armour == no enchantment
    if (affected_slot == EQ_WEAPON)
    {
        canned_msg(MSG_NOTHING_HAPPENS);
        return (false);
    }

    bool affected = true;
    item_def &item = you.inv[you.equip[ affected_slot ]];

    if (is_random_artefact( item )
        || ((item.sub_type >= ARM_CLOAK && item.sub_type <= ARM_BOOTS)
            && item.plus >= 2)
        || ((item.sub_type == ARM_SHIELD 
                || item.sub_type == ARM_BUCKLER
                || item.sub_type == ARM_LARGE_SHIELD)
            && item.plus >= 2)
        || (item.plus >= 3 && random2(8) < item.plus))
    {
        affected = false;
    }

    // even if not affected, it may be uncursed.
    if (!affected)
    {
        if (item_cursed( item ))
        {
            in_name(you.equip[affected], DESC_CAP_YOUR, str_pass);
            strcpy(info, str_pass);
            strcat(info, " glows silver for a moment.");
            mpr(info);

            do_uncurse_item( you.inv[you.equip[affected]] );
            return (true);
        }
        else
        {
            canned_msg( MSG_NOTHING_HAPPENS );
            return (false);
        }
    }

    // vVvVv    This is *here* for a reason!
    item_name(item, DESC_CAP_YOUR, str_pass);
    strcpy(info, str_pass);
    strcat(info, " glows green for a moment.");
    mpr(info);

    item.plus++;

    do_uncurse_item( item );
    you.redraw_armour_class = 1;
    return (true);
} 	 	 
//Handle a scroll
//effects for reading a scroll  
bool handle_scroll(int effect)
{   
	
// ok - now we FINALLY get to read a scroll !!! {dlb}
    you.turn_is_over = 1;
    bool id_the_scroll = true;  // to prevent unnecessary repetition
    
    // it is the exception, not the rule, that
    // the scroll will not be identified {dlb}:
    switch (effect)
    {
    case EFE_PAPER:
        scr_paper();
        break;

    case EFE_RANDOM_USELESSNESS:
        scr_random_uselessness();
        break;

    case EFE_BLINK:
        blink();
        break;
    case EFE_RANDOM_BLINK:
        random_blink(true);
        break;
    case EFE_TELEPORT:
        you_teleport();
        break;
 
    case EFE_REMOVE_CURSE:
        id_the_scroll=remove_curse(false);
        break;

    case EFE_DETECT_CURSE:
        id_the_scroll =detect_curse(false);
        break;

    case EFE_ACQUIREMENT:
        acquirement(OBJ_RANDOM);
        break;

    case EFE_MASS_FEAR:
        id_the_scroll=mass_enchantment(ENCH_FEAR, 1000, MHITYOU);
        break;
        
    case EFE_MASS_CONFUSION:
        id_the_scroll=mass_enchantment(ENCH_CONFUSION, 1000, MHITYOU);
        break;
        
    case EFE_NOISE:
        scr_noise();          
        break;

    case EFE_SUMMONING:
        scr_summoning();          
        break;

    case EFE_FORGETFULNESS:
        scr_forgetfulness();  	        
        break;

    case EFE_MAGIC_MAPPING:
        id_the_scroll=scr_magic_mapping();          
        break;

    case EFE_TORMENT:
        scr_torment();           
        break;

    case EFE_IMMOLATION:
        scr_immolation();
        break;

    case EFE_IDENTIFY:
        identify(-1);
        you.wield_change = true;
        break;
        
    case EFE_IDENTIFY_II:
        identify(-2);
        you.wield_change = true;
        break;
     
    case EFE_CURSE_WEAPON:
        id_the_scroll=curse_weapon();
        break;

    case EFE_ENCHANT_TO_HIT:
        id_the_scroll = enchant_weapon( ENCHANT_TO_HIT );
        break;

    case EFE_ENCHANT_TO_DAM:
        id_the_scroll = enchant_weapon( ENCHANT_TO_DAM );
        break;

    case EFE_ENCHANT_WEAPON_III:
        id_the_scroll=enchant_weapon_III();
        break;

    case EFE_VORPALISE:
        scr_vorpal_weapon(false);        
        break;
        
    case EFE_SURE_VORPALISE:
        scr_vorpal_weapon(true);        
        break;    
        
    case EFE_RECHARGING:
        id_the_scroll=scr_recharging();
        break;

    case EFE_ENCHANT_ARMOUR:
        id_the_scroll = enchant_armour();
        break;

    case EFE_CURSE_ARMOUR:
        id_the_scroll=curse_armour();
        break;
    
    case EFE_INVULNERABILITY:
        scr_invulnerability();        
        break;
        
    case EFE_DETECT_DANGER:
        scr_detect();        
        break;   
    }                           // end switch
    
    return id_the_scroll;
}  	  
