/*******************************************************************************************
*
* A menu with more commands
*
*
* Alex: 20.12.01
*******************************************************************************************/
#include "AppHdr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "menus.h" 
#include <ctype.h>
#include "stuff.h"

#include "output.h"      
#include "externs.h"
#include "view.h"
#include "dir.h"
#include "files.h"
#include "dungeon.h"
#include "chardump.h"
#include "mon-util.h"	
#include "hiscores.h"
#include "doc.h"
#include "morecmds.h"
#include "enum.h"
#include "invent.h"
#include "itemname.h"
#include "describe.h"
#include "strutil.h"
#include <string>
#include "objects.h"
#include "items.h"
#include "item_use.h"
#include "options.h"
#include "delay.h"
#include "usestair.h"
//Returns the name of the map of the current level
//The name is: XX_YY.txt
//  XX=level type (snake pit, vaults, regular dungeon, etc)
//  YY=level number
//Alex: 20.12.01
void map_name(char *filename)
 {
    sprintf(filename, "save/%s/maps/%02d_%02d.txt", you.your_name,you.where_are_you,you.your_level+1);
 }


void list_uniques()
  {
    int n,nRow=1;
    int mdata;
    
    clrscr();
    
    for (n=0;n<NUM_UNIQUE_CREATURES;n++)
      {
         if (you.unique_creatures[n])
           {
              mdata=mon_entry[n+ (n<=30 ? 280 : 40) ];
              gotoxy(1,nRow++);
              cprintf("%s %d %d\n",monster(mdata)->name,n,you.unique_creatures[n]);
           }   
      }   
      
    getch();  
  }

   
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MENU with the commands
 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int show_map_name(struct tMenu& mnu,int bSelec)
  {
    int n;
    char *cad;
     
     if (bSelec==4) return 1;
     //highlight on/off for the current item of the menu
     cad=(char *)mnu.data+(mnu.nItem*50);

     textcolor(WHITE);
     if (bSelec)
        {           
           textbackground(RED);           
        }   
     else
        {
           textbackground(BLACK);
        }    
     cad[20]=0;   
     gotoxy(3,mnu.nFila);cprintf(cad);
     textbackground(BLACK);
     for (n=strlen(cad)+3;n<34;n++) cprintf(" ");
  }	



// shows the list of dumped maps
int list_of_maps()
 {
   struct ffblk f; 
   char files[250];
   int nCont=-1;
   char *branch;
   char *level;
   char data[100][50];
   char data2[100][12];  
   struct tMenu mnu;
   int nItem=99;

   //searches for your screenshots up to 100.
   sprintf(str_pass,"save/%s/maps/",you.your_name);   
   strcpy(files,str_pass);
   strcat(files,"*.*");
   int done = findfirst(files, &f, FA_ARCH);
   while (!done)
        {
           if (f.ff_name[0] != '.')
              {
              	 nCont++;
               	 strcpy(data2[nCont],f.ff_name);
              	 strcpy(st_prn,str_pass);
              	 strcat(st_prn,f.ff_name);
              	 branch=strtok(st_prn,"_");
              	 level=strtok(0,".");
              	 level_name(str_pass,atoi(branch),-1,atoi(level)); 
              	 strcpy(data[nCont],str_pass);                 
              } 	
           done = findnext(&f);
        }
        
    //show the list of maps    
    mnu.nFilaMax=23;
    mnu.nFilaIni=5;
    mnu.print_item=show_map_name;
    mnu.handle_key=NULL;
    mnu.data=(char *)data;
    mnu.data2=(char *)data2;
    mnu.nMaxItems=nCont;
    mnu.nItem=0;

    for (;;)  	    
      {	
        AFormatScreen("Maps",35);
     	ClearScreen(2,5,35,23);    
        nItem=menu(mnu);
        if (nItem==-1) break;  
        level=level=(char *)mnu.data2+(nItem*12);
     	sprintf(str_pass,"save/%s/maps/%s",you.your_name,level);
     	textcolor(WHITE);
     	show_file(str_pass);     	     	     	
      }
    ClearScreen(5,23,1,35);  
 }
	
int show_morecmds(struct tMenu& mnu,int bSelec)
  {
    int n;
     char *cad;
     
     if (bSelec==4) return 1;
     //highlight on/off for the current item of the menu
     cad=(char *)mnu.data+(mnu.nItem*20);

     gotoxy(2,mnu.nFila);
     textcolor(WHITE);
     if (bSelec)
        {           
           textbackground(RED);           
        }   
     else
        {
           textbackground(BLACK);
        }    
        
     gotoxy(3,mnu.nFila);cprintf(cad);
     for (n=strlen(cad)+3;n<=17;n++) cprintf(" ");
     textbackground(BLACK);
     if (bSelec)
       {        
          ClearScreen(26,5,79,23);
          textcolor(GREEN);gotoxy(26,7);	
          switch(mnu.nItem)
            {              
              case 0:                 
                 cprintf("This option shows a list of your screenshots.");
                 gotoxy(26,8);	
                 cprintf("Selecting one of them allows you to see it");
                 gotoxy(26,9);	
                 cprintf("although you have forgotten it.");
                 break;    
              case 1:                           
                 cprintf("List of the uniques known.");                 
                 break;
              case 2:
                 cprintf("List the hight scores list.");
                 break;
              case 3:
                 cprintf("Show the character dump.");
                 break;   
            }	                      
          textcolor(LIGHTGREY);  
       }         
  }	    
    
void more_commands()
  {

#ifdef DOS_TERM
    char buffer[4800];

    window(1, 1, 80, 25);
    gettext(1, 1, 80, 25, buffer);
#endif

    
    struct tMenu mnu;
    char data[][20]=
       {
       	 "See Map",
       	 "Uniques",    
       	 "High Score List",
       	 "Char dump"
       };
    
       
    int nItem=99;
    
    mnu.nFilaMax=23;
    mnu.nFilaIni=5;
    mnu.print_item=show_morecmds;
    mnu.handle_key=NULL;
    mnu.data=(char *)data;
    mnu.data2=NULL;
    mnu.nMaxItems=sizeof(data)/sizeof(data[1])-1;            
    

    for (;;)
      {	    
        AFormatScreen("Options",20);
        nItem=menu(mnu,nItem!=99);
        switch(nItem)
          {
            case 0:
               list_of_maps();   
               break;
            case 1:
               list_uniques();
               break;   
            case 2:
               if (hiscores_dump_list())
                 show_file("scores.txt"); 
               else
                 {
                    gotoxy(26,9);
                    textcolor(LIGHTRED);
                    cprintf("The score list is empty");
                    gotoxy(26,11);
                    textcolor(GREEN);
                    cprintf("Press any key to continue");
                    getch();
                    textcolor(LIGHTGREY);
                 }         
               break;  
            case 3:
               name_save_fich(str_pass,".txt");
               if (dump_char(0, NULL) == 1)
                  {    
                    show_file(str_pass);
                  }
               else
                 {
                    gotoxy(26,9);
                    textcolor(LIGHTRED);
                    cprintf("Can show the character dump:");
                    gotoxy(26,10);
                    cprintf(str_pass);
                    gotoxy(26,12);
                    textcolor(GREEN);
                    cprintf("Press any key to continue");
                    getch();
                    textcolor(LIGHTGREY);
                 }         
               break;     
               
          }
        if (nItem==-1) break;  

      }  
    redraw_screen();

#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif

  }
         

//Dump current map
void DumpMap(bool messg)
  {
    FixedVector < int, 2 > plox; 		
    char buf[120];
    plox[0] = 0;
    map_name(str_pass);               
    show_map(plox,str_pass);
    
    if (messg)
      {
      	sprintf(buf,"Map dumped: %s",str_pass);
      	mpr(buf);
      } 	
  }	

//specify the missile to be put in the quiver slot
int Quiver(bool quiet=false,int msl_slot=-1)
 {             
   char buf[100];  
   int throw_slot;
   
   if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK,quiet);
        return false;
    }
    
    //cursed missiles can't go out of the quiver
    if (you.equip[EQ_QUIVER]!=-1)
      {
         if (item_cursed( you.inv[you.equip[EQ_QUIVER]]))
           {
              mpr2("The missiles in the quiver are stuck to it.",quiet);
              return false;
           } 
      }     
    if (msl_slot==-1)
      {  
         throw_slot = prompt_invent_item( "Which missil to put in the quiver (- for none) ?", OBJ_MISSILES ,true,'-');
         if (throw_slot == PROMPT_ABORT)
           {
               canned_msg( MSG_OK,quiet );
               return false;
            }
      }
    else
      throw_slot=msl_slot;               
    
    if (throw_slot == PROMPT_GOT_SPECIAL)  // '-' 
       {
          you.equip[EQ_QUIVER]=-1;
          mpr2("Your quiver now is empty.",quiet);
          return true;
       }       
    if (throw_slot == you.equip[EQ_WEAPON]
             && (item_cursed( you.inv[you.equip[EQ_WEAPON]] )))
    {
        mpr2("That thing is stuck to your hand!",quiet);
        return false;
    }
    
    if (you.inv[throw_slot].base_type!=OBJ_MISSILES) 
      {
        mpr2("You can only put missiles in the quiver.",quiet);
        return false;
      }  
    
    you.equip[EQ_QUIVER]=throw_slot;        
    in_name(throw_slot, DESC_INVENTORY, buf);
    sprintf(str_pass,"You put into the quiver %s",buf);
    mpr2(str_pass,quiet);
    return true;
 }     
 
 
void extended_keyinput()
  {
    char key;
        
    mesclr();
    mpr("Enter command: #");
    key=getch();
    mesclr();
    switch(key)
      {
      	//special keys need two calls to getch()
      	case 0: 
      	  key=getch();
      	  break;
      	// dump map  
      	case 'M':
      	case 'm':  
      	  mpr("#Map dump");
      	  DumpMap(true);
      	  break;
      	
      	//dump char
      	case 'D':
      	case 'd':
      	  mpr("#Dump character info"); 
          if (dump_char(0, NULL) == 1)
             strcpy(info, "Char dumped successfully.");
          else
             strcpy(info, "Char dump unsuccessful! Sorry about that.");
          mpr(info);
          break;  
       //quiver   
       case 'Q':
       case 'q':
         mpr("#Quiver");
         Quiver();
         break; 
       case 'E':
       case 'e':
         Equipment();
         break;  
      }		
  }	    
  
/*--------------------------------------------------------------------

Show the equipment screen

.- Equipment() is the main rotuine
.- show_equip  highlites on/off an item of the menu 
----------------------------------------------------------------------*/

int show_item_armour(struct tMenu& mnu,int bSelec)
  {
    int n;
    char *cad;
    string description;
     
     if (bSelec==4) return 1;
     //highlight on/off for the current item of the menu
     cad=(char *)mnu.data+(mnu.nItem*100);
     gotoxy(43,mnu.nFila);     
     textcolor(WHITE);
     if (bSelec)
        {           
           textbackground(RED);           
        }   
     else
        {
           textbackground(BLACK);
        }    
        
     cprintf(cad);
     for (n=strlen(cad)+43;n<=79;n++) cprintf(" ");
     textbackground(BLACK);              
     if (bSelec)
       {
        
          ClearScreen(2,5,41,23);
          description.reserve(500);                 
          n=mnu.data2[mnu.nItem];
          if (n!=-1)
            {
              description=get_item_description(you.inv[n],1);
              print_description(description,41-3,3,5,-1);
            }  
       }     
  }	      

void EquipMsg(char *msg)
  {
          
     gotoxy((79-strlen(msg))/2,24);textcolor(YELLOW);
     cprintf("[ %s ]",msg);
     getch();
     textcolor(LIGHTGREY);
  }          
            
void SelectAmour(int which)
  {
    struct tMenu mnu;
    char data[ENDOFPACK][100];
    char data2[ENDOFPACK];
       
    int nItem=99;
    int n,m;
    
    mnu.nFilaMax=23;
    mnu.nFilaIni=6;
    mnu.print_item=show_item_armour;
    mnu.handle_key=NULL;
    mnu.data=(char *)data;
    mnu.data2=(char*)data2;
    m=-1;
    ClearScreen(42,5,79,23);     
    for (n=0;n<ENDOFPACK;n++)
      { 
        if (!you.inv[n].quantity ||!is_valid_item(you.inv[n])) continue;
        
        if (you.inv[n].base_type!=OBJ_ARMOUR) continue;
        
        if (aObjs[OBJ_ARMOUR][you.inv[n].sub_type].slot==which)
          {
            in_name(n,DESC_INVENTORY,str_pass);m++;
            strcpy(data[m],str_pass);
            data[m][35]=0;
            data2[m]=n;
          }  
        
      } 
    if (m==-1)
      {
        EquipMsg("You haven't any suitable item for this slot.");
        return;
      }    
    mnu.nMaxItems=m;  
    nItem=menu(mnu,nItem!=99);   
    if (nItem==-1) return;
    if (!do_wear_armour(mnu.data2[mnu.nItem],true,false))
       {
          gotoxy((79-strlen(str_pass))/2,24);textcolor(YELLOW);
          cprintf("[ %s ]",str_pass);
          getch();
       } 
    
  }
  
  
int SelectWeapon()
  {
    struct tMenu mnu;
    char data[ENDOFPACK][100];
    char data2[ENDOFPACK];
       
    int nItem=99;
    int n,m;
    
    mnu.nFilaMax=23;
    mnu.nFilaIni=6;
    mnu.print_item=show_item_armour;
    mnu.handle_key=NULL;
    mnu.data=(char *)data;
    mnu.data2=(char*)data2;
    m=-1;
    ClearScreen(42,5,79,23);     
    for (n=0;n<ENDOFPACK;n++)
      { 
        if (!you.inv[n].quantity ||!is_valid_item(you.inv[n])) continue;
        
        if (you.inv[n].base_type!=OBJ_WEAPONS && you.inv[n].base_type!=OBJ_STAVES) continue;
        
        in_name(n,DESC_INVENTORY,str_pass);m++;
        str_pass[35]=0;
        strcpy(data[m],str_pass);
        data2[m]=n;
      }
    if (m==-1)
      {
        EquipMsg("You haven't any weapon.");
        return -1;
      }   
    mnu.nMaxItems=m;  
    nItem=menu(mnu,nItem!=99);   
    if (nItem==-1) 
      return -1;
    else
      return mnu.data2[mnu.nItem];        
  }  


int SelectMissiles()
  {
    struct tMenu mnu;
    char data[ENDOFPACK][100];
    char data2[ENDOFPACK];
       
    int nItem=99;
    int j,m;
    
    mnu.nFilaMax=23;
    mnu.nFilaIni=6;
    mnu.print_item=show_item_armour;
    mnu.handle_key=NULL;
    mnu.data=(char *)data;
    mnu.data2=(char*)data2;
    m=-1;
    ClearScreen(42,5,79,23);     
    for (j=0;j<ENDOFPACK;j++)
      { 
        if (!you.inv[j].quantity ||!is_valid_item(you.inv[j])) continue;

        if (you.inv[j].base_type!=OBJ_MISSILES) continue;
        
        in_name(j,DESC_INVENTORY,str_pass);m++;
        str_pass[35]=0;
        strcpy(data[m],str_pass);
        data2[m]=j;
      } 
    if (m==-1)
      {   
          EquipMsg("You haven't any missile");
          return -1;
      }     
    mnu.nMaxItems=m;  
    nItem=menu(mnu,nItem!=99);   
    if (nItem==-1) 
      return -1;
    if (!Quiver(false,mnu.data2[mnu.nItem]))
       {
          gotoxy((79-strlen(str_pass))/2,24);textcolor(YELLOW);
          cprintf("[ %s ]",str_pass);
          getch();
       }    
  }  
    
int show_equip(struct tMenu& mnu,int bSelec)
  {
    int n,m;
    char *cad;
    string description;
     
     if (bSelec==4) return 1;
     //highlight on/off for the current item of the menu
     cad=(char *)mnu.data+(mnu.nItem*120);

     gotoxy(11,mnu.nFila);
     n=mnu.data2[mnu.nItem];
     if (n==-1)
       textcolor(DARKGREY);
     else  
       textcolor(WHITE);
     if (bSelec)
        {           
           textbackground(RED);           
        }   
     else
        {
           textbackground(BLACK);
        }    
        
     cprintf(cad);
     for (m=strlen(cad)+11;m<=39;m++) cprintf(" ");
     textbackground(BLACK);
     if (bSelec)
       {        
          ClearScreen(42,5,79,23);                    
          if (n!=-1)
            {
              description.reserve(500);                    
              description=get_item_description(you.inv[n],1);
              print_description(description,79-41,43,5,-1);
            }  
       }         
  }	      
void Equipment()  
  { 
#ifdef DOS_TERM
    char buffer[4800];

    window(1, 1, 80, 25);
    gettext(1, 1, 80, 25, buffer);
#endif

    
    struct tMenu mnu;
    char data[NUM_EQUIP][120];
    char data2[NUM_EQUIP];
    char equip[NUM_EQUIP];
       
    int nItem=99;
    int n,m;
    
    //make a backup of your equip
    for (n=0;n<NUM_EQUIP;n++) equip[n]=you.equip[n];
    
    mnu.nFilaMax=23;
    mnu.nFilaIni=6;
    mnu.print_item=show_equip;
    mnu.handle_key=NULL;
    mnu.data=(char *)data;
    mnu.data2=(char*)data2;
    mnu.nMaxItems=NUM_EQUIP-1;
    for(;;)
      {         
    
    for (n=0;n<NUM_EQUIP;n++)
      {       
        m=you.equip[n];
        if (m!=-1)
          {
            in_name(m,DESC_INVENTORY,str_pass);                         
            strcpy(data[n],str_pass);
            data2[n]=m;
          }  
        else
          {
            strcpy(data[n],"(none)");  
            data2[n]=-1;            
          }  
        data[n][39]=0;  
      }
    
        AFormatScreen("Equipment",40);  
        
        textcolor(LIGHTGREY);
        gotoxy(3,6); cprintf("Weapon:");
        gotoxy(3,7); cprintf("Cloak :");
        gotoxy(3,8);cprintf("Helmet:");
        gotoxy(3,9);cprintf("Gloves:");
        gotoxy(3,10);cprintf("Boots :");
        gotoxy(3,11); cprintf("Shield:");
        gotoxy(3,12); cprintf("Armour:");
        gotoxy(3,13);cprintf("L.Ring:");
        gotoxy(3,14);cprintf("R.Ring:");
        gotoxy(3,15);cprintf("Amulet:");
        gotoxy(3,16); cprintf("Quiver:");
    
    
        nItem=menu(mnu,nItem!=99);
        switch(nItem)
          {
            case EQ_CLOAK:   
            case EQ_HELMET:
            case EQ_GLOVES:
            case EQ_BOOTS:
            case EQ_SHIELD:
            case EQ_BODY_ARMOUR:                   
               SelectAmour(nItem);
               break;
            case EQ_WEAPON:
               nItem=SelectWeapon();
               if (nItem>-1)
                 {
                    redraw_screen();
                    #ifdef DOS_TERM
                       puttext(1, 1, 80, 25, buffer);
                    #endif
                    if (nItem>-1) wield_weapon(false,nItem);    
                    more();
                 }   
               break;   
            case EQ_QUIVER:
               nItem=SelectMissiles();
               break;   
          }  
        if (nItem==-1) break;  
      }  
    
    

    redraw_screen();

#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif

    //If wearing armour takes no time, return
    if (xtraOps.ax_wear_no_time) return;
    //restore backup
    for (n=0;n<NUM_EQUIP;n++) 
       {
       	  if (equip[n]!=you.equip[n])
       	    {
       	    	int delay = property( you.inv[you.equip[n]], PARM_AC );
                if (delay < 1) delay = 1;
                if (delay) start_delay( DELAY_ARMOUR_ON, delay, you.equip[n] );
                you.equip[n]=equip[n];
       	    }	
       }	  
  }  