/*-------------------------------------------------------------

strutil.cc

Utils for managing strings

16.10.01
-------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#ifdef DOS
#include <conio.h>
#endif
#include <ctype.h>       
#include "strutil.h"           


//removes spaces, enters and tabs from the begining and end of "buf"           
void Trim(char *buf)
  {
    int n,m,len;
    int blank;
    
    n=0;
    len=strlen(buf);
   
    //search from the ending
    for(m=len-1;m>=0;m--)
      {
        //replace blanks by '\0'
        if (buf[m]==' ' || buf[m] == '\n' || buf[m] == '\t' || buf[m] == '\r') 
          {
            buf[m]=0;        
            len--;
          }  
        else
          {
            //non blank
            break;
          }
      } 
    
       
    //search at begining
    for (m=0;m<len;m++)
      { 
        //search a blank
        if (buf[m]==' ' || buf[m] == '\n' || buf[m] == '\t' || buf[m] == '\r')
          blank=1;
        else
          blank=0;
        
        //count blanks  
        if (blank)
          {
            n++;
          }
        else
          {
            //non blank
            break;
          }
      } 
   
   //move from right to left the number of blanks that are at the begining   
   if (n>0)
     {
        for(m=0;m<len;m++)
           buf[m]=buf[m+n];            
     }
     
           
   

}        




/*
 * Get some input at the cursor location.
 *
 * The buffer is assumed to have been initialized to a default string.
 * Note that this string is often "empty" (see below).
 *
 * The default buffer is displayed in yellow until cleared, which happens
 * on the first keypress, unless that keypress is Return.
 *
 * Normal chars clear the default and append the char.
 * Backspace clears the default or deletes the final char.
 * Return accepts the current buffer contents and returns TRUE.
 * Escape clears the buffer and the window and returns FALSE.
 *
 * Note that 'len' refers to the size of the buffer.  The maximum length
 * of the input is 'len-1'.
 */
bool askfor_aux(char *buf, int len,int x,int y,int bEraseLin=0)
{
    //int y, x;

    int k = 0;
    int n=0;

    char ch = '\0';

    bool done = FALSE;


    /* Locate the cursor */
    //Term_locate(&x, &y);


    /* Paranoia */
    if (len < 0) len = 0;

    /* Paranoia */
    if ((x < 0) || (x >= 80)) x = 0;


    /* Restrict the length */
    if (x + len > 80) len = 80 - x;

    /* Truncate the default entry */
    buf[len-1] = '\0';


    /* Display the default answer */
    gotoxy(x,y);for (n=1;n<=len;n++) cprintf(" ");
    gotoxy(x,y);textcolor(YELLOW);cprintf(buf);
    if (bEraseLin)
      for(n=len;n<80;n++) cprintf(" ");
    //Term_erase(x, y, len);
    //Term_putstr(x, y, -1, TERM_YELLOW, buf);

    /* Process input */
    while (!done)
    {
        /* Place cursor */
        gotoxy(x + k, y);

        /* Get a key */
        ch = getch();

        /* Analyze the key */
        switch (ch)
        {
            case ESCAPE:
            {
                k = 0;
                done = TRUE;
                break;
            }

            case '\n':
            case '\r':
            {
                k = strlen(buf);
                done = TRUE;
                break;
            }

            case 0x7F:
            case '\010':
            {
                if (k > 0) k--;
                break;
            }

            default:
            {
                if ((k < len-1) && (isprint(ch)))
                {
                    buf[k++] = ch;
                }
                else
                {
                    //gotoxy(1,24);printf("Illegal edit key!");
                }
                break;
            }
        }

        /* Terminate */
        buf[k] = '\0';

        /* Update the entry */
        gotoxy(x,y);textcolor(WHITE);
        for (n=1;n<=len;n++) cprintf(" ");
        gotoxy(x,y);cprintf(buf);
        //Term_erase(x, y, len);
        //Term_putstr(x, y, -1, TERM_WHITE, buf);
    }

    /* Done */
    return (ch != ESCAPE);
}

/*
 * Determine if string "a" is equal to string "b"
 */
bool streq(cptr a, cptr b)
{
    return (!strcmp(a, b));
}


/*
 * Determine if string "t" is a suffix of string "s"
 */
bool suffix(cptr s, cptr t)
{
    int tlen = strlen(t);
    int slen = strlen(s);

    /* Check for incompatible lengths */
    if (tlen > slen) return (FALSE);

    /* Compare "t" to the end of "s" */
    return (!strcmp(s + slen - tlen, t));
}

bool prefix(cptr s, cptr t)
{
    /* Scan "t" */
    while (*t)
    {
        /* Compare content and length */
        if (*t++ != *s++) return (FALSE);
    }

    /* Matched, we have a prefix */
    return (TRUE);
}




//---------------------------------------------------------------
//
// print_description
//
// Takes a descpr string filled up with stuff from other functions,
// and displays it with minor formatting to avoid cut-offs in mid
// word and such. The character $ is interpreted as a CR.
//
//---------------------------------------------------------------
void print_description(const string &d,int lineWidth=-1,int x=-1,int y=-1,int colour=-1)
{
    unsigned int  nextLine = string::npos;
    unsigned int  currentPos = 0;
    unsigned int  y0;
    
    if (lineWidth==-1)    
      #ifdef DOS
         lineWidth = 42; //was 52 {alex: 14.11.01}
      #else
         lineWidth = 60; // was 60 {alex: 14.11.01}
      #endif
      
    if (x==-1) x==wherex();
    if (y==-1) y==wherey();
    gotoxy(x,y);
    y0=y;
    
    
    //Alex: 23.10.01
    //The colour was LIGHGREY. 
    //But, in a shop, LIGHTGREY is confusing 
    //because they are items in the shop in LIGHTGRAY
    //The description begins next this items and is confusing
    //
    //Ex.:
    //a - item 1          Line 1 of the descripcion
    //b - item 2 very longLine 2 of the description <-- all the line in LIGHTGRAY is confusing 
    
    if (colour==-1) colour=GREEN;
      
    bool nlSearch = true;       // efficiency

    textcolor(colour);
    
    
    while(currentPos < d.length())
    {
        if (currentPos != 0)
        {
            gotoxy(x,++y);
        }

        // see if $ sign is within one lineWidth
        if (nlSearch)
        {
            nextLine = d.find('$', currentPos);
            if (nextLine >= currentPos && nextLine < currentPos + lineWidth)
            {
                cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
                currentPos = nextLine + 1;
                continue;
            }
            if (nextLine < 0)
                nlSearch = false;       // there are no newlines, don't search again.
        }

        // no newline -- see if rest of string will fit.
        if (currentPos + lineWidth >= d.length())
        {
            cprintf((d.substr(currentPos)).c_str());
            return;
        }


        // ok.. try to truncate at space.
        nextLine = d.rfind(' ', currentPos + lineWidth);

        if (nextLine > 0)
        {
            cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
            currentPos = nextLine + 1;
            continue;
        }

        // oops.  just truncate.
        nextLine = currentPos + lineWidth;

        if (nextLine > d.length())
            nextLine = d.length();

        cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
        currentPos = nextLine;
    }
}


int read_line_ext_file(char *buf,FILE *fich,int size_command=2)
{
//read line 
  buf[0]=0;
  if (!fgets( buf, 1024,fich)) return 0; 

  //normalize the string       
  Trim(buf);
       
  //skips comments and empty lines
  if (buf[0]=='#' || buf[0]=='\0') return 2;              
              
  //verifies line
  if (buf[size_command]!=':')
    {
      clrscr();
      printf("-> Line %s is no correct\n",buf);
      exit(0);
     }            
  return 1;   
}      