/*
================================================================================================
potions.cc

Quaffing potions

Alex: 15.03.02
================================================================================================
*/

#include "AppHdr.h"
#include "potions.h"

#include <string.h>

#include "externs.h"

#include "beam.h"
#include "effects.h"
#include "food.h"
#include "itemname.h"
#include "misc.h"
#include "mutation.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "skills2.h"
#include "spells2.h"
#include "spl-cast.h"
#include "stuff.h"
#include "view.h"
#include "item_use.h" //alex:16.11.01
#include "options.h" //alex:07.02.02
#include "ouch.h" //alex:07.02.02
#include "spells4.h" //alex:26.02.02
#include "effects.h" //alex:26.02.02

//Added param "bCursed"
//In Ax-Crawl, if xtraOps.ax_curses enabled, potions can be cursed.
//Then, their effects are different from normal potions
bool potion_effect(char pot_eff, int pow,int bCursed=0)
{
    bool effect = true;  // current behaviour is all potions id on quaffing

    int new_value = 0;
    int n;
    unsigned char i;
    int bad_effect=bCursed && xtraOps.ax_curses;

    if (pow > 150)
        pow = 150;
        
    //cursed potions aren't good for your body
    if (bad_effect)
      {
       	  naughty(NAUGHTY_STIMULANTS, 4 + random2(4));
      }	

    switch (pot_eff)
    {
    case POT_HEALING:
        //When cursed:
        //5%  of times, heals less of normal
        //45% of times, you lose hp
        //25% of times, POT_PORRIDGE
        //25% of times, POT_DECAY
        if (bad_effect)
          {
             int nEffect=random2(100);
             if (n<=5)
                {                	
                    inc_hp(5, false);
                    mpr("Something isn't well with this potion.");
                    break;
                } 
             if (n<=50)
                {
                  dec_hp(5 + random2(7), false);       
                  break;	
                } 	   
             if (n<=75)   
                {
                  potion_effect(POT_PORRIDGE,40,0);
                  break;	
                } 	
             potion_effect(POT_DECAY,40,0);
             break; 
          }	
        
        mpr("You feel better.");
        inc_hp(5 + random2(7), false);

        if (you.hp >= you.hp_max)
        {
            unrot_hp(1);
            set_hp(you.hp_max, false);
        }

        you.poison = 0;
        you.rotting = 0;
        you.disease = 0;
        you.conf = 0;
        break;

    case POT_HEAL_WOUNDS:
        if (bad_effect)
          {
             int nEffect=random2(100);
             if (n<=5)
                {                	
                    inc_hp(10, false);
                    mpr("Something is wrong with this potion.");
                    break;
                } 
             if (n<=50)
                {
                  dec_hp(10 + random2avg(28, 3), false);
                  break;	
                } 	   
             if (n<=75)   
                {
                  potion_effect(POT_POISON,40,0);
                  break;	
                } 	
             potion_effect(POT_DEGENERATION,40,0);
             break; 
          }	
        mpr("You feel much better.");
        inc_hp(10 + random2avg(28, 3), false);

        if (you.hp >= you.hp_max)
        {
            unrot_hp( 2 + random2avg(5, 2) );
            set_hp(you.hp_max, false);
        }
        break;

    case POT_SPEED:
        new_value=6;
        if (bad_effect)
          {
             //35% of times, your amulet helps you	
             if (wearing_amulet(AMU_RESIST_SLOW) && random2(100)<=35)
               {
               	  new_value=0;
               	  pow=0;
               	  mpr("This potion don't speed you as normal.");
               	  goto speeding;
               }	  

             //if you are speedy, 50% of times, lose speed
             if (you.haste && coinflip())
               {
               	  you.haste=you.haste/3;
               	  mpr("Your extra speed leaves you...");
               	  break;
               }
             else if (coinflip())
                 {
                    you.slow=you.haste;
                    you.haste=0;
                    mpr("Your speed turns in slowness");
                    break;	
                 }  	  
                 

             potion_effect(POT_SLOWING,40,0);
             break;                                
          }	
speeding:          
        if (you.haste > 90)
            mpr("You already have as much speed as you can handle.");
        else if (wearing_amulet(AMU_RESIST_SLOW))
        {
            mpr("Your amulet glows brightly and you gain speed.");

            if (you.slow)
            {
                you.slow = 0;
                you.haste = 10;
            }
            else if (!you.haste)
                you.haste = 50 + random2(pow);
            else 
                you.haste += (5 + random2(new_value));
        }
        else if (!you.haste)
        {
            /*mpr("You feel yourself speed up.");
            alex:07.02.02
            if (you.slow)
                you.slow = 0;
            else 
                you.haste = 40 + random2(pow);
            */
            if (you.slow)
                {
                  you.slow = 0;
                  mpr("You recover your normal speed.");
                } 
            else 
                {
                  you.haste = 40 + random2(pow);
                  mpr("You feel yourself speed up.");
                }  
        }
        else
        {
            mpr("You feel as though your speed will last longer.");
            you.haste += (1 + random2(6));
        }

        if (wearing_amulet(AMU_RESIST_SLOW))
        {
            if (you.haste > 100)
                you.haste = 100;
        }
        else
        {
            if (you.haste > 90)
                you.haste = 90;
        }

        // now some radiation is given for each cast -- bwr
        // note that berserk handles hasting itself -- bwr
        contaminate_player( 1 + random2(3) );  

        naughty(NAUGHTY_STIMULANTS, 4 + random2(4));
        break;

    case POT_MIGHT:
        {
            if (bad_effect)
               {
               	  you.might=0;
               	  //if you mighty, 50% of times lose your mighty
               	  if (you.might && coinflip())
               	    {               	       
               	       mpr("Your might abandones you.");
               	       break;	
               	    }	
               	  //else, you become exhausted  
               	  if (!you.might)
                     you.exhausted= 35 + random2(pow);
                  else 
                     you.exhausted+= (5 + random2(10));  
                  mpr("You feel exhausted");   
               	  break;
               }        		
            bool were_mighty = you.might > 0;

            strcpy(info, "You feel ");
            strcat(info, (you.might) ? "pretty" : "very");
            strcat(info, " mighty");
            strcat(info, (you.might) ? ", still" : " all of a sudden");
            strcat(info, ".");
            mpr(info);

            // conceivable max gain of +184 {dlb}
            if (!you.might)
                you.might = 35 + random2(pow);
            else 
                you.might += (5 + random2(10));

            if (!were_mighty)
                modify_stat(STAT_STRENGTH, 5, true);

            // files.cc permits values up to 215, but ... {dlb}
            if (you.might > 75)
                you.might = 75;

            naughty(NAUGHTY_STIMULANTS, 4 + random2(4));
        }
        break;

    case POT_GAIN_STRENGTH:
        if (bad_effect)
          {
             mutate(MUT_WEAK);
             break;
          }	        
        mutate(MUT_STRONG);
        break;

    case POT_GAIN_DEXTERITY:
        if (bad_effect)
          {
             mutate(MUT_DOPEY);
             break;
          }
        mutate(MUT_AGILE);
        break;

    case POT_GAIN_INTELLIGENCE:
        if (bad_effect)
          {
             mutate(MUT_CLUMSY);
             break;
          }
        mutate(MUT_CLEVER);
        break;

    case POT_LEVITATION:
        strcpy(info, "You feel");
        strcat(info, (!you.levitation) ? " very" : " more");
        strcat(info, " buoyant.");
        mpr(info);

        if (!you.levitation)
            mpr("You gently float upwards from the floor.");
            
        if (bad_effect)   
            you.levitation=random2(pow);
        else
            you.levitation += 25 + random2(pow);

        if (you.levitation > 100)
            you.levitation = 100;

        burden_change();
        break;

    case POT_POISON:
    case POT_STRONG_POISON:
        if (bad_effect)
          {
              if (random2(100)<=5) 
                 {
                    potion_effect(POT_HEALING,40,0);
                    break;                     
                 }   
          }          
        if (player_res_poison())
        {
            strcpy(info, "You feel ");
            strcat(info, (pot_eff == POT_POISON) ? "slightly" : "extremely");
            strcat(info, " nauseous.");
        }
        else
        {
            if (you.poison)
                strcpy(info, "You feel even sicker.");
            else
            {
                strcpy(info, "That liquid tasted ");
                strcat(info, (pot_eff == POT_POISON) ? "very" : "extremely");
                strcat(info, " nasty...");
            }

            you.poison += 1 + ((pot_eff == POT_POISON) ?
                               random2avg(5, 2) : 2 + random2avg(13, 2));
            if (bad_effect) 
              {
              	 you.poison*=2;
              	 mpr("It tastes like burning fire.");       
              }	 
        }

        mpr(info);
        break;

    case POT_SLOWING:
        if (wearing_amulet(AMU_RESIST_SLOW))
            mpr("You feel momentarily lethargic.");
        else if (you.slow > 90)
            {            	
            	if (bad_effect)
            	   {
            	     potion_effect(POT_PARALYSIS,40,0);
            	     break;	            	     
            	   }	
            	mpr("You already are as slow as you could be.");   
            }	
        else
        {
            if (!you.slow)
                {   
                                   
                    //Take into account that you can be hasted               
                    //alex:07.02.02
                    if (you.haste)
                       {
                          mpr("You lose your extra speed",MSGCH_WARN);
                          you.haste=0;     
                          if (wearing_amulet(AMU_RESIST_SLOW))
                             you.slow=10;
                          mpr("... and feel yourself slown down.",MSGCH_WARN);
                          break;
                       }
                    else
                       mpr("You feel yourself slow down.",MSGCH_WARN);
                 }   
            else
                mpr("You feel as though you will be slow longer.",MSGCH_WARN);

            you.slow += 10 + random2(pow);

            if (you.slow > 90)
                you.slow = 90;
            
        }
        break;

    case POT_PARALYSIS:
        strcpy(info, "You ");
        strcat(info, (you.paralysis) ? "still haven't" : "suddenly lose");
        strcat(info, " the ability to move!");
        mpr(info, MSGCH_WARN);

        new_value = 2 + random2(6 + you.paralysis);
        if (bad_effect) new_value=new_value*2;

        if (new_value > you.paralysis)
            you.paralysis = new_value;

        if (you.paralysis > 13)
            you.paralysis = 13;
        break;

    case POT_CONFUSION:
        strcpy(info, "You feel ");

        if (wearing_amulet(AMU_CLARITY))
            strcat(info, "momentarily ");
        else if (you.conf)
            strcat(info, "very ");

        strcat(info, "confused.");
        mpr(info);

        // early return for the protected: {dlb}
        if (wearing_amulet(AMU_CLARITY))
            break;

        new_value = 3 + random2(8 + you.conf);
        
        if (bad_effect)
          {
            if (random2(100)<=15)
              {
              	 mutate(MUT_CLUMSY);
              }
            else  
               new_value=new_value*2;
          }	

        if (new_value > you.conf)
            you.conf = new_value;

        if (you.conf > 20)
            you.conf = 20;
        break;

    case POT_INVISIBILITY:
        if (bad_effect)
          {            
            if (you.invis && random2(100)<=25)
              {
                you.invis = 0;
                break;
              }
            
            if (one_chance_in(5))
               {
               	  potion_effect(POT_INMATERIALITY,40,0);
               	  break;
               }	  
            if (one_chance_in(3))
               pow=0;  
            else
               {                                  
                  mpr("You feel more visible.");
               	  you.duration[DUR_AGGRAVATE]+=50;
               	  break;
               }
          }	
        mpr( (!you.invis) ? "You fade into invisibility!"
                          : "You feel safely hidden away." );

        // now multiple invisiblity casts aren't as good -- bwr
        if (!you.invis)
            you.invis = 15 + random2(pow);
        else
            you.invis += (3 + random2(6));

        // now some radiation is given for each cast -- bwr
        contaminate_player( (bad_effect ? 2 : 1) * (1 + random2(3)));  

        if (you.invis > 100)
            you.invis = 100;
        break;

    // carnivore check here? {dlb}
    case POT_PORRIDGE:          // oatmeal - always gluggy white/grey?        
        new_value=6000;
        if (bad_effect)
          {            
             if (coinflip())
               {
                  you.hunger=HS_STARVING;
               }   
             if (coinflip())
                potion_effect(POT_DECAY,40,0);  
             break;   
          }          
        mpr("That potion was really gluggy!");          
        lessen_hunger(new_value, true);
        break;

    case POT_DEGENERATION:
        if (bad_effect)
          {
            if (random2(100)<=5)
              {
                potion_effect(POT_EXPERIENCE,40,0);
                break;
              }  
            if (coinflip())
               lose_stat(STAT_RANDOM, 1 + random2avg(4, 2));
            else
               potion_effect(POT_DECAY,40,0);   
          }  
        mpr("There was something very wrong with that liquid!");
        lose_stat(STAT_RANDOM, 1 + random2avg(4, 2));
        break;

    // Don't generate randomly - should be rare and interesting
    case POT_DECAY:
        strcpy(info, "You feel ");
        strcat(info, (you.is_undead) ? "terrible."
               : "your flesh start to rot away!");
        mpr(info);

        if (!you.is_undead)
            {
              you.rotting += 10 + random2(10);
              if (bad_effect)
                {
                   if (coinflip())
                        {
                           you.rotting = you.rotting*2;
                           mpr("And it rots very fast!!");
                        }   
                   potion_effect(POT_STRONG_POISON,40,0);                        
                }    
            }  
        break;

    case POT_WATER:        
        if (bad_effect)
           {
             if (coinflip())
                potion_effect(POT_STRONG_POISON,40,0);
             else
                make_hungry(500,true);       
             break;              
           } 
        mpr("This tastes like water.");
        // we should really separate thirst from hunger {dlb}
        // Thirst would just be annoying for the player, the
        // 20 points here doesn't represesent real food anyways -- bwr
        lessen_hunger(20, true);
        break;

    case POT_EXPERIENCE:  
        new_value=1 + exp_needed( 2 + you.experience_level );
        if (bad_effect)
           {
              if (coinflip())
                {
                    new_value=new_value/3;
                    mpr("The potion seems diluted.");
                }    
              else
                {
                    drain_exp();
                    break;
                }      
           } 
        
           
        if (you.experience_level < 27)
        {
            mpr("You feel more experienced!");

            you.experience = new_value;
            level_change();
        }
        else
            mpr("A flood of memories washes over you.");
        break;         

    case POT_MAGIC:
        new_value = 5 + random2avg(19, 2);
        if (bad_effect)
          { 
            if (coinflip())
              new_value = 5;
            else
              {
                mpr("Your magic energies leave you.");
                you.magic_points=0;
                break;
              }    
          }  
        

        if (you.magic_points + new_value > you.max_magic_points)
        {
            new_value = (you.max_magic_points - you.magic_points)
                + (you.magic_points + new_value - you.max_magic_points) / 4 + 1;
        }
        
        mpr( "You feel magical!" );
        

        inc_mp( new_value, true );
        break;

    case POT_RESTORE_ABILITIES:
        // messaging taken care of within function {dlb}
        // not quite true... if no stat's are restore = no message, and
        // that's just confusing when trying out random potions (this one
        // still auto-identifies so we know what the effect is, but it
        // shouldn't require bringing up the descovery screen to do that -- bwr
        new_value=STAT_ALL;        
        if (bad_effect)
          {
            if (coinflip())
              {
                new_value=STAT_RANDOM; //even if the random stat don't need to be restored
              } 
            else
              {
                 lose_stat(STAT_RANDOM, 1 + random2avg(4, 2));
                 break;         
              }   
          }  
        if (restore_stat(STAT_ALL, false) == false)
            mpr( "You feel refreshed." );
        break;

    case POT_BERSERK_RAGE:
        if (bad_effect)
           {                                        
             make_hungry(1500,true);
             you.exhausted += 12 + random2avg(23, 2);
             if (you.haste) you.haste=1;
             you.slow += you.exhausted;           
             mpr("Fire runs through your veins!!!"); 
             break;
           } 
        go_berserk(true);
        break;

    case POT_CURE_MUTATION:
        if (bad_effect)
          {
            if (random2(100)<=10)
              mutate(100,false);
            else
              give_bad_mutation(true,true);
            break;                              
          }  
        mpr("It has a very clean taste.");
        for (i = 0; i < 7; i++)
        {
            if (random2(10) > i)
                delete_mutation(100);
        }
        break;

    case POT_MUTATION:
        if (bad_effect)
          {        
             
             if (random2(100)<=5)
               {                   
                 mpr("You feel strange.");
                 give_good_mutation(false);
               }     
             else
               {
                 mpr("You body corrupts.");
                 give_bad_mutation(true,true);
               }  
             break;
          }  
        mpr("You feel extremely strange.");
        for (i = 0; i < 3; i++)
        {
            mutate(100, false);
        }

        naughty(NAUGHTY_STIMULANTS, 4 + random2(4));
        break;
    //AX-Crawl potions      
    case POT_ESP:      
      if (bad_effect)
        {
          cast_see_invisible(50);
        }       
      detect_creatures(50);
      break;
    case POT_RESTORE_HP_MANA:
      if (bad_effect)
        {     
          if (random2(100)<=5)
            {
               give_good_mutation(false);
               break;
            }   
          mpr("Something evil corrupts this potion.");  
          potion_effect(POT_STRONG_POISON,40,0);
          mutate(100,false);
          break;                 
        } 
      you.hp=you.hp_max;
      you.magic_points=you.max_magic_points;
      mpr("Your power returns.");
      break;
   case POT_INMATERIALITY:
      if (bad_effect)
        {
          if (you.duration[DUR_INMATERIALITY]>4 && coinflip())	
             {
             	you.duration[DUR_INMATERIALITY]=1;
             	mpr("You feel more material.");
             	break;
             }
          if (one_chance_in(5))
            {
            	potion_effect(POT_INVISIBILITY,40,0);
            	break;
            }	   
          potion_effect(POT_PARALYSIS,40,0);  
          break;    	
        }
      if (you.duration[DUR_INMATERIALITY])
         you.duration[DUR_INMATERIALITY]+=random2(15);
      else
         you.duration[DUR_INMATERIALITY]=5+random2(10)+random2(5);     	   
      break;       
  case POT_HUNGER:
    if (bad_effect)
      {
      	if (one_chance_in(3))
      	   {
      	      lessen_hunger(100,0);
      	      break;
      	   }	
      	if (one_chance_in(3))
      	   {
      	      make_hungry(100,0);	
      	      mpr("You empty stomach cries."); 	
      	      potion_effect(POT_AGGRAVATE,40,0);
      	      break;	
      	   }	  
      	make_hungry(50,0); 	
      }	
    make_hungry(300,0);  
    break;
  case POT_AGGRAVATE:
    new_value=50;
    if (bad_effect)  
      {
      	if (one_chance_in(3))
      	  {
      	    potion_effect(POT_INMATERIALITY,40,0);	
      	    break;
      	  }	
      	if (one_chance_in(3))  
      	  {
      	    potion_effect(POT_INVISIBILITY,40,0);	
      	    break;	
      	  }	
      	
      	  new_value+=35;
      }
    you.duration[DUR_AGGRAVATE]+=new_value;
    break;   	
 }    
 return (effect);
}                               // end potion_effect()

