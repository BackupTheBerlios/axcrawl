/*----------------------------------------------------------
doc.cc

Routines to show documentation stored
in ASCII files.

Usage: 
        .- show_file(char *filename, int line=0, int mode=0700)
        .- do_help
        .- WriteLine(char *line)
        
Alex: 17.09.01
------------------------------------------------------------*/
#include "AppHdr.h"
#include <dir.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef DOS
#include <conio.h>
#endif

#include <file.h>
#include "strutil.h"
#include "stuff.h"
#include "AppHdr.h"
#include "externs.h"
#include "player.h"


//Commands for WriteLine
char WL_cmds[][25]=
 {
  "BLACK",
  "BLUE",
  "GREEN",
  "CYAN",
  "RED",
  "MAGENTA",
  "BROWN",
  "LIGHTGREY",
  "DARKGRAY",
  "LIGHTBLUE",
  "LIGHTGREEN",
  "LIGHTCYAN",
  "LIGHTRED",
  "LIGHTMAGENTA",
  "YELLOW",
  "WHITE",
  "POINTS",
  "NAME",
  "WAIT",
  "ENTERPLAYERNAME",
  "CLASS",
  "BACKCOLOR",
  "XY",
  "A_RACE",
  "QUOTE",
  "RACE"
 };

#define NWL_CMDS (sizeof(WL_cmds) / sizeof(WL_cmds[1]))-1

void GetQuote(char *namefich,char *buf)
 {
    FILE *fich;
    int n;
    int wich;
    int nquotes;

    fich=fopen(namefich,"r");
    if (fich==NULL) return;
    if (fgets(buf,120,fich)==NULL) return;
    nquotes=atoi(buf);
    wich=random () % nquotes;
    buf[0]=0;
    for (n=0;n<wich;n++)
      {
        if (fgets(buf,120,fich)==NULL)
          {
            break;
          }
      }
    fclose(fich);
 }

void Quote()
  {

    char *namefich;
    int  width;
    char buf[120];
    int x,y,x2,wich;

    namefich=strtok(0,":");
    if (namefich==NULL) return;
    strcpy(buf,strtok(0,":"));
    width=atoi(buf);
    x=atoi(strtok(0,":"));
    y=atoi(strtok(0,":"));
    x2=atoi(strtok(0,":"));
    GetQuote(namefich,buf);

    if (width!=0) buf[abs(width)-1]=0;

    if (x!=0 && y!=0) gotoxy(x,y);

    if (x2!=0)
      {
        width=((x2-x)-strlen(buf))/2;
        gotoxy(x+width,y);cprintf(buf);
      }
    else  
      cprintf("%*s",width,buf);

  }

//prints "a/an" species_name
void print_a_race()
  {
     bool startVowel = (strchr("aeiouAEIOU", info[0]) != NULL);
     cprintf("a%s %s", (startVowel)?"n":"",species_name(you.species)); 
  }	

//print species_name
void print_race()
  {
    cprintf(species_name(you.species));	
  }

void print_points()
  {
    cprintf("%d",game_points);	
  }	  
  
//gotoxy
void xy(int &nCol,int &nRow)
  {
    	
    nCol=atoi(strtok(0,":"));
    nRow=atoi(strtok(0,":"));
    
    gotoxy(nCol,nRow);
    
    //ListFile inc nRow before print
    nRow--;
  }	  	


//background colour
void backcolor()
  {
     char *buf;	
     int n;
          
     buf=strtok(0,":");
    
    for (n=0;n<=NWL_CMDS-4;n++)
       if (strcmp(buf,WL_cmds[n])==0)
          {
       	    textbackground(n);
       	    return;
          }
  }          

//print your class
void print_class()
  {
    cprintf(you.class_name);
  }	

//enter the player name
void enter_playername()
  {
    int width;
    int x_error,y_error;
    
    width=atoi(strtok(0,":"));
    x_error=atoi(strtok(0,":"));
    y_error=atoi(strtok(0,":"));
    askfor_aux(you.your_name,width,wherex(),wherey());
    gotoxy(x_error,y_error);
  }	

//print name of the player
void player_name()
  {
    cprintf(you.your_name);	
  }	  
  
void ParseCmd(char *command,int &nCol,int &nRow)
 {
   int n;
   char *cmd;
   cmd=strtok(command,":");
   for (n=0;n<=NWL_CMDS;n++)
     if (strcmp(cmd,WL_cmds[n])==0)
       {
          switch(n)
           {
             case NWL_CMDS-1:
               Quote();
               return;
             case NWL_CMDS:
               print_race();
               return;  
             case NWL_CMDS-2:
               print_a_race();
               return;  
             case NWL_CMDS-3:
               xy(nCol,nRow);
               return;  
             case NWL_CMDS-4:
               backcolor();
               return;    
             case NWL_CMDS-5:
               print_class();
               return;      
             case NWL_CMDS-6:
               enter_playername();  
               return;
             case NWL_CMDS-7:
               _setcursortype(false);
               getch();  
               _setcursortype(true);
               return;  
             case NWL_CMDS-8:
               player_name();  
               return;    
             case NWL_CMDS-9:
               print_points();
               return;  
             default:
               textcolor(n);return;
           }

       }


 }

void WriteLine(char *line,int &nCol,int &nRow)
 {
   char *cad;
   char *p;
   int n;
   char *str;
   str=line;
   for (;str[0];)
     {
       cad=strstr(str,"$:");
       if (cad==NULL)
         {
           cprintf(str);
           return;
         }
       n=cad-str;
       str[n]=0;
       cprintf(str);
       str[n]='$';
       cad++;cad++;
       p=cad;
       p=strstr(cad,":$");
       if (p==NULL)
        {
         cprintf(cad);return;
        }
       n=p-cad;
       cad[n]=0;
       ParseCmd(cad,nCol,nRow);
       cad[n]=':';
       p++;p++;
       str=p;
     }
  }
  


/*
 * Recursive file perusal.
 *
 * Return FALSE on "ESCAPE", otherwise TRUE.
 *
 * Process various special text in the input file, including the "menu"
 * structures used by the "help file" system.
 *
 * This function could be made much more efficient with the use of "seek"
 * functionality, especially when moving backwards through a file, or
 * forwards through a file by less than a page at a time.  XXX XXX XXX
 *
 * Consider using a temporary file, in which special lines do not appear,
 * and which could be pre-padded to 80 characters per line, to allow the
 * use of perfect seeking.  XXX XXX XXX
 *
 * Allow the user to "save" the current file.  XXX XXX XXX
 */
int show_file(char *name, int line=0, int bNotMenu=0)
{
    int i, k, n, m;

    char ch;

    /* Number of "real" lines passed by */
    int next = 0;

    /* Number of "real" lines in the file */
    int size;

    /* Backup value for "line" */
    int back = 0;

    /* This screen has sub-screens */
    bool menu = FALSE;

    /* Case sensitive search */
    bool case_sensitive = FALSE;

    /* Current help file */
    FILE *fff = NULL;

    /* Find this string (if any) */
    char *find = NULL;

    /* Jump to this tag */
    cptr tag = NULL;

    /* Hold a string to find */
    char finder[80];

    /* Hold a string to show */
    char shower[80];

    /* Filename */
    char filename[1024];

    /* Describe this thing */
    char caption[128];

    /* Path buffer */
    char path[1024];

    /* General buffer */
    char buf[1024];

    /* Lower case version of the buffer, for searching */
    char lc_buf[1024];

    /* Sub-menu information */
    char hook[10][32];

    int wid, hgt,nLines;
    
    int bFixSection;

    /* Wipe finder */
    strcpy(finder, "");

    /* Wipe shower */
    strcpy(shower, "");

    /* Wipe caption */
    strcpy(caption, "");

    /* Wipe the hooks */
    for (i = 0; i < 10; i++) hook[i][0] = '\0';

    /* Get size */
    wid=80;
    hgt=24;

    /* Copy the filename */
    strcpy(filename, name);

    n = strlen(filename);

    /* Extract the tag from the filename */
    for (i = 0; i < n; i++)
    {
        if (filename[i] == '#')
        {
            filename[i] = '\0';
            tag = filename + i + 1;
            break;
        }
    }

    /* Redirect the name */
    name = filename;
    bFixSection=0;
    
    /* openfile" */
    if (!fff)
    {
        /* Caption */
        sprintf(caption, "Help file '%s'", name);

        /* Build the filename */
        strcpy(path,name);

        /* Open the file */
        fff = fopen(path, "r");
    }

    /* Oops */
    if (!fff)
    {
        /* Message */
        gotoxy(1,hgt);printf("Cannot open '%s'.", name);
        

        /* Oops */
        return (TRUE);
    }

    /* Pre-Parse the file */
    while (TRUE)
    {
        /* Read a line or stop */
        if (!fgets(buf, 1024,fff)) break;

        /* XXX Parse "menu" items */
        if (prefix(buf, "***** "))
        {
            char b1 = '[', b2 = ']';

            /* Notice "menu" requests */
            if ((buf[6] == b1) && isdigit(buf[7]) &&
                (buf[8] == b2) && (buf[9] == ' '))
            {
                /* This is a menu file */
                menu = TRUE;

                /* Extract the menu item */
                k = D2I(buf[7]);

                /* Extract the menu item */
                strcpy(hook[k], buf + 10);
                n=strlen(hook[k]);
                for(m=0;m<n;m++)
                  if (hook[k][m]=='\r' || hook[k][m]=='\n')
                     {hook[k][m]='\0';}
            }

            /* Notice "tag" requests */
            else if (buf[6] == '<')
            {
                if (tag)
                {
                    /* Remove the closing '>' of the tag */
                    buf[strlen(buf) - 1] = '\0';

                    /* Compare with the requested tag */
                    if (streq(buf + 7, tag))
                    {
                        /* Remember the tagged line */
                        line = next;
                    }
                }
            }

            /* Skip this */
            continue;
        } 
      /* Skip lines with options */  
      else if(buf[0]=='$' && buf[1]==':') {bFixSection;continue; }

        /* Count the "real" lines */
        next++;
    }

    /* Save the number of "real" lines */
    size = next;

    /* Display the file */
    while (TRUE)
    {
        /* Clear screen */
            clrscr();

        /* Restart when necessary */
        if (line >= size) line = 0;

        /* Re-open the file if needed */
        if (next > line)
        {
            /* Close it */
            fclose(fff);

            /* Hack -- Re-Open the file */
            fff = fopen(path, "r");

            /* Oops */
            if (!fff) return (TRUE);

            /* File has been restarted */
            next = 0;
        }

        /* Goto the selected line */
        
        i=0;
        while (next < line || bFixSection)
        {
            /* Get a line */
            if (!fgets( buf, 1024,fff)) break;

            /* Skip tags/links */
            if (prefix(buf, "***** ")) continue;   
            
            
            /* Inside a fix section? */
            if (bFixSection)
              {
                 /* End of fix section?*/    
                 if(prefix(buf,"$:/FIXSECTION"))
                   {
                    bFixSection=0;
                    continue;
                   }              
                  gotoxy(1, ++i);WriteLine(buf,m,m);
              }     
            else  
              /*Fix Section ?*/
              if (prefix(buf,"$:FIXSECTION")) 
                {          
                  bFixSection=1;
                  continue;
                }
            
            

            /* Count the lines */
            next++;
        }
        
        textcolor(WHITE);
        /* Dump the next lines of the file */
        if (!bNotMenu) nLines=hgt-2; else nLines=hgt-1;
        
        for (; i < nLines; i++)
        {
            /* Hack -- track the "first" line */
            if (!i) line = next;

            /* Get a line of the file or stop */
            if (!fgets(buf, 1024,fff)) break;

            /* Hack -- skip "special" lines */
            if (prefix(buf, "***** ")) continue;

            /* Count the "real" lines */
            next++;

            /* Make a copy of the current line for searching */
            strcpy(lc_buf, buf);

            /* Make the line lower case */
            if (!case_sensitive) strupr(lc_buf);

            /* Hack -- keep searching */
            if (find && !i && !strstr(lc_buf, find)) continue;

            /* Hack -- stop searching */
            find = NULL;

            /* Dump the line */
            gotoxy(1, i+1);WriteLine(buf,m,m);

            /* Hilite "shower" */
            if (shower[0])
            {
                cptr str = lc_buf;

                //Display matches 
                while ((str = strstr(str, shower)) != NULL)
                {
                    int len = strlen(shower);

                    // Display the match 
                    gotoxy(str-lc_buf+1,i+1);textcolor(YELLOW);
                    for (m=0;m<len;m++)
                       cprintf("%c",buf[str-lc_buf+m]);
                    textcolor(WHITE);
                    //Term_putstr(str-lc_buf, i+2, len, TERM_YELLOW, &buf[str-lc_buf]);

                    // Advance 
                    str += len;
                }
            }

        }

        /* Hack -- failed search */
        if (find)
        {
            gotoxy(1,hgt-1);printf("Search string not found!");
            line = back;
            find = NULL;
            continue;
        }

        
        /* Prompt -- menu screen */
        if (!bNotMenu)
          {
             textcolor(CYAN);
             gotoxy(1,hgt-1);
             for(n=1;n<=wid;n++) cprintf("-");
             if (menu)
             {
                 /* Wait for it */
                 gotoxy(1,hgt);
                 cprintf("[Press a Number, or ESC to exit.]");        
             }
     
             /* Prompt -- small files */
             else if (size <= hgt - 2)
             {
                 /* Wait for it */
                 gotoxy(1,hgt);
                 cprintf("[Press ESC to exit.]");                 
             }
     
             /* Prompt -- large files */
             else
             {
                 /* Wait for it */
                 gotoxy(1,hgt);
                 cprintf("[Press ESC, - ,_ , + , = , space , / , &]");
             }
             textcolor(WHITE);
          } 
        /* Get a keypress */
        ch = getch();

        /* Return to last screen */
        if (ch == '?') break;

        /* Toggle case sensitive on/off */
        if (ch == '!')
        {
            case_sensitive = !case_sensitive;
        }

        /* Try showing */
        if (ch == '&')
        {
            /* Get "shower" */
            gotoxy(1,hgt-1);printf("Show:");
            shower[0]='\0';
            (void)askfor_aux(shower, 80,9,hgt-1,1);

            /* Make the "shower" lowercase */
            if (!case_sensitive) strupr(shower);
        }

        /* Try finding */
        if (ch == '/')
        {
            /* Get "finder" */
            gotoxy(1,hgt-1);printf("Find: ");
            finder[0]='\0';
            if (askfor_aux(finder, 80,8,hgt-1))
            {
                /* Find it */
                find = finder;
                back = line;
                line = line + 1;

                /* Make the "finder" lowercase */
                if (!case_sensitive) strupr(finder);

                /* Show it */
                strcpy(shower, finder);
            }
        }

        /* Go to a specific line */
        if (ch == '#')
        {
            char tmp[80];
            gotoxy(1,hgt-1);printf("Goto Line: ");
            strcpy(tmp, "\0");
            if (askfor_aux(tmp, 80,12,hgt-1),1)
            {
                line = atoi(tmp);
            }
        }

        /* Go to a specific file */
        if (ch == '%')
        {
            char ftmp[80];
            gotoxy(1,hgt-1);printf("Goto File: ");
            strcpy(ftmp, "help.hlp");
            if (askfor_aux(ftmp, 80,12,hgt-1),1)
            {
                if (!show_file(ftmp,  0, bNotMenu)) ch = ESCAPE;
            }
        }

        /* Back up one line */
        if (ch == '=')
        {
            line = line - 1;
            if (line < 0) line = 0;
        }

        /* Back up one half page */
        if (ch == '_')
        {
            line = line - ((hgt - 4) / 2);
            if (line < 0) line = 0;
        }

        /* Back up one full page */
        if (ch == '-')
        {
            line = line - (hgt - 4);
            if (line < 0) line = 0;
        }

        /* Advance one line */
        if ((ch == '\n') || (ch == '\r'))
        {
            line = line + 1;
        }

        /* Advance one half page */
        if (ch == '+')
        {
            line = line + ((hgt - 4) / 2);
            if (line < 0) line = 0;
        }

        /* Advance one full page */
        if (ch == ' ')
        {
            line = line + (hgt - 4);
        }

        /* Recurse on numbers */
        if (menu && isdigit(ch) && hook[D2I(ch)][0])
        {
            /* Recurse on that file */
            if (!show_file(hook[D2I(ch)],  0, bNotMenu)) ch = ESCAPE;
        }

        /* Exit on escape */
        if (ch == ESCAPE) break;
    }

    /* Close the file */
    fclose(fff);

    /* Done */
    return (ch != ESCAPE);
}

/*------------------------------------------------
do_help()

Shows the contents of the
ascii file doc\help.hlp

Alex: 24.09.01
------------------------------------------------*/

void do_help()
  {
  
   #ifdef DOS_TERM
    char buffer[4800];

    window(1, 1, 80, 25);
    gettext(1, 1, 80, 25, buffer);
   #endif	
       
   show_file("doc/help.hlp");
    
   redraw_screen();

   #ifdef DOS_TERM
      puttext(1, 1, 80, 25, buffer);
   #endif
        
  } 	          
  
/*------------------------------------------------
ListFile()

Executes a files without stop at 24 lines

Returns 0 if the file don't exists 

Alex: 24.09.01
------------------------------------------------*/  
int ListFile(char *namefile)
{
   
    FILE *fich;
    int nFile,nCol;
    char buf[100];
    unsigned char c;
    textbackground(BLACK);
    textcolor(WHITE);
    clrscr();
    
    fich=fopen(namefile,"r");
    if (fich==NULL) return 0;
    textcolor(WHITE);
    nCol=1;
    nFile=0;
    for (;;)
      {
        if (!fgets(buf, 100,fich)) break;
        //Trim(buf);
        if (!prefix(buf,"$:"))
          {
             gotoxy(nCol,++nFile);
          }   
        WriteLine(buf,nCol,nFile);
      }
    fclose(fich);          
    return 1;
}  