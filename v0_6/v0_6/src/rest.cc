//-------------------------------------------------
//
// Rest.c
//
// Code for handle resting 
//
// Alex: 15.11.01
//-------------------------------------------------

#include "AppHdr.h"
#include "message.h"
#include "religion.h"

#include <string.h>

#ifdef DOS
  #include <conio.h>
#endif

#include "externs.h"


//do_Resting: until you recover you HP/MP
//don't accept keyboard input. 
//When you finish resting, return getch() 
//else return '.' (search).
//
//Alex: 21.09.01

 int do_resting()
   {
      switch(you.resting)
        { 
          case 2:  
            if (you.max_magic_points == you.magic_points) 
              {
                 you.resting=0;
                 return getch();
              }   
          case 3:
            if (you.max_magic_points != you.magic_points) return '.';
            //intentional fallthrough
          case 1:
            if (you.hp_max == you.hp)
               {
                  you.resting=0;
                  return getch();                  
               }
            return '.';      
        }    
   } 
   
   
//Return true if you are resting
//Alex:21.09.01   
int still_resting()
  {
    //FixedVector < char, 2 > mfp;
    //if (mons_find(you.x_pos,you.y_pos,mfp,1)) return 1;else return you.resting;
    return you.resting;
  } 	   
  

//If you are resting, interrupt resting and show a message 
//
//Alex:15.11.01
void stop_resting(char *cad=NULL)
 {
   if (you.resting) 	
      {
      	you.resting=0;
      	if (cad==NULL)
      	   mpr("You stop resting.",MSGCH_WARN);
      	else
      	   mpr(cad,MSGCH_WARN);   
      }	
 }	  
 
//Interface to command "R" (resting)
//
//Alex: 21.09.10 
void do_rest()
 {
   unsigned char tmp;

    for (;;)
    {            
        mesclr();
        mpr("Rest until recovered");
        mpr("[H]P  [M]P  [B]oth  [C]ancel");

        tmp = (unsigned char) getch();
        tmp = toupper( tmp );
        
        mesclr();
        
        switch(tmp)
          {
            case 'H':
               you.resting=1;
               return;
            case 'M':        
               you.resting=2;
               return;
            case 'B':
               you.resting=3;
               return;
            case 'C': 
               return;
          }  
    }
 }   


 
 