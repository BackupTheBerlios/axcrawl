/*
===============================================================================

usestair.cc

Using stairs

Alex: 15.03.02

===============================================================================
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
#include "usestair.h"

#include "items.h"
#include "lev-pand.h"
#include "monplace.h"
#include "mon-util.h"
#include "monstuff.h"
#include "ouch.h"
#include "shopping.h"
#include "files.h"
#include "stuff.h"
#include "transfor.h"
#include "view.h"
#include "player.h"
#include "view.h"
#include "skills2.h"
#include "dungeon.h" //alex:{25.10.01}
#include "effects.h"

#ifdef MACROS
#include "macro.h"
#endif


//**alex: 12.07.01

struct tp_branches
   {
     int id;     
     unsigned short rock_colour;
     unsigned short floor_colour;
     char name[30];
     char flavour[50];     
   };
   
struct tp_branches branch_name[]=
   {
     {BRANCH_MAIN_DUNGEON,BROWN,LIGHTGREY,"Dungeon",""},
     {BRANCH_DIS,CYAN,CYAN,"Dis","",},
     {BRANCH_GEHENNA,RED,DARKGREY,"Gehenna",""},
     {BRANCH_VESTIBULE_OF_HELL,LIGHTGREY,LIGHTGREY,"the Vestibule of Hell",""},
     {BRANCH_COCYTUS,LIGHTCYAN,LIGHTBLUE,"Cocytus",""},
     {BRANCH_TARTARUS,DARKGREY,DARKGREY,"Tartarus",""},
     {BRANCH_INFERNO,RED,LIGHTRED,"Inferno",""},
     {BRANCH_THE_PIT,DARKGREY,RED,"The pit",""},
     {BRANCH_ORCISH_MINES,BROWN,BROWN,"Orcish mines",""},     
     {BRANCH_HIVE,BROWN,YELLOW,"Hive",""},
     {BRANCH_LAIR,BROWN,GREEN,"Lair",""},
     {BRANCH_SLIME_PITS,LIGHTGREEN,GREEN,"Slime Pits",""},     
     {BRANCH_VAULTS,BROWN,LIGHTGREY,"Vaults",""},
     {BRANCH_CRYPT,LIGHTGREY,LIGHTGREY,"Crypt",""},
     {BRANCH_HALL_OF_BLADES,LIGHTGREY,LIGHTGREY,"Hall of blades",""},
     {BRANCH_HALL_OF_ZOT,LIGHTGREY,LIGHTGREY,"Hall of Zot",""},     
     {BRANCH_ECUMENICAL_TEMPLE,LIGHTGREY,LIGHTGREY,"Ecumenical Temple","You sense an holy aura"},     
     {BRANCH_SNAKE_PIT,YELLOW,LIGHTGREEN,"Snake Pit",""},     
     {BRANCH_ELVEN_HALLS,LIGHTGREY,DARKGREY,"Elven Halls",""},
     {BRANCH_TOMB,LIGHTGREY,YELLOW,"Tomb","The sons of Death are around here."},     
     {BRANCH_SWAMP,BROWN,BROWN,"Swamp",""}
 };     
 
 
//Returns in "name" the complete name of the level identified by branch_id, level_type
//and num_level. If this params are -1, then they are your current location.
//
//If bSetColour==1, then the rock_colour and floor_colour are set for the level.
void level_name(char *name,int branch_id=-1,int level_type=-1,int num_level=-1,int bSetColour=0)
 {    
    gotoxy(1,1);
    
    if (branch_id==-1) branch_id=you.where_are_you;
    if (level_type==-1) level_type=you.level_type;
    if (num_level==-1) num_level=you.your_level+1;
    // maybe last part better expresssed as <= PIT {dlb}
    if (branch_id >= BRANCH_DIS && branch_id < BRANCH_ORCISH_MINES) num_level=num_level-26-1;
    
     /* Remember, must add this to the death_string in ouch */
    if (branch_id>= BRANCH_ORCISH_MINES
        && branch_id <= BRANCH_SWAMP)
    {
    	if (you.branch_stairs[branch_id]>num_level-1) you.branch_stairs[branch_id]=num_level-1;
        num_level=num_level - you.branch_stairs[branch_id];
        //gotoxy(1,1);
        //cprintf("%d %d %d",you.your_level,branch_id,you.branch_stairs[branch_id]);getch();
    }
    
    strcpy(name,"Level - ");	
    switch(level_type)
      {
        case LEVEL_DUNGEON:    
            if (branch_id!=BRANCH_VESTIBULE_OF_HELL)
              {
              	  itoa(num_level,st_prn,10);
                  strcat(name,st_prn);
                  strcat(name," of ");
              }
            strcat(name,branch_name[branch_id].name);
            
            if (bSetColour)
              if (branch_id==BRANCH_HALL_OF_ZOT)
                   if (you.your_level - you.branch_stairs[7] <= 1)
                     {
                        env.floor_colour = LIGHTGREY;
                        env.rock_colour = LIGHTGREY;
                     }
                   else
                     {
                        switch (you.your_level - you.branch_stairs[7])
                          {
                            case 2:
                               env.rock_colour = LIGHTGREY;env.floor_colour = BLUE;
                               break;
                            case 3:
                               env.rock_colour = BLUE;env.floor_colour = LIGHTBLUE;
                               break;
                            case 4:
                               env.rock_colour = LIGHTBLUE;env.floor_colour = MAGENTA;
                               break;
                            case 5:
                               env.rock_colour = MAGENTA;env.floor_colour = LIGHTMAGENTA;
                               break;
                           }
                     }
              else
                 {
                    env.rock_colour=branch_name[branch_id].rock_colour;	
                    env.floor_colour=branch_name[branch_id].floor_colour;
                 }	       
            
            
            break;            
        case LEVEL_LABYRINTH:                      
            strcat(name,"Laberinth");
            if (bSetColour)
               {
               	   env.floor_colour = LIGHTGREY;
                   env.rock_colour = BROWN;
               }	 
            break;
        case LEVEL_ABYSS:
            strcat(name,"Abyss");
            if (bSetColour)
               {
                   env.floor_colour = (mcolour[env.mons_alloc[9]] == BLACK) ? LIGHTGREY : mcolour[env.mons_alloc[9]];
                   env.rock_colour = (mcolour[env.mons_alloc[8]] == BLACK) ? LIGHTGREY : mcolour[env.mons_alloc[8]];
               }	
            break;
        case LEVEL_PANDEMONIUM:
            strcat(name,"Pandemonium");
            if (bSetColour)
               {
               	   env.floor_colour = (mcolour[env.mons_alloc[9]] == BLACK) ? LIGHTGREY : mcolour[env.mons_alloc[9]];
                   env.rock_colour = (mcolour[env.mons_alloc[8]] == BLACK) ? LIGHTGREY : mcolour[env.mons_alloc[8]];
               }                     
            break;
        default:
            strcat(name,"I don't know where you are");    
       }
 }       
 

//Show the name of the level
void show_name_level(int branch_id=-1,int level_type=-1,int num_level=-1,int nRow=12)
 {
   char buff[80];
   
   level_name(buff,branch_id,level_type,num_level,1);
   gotoxy(40,nRow);
   textcolor(LIGHTGREEN);
   cprintf(buff);
   textcolor(LIGHTGREY);   	
 }  	
//**alex:**


void up_stairs(void)
{
    unsigned char stair_find = grd[you.x_pos][you.y_pos];
    char old_where = you.where_are_you;
    bool was_a_labyrinth = false;
                                                         
  if (stair_find == DNGN_ENTER_SHOP)
        {   
            shop();
            return;
        }
   if ((stair_find < DNGN_STONE_STAIRS_UP_I
                    || stair_find > DNGN_ROCK_STAIRS_UP)
                && (stair_find < DNGN_RETURN_DUNGEON_I 
                    || stair_find > 150))
        {   
            mpr( "You can't go up here!" );
            return;
        }

        
    tag_followers();  // only those beside us right now can follow                                                         

    // Since the overloaded message set turn_is_over, I'm assuming that
    // the overloaded character makes an attempt... so we're doing this
    // check before that one. -- bwr
    if (!player_is_levitating()
        && you.conf 
        && (stair_find >= DNGN_STONE_STAIRS_UP_I 
            && stair_find <= DNGN_ROCK_STAIRS_UP)
        && random2(100) > you.dex)
    {
        mpr("In your confused state, you trip and fall back down the stairs.");

        ouch( roll_dice( 3 + you.burden_state, 5 ), 0, 
              KILLED_BY_FALLING_DOWN_STAIRS );

        you.turn_is_over = 1;
        return;
    }

    if (you.burden_state == BS_OVERLOADED)
    {
        mpr("You are carrying too much to climb upwards.");
        you.turn_is_over = 1;
        return;
    }

    if (you.your_level == 0
            && !yesno("Are you sure you want to leave the Dungeon?", false))
    {
        mpr("Alright, then stay!");
        return;
    }

    unsigned char old_level = you.your_level;

    // Make sure we return to our main dungeon level... labyrinth entrances
    // in the abyss or pandemonium a bit trouble (well the labyrinth does
    // provide a way out of those places, its really not that bad I suppose)
    if (you.level_type == LEVEL_LABYRINTH)
    {
        you.level_type = LEVEL_DUNGEON;
        was_a_labyrinth = true;
    }

    you.your_level--;

    int i = 0;

    if (you.your_level < 0)
    {
        mpr("You have escaped!");

        for (i = 0; i < ENDOFPACK; i++)
        {
            if (is_valid_item( you.inv[i] ) 
                && you.inv[i].base_type == OBJ_ORBS)
            {
                ouch(-9999, 0, KILLED_BY_WINNING);
            }
        }

        ouch(-9999, 0, KILLED_BY_LEAVING);
    }

    mpr("Entering...");
    you.prev_targ = MHITNOT;
    you.pet_target = MHITNOT;

    if (you.where_are_you == BRANCH_VESTIBULE_OF_HELL)
    {
        mpr("Thank you for visiting Hell. Please come again soon.");
        you.where_are_you = BRANCH_MAIN_DUNGEON;
        you.your_level = you.hell_exit;
        stair_find = DNGN_STONE_STAIRS_UP_I;
    }

    if (you.where_are_you > BRANCH_MAIN_DUNGEON
        && you.where_are_you < BRANCH_ORCISH_MINES
        && you.where_are_you != BRANCH_VESTIBULE_OF_HELL)
    {
        you.where_are_you = BRANCH_VESTIBULE_OF_HELL;
        you.your_level = 27;
    }

    switch (stair_find)
    {
    case DNGN_RETURN_DUNGEON_I:
    case DNGN_RETURN_DUNGEON_II:
    case DNGN_RETURN_DUNGEON_III:
    case DNGN_RETURN_DUNGEON_IV:
    case DNGN_EXIT_ZOT:
    case DNGN_RETURN_DUNGEON_V:
        mpr("Welcome back to the Dungeon!");
        you.where_are_you = BRANCH_MAIN_DUNGEON;
        break;
    case DNGN_RETURN_LAIR_II:
    case DNGN_RETURN_LAIR_III:
    case DNGN_RETURN_LAIR_IV:
        mpr("Welcome back to the Lair of Beasts!");
        you.where_are_you = BRANCH_LAIR;
        break;
    case DNGN_RETURN_VAULTS_II:
    case DNGN_RETURN_VAULTS_III:
        mpr("Welcome back to the Vaults!");
        you.where_are_you = BRANCH_VAULTS;
        break;
    case DNGN_RETURN_CRYPT:
        mpr("Welcome back to the Crypt!");
        you.where_are_you = BRANCH_CRYPT;
        break;
    case DNGN_RETURN_MINES:
        mpr("Welcome back to the Orcish Mines!");
        you.where_are_you = BRANCH_ORCISH_MINES;
        break;
    }

    unsigned char stair_taken = stair_find;

    if (player_is_levitating())
    {
        if (you.duration[DUR_CONTROLLED_FLIGHT])
            mpr("You fly upwards.");
        else
            mpr("You float upwards... And bob straight up to the ceiling!");
    }
    else
        mpr("You climb upwards.");

    load(stair_taken, LOAD_ENTER_LEVEL, was_a_labyrinth, old_level, old_where);

    new_level();

    viewwindow(1, true);

    you.turn_is_over = 1;

    save_game(false);
}                               // end up_stairs()

void down_stairs(bool remove_stairs, int old_level)
{
    int i;
    char old_level_type = you.level_type;
    bool was_a_labyrinth = false;
    const unsigned char stair_find = grd[you.x_pos][you.y_pos];
        
    //int old_level = you.your_level;
    bool leave_abyss_pan = false;
    char old_where = you.where_are_you;

#ifdef SHUT_LABYRINTH
    if (stair_find == DNGN_ENTER_LABYRINTH)
    {
        mpr("Sorry, this section of the dungeon is closed for fumigation.");
        mpr("Try again next release.");
        return;
    }
#endif

    // probably still need this check here (teleportation) -- bwr
    if ((stair_find < DNGN_ENTER_LABYRINTH
            || stair_find > DNGN_ROCK_STAIRS_DOWN)
        && stair_find != DNGN_ENTER_HELL
        && ((stair_find < DNGN_ENTER_DIS
                || stair_find > DNGN_TRANSIT_PANDEMONIUM)
            && stair_find != DNGN_STONE_ARCH)
        && !(stair_find >= DNGN_ENTER_ORCISH_MINES
            && stair_find < DNGN_RETURN_DUNGEON_I))
    {
        mpr( "You can't go down here!" );
        return;
    }
    if (grd[you.x_pos][you.y_pos] == DNGN_ENTER_SHOP)
      {   
        shop();
        return;
       }
    
        tag_followers();  // only those beside us right now can follow
    if (stair_find >= DNGN_ENTER_LABYRINTH
        && stair_find <= DNGN_ROCK_STAIRS_DOWN
        && you.where_are_you == BRANCH_VESTIBULE_OF_HELL)
    {
        mpr("A mysterious force prevents you from descending the staircase.");
        return;
    }                           /* down stairs in vestibule are one-way */

    if (stair_find == DNGN_STONE_ARCH)
    {
        mpr("You can't go down here!");
        return;
    }

    if (player_is_levitating() && !wearing_amulet(AMU_CONTROLLED_FLIGHT))
    {
        mpr("You're floating high up above the floor!");
        return;
    }

    if (stair_find == DNGN_ENTER_ZOT)
    {
        int num_runes = 0;

        for (i = 0; i < ENDOFPACK; i++)
        {
            if (is_valid_item( you.inv[i] )
                && you.inv[i].base_type == OBJ_MISCELLANY
                && you.inv[i].sub_type == MISC_RUNE_OF_ZOT)
            {
                num_runes += you.inv[i].quantity;
            }
        }

        if (num_runes < NUMBER_OF_RUNES_NEEDED)
        {
            switch (NUMBER_OF_RUNES_NEEDED)
            {
            case 1:
                mpr("You need a Rune to enter this place.");
                break;
            default:
                snprintf( info, INFO_SIZE, "You need at least %d Runes to enter this place.",
                        NUMBER_OF_RUNES_NEEDED);
                mpr(info);
            }
            return;
        }
    }

    if (you.level_type == LEVEL_PANDEMONIUM
            && stair_find == DNGN_TRANSIT_PANDEMONIUM)
    {
        was_a_labyrinth = true;
    }
    else
    {
        if (you.level_type != LEVEL_DUNGEON)
            was_a_labyrinth = true;

        you.level_type = LEVEL_DUNGEON;
    }

    mpr("Entering...");
    you.prev_targ = MHITNOT;
    you.pet_target = MHITNOT;

    if (stair_find == DNGN_ENTER_HELL)
    {
        you.where_are_you = BRANCH_VESTIBULE_OF_HELL;
        you.hell_exit = you.your_level;

        mpr("Welcome to Hell!");
        mpr("Please enjoy your stay.");

        more();

        you.your_level = 26;    // = 59;
    }

    if ((stair_find >= DNGN_ENTER_DIS && stair_find <= DNGN_ENTER_TARTARUS)
        || (stair_find >= DNGN_ENTER_ORCISH_MINES && stair_find <= 150))
    {
        // no idea why such a huge switch and not 100-grd[][]
        // planning ahead for re-organizaing grd[][] values - 13jan2000 {dlb}
        strcpy( info, "Welcome to " );            
        switch (stair_find)
        {
        case DNGN_ENTER_DIS:
            strcat(info, "the Iron City of Dis!");
            you.where_are_you = BRANCH_DIS;
            you.your_level = 26;
            break;
        case DNGN_ENTER_GEHENNA:
            strcat(info, "Gehenna!");
            you.where_are_you = BRANCH_GEHENNA;
            you.your_level = 26;
            break;
        case DNGN_ENTER_COCYTUS:
            strcat(info, "Cocytus!");
            you.where_are_you = BRANCH_COCYTUS;
            you.your_level = 26;
            break;
        case DNGN_ENTER_TARTARUS:
            strcat(info, "Tartarus!");
            you.where_are_you = BRANCH_TARTARUS;
            you.your_level = 26;
            break;
        case DNGN_ENTER_ORCISH_MINES:
            strcat(info, "the Orcish Mines!");
            you.where_are_you = BRANCH_ORCISH_MINES;
            break;
        case DNGN_ENTER_HIVE:
            strcpy(info, "You hear a buzzing sound coming from all directions.");
            you.where_are_you = BRANCH_HIVE;
            break;
        case DNGN_ENTER_LAIR:
            strcat(info, "the Lair of Beasts!");
            you.where_are_you = BRANCH_LAIR;
            break;
        case DNGN_ENTER_SLIME_PITS:
            strcat(info, "the Pits of Slime!");
            you.where_are_you = BRANCH_SLIME_PITS;
            break;
        case DNGN_ENTER_VAULTS:
            strcat(info, "the Vaults!");
            you.where_are_you = BRANCH_VAULTS;
            break;
        case DNGN_ENTER_CRYPT:
            strcat(info, "the Crypt!");
            you.where_are_you = BRANCH_CRYPT;
            break;
        case DNGN_ENTER_HALL_OF_BLADES:
            strcat(info, "the Hall of Blades!");
            you.where_are_you = BRANCH_HALL_OF_BLADES;
            break;
        case DNGN_ENTER_ZOT:
            strcat(info, "the Hall of Zot!");
            you.where_are_you = BRANCH_HALL_OF_ZOT;
            break;
        case DNGN_ENTER_TEMPLE:
            strcat(info, "the Ecumenical Temple!");
            you.where_are_you = BRANCH_ECUMENICAL_TEMPLE;
            break;
        case DNGN_ENTER_SNAKE_PIT:
            strcat(info, "the Snake Pit!");
            you.where_are_you = BRANCH_SNAKE_PIT;
            break;
        case DNGN_ENTER_ELVEN_HALLS:
            strcat(info, "the Elven Halls!");
            you.where_are_you = BRANCH_ELVEN_HALLS;
            break;
        case DNGN_ENTER_TOMB:
            strcat(info, "the Tomb!");
            you.where_are_you = BRANCH_TOMB;
            break;
        case DNGN_ENTER_SWAMP:
            strcat(info, "the Swamp!");
            you.where_are_you = BRANCH_SWAMP;
            break;
        }

        mpr(info);
    }

    if (stair_find == DNGN_ENTER_LABYRINTH)
    {
        you.level_type = LEVEL_LABYRINTH;
        grd[you.x_pos][you.y_pos] = DNGN_FLOOR;
    }

    if (stair_find == DNGN_ENTER_ABYSS)
    {
        you.level_type = LEVEL_ABYSS;
    }

    if (stair_find == DNGN_ENTER_PANDEMONIUM)
    {
        you.level_type = LEVEL_PANDEMONIUM;
    }

    if (you.level_type == LEVEL_LABYRINTH || you.level_type == LEVEL_ABYSS
        || you.level_type == LEVEL_PANDEMONIUM)
    {
        char glorpstr[kFileNameSize];
        char del_file[kFileNameSize];
        int sysg;

#ifdef SAVE_DIR_PATH
        snprintf( glorpstr, sizeof(glorpstr), 
                  SAVE_DIR_PATH "%s%d", you.your_name, (int) getuid() );
#else
        strncpy(glorpstr, you.your_name, kFileNameLen);

        // glorpstr [strlen(glorpstr)] = 0;
        // This is broken. Length is not valid yet! We have to check if we got
        // a trailing NULL; if not, write one:
        /* is name 6 chars or more? */
        if (strlen(you.your_name) > kFileNameLen - 1)
            glorpstr[kFileNameLen] = '\0';
#endif

        strcpy(del_file, glorpstr);
        strcat(del_file, ".lab");

#ifdef DOS
        strupr(del_file);
#endif
        sysg = unlink(del_file);

#if DEBUG_DIAGNOSTICS
        strcpy( info, "Deleting: " );
        strcat( info, del_file );
        mpr( info, MSGCH_DIAGNOSTIC );
        more();
#endif
    }

    if (stair_find == DNGN_EXIT_ABYSS || stair_find == DNGN_EXIT_PANDEMONIUM)
    {
        leave_abyss_pan = true;
        mpr("You pass through the gate, and find yourself at the top of a staircase.");
        more();
    }

    if (!player_is_levitating()
        && you.conf 
        && (stair_find >= DNGN_STONE_STAIRS_DOWN_I 
            && stair_find <= DNGN_ROCK_STAIRS_DOWN)
        && random2(100) > you.dex)
    {
        mpr("In your confused state, you trip and fall down the stairs.");

        // Nastier than when climbing stairs, but you'll aways get to 
        // your destination, -- bwr
        ouch( roll_dice( 6 + you.burden_state, 10 ), 0, 
              KILLED_BY_FALLING_DOWN_STAIRS );
    }

    if (you.level_type == LEVEL_DUNGEON)
        you.your_level++;

    int stair_taken = stair_find;

    //unsigned char save_old = 1;

    if (you.level_type == LEVEL_LABYRINTH || you.level_type == LEVEL_ABYSS)
        stair_taken = DNGN_FLOOR;       //81;

    if (you.level_type == LEVEL_PANDEMONIUM)
        stair_taken = DNGN_TRANSIT_PANDEMONIUM;

    if (remove_stairs)
        grd[you.x_pos][you.y_pos] = DNGN_FLOOR;

    switch (you.level_type)
    {
    case LEVEL_LABYRINTH:
        mpr("You enter a dark and forbidding labyrinth.");
        break;

    case LEVEL_ABYSS:
        mpr("You enter the Abyss!");
        mpr("To return, you must find a gate leading back.");
        break;

    case LEVEL_PANDEMONIUM:
        if (old_level_type == LEVEL_PANDEMONIUM)
            mpr("You pass into a different region of Pandemonium.");
        else
        {
            mpr("You enter the halls of Pandemonium!");
            mpr("To return, you must find a gate leading back.");
        }
        break;

    default:
        mpr("You climb downwards.");
        break;
    }

    load(stair_taken, LOAD_ENTER_LEVEL, was_a_labyrinth, old_level, old_where);

    new_level();

    viewwindow(1, true);

    unsigned char pc = 0;
    unsigned char pt = random2avg(28, 3);

    switch (you.level_type)
    {
    case LEVEL_LABYRINTH:
        you.your_level++;
        break;

    case LEVEL_ABYSS:
        grd[you.x_pos][you.y_pos] = DNGN_FLOOR;

        if (old_level_type != LEVEL_PANDEMONIUM)
            you.your_level--;   // Linley-suggested addition 17jan2000 {dlb}

        init_pandemonium();     /* colours only */

        if (you.where_are_you >= BRANCH_DIS
            && you.where_are_you < BRANCH_ORCISH_MINES)
        {
            // ie if you're in Hell
            you.where_are_you = BRANCH_MAIN_DUNGEON;
            you.your_level = you.hell_exit - 1;
        }
        break;

    case LEVEL_PANDEMONIUM:
        if (old_level_type == LEVEL_PANDEMONIUM)
        {
            init_pandemonium();
            for (pc = 0; pc < pt; pc++)
                pandemonium_mons();
        }
        else
        {
            if (old_level_type != LEVEL_ABYSS)
                you.your_level--;       // Linley-suggested addition 17jan2000 {dlb}

            init_pandemonium();

            for (pc = 0; pc < pt; pc++)
                pandemonium_mons();

            if (you.where_are_you >= BRANCH_DIS
                && you.where_are_you < BRANCH_ORCISH_MINES)
            {
                // ie if you're in Hell
                you.where_are_you = BRANCH_MAIN_DUNGEON;
                you.hell_exit = 26;
                you.your_level = 26;
            }
        }
        break;

    default:
        break;
    }

    // new_level();

    you.turn_is_over = 1;

    save_game(false);
}

void new_level(void)
{
        
    textcolor(LIGHTGREY);
    
    //when moving up/down stairs, the draw_border isn't called
    //alex:03.10.01
    draw_border(you.your_name, player_title(), you.species); 
        
    show_name_level();        
}      



  
tp_var_log var_log={-1,-1,-1};

/*

   flog(text)
   
   Append to the log the text contained in param "text"

   The log is "nomplayer.nts"
   
   ALEX 13.07.01
*/
void flog(char *text)
  { 
    char charFile[kFileNameSize]; 	   
    name_save_fich(charFile,".nts"); 	 
    FILE *saveFile = fopen(charFile, "at");
    //mpr(charFile);
    char l_name[80];
    
    if (saveFile == NULL)
    {   mpr("No puedo crear notas");
        return;
    }

    if   (you.where_are_you!=var_log.where_are_you 
        || you.level_type!=var_log.level_type
        || you.your_level!=var_log.level_number)
     {       
        level_name(l_name,you.where_are_you,you.level_type,you.your_level+1);
        fprintf(saveFile,"\n------------------------------------------------\n");
        fprintf(saveFile,"LEVEL: %s\n",l_name);
        fprintf(saveFile,"------------------------------------------------\n\n");
        var_log.level_number=you.your_level;
        var_log.where_are_you=you.where_are_you;
        var_log.level_type=you.level_type;
     }    
    fprintf(saveFile,"%s\n\n",text);
    fclose(saveFile);
  } 	
     
     