#ifndef _OBJECT_
#define _OBJECT_
#include "enum.h"

/////////////////////////////////////////////////////////////
//For items
/////////////////////////////////////////////////////////////
struct tpWeapon
 {
   char name[30]; //name
   char *descrip; //pointer to the description
   char sub_type;
   
   int damage,hit,speed;
   int ac;   //plus to AC, usually only for armour
   int ev;   //plus to evasion, usually only for armour 
   int mass; //weight of the item
   int gold; //base value (gold) of the item  
   int stat; //1=strong, 2=strong2,3 dexterous, 4 dexterous
   int hands; //1=one hand, 2=two hands, 3=one or two
   int skill; //type of SK_xxx (see enum.h)  	
   int prob;  //probability of being created
   int slot;  //in whick slot: EQ_BODY_ARMOUR,EQ_CLOAK, etc         
   
   int power;    //special power of the weapon or regular effect of an object
   
   int level;    //min level to be generated
   
   int unid;     //index of array with descriptions when undefined 
      
      
   //when confused, the scroll can have (randomly) one of the two next effects
   int effect_confused;    
   //effect of the scroll when cursed and confused
   int effect_confused_cursed;
    
   //effect when cursed   
   int effect_cursed;
        
 };	
 
typedef struct tpWeapon TWeapon; 

#define NUM_OBJECTS 5

extern char *aDescObjs[NUM_OBJECT_CLASSES];

extern TWeapon *aObjs[NUM_OBJECT_CLASSES];  //holds an array with the data of the weapons
extern int NObjs[NUM_OBJECT_CLASSES];      //number of objects

void load_weapons(char *filename,int nWich);
 
void init_objects();

#endif