#ifndef _MONMEEM_
  #define _MONMEEM_

extern int MONDATASIZE; //in mon-util


// last updated 11.07.01 (Alex)
// Adds a text to the log of the adventure
  void flog(char *text);                   
  
// writes to the log the identification of an item  
  void log_first_id(int nitem,int setting);

  
// last updated 11.07.01 (Alex)                                           
// Update monster memories
/* ***********************************************************************
 * called from: fight.cc -> monster_die
 * *********************************************************************** */
void ActEstadisticas(struct monsters *mons_killed,int mid,int exp);

// last updated 11.07.01 (Alex)                                              
// Show monster memories
/* ***********************************************************************
 * called from: describe.cc -> describe_monster
 * *********************************************************************** */
void MonsterRecall(int mtype,string &descrip);    
                               
                               
// last updated 11.07.01 (Alex)                               
//Calc the damage made that a monster does per round
/* ***********************************************************************
* called from: ouch.cc -> ouch
* *********************************************************************** */
 void ActDamage(int dam,int mtype);


//---------------------------------------------------------------------------
/*
  Emulates the monster recall of Angband
  carrying estatistics about monsters killed by the player  

  ALEX 11.07.01
  
*/  
struct tp_monsters_recall
 {  
   int type; 
   unsigned int kills;        // number of kills of this monster
   float evasion;      //"normal" evasion of this monster
   float hp;
   float armour_class;     
   unsigned int exp;         //experience of the monsters
   float damage;         //"normal" damage per attack
   unsigned int min_damage;         //minim damage of the  attacks
   unsigned int max_damage;         //max damage of the  attacks
   unsigned int nAttacks;     //number of attacks
   
   unsigned int first_level; //first level of a kill of this monster
   unsigned int first_level_type;  //type of the level of a kill of this monster
   unsigned int first_branch;      
   
   unsigned int last_level; //last level of a kill of this monster
   unsigned int last_level_type;  //last of the level of a kill of this monster
   unsigned int last_branch;      
   
 }; 
 
extern tp_monsters_recall *monsters_recall;

//Initializes the monster memories               
//Alex: 11.07.01
void mon_recall_init();
void mon_recall_empty();
//---------------------------------------------------------------------------


//Show the memory monster
//10.09.01
void show_memory_monster();



#endif