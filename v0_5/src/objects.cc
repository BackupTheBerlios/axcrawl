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

//#include "stuff.h"


/*
ID: sub_type of the weapon as in enum.h
NM: name of the weapon
HT: base accuracity of the weapon                                       
SP: speed
DG: damage base
WG: mass
HN: hands required (1,2, or both=3)
GD: gold, base value of the item 
SK: skill, flag from array aSkills[]
ST: stat, flag from array aStats[] 
EV:plus to evasion
AC:plus to armour
SL:slot EQ_BODY_ARMOUR,EQ_CLOAK, etc
DC: first line of the description of the weapon
DC: second line of the description of the weapon
DC: ...........................................
DC: ...........................................
DC: last line of the description of the weapon 
PW: power 	
PB: probability
LV: min level required
///// for scrolls
IN: sub_type of the weapon specified by a flag
C1: one of the two effects when confused
C2: other of the two effects when confused
PC: effect when cursed
CC: efect when cursed and confused
*/

/////////////////////////////////////////////////////
// Database
/////////////////////////////////////////////////////

//stores the description of all the weapons.
//each description ends in a '\0' character.
//the aWeapons[].description points to the first character
//of the description of a weapon
char *aDescObjs[NUM_OBJECTS_CLASSES];

//holds an array with the data of the weapons
TWeapon *aObjs[NUM_OBJECTS_CLASSES];  

//number of objects
int NObjs[NUM_OBJECTS_CLASSES];      

/////////////////////////////////////////////////////
// Flags
/////////////////////////////////////////////////////

struct tpParseTypeFlag
  {
    char text[40]; //text like "POTION", "SCROLL", etc
    int id;        //code defined in enum.h for that item
  };  
     
typedef  struct tpParseTypeFlag TParseTypeFlag;



//Search one token in one array of pairs (text,id)
//Return the id, if found, or -1 if not found
int ParseFlag(char *token,TParseTypeFlag *aTokens,int nMaxItems)
  {
    int n;
    //normalize string
    strupr(token);
    for (n=0;n<nMaxItems;n++)
      if (strcmp(aTokens[n].text,token)==0) return aTokens[n].id;
    gotoxy(1,1);
    cprintf("NO TOKEN %s",token);getch();
    return -1;  
 };        
  
//////////////////////////////////////////////////////////////
//Flags for stats
//////////////////////////////////////////////////////////////

TParseTypeFlag aStats[]={{"S",1},{"S+",2},{"D",3},{"D+",4}};
#define NUM_STATS sizeof(aStats)/sizeof(aStats[1])

//////////////////////////////////////////////////////////////
//Flags for slots
//////////////////////////////////////////////////////////////

TParseTypeFlag aSlots[]=
  {
     {"WEAPON",      EQ_WEAPON},
     {"CLOAK",       EQ_CLOAK},
     {"HELMET",      EQ_HELMET},
     {"GLOVES",      EQ_GLOVES},
     {"BOOTS",       EQ_BOOTS},
     {"SHIELD",      EQ_SHIELD},
     {"BODY",        EQ_BODY_ARMOUR},
     {"QUIVER",      EQ_QUIVER}
  };
#define NUM_SLOTS sizeof(aSlots)/sizeof(aSlots[1])


//////////////////////////////////////////////////////////////
//Flags for effects of scrolls
//////////////////////////////////////////////////////////////  
    
TParseTypeFlag aEffects[]=
  { 
    {"CURSE_WEAPON",       EFE_CURSE_WEAPON          },
    {"IDENTIF",            EFE_IDENTIFY              },
    {"IDENTIFY_II",        EFE_IDENTIFY_II           },
    {"PARALISYS",          EFE_PARALISYS             },
    {"TELEPORT_II",        EFE_TELEPORT_II           },
    {"ENCHANT_ARMOUR",     EFE_ENCHANT_ARMOUR        },
    {"REMOVE_CURSE",       EFE_REMOVE_CURSE          },
    {"CURSE_ARMOUR",       EFE_CURSE_ARMOUR          },
    {"MASS_CONFUSION",     EFE_MASS_CONFUSION        },
    {"DETECT_CURSE",       EFE_DETECT_CURSE          },
    {"RANDOM_BLINK",       EFE_RANDOM_BLINK          },
    {"MAGIC_MAPPING",      EFE_MAGIC_MAPPING         },
    {"NOISE",              EFE_NOISE                 },
    {"FORGETFULNESS",      EFE_FORGETFULNESS         },
    {"CURSE",              EFE_CURSE                 },
    {"BERSERKER",          EFE_BERSERKER             },
    {"RESTORE_HP_MANA",    EFE_RESTORE_HP_MANA       }, 
    {"PURIFICATION",       EFE_PURIFICATION          },
    {"ENCHANT_TO_HIT",     EFE_ENCHANT_TO_HIT        },
    {"ENCHANT_TO_DAM",     EFE_ENCHANT_TO_DAM        },
    {"MASS_FEAR",          EFE_MASS_FEAR             },
    {"TELEPORT",           EFE_TELEPORT              },
    {"IDENTIFY",           EFE_IDENTIFY              },
    {"BLINK",              EFE_BLINK                 },
    {"TORMENT",            EFE_TORMENT               },
    {"ENCHANT_WEAPON_III", EFE_ENCHANT_WEAPON_III        },
    {"SURE_VORPALISE",     EFE_SURE_VORPALISE        },
    {"INVULNERABILITY",    EFE_INVULNERABILITY       },
    {"INMATERIALITY",      EFE_INMATERIALITY         },
    {"INVISIBILITY",       EFE_INVISIBILITY          },
    {"PAPER",              EFE_PAPER                 },
    {"IMMOLATION",         EFE_IMMOLATION            },
    {"RANDOM_USELESSNESS", EFE_RANDOM_USELESSNESS    },
    {"ACQUIREMENT"   ,     EFE_ACQUIREMENT           },
    {"SUMMONING",          EFE_SUMMONING             },
    {"VORPALISE",          EFE_VORPALISE             },
    {"RECHARGING",         EFE_RECHARGING            },
    {"DETECT_DANGER",      EFE_DETECT_DANGER         },
       
 };   
     
     	
//////////////////////////////////////////////////////////////
//Flags for skills                    
//////////////////////////////////////////////////////////////  
TParseTypeFlag aSkills[]=             
{                                     
  {"FIGHTING",           SK_FIGHTING  },        
  {"SHORT_BLADES",       SK_SHORT_BLADES  },        
  {"LONG_SWORDS",        SK_LONG_SWORDS  },        
  {"UNUSED_1",           SK_UNUSED_1  },        
  {"AXES",               SK_AXES  },        
  {"MACES_FLAILS",       SK_MACES_FLAILS  },        
  {"POLEARMS",           SK_POLEARMS  },        
  {"STAVES",             SK_STAVES  },        
  {"SLINGS",             SK_SLINGS  },        
  {"BOWS",               SK_BOWS  },        
  {"CROSSBOWS",          SK_CROSSBOWS },
  {"DARTS",              SK_DARTS },
  {"THROWING",           SK_THROWING },
  {"ARMOUR",             SK_ARMOUR },
  {"DODGING",            SK_DODGING },
  {"STEALTH",            SK_STEALTH },
  {"STABBING",           SK_STABBING },
  {"SHIELDS",            SK_SHIELDS },
  {"TRAPS_DOORS",        SK_TRAPS_DOORS },
  {"UNARMED_COMBAT",     SK_UNARMED_COMBAT },
  {"SPELLCASTING",       SK_SPELLCASTING },
  {"CONJURATIONS",       SK_CONJURATIONS },
  {"ENCHANTMENTS",       SK_ENCHANTMENTS },
  {"SUMMONINGS",         SK_SUMMONINGS },
  {"NECROMANCY",         SK_NECROMANCY },
  {"TRANSLOCATIONS",     SK_TRANSLOCATIONS },
  {"TRANSMIGRATION",     SK_TRANSMIGRATION },
  {"DIVINATIONS",        SK_DIVINATIONS },
  {"FIRE_MAGIC",         SK_FIRE_MAGIC },
  {"ICE_MAGIC",          SK_ICE_MAGIC },
  {"AIR_MAGIC",          SK_AIR_MAGIC },
  {"EARTH_MAGIC",        SK_EARTH_MAGIC },
  {"POISON_MAGIC",       SK_POISON_MAGIC },
  {"INVOCATIONS",        SK_INVOCATIONS },
  {"CROSSBOWS",          SK_CROSSBOWS}
 };

#define NUM_SKILLS sizeof(aSkills)/sizeof(aSkills[0])


//////////////////////////////////////////////////////////////
//initial properties of a item   
//////////////////////////////////////////////////////////////
void init_item(int nWich,int nItem)
  {     
    strcpy(aObjs[nWich][nItem].name,"unasigned");        
    aObjs[nWich][nItem].descrip=NULL;    
    aObjs[nWich][nItem].damage=0;
    aObjs[nWich][nItem].hit=0;
    aObjs[nWich][nItem].speed=0;
    aObjs[nWich][nItem].mass=0;  
    aObjs[nWich][nItem].stat=0;
    aObjs[nWich][nItem].hands=0;
    aObjs[nWich][nItem].skill=0;
    aObjs[nWich][nItem].prob=0;
    aObjs[nWich][nItem].ev=0;
    aObjs[nWich][nItem].ac=0;
    aObjs[nWich][nItem].slot=-1;
    aObjs[nWich][nItem].power=-1;
    aObjs[nWich][nItem].effect_confused=-1;
    aObjs[nWich][nItem].effect_cursed=-1;
    aObjs[nWich][nItem].effect_confused_cursed=-1;
    aObjs[nWich][nItem].level=0;
    aObjs[nWich][nItem].gold=1;
    aObjs[nWich][nItem].unid=-1;
    switch(nWich)
      {
      	case OBJ_POTIONS:
      	  aObjs[nWich][nItem].mass=40; 
      	  break;
      	case OBJ_SCROLLS:
      	  aObjs[nWich][nItem].mass=20; 
      	  break;
      	default:
      	  aObjs[nWich][nItem].mass=150;         	  
      }	
    
    
 }             
      
//////////////////////////////////////////////////////////////
//load the items from file "filename"
//////////////////////////////////////////////////////////////
void load_weapons(char *filename,int nWich)
 {
   FILE *fich;
   char buf[1024]; //to read a line
   int nIdItem;   //id of the actual item
   int nWeapon;     //actual item of the array 

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
   

   //init database
   aObjs[nWich]=NULL;
   aDescObjs[nWich]=NULL;
   NObjs[nWich]=-1;
   nWeapon=0;
   
   //read lines of file 
   while (1)
     { 
       //read line 
       buf[0]=0;
       if (!fgets( buf, 1024,fich)) break; 

       //normalize the string       
       Trim(buf);
       
       //skips comments and empty lines
       if (buf[0]=='#' || buf[0]=='\0') continue;              
              
       //verifies line
       if (buf[2]!=':')
         {
           clrscr();
           printf("%s -> Line %s is no correct\n",filename,buf);
           exit(0);
         } 
           
       //first line of file must have the number of items
       if (strncmp(buf,"NI",2)==0)
         {
           NObjs[nWich]=atoi(&buf[3])+1;
           if (NObjs[nWich]==1)
             {
               clrscr();
               printf("%s -> Line %s is no correct\n",filename,buf);
               exit(0);
             }
           clrscr();  
           aObjs[nWich]=(TWeapon *)calloc(NObjs[nWich],sizeof(TWeapon));
           aDescObjs[nWich]=(char *)calloc(filelength(fileno(fich)),sizeof(char));
           p=aDescObjs[nWich];
           nWeapon=-1;
           continue;      
         } 
       
       ////////////////////////////////////
       //parse the line
       ////////////////////////////////////
       
       //id of the item.
       if (strncmp(buf,"ID",2) ==0)
          {               
               
               nIdItem=atoi(&buf[3]);
               nWeapon=nIdItem;
               if (nWeapon==NObjs[nWich])
                 {
                   clrscr();
                   printf("%s ->There are more items that especified in 'NI:' command\n",filename);
                   exit(0);                   
                 }  

               init_item(nWich,nWeapon);
               //ends the description of the previous weapon
               *p='\0';
               p++;  
               continue;
           } 
        if (strncmp(buf,"IN",2) ==0)
          {               
                           
               switch(nWich)            
                 {
                   case NOBJ_SCROLL:
                      nIdItem=ParseFlag(&buf[3],aScrolls,NUM_FLAG_SCROLLS);
                      break;
                   default:
                      clrscr();
                      printf("IN: not supported  for this type of object");
                      exit(0);
                      break;
                 }     
                  
               
               nWeapon=nIdItem;
               if (nWeapon==NObjs[nWich])
                 {
                   clrscr();
                   printf("%s ->There are more items that especified in 'NI:' command\n",filename);
                   exit(0);                   
                 }                                 
               init_item(nWich,nWeapon);
               //ends the description of the previous weapon
               *p='\0';
               p++;  
               continue;
          }    
        //name of the weapon
        if (strncmp(buf,"NM",2) ==0)
          {
               strncpy(aObjs[nWich][nWeapon].name,&buf[3],30);
               continue;
          } 
        //description of the weapon
        if (strncmp(buf,"DC",2) ==0)
          {    
               //first line of the description?
               if (aObjs[nWich][nWeapon].descrip==NULL)
                 {
                   aObjs[nWich][nWeapon].descrip=p;                   
                 }  
                      
               strcpy(p,&buf[3]);
               p=p+strlen(buf)-3;  
               continue;
               
         }  
        //pluses to hit
        if (strncmp(buf,"HT",2)==0)
          {
               aObjs[nWich][nWeapon].hit=atoi(&buf[3]);
               continue;
          }
        //speed
        if (strncmp(buf,"SP",2)==0)
          {
               aObjs[nWich][nWeapon].speed=atoi(&buf[3]);
               continue;
          }  
        //hands
        if (strncmp(buf,"HN",2)==0)
          {
               aObjs[nWich][nWeapon].hands=atoi(&buf[3]);
               continue;
          }    
        //pluses to dam
        if (strncmp(buf,"DG",2)==0)
          {
               aObjs[nWich][nWeapon].damage=atoi(&buf[3]);
               continue;
          }              
        //weight
        if (strncmp(buf,"WG",2)==0)
          {
               aObjs[nWich][nWeapon].mass=atoi(&buf[3]);
               continue;
          } 
        //ac
        if (strncmp(buf,"AC",2)==0)
          {
               aObjs[nWich][nWeapon].ac=atoi(&buf[3]);
               continue;
          }                
        //probability
        if (strncmp(buf,"PB",2)==0)
          {
               aObjs[nWich][nWeapon].prob=atoi(&buf[3]);
               continue;
          }                                              
        
        //stat
        if (strncmp(buf,"ST",2)==0)
          {    
               
               aObjs[nWich][nWeapon].stat=ParseFlag(&buf[3],aStats,NUM_STATS);
               continue;
          }                
        //skill
        if (strncmp(buf,"SK",2)==0)
          {    
               
               aObjs[nWich][nWeapon].skill=ParseFlag(&buf[3],aSkills,NUM_SKILLS);
               continue;
          } 
        //slot
        if (strncmp(buf,"SL",2)==0)
          {
               aObjs[nWich][nWeapon].slot=ParseFlag(&buf[3],aSlots,NUM_SLOTS);
               continue;              
          }
       //power       
       if (strncmp(buf,"PW",2)==0)
          {
               aObjs[nWich][nWeapon].power=ParseFlag(&buf[3],aEffects,NUM_EFFECTS);
               continue;              
          }
       //gold
        if (strncmp(buf,"GD",2)==0)
          {
               aObjs[nWich][nWeapon].gold=atoi(&buf[3]);
               continue;
          }      
       //confuse effect 1      
       if (strncmp(buf,"C1",2)==0)
          {
               aObjs[nWich][nWeapon].effect_confused1=ParseFlag(&buf[3],aEffects,NUM_EFFECTS);
               continue;              
          }   
       //confuse effect 2   
       if (strncmp(buf,"C2",2)==0)
          {
               aObjs[nWich][nWeapon].effect_confused2=ParseFlag(&buf[3],aEffects,NUM_EFFECTS);
               continue;              
          }      
       //level
        if (strncmp(buf,"LV",2)==0)
          {
               aObjs[nWich][nWeapon].level=atoi(&buf[3]);
               continue;
          }                                                         
   }  
   //ends the description of the previous weapon
   *p='\0';  
   fclose(fich);
}           
 
 
 void init_objects()
   {
      load_weapons("weapons.txt",NOBJ_WPN);   

      load_weapons("missil.txt",NOBJ_MISSIL);   
      	

      load_weapons("armour.txt",NOBJ_ARM);   
      
      load_weapons("scrolls.txt",NOBJ_SCROLL);   
      
      load_weapons("potions.txt",NOBJ_POTION);   
  }      
     