/*
 userscr.cc
 
 User defined screens.
 
 It has rotuines to show the user defined screens: 
 
    .- an intro screen 
    .- to ask the name of the player.
    .- to say "see  you soon" when leaving the game
 
 These screens are user-definined by external files stored in 
 subdirectory "data"
 
 
 Alex: 16.11.01
*/ 

#include "AppHdr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include "externs.h"
#include "doc.h" 
#include "enum.h"
#include "version.h"

void openingScreen(void)
{
    if (!Options.skip_intro) 
      if (!ListFile("data/intro.txt"))
        {
        	cprintf("Hello, welcome to Dungeon Crawl " VERSION "!");
                cprintf(EOL "(c) Copyright 1997-2001 Linley Henzell");
                cprintf(EOL "Please consult crawl.txt for instructions and legal details." EOL);
        }	
}                               // end openingScreen()


void player_id()
  {
    
    char namefile[]="data/nmplayer.txt";
    
    
    if(!ListFile(namefile)) 	    
      {
      	clrscr();
      	cprintf("Not found data/nmplayer.txt");
      	exit(1);
       } 	     
  }
	
                             // end enterPlayerName()

bool verifyPlayerName(int blankOK)
{ 
    if (strlen(you.your_name)==0 && blankOK) return true;
    	
    if (strlen(you.your_name)<3)
      {
      	  cprintf("A somewhat short name for that brave adventurer");
      	  return false;
      }
       	  
#if defined(DOS) || defined(WIN32CONSOLE)
    static int william_tanksley_asked_for_this = 2;

    // quick check for CON -- blows up real good under DOS/Windows
    if (stricmp(you.your_name, "con") == 0)
    {
        cprintf("Sorry, that name gives your OS a headache.");
        return false;
    }

    // quick check for LPTx -- thank you,  Mr. Tanksley!   ;-)
    if (strnicmp(you.your_name, "LPT", 3) == 0)
    {
        switch (william_tanksley_asked_for_this)
        {
            case 2:
                cprintf("Hello, William!  How is work on Omega going?");
                break;
            case 1:
                cprintf("Look, it's just not a legal name.");
                break;
            case 0:
                strcpy(you.your_name, "William");
                return true;
        } // end switch

        william_tanksley_asked_for_this --;
        return false;
    }
#endif

     for (unsigned int i = 0; i < strlen(you.your_name); i++)
    {


        if (you.your_name[i] == ':')
        {
            cprintf("No colons, please.");
            return false;
        }

        // for other systems we'll be super-weak and rule out
        // everything but alpha-numeric characters and underscore
        if (!isalnum(you.your_name[i]) && you.your_name[i] !='_')
        {
            cprintf("No non-alphanumerics, please.");
            return false;
        }

    }     return true;
}                               // end verifyPlayerName()


void enterPlayerName(bool blankOK)
{
    bool acceptable_name = false;
   
    do
    {   
        player_id();
        acceptable_name = verifyPlayerName(blankOK);        
        if (acceptable_name) return;
        getch();
    }
    while (1);
}  



int see_you_soon()
  {
    return ListFile("data/seesoon.txt"); 	
  }
  
  
int death_screen()
  {
    int n=ListFile("data/death.txt"); 		
    clrscr();
    return n;
  }	
