#ifndef _USESTAIR_
#define _USESTAIR_


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr - effects - spells3
 * *********************************************************************** */
void down_stairs(bool remove_stairs, int old_level);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr - misc - player - stuff
 * *********************************************************************** */
void new_level(void);


// last updated 12may2000 {dlb}
/* ***********************************************************************
 * called from: acr
 * *********************************************************************** */
void up_stairs(void);
                         
                         
//Show the name of the level
void show_name_level(int branch_id=-1,int level_type=-1,int num_level=-1,int nRow=12);

//Returns in "name" the complete name of the level identified by branch_id, level_type
//and num_level. If this params are -1, then they are your current location.
//
//If bSetColour==1, then the rock_colour and floor_colour are set for the level.
void level_name(char *name,int branch_id=-1,int level_type=-1,int num_level=-1,int bSetColour=0);                         

/* public variables to store the actual level
   Only when changing of level, append to the log 
   "Level: X of Branch"
*/

struct tp_var_log 
  {
    int where_are_you;
    int level_type;
    int level_number;   
  };
extern tp_var_log var_log;
#endif