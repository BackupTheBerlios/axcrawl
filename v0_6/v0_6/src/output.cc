/*
 *  File:       output.cc
 *  Summary:    Functions used to print player related info.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *     <3>      22.10.01        ALX    AX-Crawl style      
 *
 *      <2>      5/20/99        BWR    Efficiency changes for curses.
 *      <1>      -/--/--        LRH    Created
 */

#include "AppHdr.h"
#include "output.h"

#include <stdlib.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "itemname.h"
#include "ouch.h"
#include "player.h"
#include "view.h" //alex:22.10.01
#include "options.h"

//--------------------------------------------------------------------------------------------------------
//Display status of the player below their stats
//
//Alex:22.10.01

void DispStatus(int& ncol,int&nrow,int& ncont,char *text)
  {                    
    const int NUMBER_OF_LINES = get_number_of_lines(); 
    int n;
    n=strlen(text)+3;    
    if (ncol>79) return;
    gotoxy(ncol,nrow);
    cprintf(text);
    ncont++;
    ncol+=n;
  } 	
     
//See your status and call DispStatus to show it
void display_status() 
  {
   
        int j;
        char ft;
        int n=0;
        int ncol=1;
        int nrow = get_number_of_lines(); 
        textcolor(LIGHTCYAN);
        gotoxy(1,nrow);cprintf("%79s"," ");
        n=0;  
        if (you.duration[DUR_PRAYER]) DispStatus(ncol,nrow,n,"Praying");
        
        if (you.poison)
          {
            strcpy(info, "");
            strcat(info, " poison");
            strcat(info, (you.poison > 10) ? "+++" :
                     (you.poison > 5)  ? "++" :
                     (you.poison > 3)  ? "+"
                                       : "");
           
           DispStatus(ncol,nrow,n,info);
         }                        
        
        if (you.disease)
         {
           strcpy(info, "");
           strcat(info, (you.disease > 120) ? "Bad " :
                     (you.disease >  40) ? ""
                                         : "Mid ");
           strcat(info, "disease");
           DispStatus(ncol,nrow,n,info);
         }
        
        if (you.duration[DUR_LIQUID_FLAMES]) DispStatus(ncol,nrow,n,"In Flames");        
        if (you.duration[DUR_SILENCE])  DispStatus(ncol,nrow,n,"Silence"); 
        if (you.conf) DispStatus(ncol,nrow,n,"Conf");        
        if (you.paralysis) DispStatus(ncol,nrow,n,"Paralysed");
        if (you.exhausted) DispStatus(ncol,nrow,n,"Exhausted");
        if (you.slow) DispStatus(ncol,nrow,n,"Slow");
        if (you.haste) DispStatus(ncol,nrow,n,"Fast");
        if (you.might) DispStatus(ncol,nrow,n,"Mighty");
        if (you.berserker) DispStatus(ncol,nrow,n,"Berseker");       
        if (you.levitation) DispStatus(ncol,nrow,n,"Levitating");
        if (you.duration[DUR_ICY_ARMOUR]) DispStatus(ncol,nrow,n,"Icy Shield");
        if (you.duration[DUR_REPEL_MISSILES]) DispStatus(ncol,nrow,n,"Prot. Missils");
        if (you.duration[DUR_DEFLECT_MISSILES]) DispStatus(ncol,nrow,n,"Repel Missils");        
        if (you.invis) DispStatus(ncol,nrow,n,"Invis.");
     //   if (you.duration[DUR_INMATERIALITY]) DispStatus(ncol,nrow,n,"Inmaterial");       
     //   if (you.duration[DUR_INVULNERABILTY]) DispStatus(ncol,nrow,n,"Invulnerable");       
       
        switch (you.attribute[ATTR_TRANSFORMATION])
         {
           case TRAN_SPIDER:
              DispStatus(ncol,nrow,n,"Spider");
              break;
           case TRAN_BLADE_HANDS:
              DispStatus(ncol,nrow,n,"Blade Hands");
              break;
           case TRAN_STATUE:
              DispStatus(ncol,nrow,n,"Statue");
              break;
           case TRAN_ICE_BEAST:
              DispStatus(ncol,nrow,n,"Ice Beast");
              break;
           case TRAN_DRAGON:
              DispStatus(ncol,nrow,n,"Dragon");
              break;
           case TRAN_LICH:
              DispStatus(ncol,nrow,n,"Lich");
              break;
           case TRAN_SERPENT_OF_HELL:
              DispStatus(ncol,nrow,n,"Dem. Serp.");
              break;
           case TRAN_AIR:
              DispStatus(ncol,nrow,n,"Gass Cloud");
              break; 
         }

           
        
        if (you.duration[DUR_REGENERATION]) DispStatus(ncol,nrow,n,"Regene.");

        if (you.duration[DUR_SWIFTNESS]) DispStatus(ncol,nrow,n,"Swiftly");

        if (you.duration[DUR_INSULATION]) DispStatus(ncol,nrow,n,"Insulated");

        if (you.duration[DUR_STONEMAIL]) DispStatus(ncol,nrow,n,"Stone Scales");

        if (you.duration[DUR_CONTROLLED_FLIGHT]) DispStatus(ncol,nrow,n,"Control. Fly");

        if (you.duration[DUR_TELEPORT]) DispStatus(ncol,nrow,n,"About Tele.");

        if (you.duration[DUR_CONTROL_TELEPORT]) DispStatus(ncol,nrow,n,"Tele. Control");

        if (you.duration[DUR_DEATH_CHANNEL]) DispStatus(ncol,nrow,n,"Chan Dead");
        
        if (you.duration[DUR_FORESCRY])    DispStatus(ncol,nrow,n,"Forescry");
        
        if (you.duration[DUR_INFECTED_SHUGGOTH_SEED])      DispStatus(ncol,nrow,n,"Parasite");
        
        if (you.duration[DUR_STONESKIN]) DispStatus(ncol,nrow,n,"Stone Skin");               
 
        if (you.rotting || you.species == SP_GHOUL) DispStatus(ncol,nrow,n,"Rotting");
            
        if (you.confusing_touch) DispStatus(ncol,nrow,n,"Conf Touch");
    
        if (you.sure_blade) DispStatus(ncol,nrow,n,"Sure Blade");    
        
        //if (ring_weared(RING_CONFLICT)>-1) DispStatus(ncol,nrow,n,"Conflict");    
        
       // if (you_aggravate()) DispStatus(ncol,nrow,n,"Aggravate");    
                        
        bCanUseLastLine=n>0;
  }  
//
//Prints your stats
//  
void print_stats(int force=0,int nRowIni=3)
{
    textcolor(LIGHTGREY);

    char temp_quant[15];
    int n;
    
    if (you.redraw_hit_points ||force)
    {
        int max_max_hp = you.hp_max + player_rotted();

        if (Options.hp_warning 
            && (you.hp <= (you.hp_max * Options.hp_warning) / 100))
        {
            textcolor(RED);
        }
        else if (Options.hp_attention 
            && (you.hp <= (you.hp_max * Options.hp_attention) / 100))
        {
            textcolor(YELLOW);
        }

        itoa(you.hp, st_prn, 10);
        gotoxy(44, nRowIni);
        cprintf(st_prn);

        textcolor(LIGHTGREY);
        itoa(you.hp_max, st_prn, 10);

        cprintf("/");
        cprintf(st_prn);

        if (max_max_hp != you.hp_max)
        {
            cprintf(" (");
            itoa(max_max_hp, st_prn, 10);
            cprintf(st_prn);
            cprintf(")");
        }

#ifdef LINUX
        clear_to_end_of_line();
#else
        cprintf("        ");
#endif

        you.redraw_hit_points = 0;
    }

    if (you.redraw_magic_points || force)
    {
        itoa(you.magic_points, st_prn, 10);
        gotoxy(47, nRowIni+1);
        cprintf(st_prn);
        itoa(you.max_magic_points, st_prn, 10);
        cprintf("/");
        cprintf(st_prn);

#ifdef USE_CURSES
        clrtoeol();
#else
        clreol();
#endif

        you.redraw_magic_points = 0;
    }

    if (you.redraw_strength || force)
    {
        if (you.strength < 0)
            you.strength = 0;
        else if (you.strength > 72)
            you.strength = 72;

        if (you.max_strength > 72)
            you.max_strength = 72;

        itoa(you.strength, st_prn, 10);
        gotoxy(45, nRowIni+4);

        if (you.strength < you.max_strength)
            textcolor(YELLOW);

        cprintf(st_prn);

        if (you.strength != you.max_strength || force)
        {
            textcolor(LIGHTGREY);
            itoa(you.max_strength, st_prn, 10);
            cprintf(" (");
            cprintf(st_prn);
            cprintf(")   ");
        }
        else
        {
            cprintf("       ");
        }

        you.redraw_strength = 0;

        if (you.strength < 1)
            ouch(-9999, 0, KILLED_BY_WEAKNESS);

        burden_change((nRowIni!=3));
    }

    if (you.redraw_intelligence || force)
    {
        if (you.intel < 0)
            you.intel = 0;
        else if (you.intel > 72)
            you.intel = 72;

        if (you.max_intel > 72)
            you.max_intel = 72;

        itoa(you.intel, st_prn, 10);
        gotoxy(45, nRowIni+5);

        if (you.intel < you.max_intel)
            textcolor(YELLOW);

        cprintf(st_prn);

        if (you.intel != you.max_intel)
        {
            textcolor(LIGHTGREY);
            itoa(you.max_intel, st_prn, 10);
            cprintf(" (");
            cprintf(st_prn);
            cprintf(")   ");
        }
        else
        {
            cprintf("       ");
        }

        you.redraw_intelligence = 0;

        if (you.intel < 1)
            ouch(-9999, 0, KILLED_BY_STUPIDITY);
    }

    if (you.redraw_dexterity || force)
    {
        if (you.dex < 0)
            you.dex = 0;
        else if (you.dex > 72)
            you.dex = 72;

        if (you.max_dex > 72)
            you.max_dex = 72;

        itoa(you.dex, st_prn, 10);
        gotoxy(45, nRowIni+6);

        if (you.dex < you.max_dex)
            textcolor(YELLOW);

        cprintf(st_prn);

        if (you.dex != you.max_dex)
        {
            textcolor(LIGHTGREY);
            itoa(you.max_dex, st_prn, 10);
            cprintf(" (");
            cprintf(st_prn);
            cprintf(")   ");
        }
        else
        {
            cprintf("       ");
        }

        you.redraw_dexterity = 0;

        if (you.dex < 1)
            ouch(-9999, 0, KILLED_BY_CLUMSINESS);

        textcolor(LIGHTGREY);
    }

    if (you.redraw_armour_class || force)
    {
        itoa(player_AC(), st_prn, 10);
        gotoxy(44, nRowIni+2);
        cprintf(st_prn);

        if (strlen(st_prn) <= 1)
            cprintf(" ");
        if (strlen(st_prn) <= 2)
            cprintf(" ");
        if (strlen(st_prn) <= 3)
            cprintf(" ");

        if (you.duration[DUR_CONDENSATION_SHIELD])      //jmf: added 24mar2000
            textcolor(LIGHTBLUE);

        cprintf("(");
        itoa(player_shield_class(), st_prn, 10);
        cprintf(st_prn);
        cprintf(")   ");

        if (you.duration[DUR_CONDENSATION_SHIELD])      //jmf: added 24mar2000
            textcolor(LIGHTGREY);

        you.redraw_armour_class = 0;
    }

    if (you.redraw_evasion|| force)
    {
        itoa(player_evasion(), st_prn, 10);
        gotoxy(44, nRowIni+3);
        cprintf(st_prn);
        cprintf("  ");

        you.redraw_evasion = 0;
    }

    if (you.redraw_gold|| force)
    {
        itoa(you.gold, st_prn, 10);
        gotoxy(46, nRowIni+7);
        cprintf(st_prn);
        cprintf("    ");

        you.redraw_gold = 0;
    }

    if (you.redraw_experience|| force)
    {
        itoa(you.experience_level, temp_quant, 10);
        itoa(you.experience, st_prn, 10);
        gotoxy(52, nRowIni+8);
        cprintf(temp_quant);
        cprintf("/");
        cprintf(st_prn);

        cprintf(" (");
        itoa(you.exp_available, st_prn, 10);
        cprintf(st_prn);
        cprintf(")    ");
        
        //**ALEX: 11.07.01
        // Exp for next level
        if (you.experience_level<27)
          {
            gotoxy(67,nRowIni+8);cprintf("Next:" );
            int xp_needed = (exp_needed(you.experience_level+2)- you.experience) + 1;
            itoa(xp_needed,st_prn,10);
            cprintf(st_prn);
            clreol();
          }  
        //**ALEX:**
                       
        you.redraw_experience = 0;
    }

    if (you.redraw_hunger|| force)
    {
        gotoxy(40, nRowIni+11);

        switch (you.hunger_state)
        {
        case HS_ENGORGED:
            textcolor(BLUE);
            cprintf("Engorged");
            textcolor(LIGHTGREY);
            break;

        case HS_FULL:
            textcolor(GREEN);
            cprintf("Full    ");
            textcolor(LIGHTGREY);
            break;

        case HS_SATIATED:
#ifdef USE_CURSES
            clrtoeol();
#else
            clreol();
#endif
            break;

        case HS_HUNGRY:
            textcolor(YELLOW);
            cprintf("Hungry  ");
            textcolor(LIGHTGREY);
            break;

        case HS_STARVING:
            textcolor(RED);
            cprintf("Starving");
            textcolor(LIGHTGREY);
            break;
        }

        you.redraw_hunger = 0;
    }

    if (you.redraw_burden|| force)
    {
        gotoxy(40, nRowIni+12);
        int capacity=carrying_capacity(); //{alex:18.07.01}
        
        switch (you.burden_state)
        {
        case BS_OVERLOADED:
            textcolor(YELLOW);
            cprintf("Overloaded");
            textcolor(LIGHTGREY);
            break;

        case BS_ENCUMBERED:
            textcolor(LIGHTRED);
            cprintf("Encumbered");
            textcolor(LIGHTGREY);
            break;

        case BS_UNENCUMBERED: 
            //{alex:18.07.01}
            textcolor(GREEN); 
            cprintf("Unencumbered");

//--- alex:18.07.01            
//#ifdef USE_CURSES  
//            clrtoeol();
//#else
//            cprintf("          ");
//#endif
//            break;
//---
        }
        if (xtraOps.ax_show_weight)         
            cprintf(" (%d Max:%d)   ",you.burden,capacity); //alex:18.07.01
        else
            clreol();
        
        you.redraw_burden = 0;
    }

    //if (wield_change|| force) {alex;16.10.01}
    {
        gotoxy(40, nRowIni+10);
#ifdef USE_CURSES
        clrtoeol();
#else
        clreol();
#endif

        if (you.equip[EQ_WEAPON] != -1)
        {
            gotoxy(40, nRowIni+10);
            textcolor(you.inv[you.equip[EQ_WEAPON]].colour);
            
            in_name( you.equip[EQ_WEAPON], DESC_PLAIN, str_pass );
            str_pass[40] = 0;
            cprintf(str_pass);
            textcolor(LIGHTGREY);
        }
        else
        {
            gotoxy(40, nRowIni+10);

            if (you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS)
            {
                textcolor(RED);
                cprintf("Blade Hands");
                textcolor(LIGHTGREY);
            }
            else
            {
                textcolor(LIGHTGREY);
                cprintf("Nothing wielded");
            }
        }            
        you.wield_change = false;
    }
    
    //Show the numbers of turns
    //alex: 16.10.01
    //gotoxy(68,nRowIni+7);
    //textcolor(LIGHTGRAY);cprintf("Turns:%d",turns);
    
    //Show the level of spells availables
    //alex: 10.10.01
    gotoxy(40,nRowIni+13);
    n=player_spell_levels();
    if (n)
      {
        strcpy(info, "Level");
        if (n>1)
           strcat(info, "s");

        strcat(info, " of spells availables: ");
        textcolor(WHITE);
        itoa(n, st_prn, 10);   
        strcat(info, st_prn);
        cprintf(info);
      }   
      
#ifdef USE_CURSES
        clrtoeol();
#else
        clreol();
#endif            

//When rolling a player, nRowIni!=3
if (nRowIni==3) display_status();

#ifdef USE_CURSES
    refresh();
#endif


}                               // end print_stats()
