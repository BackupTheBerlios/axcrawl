/*********************************************************
*
* monster memories.
*
* Author: Alex
* Date:   16.07.01
*********************************************************/


#include "AppHdr.h" 
#include "files.h"

#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef DOS
#include <conio.h>
#include <file.h>
#endif

#ifdef LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef USE_EMX
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef MAC
#include <stat.h>
#else
#include <sys/stat.h>
#endif

#include "externs.h"

#include "debug.h"
#include "dungeon.h"
#include "itemname.h"
#include "items.h"
#include "message.h"
#include "misc.h"
#include "monstuff.h"
#include "mon-util.h"
#include "mstuff2.h"
#include "player.h"
#include "randart.h"
#include "skills2.h"
#include "stuff.h"
#include "tags.h"
#include "wpn-misc.h"
#include "files.h"
#include "menus.h"
#include "monmem.h"
#include "options.h"
#include "strutil.h"
#include "usestair.h"

tp_monsters_recall *monsters_recall;


/*
  ActEstadisticas()
    
  Update estatistics for each monster killed
  
  Params:
  
    .- monster_killed: monster killed
    .- mid           : position of the monster in array env.mons[]
    .- exp           : points of exp gained for killing the monter
  
  
  Alex 11.07.01
*/  

void ActEstadisticas(struct monsters *mons_killed,int mid,int exp)
 {
   char texto[200];
   int type_killed=mons_killed->type;
   	
   if (type_killed>=NUM_MONSTERS)
     {
     	strcpy(str_pass,"Error en monster_killed:");
     	itoa(type_killed,st_prn,10);
     	strcat(str_pass,st_prn);
     	mpr(str_pass);
     	
     	return;
     }	 	
   
   type_killed=mon_entry[type_killed];   
   monsters_recall[type_killed].exp+=exp;
   monsters_recall[type_killed].kills++;    
   
   /*itoa(monster_killed,st_prn,10);
   strcpy(str_pass,"Monster:");
   strcat(str_pass,st_prn);
   mpr(str_pass);   
   itoa(monsters_recall[monster_killed].exp,st_prn,10);
   strcpy(str_pass,"Exp:");
   strcat(str_pass,st_prn);
   mpr(str_pass); */
   
   monsters_recall[type_killed].hp=(monsters_recall[type_killed].hp+menv[mid].max_hit_points)/monsters_recall[type_killed].kills;
   
   monsters_recall[type_killed].evasion=(monsters_recall[type_killed].evasion+menv[mid].evasion)/monsters_recall[type_killed].kills;
   monsters_recall[type_killed].armour_class=(monsters_recall[type_killed].armour_class+menv[mid].armour_class)/monsters_recall[type_killed].kills;

   if (monsters_recall[type_killed].kills==1) 
     {
        monsters_recall[type_killed].first_level=you.your_level+1;
        monsters_recall[type_killed].first_level_type=you.level_type;
        monsters_recall[type_killed].first_branch=you.where_are_you;
     }  
          
   monsters_recall[type_killed].last_level=you.your_level+1;
   monsters_recall[type_killed].last_level_type=you.level_type;
   monsters_recall[type_killed].last_branch=you.where_are_you;
         
 }   


/*
   MonsterRecall()
   
   Appends the monster memories to the param "descrip" 
   
   Alex 11.07.01
*/   

void MonsterRecall(int mtype,string &descrip)
 {
   int nRes;
   int monster_data;   
   
      
   descrip+="$You have kill the monster ";
   monster_data=mon_entry[mtype];
   
   itoa(monsters_recall[monster_data].kills, st_prn, 10);  
   descrip+=st_prn;
   descrip+=" times.";
        
   if (monsters_recall[monster_data].kills==0) return;
        
   descrip+="$The first was in ";
   level_name(str_pass,monsters_recall[monster_data].first_branch,monsters_recall[monster_data].first_level_type,monsters_recall[monster_data].first_level);
   descrip+=str_pass;
   descrip+=".";
   
   if (monsters_recall[monster_data].kills>1) 
      {
      	 descrip+="$The last was in ";
         level_name(str_pass,monsters_recall[monster_data].last_branch,monsters_recall[monster_data].last_level_type,monsters_recall[monster_data].last_level);
         descrip+=str_pass;
         descrip+=".";
      }
            

   if (monsters_recall[monster_data].nAttacks>0)
     {
       descrip+="$It has hit you ";
       itoa(monsters_recall[monster_data].nAttacks, st_prn, 10);  
       descrip+=st_prn;
       descrip+=" times, doing ";    
       sprintf(st_prn,"%.1f",monsters_recall[monster_data].damage);  
       descrip+=st_prn;
       descrip+=" points of damage";
   
       descrip+="$Their max hit was for ";  
       itoa(monsters_recall[monster_data].max_damage, st_prn, 10);  
       descrip+=st_prn;
       descrip+=" and the min hit was for ";  
       itoa(monsters_recall[monster_data].min_damage, st_prn, 10);  
       descrip+=st_prn;
       descrip+=".";
     }
       
   descrip+="$It gives you ";
   itoa(monsters_recall[monster_data].exp, st_prn, 10);  
   descrip+=st_prn;
   descrip+=" exp. points";
   if (monsters_recall[monster_data].exp>1) descrip+="s";
   descrip+=".";
                                   
   descrip+="$It has about ";
   sprintf(st_prn,"%.1f",monsters_recall[monster_data].hp);  
   descrip+=st_prn;
   descrip+=" hit points, ";
   
   sprintf(st_prn,"%.1f",monsters_recall[monster_data].evasion);  
   descrip+=st_prn;
   descrip+=" of evasion, ";
   
   sprintf(st_prn,"%.1f",monsters_recall[monster_data].armour_class);  
   descrip+=st_prn;
   descrip+=" of armour class.";   
   
   
   if (mons_class_see_invis(mtype)) descrip+="It sees invisible things.";
   
   switch(mons_holiness(mtype))
     {
       case MH_HOLY:descrip+="$It is a holy being.";break;
       case MH_UNDEAD:descrip+="$It is one of the undead.";break;
       case MH_DEMONIC:descrip+="$It is demonic.";break;
     }  

            
   nRes=0;
   //if (mon_resist_mag(monster_data,1)) {descrip+="magic.";nRes++;}  
   if (mons_class_res_elec(mtype)) {if (nRes++>0) descrip+=",";else descrip+="$It resists ";descrip+="electricity";}  
   if (mons_class_res_poison(mtype)) {if (nRes++>0) descrip+=",";else descrip+="$It resists ";descrip+="poison";}  
   if (mons_class_res_fire(mtype)) {if (nRes++>0) descrip+=",";else descrip+="$It resists ";descrip+="fire";}  
   if (mons_class_res_cold(mtype)) {if (nRes++>0) descrip+=",";else descrip+="$It resists ";descrip+="cold";}  
   
   
   switch(mons_itemuse(mtype))
     {
       case 1:descrip+="$It can open doors.";break;	
       case 3:descrip+="$It can use weapons/armour.";break;	
     } 	
     
   if (mons_class_flies(mtype)) descrip+="$It can fly.";
     
 }   
 
 //Calc the damage made that a monster does per round
 void ActDamage(int dam,int mtype)
   { 
     
     if (mtype>=0 && mtype<NUM_MONSTERS)
       {
          monsters_recall[mtype].nAttacks++;   
          monsters_recall[mtype].damage+=dam;
          if (monsters_recall[mtype].min_damage>dam) monsters_recall[mtype].min_damage=dam;
          if (monsters_recall[mtype].max_damage<dam) monsters_recall[mtype].max_damage=dam;
          
       }   
   } 

     

  
  
/* 
   mon_recall_init()
                  
   Initializes the monster memories               
   
   Alex: 11.07.01
*/               

void mon_recall_init()
 {          
    monsters_recall=(tp_monsters_recall *)calloc(NUM_MONSTERS,sizeof(tp_monsters_recall));            
 }       


void mon_recall_empty()
 {
   int x;
   	
   for (x = 0; x < NUM_MONSTERS; x++)
       {             
           monsters_recall[x].kills=0;
           monsters_recall[x].evasion=0;
           monsters_recall[x].hp=0;
           monsters_recall[x].exp=0;
           monsters_recall[x].armour_class=0;
           monsters_recall[x].nAttacks=0;
           monsters_recall[x].damage=0;
           monsters_recall[x].min_damage=0;
           monsters_recall[x].max_damage=0;
           monsters_recall[x].first_level=0;
       }  	
 } 	

int show_item(struct tMenu& mnu,int bSelec)
  {
     int n;
     int *cad;
     string descrip;
     
     descrip.reserve(500);
     
     if (bSelec==4)
       {
       	  gotoxy(3,mnu.nFila);
       	  cprintf("                                                   ");
       	  return 1;
       }	  
     else
        {  
          cad=(int *)mnu.data+mnu.nItem*sizeof(int);
          n=*cad;
          if (n<0 || n> NUM_MONSTERS) 
           {
       	      gotoxy(1,1);cprintf("Mal %d %d",n,mnu.nItem);getch();
       	      gotoxy(1,1);cprintf("              ");
       	      return 0;
           }	 
     
          if (bSelec)
             textcolor(CYAN);
          else
             textcolor(WHITE);
          gotoxy(3,mnu.nFila);
          cprintf("%s",monster(n)->name);
     
          textcolor(WHITE);
          gotoxy(34,mnu.nFila);cprintf("%d",monsters_recall[n].kills);
          gotoxy(45,mnu.nFila);cprintf("%d",monsters_recall[n].exp);
     
          textcolor(monster(n)->colour);
          gotoxy(40,mnu.nFila);cprintf("%c",monster(n)->showchar);
       } 
     
     if (bSelec)
      {
      	ClearScreen(50,5,79,23);
      	MonsterRecall(monster(n)->mc,descrip);
      	print_description(descrip,29,50,5,GREEN);
      }	
     gotoxy(3,mnu.nFila);
  }

int look_monsters_by_type(int *data,int type)
  {
   int n,nCount=-1;
   int *p;
   for (n=0;n<NUM_MONSTERS;n++)
     {
     	if (monsters_recall[n].kills>0) 
     	  {
     	    nCount++;	
     	    p=data+nCount*sizeof(int);
     	    *p=n;     	    
     	    //gotoxy(1,1);cprintf("Kill :%d",*p);getch();
     	  }
     	    
     }	
   
   return nCount;
  }
   	
/*
   show_memory_monster()
   
   Shows the memory monter
   
   Alex:10.09.01
*/   
void show_memory_monster()
  {
    #ifdef DOS_TERM
      char buffer[4800];

      window(1, 1, 80, 25);
      gettext(1, 1, 80, 25, buffer);
    #endif 	
    
    struct tMenu mnu;
    int data[NUM_MONSTERS];
    int nItem,n,nCol;
    
    mnu.nFilaMax=23;
    mnu.nFilaIni=5;
    mnu.nMaxItems=NUM_MONSTERS;
    mnu.print_item=show_item;
    mnu.handle_key=NULL;
    mnu.data=(char *)data;
    mnu.data2=NULL;

    textbackground(BLACK);
    textcolor(WHITE);    
    clrscr();

   textbackground(BLACK);   
   clrscr();
   textcolor(YELLOW);
   gotoxy(3,2);cprintf("Monster");
   gotoxy(34,2);cprintf("Kills");
   gotoxy(40,2);cprintf("Sym");
   gotoxy(45,2);cprintf("Exp");
   textcolor(MAGENTA);   
   Box(1,1,80,3);
   Box(1,4,80,24);
        
   textbackground(BLACK);
   
   for (n=5;n<=23;n++)
     {
       gotoxy(nCol+1,n);putch(186);
     }   
     
   gotoxy(nCol+1,4);putch(203);
   gotoxy(nCol+1,24);putch(202);

    
    mnu.nMaxItems=look_monsters_by_type(&data[0],0);
    /*int n;    
    int *p;
   for (n=0;n<=mnu.nMaxItems;n++)
     {
     	p=(int *)mnu.data+n*sizeof(int);
     	gotoxy(1,1);cprintf("Kill *** :%d %d ",data[n],*p);getch();
     } 	        
   gotoxy(1,1);cprintf("                      ");   */
    
    nItem=menu(mnu);
       
  	
    redraw_screen();	
    
    #ifdef DOS_TERM
       puttext(1, 1, 80, 25, buffer);
    #endif
  }
	
	
	