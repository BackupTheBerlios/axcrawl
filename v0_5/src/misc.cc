/*
 *  File:       misc.cc
 *  Summary:    Misc functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *   <3>   11/14/99      cdl    evade with random40(ev) vice random2(ev)
 *   <2>    5/20/99      BWR    Multi-user support, new berserk code.
 *   <1>    -/--/--      LRH    Created
 */


#include "AppHdr.h"
#include "misc.h"

#include <string.h>
#if !(defined(__IBMCPP__) || defined(__BCPLUSPLUS__))
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "cloud.h"
#include "fight.h"
#include "files.h"
#include "food.h"
#include "remove.h"
#include "potions.h"
#include "items.h"
#include "itemname.h"
#include "lev-pand.h"
#include "monplace.h"
#include "mon-util.h"
#include "monstuff.h"
#include "ouch.h"
#include "player.h"
#include "shopping.h"
#include "skills.h"
#include "skills2.h"
#include "spells3.h"
#include "spl-cast.h"
#include "stuff.h"
#include "transfor.h"
#include "view.h"

#ifdef MACROS
#include "macro.h"
#endif

bool scramble(void);
bool trap_item(char base_type, char sub_type, char beam_x, char beam_y);
static void dart_trap(bool trap_known, int trapped, struct bolt &pbolt);

// void place_chunks(int mcls, unsigned char rot_status, unsigned char chx,
//                   unsigned char chy, unsigned char ch_col)
void turn_corpse_into_chunks( item_def &item )
{
    const int mons_class = item.plus; 
    const int max_chunks = mons_weight( mons_class ) / 150;

    ASSERT( item.base_type == OBJ_CORPSES );

    item.base_type = OBJ_FOOD;
    item.sub_type = FOOD_CHUNK;
    item.quantity = 1 + random2( max_chunks );

    item.quantity = stepdown_value( item.quantity, 4, 4, 12, 12 );

    // seems to me that this should come about only
    // after the corpse has been butchered ... {dlb}
    if (monster_descriptor( mons_class, MDSC_LEAVES_HIDE ) && !one_chance_in(3))
    {
        int o = get_item_slot( 100 + random2(200) );
        if (o == NON_ITEM)
            return;

        mitm[o].quantity = 1;

        // these values are common to all: {dlb}
        mitm[o].base_type = OBJ_ARMOUR;
        mitm[o].plus = 0;
        mitm[o].plus2 = 0;
        mitm[o].special = 0;
        mitm[o].flags = 0;
        mitm[o].colour = mons_colour( mons_class );

        // these values cannot be set by a reasonable formula: {dlb}
        switch (mons_class)
        {
        case MONS_DRAGON:
            mitm[o].sub_type = ARM_DRAGON_HIDE;
            break;
        case MONS_TROLL:
            mitm[o].sub_type = ARM_TROLL_HIDE;
            break;
        case MONS_ICE_DRAGON:
            mitm[o].sub_type = ARM_ICE_DRAGON_HIDE;
            break;
        case MONS_STEAM_DRAGON:
            mitm[o].sub_type = ARM_STEAM_DRAGON_HIDE;
            break;
        case MONS_MOTTLED_DRAGON:
            mitm[o].sub_type = ARM_MOTTLED_DRAGON_HIDE;
            break;
        case MONS_STORM_DRAGON:
            mitm[o].sub_type = ARM_STORM_DRAGON_HIDE;
            break;
        case MONS_GOLDEN_DRAGON:
            mitm[o].sub_type = ARM_GOLD_DRAGON_HIDE;
            break;
        case MONS_SWAMP_DRAGON:
            mitm[o].sub_type = ARM_SWAMP_DRAGON_HIDE;
            break;
        default:
            // future implementation {dlb}
            mitm[o].sub_type = ARM_ANIMAL_SKIN;
            break;
        }

        move_item_to_grid( &o, item.x, item.y ); 
    }
}                               // end place_chunks()

char search_around(void)
{
    char srx = 0;
    char sry = 0;
    int i;

    for (srx = you.x_pos - 1; srx < you.x_pos + 2; srx++)
    {
        for (sry = you.y_pos - 1; sry < you.y_pos + 2; sry++)
        {
            // don't exclude own square; may be levitating
            if (grd[srx][sry] == DNGN_SECRET_DOOR
                && random2(17) <= 1 + you.skills[SK_TRAPS_DOORS])
            {
                grd[srx][sry] = DNGN_CLOSED_DOOR;
                mpr("You found a secret door!");
                exercise(SK_TRAPS_DOORS, ((coinflip())? 2 : 1));
            }

            if (grd[srx][sry] == DNGN_UNDISCOVERED_TRAP
                && random2(17) <= 1 + you.skills[SK_TRAPS_DOORS])
            {
                i = trap_at_xy(srx, sry);

                if (i != -1)
                    grd[srx][sry] = trap_category(env.trap[i].type);

                mpr("You found a trap!");
            }
        }
    }

    you.turn_is_over = 1;
    return 1;
}                               // end search_around()

void in_a_cloud(void)
{
    int cl = env.cgrid[you.x_pos][you.y_pos];
    int hurted = 0;
    int resist;

    if (you.duration[DUR_CONDENSATION_SHIELD] > 0)
    {
        mpr("Your icy shield dissipates!", MSGCH_DURATION);
        you.duration[DUR_CONDENSATION_SHIELD] = 0;
        you.redraw_armour_class = 1;
    }

    switch (env.cloud[cl].type)
    {
    case CLOUD_FIRE:
    case CLOUD_FIRE_MON:
        if (you.fire_shield)
            return;

        mpr("You are engulfed in roaring flames!");

        resist = player_res_fire();

        if (resist <= 0)
        {
            hurted += ((random2avg(23, 3) + 10) * you.time_taken) / 10;

            if (resist < 0)
                hurted += ((random2avg(14, 2) + 3) * you.time_taken) / 10;

            hurted -= random2(player_AC());

            if (hurted < 1)
                hurted = 0;
            else
                ouch( hurted, cl, KILLED_BY_CLOUD );
        }
        else
        {
            canned_msg(MSG_YOU_RESIST);
            hurted += ((random2avg(23, 3) + 10) * you.time_taken) / 10;
            hurted /= (1 + resist * resist);
            ouch( hurted, cl, KILLED_BY_CLOUD );
        }
        scrolls_burn(7, OBJ_SCROLLS);
        break;

    case CLOUD_STINK:
    case CLOUD_STINK_MON:
        // If you don't have to breathe, unaffected
        mpr("You are engulfed in noxious fumes!");
        if (player_res_poison())
            break;

        hurted += (random2(3) * you.time_taken) / 10;
        if (hurted < 1)
            hurted = 0;
        else
            ouch((hurted * you.time_taken) / 10, cl, KILLED_BY_CLOUD);

        if (1 + random2(27) >= you.experience_level)
        {
            mpr("You choke on the stench!");
            confuse_player( (coinflip() ? 3 : 2) );
        }
        break;

    case CLOUD_COLD:
    case CLOUD_COLD_MON:
        mpr("You are engulfed in freezing vapours!");

        resist = player_res_cold();

        if (resist <= 0)
        {
            hurted += ((random2avg(23, 3) + 10) * you.time_taken) / 10;

            if (resist < 0)
                hurted += ((random2avg(14, 2) + 3) * you.time_taken) / 10;

            hurted -= random2(player_AC());
            if (hurted < 0)
                hurted = 0;

            ouch((hurted * you.time_taken) / 10, cl, KILLED_BY_CLOUD);
        }
        else
        {
            canned_msg(MSG_YOU_RESIST);
            hurted += ((random2avg(23, 3) + 10) * you.time_taken) / 10;
            hurted /= (1 + resist * resist);
            ouch(hurted, cl, KILLED_BY_CLOUD);
        }
        scrolls_burn(7, OBJ_POTIONS);
        break;

    case CLOUD_POISON:
    case CLOUD_POISON_MON:
        // If you don't have to breathe, unaffected
        mpr("You are engulfed in poison gas!");
        if (!player_res_poison())
        {
            ouch((random2(10) * you.time_taken) / 10, cl, KILLED_BY_CLOUD);
            poison_player(1);
        }
        break;

    case CLOUD_GREY_SMOKE:
    case CLOUD_BLUE_SMOKE:
    case CLOUD_PURP_SMOKE:
    case CLOUD_BLACK_SMOKE:
    case CLOUD_GREY_SMOKE_MON:
    case CLOUD_BLUE_SMOKE_MON:
    case CLOUD_PURP_SMOKE_MON:
    case CLOUD_BLACK_SMOKE_MON:
        mpr("You are engulfed in a cloud of smoke!");
        break;

    case CLOUD_STEAM:
    case CLOUD_STEAM_MON:
        mpr("You are engulfed in a cloud of scalding steam!");
        if (you.species == SP_PALE_DRACONIAN && you.experience_level > 5)
        {
            mpr("It doesn't seem to affect you.");
            return;
        }

        if (!player_equip( EQ_BODY_ARMOUR, ARM_STEAM_DRAGON_ARMOUR ))
        {
            mpr("It doesn't seem to affect you.");
            return;
        }

        hurted += (random2(6) * you.time_taken) / 10;
        if (hurted < 0 || player_res_fire() > 0)
            hurted = 0;

        ouch((hurted * you.time_taken) / 10, cl, KILLED_BY_CLOUD);
        break;

    case CLOUD_MIASMA:
    case CLOUD_MIASMA_MON:
        mpr("You are engulfed in a dark miasma.");

        if (player_prot_life() > random2(3))
            return;

        poison_player(1);

        hurted += (random2avg(12, 3) * you.time_taken) / 10;    // 3

        if (hurted < 0)
            hurted = 0;

        ouch(hurted, cl, KILLED_BY_CLOUD);
        potion_effect(POT_SLOWING, 5);

        if (you.hp_max > 4 && coinflip())
            rot_hp(1);

        break;
    }

    return;
}                               // end in_a_cloud()


void merfolk_start_swimming(void)
{
    FixedVector < char, 8 > removed;

    if (you.attribute[ATTR_TRANSFORMATION] != TRAN_NONE)
        untransform();

    for (int i = EQ_WEAPON; i < EQ_RIGHT_RING; i++)
    {
        removed[i] = 0;
    }

    if (you.equip[EQ_BOOTS] != -1)
        removed[EQ_BOOTS] = 1;

    // Perhaps a bit to easy for the player, but we allow merfolk
    // to slide out of heavy body armour freely when entering water,
    // rather than handling emcumbered swimming. -- bwr
    if (!player_light_armour())
    {
        // Can't slide out of just the body armour, cloak comes off -- bwr
        if (you.equip[EQ_CLOAK])
            removed[EQ_CLOAK] = 1;

        removed[EQ_BODY_ARMOUR] = 1;
    }

    remove_equipment(removed);
}



static void dart_trap(bool trap_known, int trapped, struct bolt &pbolt)
{
    int damage_taken = 0;
    int trap_hit, your_dodge;

    if (random2(10) < 2 || (trap_known && !one_chance_in(4)))
    {
        strcpy(info, "You avoid triggering a");
        strcat(info, pbolt.beam_name);
        strcat(info, " trap.");
        mpr(info);
        return;
    }

    if (you.equip[EQ_SHIELD] != -1)
        exercise(SK_SHIELDS, (random2(3)) / 2);

    strcpy(info, "A");
    strcat(info, pbolt.beam_name);
    strcat(info, " shoots out and ");

    if (random2(50 + 10 * you.shield_blocks * you.shield_blocks) 
                                                < player_shield_class())
    {
        you.shield_blocks++;
        strcat(info, "hits your shield.");
        mpr(info);
        goto out_of_trap;
    }

    // note that this uses full ( not random2limit(foo,40) ) player_evasion.
    trap_hit = (20 + (you.your_level * 2)) * random2(200) / 100;

    your_dodge = player_evasion() + random2(you.dex) / 3
                            - 2 + (you.duration[DUR_REPEL_MISSILES] * 10);

    if (trap_hit >= your_dodge && you.duration[DUR_DEFLECT_MISSILES] == 0)
    {
        strcat(info, "hits you!");
        mpr(info);

        if ((strstr( pbolt.beam_name, "needle" ) != NULL)
                && random2(100) < 50 - (3 * player_AC()) / 2
                && !player_res_poison())
        {
            poison_player( 1 + random2(3) );
        }

        damage_taken = roll_dice( pbolt.damage );
        damage_taken -= random2(player_AC() + 1);

        if (damage_taken > 0)
            ouch(damage_taken, 0, KILLED_BY_TRAP);
    }
    else
    {
        strcat(info, "misses you.");
        mpr(info);
    }

    if (player_light_armour() && coinflip())
        exercise(SK_DODGING, 1);

  out_of_trap:

    pbolt.target_x = you.x_pos;
    pbolt.target_y = you.y_pos;

    if (coinflip())
        itrap(pbolt, trapped);
}                               // end dart_trap()

//
// itrap takes location from target_x, target_y of bolt strcture.
//

void itrap( struct bolt &pbolt, int trapped )
{
    int base_type = OBJ_MISSILES;
    int sub_type = MI_DART;

    switch (env.trap[trapped].type)
    {
    case TRAP_DART:
        base_type = OBJ_MISSILES;
        sub_type = MI_DART;
        break;
    case TRAP_ARROW:
        base_type = OBJ_MISSILES;
        sub_type = MI_ARROW;
        break;
    case TRAP_BOLT:
        base_type = OBJ_MISSILES;
        sub_type = MI_BOLT;
        break;
    case TRAP_SPEAR:
        base_type = OBJ_WEAPONS;
        sub_type = WPN_SPEAR;
        break;
    case TRAP_AXE:
        base_type = OBJ_WEAPONS;
        sub_type = WPN_HAND_AXE;
        break;
    case TRAP_NEEDLE:
        base_type = OBJ_MISSILES;
        sub_type = MI_NEEDLE;
        break;
    default:
        return;
    }

    trap_item( base_type, sub_type, pbolt.target_x, pbolt.target_y );

    return;
}                               // end itrap()

void handle_traps(char trt, int i, bool trap_known)
{
    struct bolt beam;

    switch (trt)
    {
    case TRAP_DART:
        strcpy(beam.beam_name, " dart");
        beam.damage = dice_def( 1, 4 + (you.your_level / 2) );
        dart_trap(trap_known, i, beam);
        break;

    case TRAP_NEEDLE:
        strcpy(beam.beam_name, " needle");
        beam.damage = dice_def( 1, 0 );
        dart_trap(trap_known, i, beam);
        break;

    case TRAP_ARROW:
        strcpy(beam.beam_name, "n arrow");
        beam.damage = dice_def( 1, 7 + you.your_level );
        dart_trap(trap_known, i, beam);
        break;

    case TRAP_BOLT:
        strcpy(beam.beam_name, " bolt");
        beam.damage = dice_def( 1, 13 + you.your_level );
        dart_trap(trap_known, i, beam);
        break;

    case TRAP_SPEAR:
        strcpy(beam.beam_name, " spear");
        beam.damage = dice_def( 1, 10 + you.your_level );
        dart_trap(trap_known, i, beam);
        break;

    case TRAP_AXE:
        strcpy(beam.beam_name, "n axe");
        beam.damage = dice_def( 1, 15 + you.your_level );
        dart_trap(trap_known, i, beam);
        break;

    case TRAP_TELEPORT:
        mpr("You enter a teleport trap!");

        if (scan_randarts(RAP_PREVENT_TELEPORTATION))
            mpr("You feel a weird sense of stasis.");
        else
            you_teleport2(true);
        break;

    case TRAP_AMNESIA:
        mpr("You feel momentarily disoriented.");
        if (!wearing_amulet(AMU_CLARITY))
            forget_map(random2avg(100, 2));
        break;

    case TRAP_BLADE:
        if (trap_known && one_chance_in(3))
            mpr("You avoid triggering a blade trap.");
        else if (random2limit(player_evasion(), 40)
                        + (random2(you.dex) / 3) + (trap_known ? 3 : 0) > 8)
        {
            mpr("A huge blade swings just past you!");
        }
        else
        {
            mpr("A huge blade swings out and slices into you!");
            ouch( (you.your_level * 2) + random2avg(29, 2)
                    - random2(1 + player_AC()), 0, KILLED_BY_TRAP );
        }
        break;

    case TRAP_ZOT:
    default:
        mpr((trap_known) ? "You enter the Zot trap."
                         : "Oh no! You have blundered into a Zot trap!");
        miscast_effect( SPTYP_RANDOM, random2(30) + you.your_level,
                        75 + random2(100), 3 );
        break;
    }
}                               // end handle_traps()

void disarm_trap( struct dist &disa )
{
    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    int i, j;

    for (i = 0; i < MAX_TRAPS; i++)
    {
        if (env.trap[i].x == you.x_pos + disa.dx
            && env.trap[i].y == you.y_pos + disa.dy)
        {
            break;
        }

        if (i == MAX_TRAPS - 1)
        {
            mpr("Error - couldn't find that trap.");
            return;
        }
    }

    if (trap_category(env.trap[i].type) == DNGN_TRAP_MAGICAL)
    {
        mpr("You can't disarm that trap.");
        return;
    }

    if (random2(you.skills[SK_TRAPS_DOORS] + 2) <=
        random2(you.your_level + 5))
    {
        mpr("You failed to disarm the trap.");

        you.turn_is_over = 1;

        if (random2(you.dex) > 5 + random2(5 + you.your_level))
            exercise(SK_TRAPS_DOORS, 1 + random2(you.your_level / 5));
        else
        {
            handle_traps(env.trap[i].type, i, false);

            if (coinflip())
                exercise(SK_TRAPS_DOORS, 1);
        }

        return;
    }

    mpr("You have disarmed the trap.");

    struct bolt beam;

    beam.target_x = you.x_pos + disa.dx;
    beam.target_y = you.y_pos + disa.dy;

    if (env.trap[i].type != TRAP_BLADE
        && trap_category(env.trap[i].type) == DNGN_TRAP_MECHANICAL)
    {
        for (j = 0; j < 20; j++)
        {
            // places items (eg darts), which will automatically stack
            itrap(beam, i);

            if (j > 10 && one_chance_in(3))
                break;
        }
    }

    grd[you.x_pos + disa.dx][you.y_pos + disa.dy] = DNGN_FLOOR;
    env.trap[i].type = TRAP_UNASSIGNED;
    you.turn_is_over = 1;

    // reduced from 5 + random2(5)
    exercise(SK_TRAPS_DOORS, 1 + random2(5) + (you.your_level / 5));
}                               // end disarm_trap()

void manage_clouds(void)
{
    // amount which cloud dissipates - must be unsigned! {dlb}
    unsigned int dissipate = 0;

    for (unsigned char cc = 0; cc < MAX_CLOUDS; cc++)
    {
        if (env.cloud[cc].type == CLOUD_NONE)   // no cloud -> next iteration
            continue;

        dissipate = you.time_taken;

        // water -> flaming clouds:
        // lava -> freezing clouds:
        if ((env.cloud[cc].type == CLOUD_FIRE
                || env.cloud[cc].type == CLOUD_FIRE_MON)
            && grd[env.cloud[cc].x][env.cloud[cc].y] == DNGN_DEEP_WATER)
        {
            dissipate *= 4;
        }
        else if ((env.cloud[cc].type == CLOUD_COLD
                    || env.cloud[cc].type == CLOUD_COLD_MON)
                && grd[env.cloud[cc].x][env.cloud[cc].y] == DNGN_LAVA)
        {
            dissipate *= 4;
        }

        // double the amount when slowed - must be applied last(!):
        if (you.slow)
            dissipate *= 2;

        // apply calculated rate to the actual cloud:
        env.cloud[cc].decay -= dissipate;

        // check for total dissipatation and handle accordingly:
        if (env.cloud[cc].decay < 1)
            delete_cloud( cc );
    }

    return;
}                               // end manage_clouds()

void weird_writing(char stringy[40])
{
    int temp_rand = 0;          // for probability determinations {dlb}

    temp_rand = random2(15);

    // you'll see why later on {dlb}
    strcpy(stringy, (temp_rand == 0) ? "writhing" :
                    (temp_rand == 1) ? "bold" :
                    (temp_rand == 2) ? "faint" :
                    (temp_rand == 3) ? "spidery" :
                    (temp_rand == 4) ? "blocky" :
                    (temp_rand == 5) ? "angular" :
                    (temp_rand == 6) ? "shimmering" :
                    (temp_rand == 7) ? "glowing" : "");

    if (temp_rand < 8)
        strcat(stringy, " ");   // see above for reasoning {dlb}

    temp_rand = random2(14);

    strcat(stringy, (temp_rand ==  0) ? "yellow" :
                    (temp_rand ==  1) ? "brown" :
                    (temp_rand ==  2) ? "black" :
                    (temp_rand ==  3) ? "purple" :
                    (temp_rand ==  4) ? "orange" :
                    (temp_rand ==  5) ? "lime-green" :
                    (temp_rand ==  6) ? "blue" :
                    (temp_rand ==  7) ? "grey" :
                    (temp_rand ==  8) ? "silver" :
                    (temp_rand ==  9) ? "gold" :
                    (temp_rand == 10) ? "umber" :
                    (temp_rand == 11) ? "charcoal" :
                    (temp_rand == 12) ? "pastel" :
                    (temp_rand == 13) ? "mauve"
                                      : "colourless");

    strcat(stringy, " ");

    temp_rand = random2(14);

    strcat(stringy, (temp_rand == 0) ? "writing" :
                    (temp_rand == 1) ? "scrawl" :
                    (temp_rand == 2) ? "sigils" :
                    (temp_rand == 3) ? "runes" :
                    (temp_rand == 4) ? "hieroglyphics" :
                    (temp_rand == 5) ? "scrawl" :
                    (temp_rand == 6) ? "print-out" :
                    (temp_rand == 7) ? "binary code" :
                    (temp_rand == 8) ? "glyphs" :
                    (temp_rand == 9) ? "symbols"
                                     : "text");

    return;
}                               // end weird_writing()

// must be a better name than 'place' for the first parameter {dlb}
void fall_into_a_pool(bool place, unsigned char terrain)
{
    bool escape = false;
    FixedVector< char, 2 > empty;

    if (you.species == SP_MERFOLK && terrain == DNGN_DEEP_WATER)
    {
        // These can happen when we enter deep water directly -- bwr
        merfolk_start_swimming();
        return;
    }

    strcpy(info, "You fall into the ");

    strcat(info, (terrain == DNGN_LAVA)       ? "lava" :
                 (terrain == DNGN_DEEP_WATER) ? "water"
                                              : "programming rift");

    strcat(info, "!");
    mpr(info);

    more();
    mesclr();

    if (terrain == DNGN_LAVA)
    {
        const int resist = player_res_fire();

        if (resist <= 0)
        {
            mpr( "The lava burns you to a cinder!" );
            ouch( -9999, 0, KILLED_BY_LAVA );
        }
        else
        {
            // should boost # of bangs per damage in the future {dlb}
            mpr( "The lava burns you!" );
            ouch( (10 + random2avg(100, 2)) / resist, 0, KILLED_BY_LAVA );
        }

        if (you.duration[DUR_CONDENSATION_SHIELD] > 0)
        {
            mpr("Your icy shield dissipates!", MSGCH_DURATION);
            you.duration[DUR_CONDENSATION_SHIELD] = 0;
            you.redraw_armour_class = 1;
        }
    }

    // a distinction between stepping and falling from you.levitation
    // prevents stepping into a thin stream of lava to get to the other side.
    if (scramble())
    {
        if (place)
        {
            if (empty_surrounds(you.x_pos, you.y_pos, DNGN_FLOOR, false, empty))
            {
                you.x_pos = empty[0];
                you.y_pos = empty[1];
                escape = true;
            }
            else
                escape = false;
        }
        else
            escape = true;
    }
    else
    {
        // that is, don't display following when fall from levitating
        if (!place)
            mpr("You try to escape, but your burden drags you down!");
    }

    if (escape)
    {
        mpr("You manage to scramble free!");

        if (terrain == DNGN_LAVA)
            scrolls_burn(10, OBJ_SCROLLS);

        return;
    }

    mpr("You drown...");

    if (terrain == DNGN_LAVA)
        ouch(-9999, 0, KILLED_BY_LAVA);
    else if (terrain == DNGN_DEEP_WATER)
        ouch(-9999, 0, KILLED_BY_WATER);

    // Okay, so you don't trigger a trap when you scramble onto it.
    //I really can't be bothered right now.
}                               // end fall_into_a_pool()

bool scramble(void)
{
    int max_carry = carrying_capacity();

    if ((max_carry / 2) + random2(max_carry / 2) <= you.burden)
        return false;
    else
        return true;
}                               // end scramble()

void weird_colours(unsigned char coll, char wc[30])
{
    unsigned char coll_div16 = coll / 16; // conceivable max is then 16 {dlb}

    // Must start with a consonant!
    strcpy(wc, (coll_div16 == 0 || coll_div16 ==  7) ? "brilliant" :
               (coll_div16 == 1 || coll_div16 ==  8) ? "pale" :
               (coll_div16 == 2 || coll_div16 ==  9) ? "mottled" :
               (coll_div16 == 3 || coll_div16 == 10) ? "shimmering" :
               (coll_div16 == 4 || coll_div16 == 11) ? "bright" :
               (coll_div16 == 5 || coll_div16 == 12) ? "dark" :
               (coll_div16 == 6 || coll_div16 == 13) ? "shining"
                                                     : "faint");

    strcat(wc, " ");

    while (coll > 17)
        coll -= 10;

    strcat(wc, (coll ==  0) ? "red" :
               (coll ==  1) ? "purple" :
               (coll ==  2) ? "green" :
               (coll ==  3) ? "orange" :
               (coll ==  4) ? "magenta" :
               (coll ==  5) ? "black" :
               (coll ==  6) ? "grey" :
               (coll ==  7) ? "silver" :
               (coll ==  8) ? "gold" :
               (coll ==  9) ? "pink" :
               (coll == 10) ? "yellow" :
               (coll == 11) ? "white" :
               (coll == 12) ? "brown" :
               (coll == 13) ? "aubergine" :
               (coll == 14) ? "ochre" :
               (coll == 15) ? "leaf green" :
               (coll == 16) ? "mauve" :
               (coll == 17) ? "azure"
                            : "colourless");

    return;
}                               // end weird_colours()

bool go_berserk(bool intentional)
{
    if (you.berserker)
    {
        if (intentional)
            mpr("You're already berserk!");
        // or else you won't notice -- no message here.
        return false;
    }

    if (you.exhausted)
    {
        if (intentional)
            mpr("You're too exhausted to go berserk.");
        // or else they won't notice -- no message here
        return false;
    }

    if (you.is_undead == US_UNDEAD || you.species == SP_GHOUL)
    {
        if (intentional)
            mpr("You cannot raise a blood rage in your lifeless body.");
        // or else you won't notice -- no message here
        return false;
    }

    mpr("A red film seems to cover your vision as you go berserk!");
    mpr("You feel yourself moving faster!");
    mpr("You feel mighty!");

    you.berserker += 20 + random2avg(19, 2);

    calc_hp();
    you.hp *= 15;
    you.hp /= 10;

    deflate_hp(you.hp_max, false);

    if (!you.might)
        modify_stat(STAT_STRENGTH, 5, true);

    you.might += you.berserker;
    you.haste += you.berserker;

    if (you.berserk_penalty != NO_BERSERK_PENALTY)
        you.berserk_penalty = 0;

    return true;
}                               // end go_berserk()

bool trap_item(char base_type, char sub_type, char beam_x, char beam_y)
{
    item_def  item;

    item.base_type = base_type;
    item.sub_type = sub_type;
    item.plus = 0;
    item.plus2 = 0;
    item.flags = 0;
    item.special = 0;
    item.quantity = 1;
    item.colour = LIGHTCYAN;

    if (base_type == OBJ_MISSILES)
    {
        if (sub_type == MI_NEEDLE)
        {
            set_item_ego_type( item, OBJ_MISSILES, SPMSL_POISONED );
            item.colour = WHITE;
        }
        else
        {
            set_item_ego_type( item, OBJ_MISSILES, SPMSL_NORMAL );
        }
    }
    else
    {
        set_item_ego_type( item, OBJ_WEAPONS, SPWPN_NORMAL );
    }

    if (igrd[beam_x][beam_y] != NON_ITEM)
    {
        if (items_stack( item, mitm[ igrd[beam_x][beam_y] ] ))
        {
            inc_mitm_item_quantity( igrd[beam_x][beam_y], 1 );
            return (false);
        }

        // don't want to go overboard here. Will only generate up to three
        // separate trap items, or less if there are other items present.
        if (mitm[ igrd[beam_x][beam_y] ].link != NON_ITEM)
        {
            if (mitm[ mitm[ igrd[beam_x][beam_y] ].link ].link != NON_ITEM)
                return (false);
        }
    }                           // end of if igrd != NON_ITEM

    return (!copy_item_to_grid( item, beam_x, beam_y, 1 ));
}                               // end trap_item()

// returns appropriate trap symbol for a given trap type {dlb}
unsigned char trap_category(unsigned char trap_type)
{
    switch (trap_type)
    {
    case TRAP_TELEPORT:
    case TRAP_AMNESIA:
    case TRAP_ZOT:
        return (DNGN_TRAP_MAGICAL);

    case TRAP_DART:
    case TRAP_ARROW:
    case TRAP_SPEAR:
    case TRAP_AXE:
    case TRAP_BLADE:
    case TRAP_BOLT:
    case TRAP_NEEDLE:
    default:                    // what *would* be the default? {dlb}
        return (DNGN_TRAP_MECHANICAL);
    }
}                               // end trap_category()

// returns index of the trap for a given (x,y) coordinate pair {dlb}
int trap_at_xy(int which_x, int which_y)
{

    for (int which_trap = 0; which_trap < MAX_TRAPS; which_trap++)
    {
        if (env.trap[which_trap].x == which_x
            && env.trap[which_trap].y == which_y)
        {
            return (which_trap);
        }
    }

    // no idea how well this will be handled elsewhere: {dlb}
    return (-1);
}                               // end trap_at_xy()
