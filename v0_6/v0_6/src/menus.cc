/*******************************************************************************************
*
* Stupid routines for handing scrolling menus
*
* The user must supply the exact routines to load and show
* each one of their menus. 
*
* Alex: 25.07.01
*******************************************************************************************/
#include "AppHdr.h"
#include "stdio.h"
#include "conio.h"
#include "string.h"
#include "menus.h"
#include "io.h"
#include "stdlib.h"
#include "stuff.h"

int bKeySpecial=0;  
unsigned char gt_c(void)
{
    unsigned char gotched;

    gotched= getch();

    if (gotched == 0)
        { 
           bKeySpecial=gotched;
           gotched = getch();
        }
    else
        bKeySpecial=0;        
           

    return gotched;
}


int clear_items(struct tMenu mnu)
  {                             
   
    for (mnu.nItem=0,mnu.nFila=mnu.nFilaIni;mnu.nFila<=mnu.nFilaMax &&mnu.nItem<=mnu.nMaxItems;mnu.nFila++)
       {
          
          mnu.print_item(mnu,4);
       }             
  }
  
int show_items(struct tMenu mnu)
  {   
   
    for (mnu.nFila=mnu.nFilaIni,mnu.nItem=0;mnu.nFila<=mnu.nFilaMax && mnu.nItem<=mnu.nMaxItems;mnu.nFila++,mnu.nItem++)
       {                        
          mnu.print_item(mnu,4);
          mnu.print_item(mnu,0);
       }             
  }

//Show the items of the menu 
//but respecting mnu.nFila and mnu.nItem
int show_items2(struct tMenu mnu)
  { 
    int nItem=mnu.nItem;
    
    int nFila=mnu.nFila;	  
     
    clear_items(mnu); 
    for (mnu.nItem=mnu.nItemFirstRow,mnu.nFila=mnu.nFilaIni;mnu.nFila<=mnu.nFilaMax && mnu.nItem<=mnu.nMaxItems;mnu.nFila++,mnu.nItem++)
       {   	  
          mnu.print_item(mnu,0);
       }
    mnu.nItem=nItem;
    mnu.nFila=nFila;   
    if (mnu.nMaxItems>=0 && nItem>=0) mnu.print_item(mnu,1);                
  }

  
int menu(struct tMenu& mnu,int bRespectActualState=0)
  {
    unsigned char c;
    int nItem;
    if (!bRespectActualState)
      {
        show_items(mnu);
        mnu.nItem=0;
        mnu.nFila=mnu.nFilaIni;        
        mnu.nItemFirstRow=0;
        if (mnu.nMaxItems>=0) mnu.print_item(mnu,1);           
      } 
    else
      {
       show_items2(mnu); 
      }  
    
    for(;;)
      {
      	 bKeySpecial=0;
      	 _setcursortype(false);
         c=getch();
         _setcursortype(true);
         switch(c)
           {
             case 27:
                mnu.print_item(mnu,0);    
                return -1;
                break;
             case 13:
               if (mnu.handle_key!=NULL) 
                   {
                      nItem=mnu.handle_key(mnu,c);
                      switch(nItem)
                      	{
                      	  case -2:
                      	       break;
                      	  default:
                      	       return nItem;   
                      	 }     
                   }
               else       
                  return mnu.nItem;
               break;
             case 0:
                bKeySpecial=1;
                _setcursortype(false);
                c=getch();
                _setcursortype(false);
                //gotoxy(1,40);cprintf("%d %d %d",mnu.nItem,mnu.nMaxItems,mnu.nFilaIni);
                //getch();
                switch(c)
                  {
                  //up
                    case 72:

                        if (mnu.nItem<=0) break;
                        
                        mnu.print_item(mnu,0);
                        mnu.nItem--;
                        mnu.nFila--;
                        if (mnu.nFila<mnu.nFilaIni)
                            {
                               mnu.nFila=mnu.nFilaIni;
                               mnu.nItemFirstRow--;
                               show_items2(mnu);
                             }
                        else     
                           mnu.print_item(mnu,1);
                        break;
                  //down
                     case 80:                                          
                        if (mnu.nItem>=mnu.nMaxItems) break;
                         mnu.print_item(mnu,0);
                         mnu.nItem++;
                         mnu.nFila++;
                         if (mnu.nFila>mnu.nFilaMax)
                            {
                               mnu.nFila--;
                               nItem=mnu.nItem;
                               mnu.nItemFirstRow++;
                               //mnu.nItem=mnu.nItem-(mnu.nFilaMax-mnu.nFilaIni);
                               show_items2(mnu);
                               mnu.nFila=mnu.nFilaMax;
                               mnu.nItem=nItem;
                             }
                        mnu.print_item(mnu,1);
                        break;                                       
                     default:
                        if (mnu.handle_key!=NULL) 
                      	  {
                      	    nItem=mnu.handle_key(mnu,c); 	 
                      	    switch(nItem)
                      	      {
                      	        case -2:
                      	           break;
                      	        default:
                      	           return nItem;   
                      	      }     
                      	     
                      	  }   
                }
                break; 
               default: 
                        if (mnu.handle_key!=NULL) 
                      	  {
                      	    nItem=mnu.handle_key(mnu,c); 	 
                      	    switch(nItem)
                      	      {
                      	        case -2:
                      	           break;
                      	        default:
                      	           return nItem;   
                      	      }     
                      	     
                      	  }     
                //gotoxy(1,1);printf("%c  %d",c,c);
           }
      }
  }

int menu2(struct tMenu& mnu,void (*scrn_update)(),int bRespectActualState=0)
{ 
    #ifdef DOS_TERM
      char buffer[4800];
  
      window(1, 1, 80, 25);
      gettext(1, 1, 80, 25, buffer);
   #endif 
	 
   scrn_update();
   	 
   int n=menu(mnu,bRespectActualState); 
   redraw_screen();
    #ifdef DOS_TERM
      puttext(1, 1, 80, 25, buffer);
    #endif	
    
    return n; 
}	

void Box(int x1,int y1,int x2,int y2)
 {
    int n;
    
    gotoxy(x1,y1);cprintf("É");
    gotoxy(x1,y2);cprintf("È");
    gotoxy(x2,y1);cprintf("»");
    gotoxy(x2,y2);cprintf("¼");
    
    for(n=x1+1;n<x2;n++)
      {
        gotoxy(n,y1);cprintf("Í");
        gotoxy(n,y2);cprintf("Í");
      }
    for(n=y1+1;n<y2;n++)
      {
       gotoxy(x1,n);cprintf("º");
       gotoxy(x2,n);cprintf("º");
      }
 }


void ClearScreen(int nColIni,int nRowIni,int nColEnd,int nRowEnd,int colour=0)
  {
    int nCont,width;	
    textbackground(colour);
    textcolor(colour);
    width=nColEnd-nColIni;
    for (nCont=nRowIni;nCont<=nRowEnd;nCont++)
            {
                gotoxy(nColIni,nCont);cprintf("%*s",width,"");
            }      
  }	
  
char *read_desc_fich(char *nomfich)
 {
   char *races;
   FILE *fich=fopen(nomfich,"rt+");
   int flen;

   if (fich == NULL) {return NULL;}

   flen=filelength(fileno(fich));


   races=(char *)malloc(flen);
   flen=fread(races,1,flen,fich);
   fclose(fich);
   return races;
 }



void AFormatScreen(char *Text,int nCol,int colour=MAGENTA)
  {     
   int n;
    
   textbackground(BLACK);   
   clrscr();
   textcolor(colour);    
   Box(1,1,80,24);
   textcolor(YELLOW);
   gotoxy(5,2);cprintf(Text);
   textcolor(colour);       
   gotoxy(2,3);
   for(n=2;n<=79;n++) putch(205);
   gotoxy(80,3);putch(185);    
   gotoxy(1,3);putch(204);
   
   textbackground(BLACK);
        
   textcolor(colour);       
   
   if (nCol>-1)
     {
       for (n=4;n<=23;n++)
         {
           gotoxy(nCol+1,n);putch(186);
         }   
     
       gotoxy(nCol+1,3);putch(203);
       gotoxy(nCol+1,24);putch(202);
    }   
   
   textcolor(WHITE);
  }  	  