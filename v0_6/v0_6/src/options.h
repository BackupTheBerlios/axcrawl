/*-------------------------------------------------------------------------

options.h

Menus for changing some options

Alex: 25.10.01
-------------------------------------------------------------------------*/

#ifndef _OPTIONS_
   #define _OPTIONS_
 
   //Menu of options
   //Called from acr.cc->input()
   //Alex: 25.10.01
   //void menu_options();
   
   
   //Extra options for AX-PAtch
   struct tpExtraOps
     {
       //show weight of the inventory
       int ax_show_weight;
     
       //Add to the character dump file the number of kills
       int ax_dump_with_monster_kills;	
    
       //When dead, prompt to revive
       int ax_cheat_dead;
    
       //Show damage done to monsters
       int ax_show_dam;
       
       //wear armour takes no time in equipment screen
       int ax_wear_no_time;
       
       //no stair delay
       int ax_no_stair_delay;
       
       //autopickups 
       long ax_autopickups;
       
       //don't autopick bad potions {11.12.01)
       int ax_no_pick_bad_potions;
       
       //don't autopick bad scrolls {11.12.01)
       int ax_no_pick_bad_scrolls;
       
       //don't autopick bad jewellery {11.12.01)
       int ax_no_pick_bad_jewellery;  
       
       //don't autopick empty wands{05.02.02}
       int ax_no_pick_empty_wands;    
       
       //number of ax_cheat_dead
       int nDeathAvoids;
                               
       //the name of the objects includes their inscription {29.11.01}
       int ax_show_inscriptions;
       
       //merge items with differents inscriptions {29.11.01}
       int ax_merge_inscriptions;
       
       
       //use old fight system {11.12.01}
       int USE_NEW_COMBAT_STATS;
       
       
       //fighting rules for ax:
       //  .- changes stabbing
       int ax_fight_rules;
              
       //extra objects of ax-crawl
       int ax_items;
                     
       //if true, cursed items have bad effects.
       //Ex: with the old system, cursing a potion makes
       //a potion of decay. Now, quaffing cursing a potion have 
       //a bad effect.
       int ax_curses;                          
                                          
       //defect/repel missiles a bit more usseful
       //alex:07.02.02                                          
       int ax_raise_defensive_spell_power;
       
       
       //auto-quiver
       int ax_quiver;
          
       int ax_firing;   
  
       //new ax crawl effects for reading scrolls when confused       
       int ax_confuse_rules;
       
     };
 
    typedef  struct tpExtraOps  tExtraOps; 

    extern tExtraOps xtraOps;


#endif   