#ifndef _STRUTIL_

#include <stdio.h>

#define _STRUTIL_    
  #ifndef ESCAPE 
     //#define ESCAPE 27
     #define ESCAPE '\x1b'
 #endif
   
#define FALSE 0
#define TRUE 1

#define D2I(X)	((X) - '0')

typedef const char *cptr;
void Trim(char *buf);
bool askfor_aux(char *buf, int len,int x,int y,int bEraseLin=0);
bool streq(cptr a, cptr b);
bool suffix(cptr s, cptr t);
bool prefix(cptr s, cptr t);

//returns:
// .- 1 if line read ok
// .- 2 if line is blank or comment
// .- 0 if end of file 
int read_line_ext_file(char *buf,FILE *fich,int size_command=2);

//prints a string in x,y, with the colour specified.
//If the string don't fit, it splits to another line
//Called from describe.cc           
//Alex:14.11.01                     
#include <string>
void print_description(const string &d,int lineWidth=-1,int x=-1,int y=-1,int colour=-1);
#endif
