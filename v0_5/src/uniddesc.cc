/*
==============================================================================

uniddesc.cc

Description for un-identified items.

Each type of objects has a file unidxxxx.txt 

xxxx is:

pot       for potions
scr       for scrolls
wnd       for wands
stv       for staves
amu       for amulets
rng       for rings

These (normally) aren't given randomly, but explicity  the data file 
for each of type:

msc       for miscellany
arm       for armour
wpn       for weapons
  


The file has a first line with the number of lines (minus the first)

Each line of the file is a description that will be given, randomly, to items
of the class xxxx 

Alex: 01.04.02

==============================================================================
*/


#include <dir.h> 
#include <stdlib.h>
#include <stdio.h>
#include "io.h"
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <file.h>
#include "enum.h"
#include "strutil.h"
#include "objects.h"
#include "uniddesc.h"


//the whole file with text for un-id. descriptions is stored
//in aDescUnId[type_object]
char *aDescUnId[NUM_OBJECT_CLASSES];                                  

//the exact un-id. text for an item is in aPtrDescUnid[type_object][subtype]
char *aPtrDescUnId[NUM_OBJECT_CLASSES];
                                             
//number of un-id.texts for each type of objects
int NDescUnId[NUM_OBJECTS_CLASSES];                                                         
                                                   
//////////////////////////////////////////////////////////////
//load the text for descriptions of unidentified items
//////////////////////////////////////////////////////////////
void load_uniddesc(char *filename,int nWich)
 {
   FILE *fich;
   char buf[1024]; //to read a line
   int nIdItem;   //id of the actual item
   int nItem;     //actual item of the array 

   int n,m;
   char *p;
   
   //open file
   strcpy(buf,"data/");
   strcat(buf,filename);
   
   fich=fopen(buf,"rt");
   if (fich==NULL) 
     {
        clrscr();
        printf("Can read file %s\n",buf);
        exit(0);
     }  
   

   //first line has the number of un-id. texts availables in this file
   read_line_ext_file(buf,fich);
   NDescUnId[nWich]=atoi(&buf[3])+1;   
   if ( NDescUnId[nWich]==1)
     {
       clrscr();
       printf("%s -> Line %s is no correct\n",filename,buf);
       exit(0);
      }
   
   //allocate memory
   aPtrDescUnId[nWich]=(char *)calloc(NDescUnId[nWich],sizeof(cptr));
   aDescUnId[nWich]=(char *)calloc(filelength(fileno(fich)),sizeof(char));
   p=aDescUnId[nWich];
   nItem=-1;
                              
   //read lines of file 
   while (1)
    { 
       if (!fgets( buf, 1024,fich)) break; 
       ++nItem;
       //pointer to the text
       aPtrDescUnId[nWich][nItem]=p;
       
       //store text
       Trim(buf);
       strcpy(p,&buf);
       p=p+strlen(buf);
       //must finish in '0'
       *p='\0'; 
       //no problem. filelength() calculated the space with \n and \r
       //that aren't returned by fgets()
       p++;
    }
   fclose(fich);
}       


//////////////////////////////////////////////////////////////
//returns the text for un-identified names of items       
//////////////////////////////////////////////////////////////
char *uniddesc(int base_type,int sub_type)
 {
   return aPtrDescUnId[base_type][sub_type];
 }            
       
                                            