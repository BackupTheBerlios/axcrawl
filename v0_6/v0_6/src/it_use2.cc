/*
 *  File:       it_use2.cc
 *  Summary:    Functions for using wands, and weapon/armour removal.4\3
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *  <5>  21.03.02   alx    potions of slow and speed anules each other
 *  
 *
 *      26jun2000   jmf   added ZAP_MAGMA
 *  <4> 19mar2000   jmf   Added ZAP_BACKLIGHT and ZAP_SLEEP
 *  <3>     10/1/99         BCR             Changed messages for speed and
 *                                          made amulet resist slow up speed
 *  <2>     5/20/99         BWR             Fixed bug with RAP_METABOLISM
 *                                          and RAP_NOISES artefacts/
 *  <1>     -/--/--         LRH             Created
 */

#include "AppHdr.h"
#include "it_use2.h"

#include <string.h>

#include "externs.h"

#include "beam.h"
#include "effects.h"
#include "food.h"
#include "itemname.h"
#include "misc.h"
#include "mutation.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "skills2.h"
#include "spells2.h"
#include "spl-cast.h"
#include "stuff.h"
#include "view.h"



void unwield_item(char unw)
{
    you.special_wield = SPWLD_NONE;
    you.wield_change = true;

    if (you.inv[unw].base_type == OBJ_WEAPONS)
    {
        if (is_fixed_artefact( you.inv[unw] ))
        {
            switch (you.inv[unw].special)
            {
            case SPWPN_SINGING_SWORD:
                mpr("The Singing Sword sighs.");
                break;
            case SPWPN_WRATH_OF_TROG:
                mpr("You feel less violent.");
                break;
            case SPWPN_SCYTHE_OF_CURSES:
            case SPWPN_STAFF_OF_OLGREB:
                you.inv[unw].plus = 0;
                you.inv[unw].plus2 = 0;
                break;
            case SPWPN_STAFF_OF_WUCAD_MU:
                you.inv[unw].plus = 0;
                you.inv[unw].plus2 = 0;
                miscast_effect(SPTYP_DIVINATION, 9, 90, 100);
                break;
            default:
                break;
            }

            return;
        }

        int brand = get_weapon_brand( you.inv[unw] );

        if (is_random_artefact( you.inv[unw] ))
            unuse_randart(unw);

        if (brand != SPWPN_NORMAL)
        {
            in_name(unw, DESC_CAP_YOUR, str_pass);
            strcpy(info, str_pass);

            switch (brand)
            {
            case SPWPN_FLAMING:
                strcat(info, " stops flaming.");
                mpr(info);
                break;

            case SPWPN_FREEZING:
            case SPWPN_HOLY_WRATH:
                strcat(info, " stops glowing.");
                mpr(info);
                break;

            case SPWPN_ELECTROCUTION:
                strcat(info, " stops crackling.");
                mpr(info);
                break;

            case SPWPN_VENOM:
                strcat(info, " stops dripping with poison.");
                mpr(info);
                break;

            case SPWPN_PROTECTION:
                mpr("You feel less protected.");
                you.redraw_armour_class = 1;
                break;

            case SPWPN_VAMPIRICISM:
                mpr("You feel the strange hunger wane.");
                break;

            /* case 8: draining
               case 9: speed, 10 slicing etc */

            case SPWPN_DISTORTION:
                // Removing the translocations skill reduction of effect,
                // it might seem sensible, but this brand is supposted
                // to be dangerous because it does large bonus damage,
                // as well as free teleport other side effects, and
                // even with the miscast effects you can rely on the
                // occasional spatial bonus to mow down some opponents.
                // It's far too powerful without a real risk, especially
                // if it's to be allowed as a player spell. -- bwr

                // int effect = 9 - random2avg( you.skills[SK_TRANSLOCATIONS] * 2, 2 );
                miscast_effect(SPTYP_TRANSLOCATION, 9, 90, 100);
                break;

                // when more are added here, *must* duplicate unwielding
                // effect in vorpalise weapon scroll effect in read_scoll
            }                   // end switch


            if (you.duration[DUR_WEAPON_BRAND])
            {
                you.duration[DUR_WEAPON_BRAND] = 0;
                set_item_ego_type( you.inv[unw], OBJ_WEAPONS, SPWPN_NORMAL );
                mpr("Your branding evaporates.");
            }
        }                       // end if
    }

    if (player_equip( EQ_STAFF, STAFF_POWER ))
    {
        // XXX: Ugly hack so that thhis currently works (don't want to
        // mess with the fact that currently this function doesn't 
        // actually unwield the item, but we need it out of the player's
        // hand for this to work. -- bwr
        int tmp = you.equip[ EQ_WEAPON ];

        you.equip[ EQ_WEAPON ] = -1;
        calc_mp();
        you.equip[ EQ_WEAPON ] = tmp;
    }

    return;
}                               // end unwield_item()

// This does *not* call ev_mod!
void unwear_armour(char unw)
{
    you.redraw_armour_class = 1;
    you.redraw_evasion = 1;

    switch (get_armour_ego_type( you.inv[unw] ))
    {
    case SPARM_RUNNING:
        mpr("You feel rather sluggish.");
        break;

    case SPARM_FIRE_RESISTANCE:
        mpr("\"Was it this warm in here before?\"");
        break;

    case SPARM_COLD_RESISTANCE:
        mpr("You catch a bit of a chill.");
        break;

    case SPARM_POISON_RESISTANCE:
        if (!player_res_poison())
            mpr("You feel less healthy.");
        break;

    case SPARM_SEE_INVISIBLE:
        if (!player_see_invis())
            mpr("You feel less perceptive.");
        break;

    case SPARM_DARKNESS:        // I do not understand this {dlb}
        if (you.invis)
            you.invis = 1;
        break;

    case SPARM_STRENGTH:
        modify_stat(STAT_STRENGTH, -3, false);
        break;

    case SPARM_DEXTERITY:
        modify_stat(STAT_DEXTERITY, -3, false);
        break;

    case SPARM_INTELLIGENCE:
        modify_stat(STAT_INTELLIGENCE, -3, false);
        break;

    case SPARM_PONDEROUSNESS:
        mpr("That put a bit of spring back into your step.");
        // you.speed -= 2;
        break;

    case SPARM_LEVITATION:
        //you.levitation++;
        if (you.levitation)
            you.levitation = 1;
        break;

    case SPARM_MAGIC_RESISTANCE:
        mpr("You feel less resistant to magic.");
        break;

    case SPARM_PROTECTION:
        mpr("You feel less protected.");
        break;

    case SPARM_STEALTH:
        mpr("You feel less stealthy.");
        break;

    case SPARM_RESISTANCE:
        mpr("You feel hot and cold all over.");
        break;

    case SPARM_POSITIVE_ENERGY:
        mpr("You feel vulnerable.");
        break;

    case SPARM_ARCHMAGI:
        mpr("You feel strangely numb.");
        break;
    }

    if (is_random_artefact( you.inv[unw] ))
        unuse_randart(unw);

    return;
}                               // end unwear_armour()

void unuse_randart(unsigned char unw)
{
    ASSERT( is_random_artefact( you.inv[unw] ) );

    FixedVector< char, RA_PROPERTIES > proprt;
    randart_wpn_properties( you.inv[unw], proprt );

    if (proprt[RAP_AC])
        you.redraw_armour_class = 1;

    if (proprt[RAP_EVASION])
        you.redraw_evasion = 1;

    // modify ability scores
    modify_stat( STAT_STRENGTH,     -proprt[RAP_STRENGTH],     true );
    modify_stat( STAT_INTELLIGENCE, -proprt[RAP_INTELLIGENCE], true );
    modify_stat( STAT_DEXTERITY,    -proprt[RAP_DEXTERITY],    true );

    if (proprt[RAP_NOISES] != 0)
        you.special_wield = SPWLD_NONE;
}                               // end unuse_randart()
