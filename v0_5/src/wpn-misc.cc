/*
 *********************************************************************
 *  File:       wpn-misc.cc                                          *
 *  Summary:    temporary home for weapon f(x) until struct'ed       *
 *  Written by: don brodale <dbrodale@bigfootinteractive.com>        *
 *                                                                   *
 *  Changelog(most recent first):                                    *
 *                                                                   *
 *  <00>     12jun2000     dlb     created after little thought      *
 *********************************************************************
 */

#include "AppHdr.h"
#include "wpn-misc.h"

#include "externs.h"

//------
//alex:03.12.01
#include "externs.h"
#include "menus.h"   
#include "message.h"  
#include "fight.h"
#include "stuff.h"
#include "invent.h"
#include "string"
#include "string.h"
#include "describe.h"
#include "player.h"
#include "strutil.h"
#include "shopping.h"
#ifdef DOS
#include <conio.h>
#include "itemname.h"
#include <stdio.h>
#include "enum.h"
//#include "objects.h"
#endif
//------


// all of this will be replaced by a struct and data handlers {dlb}:

/*
 **************************************************
 *                                                *
 *             BEGIN PUBLIC FUNCTIONS             *
 *                                                *
 **************************************************
*/

char damage_type(unsigned char wclass, unsigned char wtype)
{
    char type_damage = DVORP_CRUSHING;  // this is the default, btw {dlb}

    if (wclass == OBJ_WEAPONS)
    {
        switch (wtype)
        {
        case WPN_DAGGER:
        case WPN_DEMON_BLADE:
        case WPN_DOUBLE_SWORD:
        case WPN_GREAT_SWORD:
        case WPN_KATANA:
        case WPN_KNIFE:
        case WPN_LONG_SWORD:
        case WPN_QUICK_BLADE:
        case WPN_SABRE:
        case WPN_SCIMITAR:
        case WPN_SCYTHE:
        case WPN_SHORT_SWORD:
        case WPN_TRIPLE_SWORD:
            type_damage = DVORP_SLICING;
            break;

        case WPN_DEMON_TRIDENT:
        case WPN_EVENINGSTAR:
        case WPN_GIANT_SPIKED_CLUB:
        case WPN_MORNINGSTAR:
        case WPN_SPEAR:
        case WPN_SPIKED_FLAIL:
        case WPN_TRIDENT:
            type_damage = DVORP_PIERCING;
            break;

        case WPN_AXE:
        case WPN_BATTLEAXE:
        case WPN_BROAD_AXE:
        case WPN_EXECUTIONERS_AXE:
        case WPN_GLAIVE:
        case WPN_HALBERD:
        case WPN_HAND_AXE:
            type_damage = DVORP_CHOPPING;
            break;
        }
    }

    return (type_damage);
}                               // end damage_type()

bool can_cut_meat(unsigned char wclass, unsigned char wtype)
{
    int type = damage_type( wclass, wtype );

    if (type == DVORP_CHOPPING || type == DVORP_SLICING)
        return (true);

    return (false);
}

int hands_reqd_for_weapon(unsigned char wclass, unsigned char wtype)
{
    int reqd_hands = HANDS_ONE_HANDED;

    switch (wclass)
    {
    case OBJ_WEAPONS:
        switch (wtype)
        {
        case WPN_HALBERD:
        case WPN_SCYTHE:
        case WPN_GLAIVE:
        case WPN_QUARTERSTAFF:
        case WPN_BATTLEAXE:
        case WPN_EXECUTIONERS_AXE:
        case WPN_GREAT_SWORD:
        case WPN_TRIPLE_SWORD:
        case WPN_GREAT_MACE:
        case WPN_GREAT_FLAIL:
        case WPN_GIANT_CLUB:
        case WPN_GIANT_SPIKED_CLUB:
            reqd_hands = HANDS_TWO_HANDED;
            break;

        case WPN_SPEAR:
        case WPN_TRIDENT:
        case WPN_DEMON_TRIDENT:
        case WPN_AXE:
        case WPN_BROAD_AXE:
        case WPN_KATANA:
        case WPN_DOUBLE_SWORD:
            reqd_hands = HANDS_ONE_OR_TWO_HANDED;
            break;
        }
        break;

    case OBJ_STAVES:
        reqd_hands = HANDS_TWO_HANDED;
        break;
    }

    return (reqd_hands);
}                               // end hands_reqd_for_weapon()

bool is_demonic(unsigned char weapon_subtype)
{
    switch (weapon_subtype)
    {
    case WPN_DEMON_BLADE:
    case WPN_DEMON_WHIP:
    case WPN_DEMON_TRIDENT:
        return true;

    default:
        return false;
    }
}                               // end is_demonic()

bool launches_things( unsigned char weapon_subtype )
{
    switch (weapon_subtype)
    {
    case WPN_SLING:
    case WPN_BOW:
    case WPN_CROSSBOW:
    case WPN_HAND_CROSSBOW:
    case WPN_BLOWGUN:
        return (true);

    default:
        return (false);
    }
}                               // end launches_things()

unsigned char launched_by(unsigned char weapon_subtype)
{
    switch (weapon_subtype)
    {
    case WPN_BLOWGUN:
        return MI_NEEDLE;
    case WPN_SLING:
        return MI_STONE;
    case WPN_BOW:
        return MI_ARROW;
    case WPN_CROSSBOW:
        return MI_BOLT;
    case WPN_HAND_CROSSBOW:
        return MI_DART;
    default:
        return MI_EGGPLANT;     // lame debugging code :P {dlb}
    }
}                               // end launched_by()

// this function returns the skill that the weapon would use in melee
char weapon_skill(unsigned char wclass, unsigned char wtype)
{
    char skill2use = SK_FIGHTING;

    if (wclass == OBJ_STAVES
        && (wtype < STAFF_SMITING || wtype > STAFF_AIR))
    {
        skill2use = SK_STAVES;
    }
    else if (wclass != OBJ_WEAPONS)
        skill2use = SK_FIGHTING;
    else
    {
        switch (wtype)
        {
        case WPN_CLUB:
        case WPN_MACE:
        case WPN_HAMMER:
        case WPN_ANCUS:
        case WPN_WHIP:
        case WPN_FLAIL:
        case WPN_MORNINGSTAR:
        case WPN_GIANT_CLUB:
        case WPN_GIANT_SPIKED_CLUB:
        case WPN_EVENINGSTAR:
        case WPN_DEMON_WHIP:
        case WPN_SPIKED_FLAIL:
        case WPN_GREAT_FLAIL:
        case WPN_GREAT_MACE:
        case WPN_BOW:
        case WPN_BLOWGUN:
        case WPN_CROSSBOW:
        case WPN_HAND_CROSSBOW:
            skill2use = SK_MACES_FLAILS;
            break;

        case WPN_KNIFE:
        case WPN_DAGGER:
        case WPN_SHORT_SWORD:
        case WPN_QUICK_BLADE:
        case WPN_SABRE:
            skill2use = SK_SHORT_BLADES;
            break;

        case WPN_LONG_SWORD:
        case WPN_SCIMITAR:
        case WPN_KATANA:
        case WPN_DOUBLE_SWORD:
        case WPN_DEMON_BLADE:
        case WPN_GREAT_SWORD:
        case WPN_TRIPLE_SWORD:
            skill2use = SK_LONG_SWORDS;
            break;

        case WPN_HAND_AXE:
        case WPN_AXE:
        case WPN_BROAD_AXE:
        case WPN_BATTLEAXE:
        case WPN_EXECUTIONERS_AXE:
            skill2use = SK_AXES;
            break;

        case WPN_SPEAR:
        case WPN_HALBERD:
        case WPN_GLAIVE:
        case WPN_SCYTHE:
        case WPN_TRIDENT:
        case WPN_DEMON_TRIDENT:
            skill2use = SK_POLEARMS;
            break;

        case WPN_QUARTERSTAFF:
            skill2use = SK_STAVES;
            break;
        }
    }

    return (skill2use);
}                               // end weapon_skill()
/*
 **************************************************
 *                                                *
 *              END PUBLIC FUNCTIONS              *
 *                                                *
 **************************************************
*/


/*----------------------------------------------------------------

Detailed description of a weapon.

Alex: 3.12.01
----------------------------------------------------------------*/
//handle keyboard of the menu
int key_desc_weap(struct tMenu& mnu,int c)
 {
   int n;
   char *cad;
   if (c==13) return mnu.nItem;   
   for (n=0;n<mnu.nMaxItems;n++)
     {
       cad=mnu.data+15*n;
       if (cad[0] == c) return n;	
     }
   return -2;  		
 } 	


//Detailed info of the accuracity
string desc_hit(const item_def &item,int bOnlyTotal=0)
  {
       string description;

       description.reserve(400);

       description = "";
  	  	
       int wpn_skill = weapon_skill( item.base_type, item.sub_type );
       int bonus,hit=0,min;
        
        //your stats        
        description+="$Your hit base=";                         
        bonus=calc_stat_to_hit_base();
        append_value(description, bonus, false);
        hit+= bonus;
        description+="=";
        append_value(description, hit, false);
        min=hit;
                               
        //if amulet of innacuracity
        if (wearing_amulet(AMU_INACCURACY) && !item_not_ident( you.inv[ you.equip[EQ_AMULET]], ISFLAG_KNOW_PLUSES))
          {
            hit -= 5;
            description+="$     + Amulet of inaccuracity -5=";
            append_value(description, hit, false);
           }  
           
        //invisible but player can't see invisible
        if (you.invis && !player_see_invis())
           {
             hit -= 5;
             description+="$     +Don't see yourself -5=";
             append_value(description, hit, false);
            }  
        
        //your skills figthing
        description+="$     +Skills fighting 1D";
        bonus=you.skills[SK_FIGHTING];
        append_value(description, bonus, false);
        hit+= bonus;
        description+="=";
        append_value(description, hit, false);
        
        //your skills with the weapon
        description+="$     + Skills Weap. 1D";
        bonus=you.skills[wpn_skill];
        append_value(description, bonus, false);
        hit+= bonus;
        description+="=";
        append_value(description, hit, false);        
        
        //weapon bonus
        bonus=item.plus;
        if ((!item_not_ident(item, ISFLAG_KNOW_PLUSES )) && bonus>0)
          {
              description+="$     + Weap. bonus ";
              append_value(description, bonus,false);
              hit+= bonus;
              min+=bonus;
              description+="=";
              append_value(description, hit, false);        
          }    
          
        //base of the weapon       
  	    description += "$     + Accuracy rating ";
        bonus=property( item, PWPN_HIT );    
        append_value(description, bonus, true);
        hit+=bonus;
        description+="=";
        append_value(description, hit, true);
                
          
        //racial bonus
        if (cmp_equip_race(item, ISFLAG_ELVEN )
                && player_genus(GENPC_ELVEN))
            {
                description+="$     + Race Bonus 1D2";
                hit+=2;
                min++;
                description+="=";
                append_value(description, hit, false);
            }          
        if (player_is_swimming())
            {
                description+="$     + Race Bonus 1D5";
                hit+=5;
                min++;
                description+="=";
                append_value(description, hit, false);  
            }          
        
        //slayig bonus
        bonus=slaying_bonus(PWPN_HIT);
        if (bonus>0)
            {
                description+="$     + Slaying bonus ";
                append_value(description, bonus, false);  
                hit+=bonus;
                description+="=";
                append_value(description, hit, false);  
            }   
        
        //sure blade    
        if (wpn_skill == SK_SHORT_BLADES && you.sure_blade) 
            {
                description+="$     + Sure Blade 5+Min(1D";
                append_value(description, you.sure_blade, false);  
                description+=",10)";
                hit+=5+10;
                min+=5;
                description+="=";
                append_value(description, hit, false);      
            }  
            
        //confusing touch
        if (you.confusing_touch)      
          {
            description+="$     + Confusing touch 1D"; 
            append_value(description,you.dex,false);
            hit+=you.dex;
            description+="=";
            append_value(description,hit,false);            
          }  
          
        //Transformations
        if (you.attribute[ATTR_TRANSFORMATION] != TRAN_NONE)
        {
            switch (you.attribute[ATTR_TRANSFORMATION])
            {
            case TRAN_BLADE_HANDS:
                hit += 12;
                break;
            case TRAN_STATUE:
                hit += 9;
                break;
            case TRAN_SERPENT_OF_HELL:
            case TRAN_DRAGON:
            case TRAN_LICH:
            case TRAN_ICE_BEAST:
            case TRAN_SPIDER:
                hit += 10;
                break;
            case TRAN_AIR:
                hit = 0;
                min=0;
                break;
            }
        }
                  
        //TOTAL
        if (bOnlyTotal) 
           description="";
        else 
           description+="$$Max Accuracity=";        
                           
        append_value(description,hit, false);   
        if (bOnlyTotal) return description;
        
        description+="$Min Accuracity=";        
        append_value(description,min, false);   
        
                
        //Other
        description+="$$     When starving -3=";
        append_value(description,min-3, false);   
        description+=" to ";
        append_value(description,hit-3, false);   
                
        description+="$$     With shield, uncumbered, non heavy armour$     and 1D20 < Sk. Unarmed Combat (";
        append_value(description,you.skills[SK_UNARMED_COMBAT], false); 
        description+=")=";
        append_value(description,min, false); 
        description+=" to ";
        append_value(description,hit+3, false); 
        
        return (description);
  }
	
	
//Detailed info of the damage of a weapon
string desc_damage( const item_def &item,int bOnlyTotal=0)
  {
       string description;

       description.reserve(400);

       description = "";
  	  	
       int wpn_skill = weapon_skill( item.base_type, item.sub_type );
       int bonus,dam=1,min;       
        
        //base damage
        description = "$DAMAGE RATING: ";
        dam=property( item, PWPN_DAMAGE );
        append_value(description, dam, false);
        
        //Your str/dex
        description+="$     * YourStats 1D";
        bonus=calc_stat_to_dam_base(1,78,RM_MAXIMICE)/78; 
        append_value(description, bonus, false);
        dam=dam*bonus;
        min=dam;
        description+="=";
        append_value(description, dam, false);        
        
        //Slaying bonus        
        bonus=slaying_bonus(PWPN_DAMAGE);
        description+="$     + Slaying bonus ";
        append_value(description, bonus, false);
        dam=dam+bonus;
        min=dam;
        description+="=";
        append_value(description, dam, false);        
                
        //Plus your skills                
        description+="$     * skills (25+1D";
        append_value(description, you.skills[ wpn_skill ],false);
        description+=")/25";
        dam=(dam*(25 + you.skills[ wpn_skill ]))/25;
        description+="=";
        append_value(description, dam, false);

        //Plus your skills fighting
        description+="$     * skills fighting (30+1D";
        append_value(description, you.skills[SK_FIGHTING],false);
        description+=")/30";
        dam=(dam*(30 + you.skills[SK_FIGHTING]))/30;
        description+="=";
        append_value(description, dam, false);
        
        //Plus Weap. Plus:
        bonus=item.plus2;
        if ((!item_not_ident(item, ISFLAG_KNOW_PLUSES )) && bonus>0)
           {
             description+="$     + Weap. Plus ";             
             if (bonus>-1) description+=" 1D"; else min=min+bonus;
             append_value(description, bonus, false);
             dam=dam+bonus;
             description+="=";
             append_value(description, dam, false);
           }  
        
        //Race pluses
         if (cmp_equip_race( item, ISFLAG_DWARVEN )
                && player_genus(GENPC_DWARVEN))
            {
                description+="$     + Race Bonus 1D3";
                dam=dam+3;
                description+="=";
                append_value(description, dam, false);
            }
        if (cmp_equip_race(item, ISFLAG_ORCISH )
                && you.species == SP_HILL_ORC)
            {
                description+="$     + Race Bonus 1D1";
                dam=dam+1;
                description+="=";
                append_value(description, dam, false);
            }    
            
        if (bOnlyTotal) 
           description="";
        else 
           description+="$$Max Damage=";        
                            
        append_value(description,dam, false);   
        if (bOnlyTotal) return description;
        
        description+="$Min Damage=";        
        append_value(description,min, false);   
        
        //Other    
        description+="$$     When mighty 1+1D10=";
        bonus=min+1;
        append_value(description,bonus, false);   
        description+=" to ";
        bonus=dam+10;
        append_value(description,bonus, false); 
        
        description+="$     When starving -1D5=";
        append_value(description,min, false);   
        description+=" to ";
        bonus=dam-5;
        append_value(description,bonus, false);   
  
        int hands_reqd = hands_reqd_for_weapon( item.base_type,item.sub_type );
                                            
                                            
        description+="$$     With shield, uncumbered, non heavy armour$     and 1D20 < Sk. Unarmed Combat (";
        append_value(description,you.skills[SK_UNARMED_COMBAT], false); 
        description+=")=";
        append_value(description,min, false); 
        description+=" to ";
        append_value(description,dam+3, false); 
          
        return (description);
  }


//Detailed info of the accuracity when unarmed
string desc_unarmed(int bOnlyTotal=0)
  {
       string description;

       description.reserve(400);

       description = "";
  	  	
       int bonus,hit=0,min;
        
        //your stats        
        description+="$Your hit base=";                         
        bonus=calc_stat_to_hit_base();
        append_value(description, bonus, false);
        hit+= bonus;
        description+="=";
        append_value(description, hit, false);
        min=hit;
                               
        //if amulet of innacuracity
        if (wearing_amulet(AMU_INACCURACY) && !item_not_ident( you.inv[ you.equip[EQ_AMULET]], ISFLAG_KNOW_PLUSES))
          {
            hit -= 5;
            description+="$     + Amulet of inaccuracity -5=";
            append_value(description, hit, false);
           }  
           
        //invisible but player can't see invisible
        if (you.invis && !player_see_invis())
           {
             hit -= 5;
             description+="$     +Don't see yourself -5=";
             append_value(description, hit, false);
            }  
        
        //your skills figthing
        description+="$     +Skills fighting 1D";
        bonus=you.skills[SK_FIGHTING];
        append_value(description, bonus, false);
        hit+= bonus;
        description+="=";
        append_value(description, hit, false);
        
        //Racial bonus when un-armed
        if (you.species==SP_TROLL)  
           {
              bonus=4;
           }   
        else if (you.species==SP_GHOUL) 
           {
             bonus=2;
           } 
        else if (player_is_swimming())
            {
                bonus=5;
                min++;
            }                 
        else bonus=0;
        
        if (bonus>0) 
           {
              description+="$     + Racial bonus ";
              append_value(description,bonus,false);
              description+="=";
              append_value(description,hit,false);
           }    
        
        
        //unarmed skill
        bonus=you.skills[SK_UNARMED_COMBAT];
        description+="$     + Skills Unarmed Combat ";
        append_value(description,bonus,false);
        description+="=";
        append_value(description,hit,false);
                                      
        //slayig bonus
        bonus=slaying_bonus(PWPN_HIT);
        if (bonus>0)
            {
                description+="$     + Slaying bonus ";
                append_value(description, bonus, false);  
                hit+=bonus;
                description+="=";
                append_value(description, hit, false);  
            }       
        
            
        //confusing touch
        if (you.confusing_touch)      
          {
            description+="$     + Confusing touch 1D"; 
            append_value(description,you.dex,false);
            hit+=you.dex;
            description+="=";
            append_value(description,hit,false);            
          }  
          
        //Transformations
        if (you.attribute[ATTR_TRANSFORMATION] != TRAN_NONE)
        {
            switch (you.attribute[ATTR_TRANSFORMATION])
            {
            case TRAN_BLADE_HANDS:
                hit += 12;
                break;
            case TRAN_STATUE:
                hit += 9;
                break;
            case TRAN_SERPENT_OF_HELL:
            case TRAN_DRAGON:
            case TRAN_LICH:
            case TRAN_ICE_BEAST:
            case TRAN_SPIDER:
                hit += 10;
                break;
            case TRAN_AIR:
                hit = 0;
                min=0;
                break;
            }
        }
            
        //TOTAL
        if (bOnlyTotal) 
           description="";
        else 
           description+="$$Max Accuracity=";        
                            
        append_value(description,hit, false);   
        if (bOnlyTotal) return description;
        
        description+="$Min Accuracity=";        
        append_value(description,min, false);   
                
        //Other
        description+="$$     When starving -3=";
        append_value(description,min-3, false);   
        description+=" to ";
        append_value(description,hit-3, false);   
                
        
        return (description);
  }
  
  
//Detailed info of the accuracity of the second attack
string desc_hit_2_attack(int bOnlyTotal=0)
  {
       string description;

       description.reserve(400);

       description = "";
  	  	
       int bonus,hit=0,min=0;
        
        //your stats        
        description+="$Your hit base=";                         
        bonus=13 + you.dex / 2 + you.skills[SK_UNARMED_COMBAT] / 2 + you.skills[SK_FIGHTING] / 5;
        append_value(description, bonus, false);
        hit+= bonus;
        min=hit;
                               
        //if amulet of innacuracity
        if (wearing_amulet(AMU_INACCURACY) && !item_not_ident( you.inv[ you.equip[EQ_AMULET]], ISFLAG_KNOW_PLUSES))
          {
            hit -= 5;
            description+="$     + Amulet of inaccuracity -5=";
            append_value(description, hit, false);
           }  
           
        
                                      
        //slayig bonus
        bonus=slaying_bonus(PWPN_HIT);
        if (bonus>0)
            {
                description+="$     + Slaying bonus ";
                append_value(description, bonus, false);  
                hit+=bonus;
                description+="=";
                append_value(description, hit, false);  
            }       
        
            
        //TOTAL
        if (bOnlyTotal) 
           description="";
        else 
           description+="$$Max Accuracity=";        
                            
        append_value(description,hit, false);   
        
        return (description);
  }    
  
  
//Shows info about damage of seccond attack
string desc_dam_2_attack(int bOnlyTotal=0)
 {
       string description;
       int damage=3,bonus,punch,kick,headbutt,tail;
       int transformed;
       
       description.reserve(1000);

       description = "SECOND ATTACKS:$";
       
       transformed=you.attribute[ATTR_TRANSFORMATION] == TRAN_SERPENT_OF_HELL
                    || you.attribute[ATTR_TRANSFORMATION] == TRAN_ICE_BEAST
                    || you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON
                    || you.attribute[ATTR_TRANSFORMATION] == TRAN_SPIDER;
 	//Kick
        kick=0;
        if (!transformed)
         {                    
           if (you.mutation[MUT_HOOVES]|| you.species == SP_CENTAUR)       
             {
               description+="$Hooves or centaur: 10";
               kick+=10;
             }	
           else
             {
               kick+=5;  
               description+="$Kick: 5";
             }  	  
         }    
       
       //Tail
       tail=0;
       if (((you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON
                    || player_genus(GENPC_DRACONIAN)
                    || (you.species == SP_MERFOLK && player_is_swimming())
                    || you.mutation[ MUT_STINGER ])
               && you.attribute[ATTR_TRANSFORMATION] != TRAN_SPIDER
               && you.attribute[ATTR_TRANSFORMATION] != TRAN_ICE_BEAST))
        {            
           description+="$Tail 6 ";
           tail=6;
           if (you.mutation[ MUT_STINGER ] > 0)
                {
                    bonus= (you.mutation[ MUT_STINGER ] * 2 - 1);
                    tail+=bonus;
                    description+=" + Mutation ";
                    append_value(description,bonus,false);
                    
                }           
           if (you.species == SP_GREY_DRACONIAN && you.experience_level >= 7)
                {
                    tail+= 12;
                    description+=" + Draconian ";
                    append_value(description,bonus,false);         
                }      
            description+="=";
            append_value(description,tail,false);              
           
        }  
 
       //Punch
       if (!transformed)
         {
           punch = 5 + you.skills[SK_UNARMED_COMBAT] / 3;
           description+="$Punch: ";
           append_value(description,punch,false);
           if (you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS)
               {             
                    punch += 6;
                    description+="  + Blade Hands 6";
                    description+="=";
                    append_value(description,punch,false);
               }
         }	           
          
       //Headbutt
       headbutt=0;
       if (!transformed)
        {           
          description+="$Headbutt: 5";
          headbutt=5;
          if (you.species==SP_MINOTAUR)
            {
              headbutt+=5;
              description+="$     + minotaur 5=";
              append_value(description,headbutt,false);                  
            }
          if (you.mutation[MUT_HORNS])
            {
              bonus=you.mutation[MUT_HORNS] * 3;
              headbutt+=bonus;
              description+="     + mutation horns ";
              append_value(description,bonus,false);
              description+="=";
              append_value(description,headbutt,false);
           }   
        }
       
       if (you.equip[EQ_HELMET] != -1
             && (cmp_helmet_type( you.inv[you.equip[EQ_HELMET]], THELM_HELMET )
                 || cmp_helmet_type( you.inv[you.equip[EQ_HELMET]], THELM_HELM )))
        {
           bonus= 2;
           if (cmp_helmet_desc( you.inv[you.equip[EQ_HELMET]], THELM_DESC_SPIKED )
                        || cmp_helmet_desc( you.inv[you.equip[EQ_HELMET]], THELM_DESC_HORNED ))
             {
                        bonus+= 3;
             }
           description+="$     + Helm ";
           append_value(description,bonus,false);
           headbutt+=bonus;
           description+="=";
           append_value(description,headbutt,false);         
        }
        
      
      int dammod = 10;
      
      //your stats
      description+="$$* Your Stats: 1D" ;
      damage = calc_stat_to_dam_base(-1,10,RM_MAXIMICE)/10;
      append_value(description,damage,false);
      kick=kick*damage/10;
      tail=tail*damage/10;
      headbutt=headbutt*damage/10;
      punch=punch*damage/10;
      description+="$     Kick=";
      append_value(description,kick,false);
      description+="  Punch=";
      append_value(description,punch,false);
      description+="  Tail=";
      append_value(description,tail,false);
      description+="  Headbutt=";
      append_value(description,headbutt,false);
      
      //slaying bonus
      bonus=slaying_bonus(PWPN_DAMAGE);
      if (bonus>0)
        {
          description+="$+Slaying bonus ";
          append_value(description,bonus,false);
          kick+=bonus;
          tail+=bonus;
          headbutt+=bonus;
          punch+=bonus;
          append_value(description,kick,false);
          description+="  Punch=";
          append_value(description,punch,false);
          description+="  Tail=";
          append_value(description,tail,false);
          description+="  Headbutt=";
          append_value(description,headbutt,false);
        }    
      
      //skills fighting
      kick *= 40 + you.skills[SK_FIGHTING];
      kick /= 40;
      description+="$* Skill Fighting 1D ";
      append_value(description,you.skills[SK_FIGHTING],false);
      kick *= 40 + you.skills[SK_FIGHTING];kick /= 40;
      tail *= 40 + you.skills[SK_FIGHTING];tail /= 40;
      headbutt *= 40 + you.skills[SK_FIGHTING];headbutt/= 40;
      punch*= 40 + you.skills[SK_FIGHTING];punch/= 40;
      description+="$     Kick=";
      append_value(description,kick,false);
      description+="  Punch=";
      append_value(description,punch,false);
      description+="  Tail=";
      append_value(description,tail,false);
      description+="  Headbutt=";
      append_value(description,headbutt,false);
      
      //skills unarmed combat
      description+="$* Skill Fighting 1D ";
      append_value(description,you.skills[SK_UNARMED_COMBAT],false);
      kick *= 25 + you.skills[SK_UNARMED_COMBAT];kick /= 25;
      tail *= 25 + you.skills[SK_UNARMED_COMBAT];tail /= 25;
      headbutt *= 25 + you.skills[SK_UNARMED_COMBAT];headbutt/= 25;
      punch*= 25 + you.skills[SK_UNARMED_COMBAT];punch/= 25;
      
      if (bOnlyTotal) 
           description="";
      else    
           description+="$     ";
      description+="Kick=";
      append_value(description,kick,false);
      description+="  Punch=";
      append_value(description,punch,false);
      description+="  Tail=";
      append_value(description,tail,false);
      description+="  Headbutt=";
      append_value(description,headbutt,false);
      if (bOnlyTotal) return description;
      
      //Other    
        description+="$$     + When mighty 1+1D10";                
        description+="$     + When starving -1D5";
      
      return description;  
 }	 

//Shows info about damage when unarmed
string desc_dam_unarmed(int bOnlyTotal=0)
 {
       string description;
       int damage=3,bonus,punch,kick,headbutt,tail;

       description.reserve(1000);
       
  	  	
       //confusing touch
       if (you.confusing_touch) 
          {
            damage=0;
            description="$Damage using Confusing Touch=0";
          }  
       
       //transformations	
       if (you.attribute[ATTR_TRANSFORMATION] != TRAN_NONE)
        {
            description+="$Transformation ";	
            switch (you.attribute[ATTR_TRANSFORMATION])
            {
            case TRAN_SPIDER:
                damage = 9;
                break;
            case TRAN_BLADE_HANDS:
                damage = 12 + (you.strength / 4) + (you.dex / 4);
                break;
            case TRAN_STATUE:
                damage = 12 + you.strength;
                break;
            case TRAN_ICE_BEAST:
                damage = 10;
                break;
            case TRAN_SERPENT_OF_HELL:
            case TRAN_DRAGON:
                damage = 20 + you.strength;
                break;
            case TRAN_LICH:
                damage = 5;
                break;
            case TRAN_AIR:
                damage = 0;
                break;
            }
          append_value(description,damage,false);  
        }	                         	                          	                          
      else
        {         
           if (!you.confusing_touch) description+="$Damage of your bare hands: 3";  
 
           //Claws
           if (you.equip[ EQ_GLOVES ] == -1)
            {
               // claw damage only applies for bare hands            
               if (you.species == SP_TROLL)               
                   {
                     description+="$    + Race Claws: 5 =8";
                     damage += 5;
                   }  
               else if (you.species == SP_GHOUL)
                   {
                     description+="$    + Race Claws: 2 =7";	
                     damage += 2;
                   }  
              if (you.mutation[MUT_CLAWS]>0) 
                   {
                     description+=" + Mut. Claws: ";
                     bonus=you.mutation[ MUT_CLAWS ];
                     append_value(description,bonus, false);   	
                     damage += (bonus * 2);
                     description+=" =";
                     append_value(description,damage, false);   	
                   } 	
            }             
        }      
        
        
       //skills
       bonus = you.skills[SK_UNARMED_COMBAT];   
       description+="$     + Sk. Unarmed combat  ";
       append_value(description,bonus, false);         
       description+=" =";
       damage+=bonus;
       append_value(description,damage, false);  
        
       if (player_in_water() && !player_is_swimming())
          {
            description+="     + Merfolk Swiming (1D10+1D10)/2";
            bonus = 10;
            description+=" =";
            damage+=bonus;
            append_value(description,damage, false);  
          }   
    	                                    
       //Stats
       description+="$     * YourStats 1D";
       bonus=calc_stat_to_dam_base(1,78,RM_MAXIMICE)/78; 
       append_value(description, bonus, false);
       damage=damage*bonus;
       description+="=";
       append_value(description, damage, false);
       
       //Plus your skills fighting
       description+="$     * skills fighting (30+1D";
       append_value(description, you.skills[SK_FIGHTING],false);
       description+=")/30";
       damage=(damage*(30 + you.skills[SK_FIGHTING]))/30;
       description+="=";
       append_value(description, damage, false);
              
       //Total           
       if (bOnlyTotal) 
           description="";
        else 
           description+="$$Max Damage=";        
                            
        append_value(description,damage, false);   
        if (bOnlyTotal) return description;
       
       //Other    
        description+="$$     When mighty 1+1D10=";
        append_value(description,damage+10, false); 
        
        description+="$     When starving -1D5=";
        append_value(description,damage-5, false);   
  
        int hands_reqd = HANDS_ONE_HANDED;
                                            
                                            
        description+="$$     With shield, uncumbered, non heavy armour$     and 1D20 < Sk. Unarmed Combat (";
        append_value(description,you.skills[SK_UNARMED_COMBAT], false); 
        description+=")=";
        append_value(description,damage+3, false); 
        
        return description; 
       
 }	


string desc_brief(const item_def &item)
  {
    string description;
    
    description.reserve(400);
    
    description="";	
    
    item_name(item, DESC_INVENTORY_EQUIP, str_pass );
    
    description=str_pass;
    
    description += "$$Damage rating: ";
    append_value(description, property( item, PWPN_DAMAGE ), false);
        
    description += "$Accuracy rating: ";
    append_value(description, property( item, PWPN_HIT ), false);

    description += "$Attack delay: ";
    append_value(description, property( item, PWPN_SPEED ), false);

    description+="$$Max Damage=";
    description+=desc_damage(item,1);
    description+="$Max Hit=";
    description+=desc_hit(item,1);
       	    
    description+="$Max Dam. Unarmed=";
    description+=desc_dam_unarmed(1);
    description+="$Max Hit Unarmed=";
    description+=desc_unarmed(1);
    
    return description;
	
  }	
//Shows info about the weapon
int show_desc_weap(struct tMenu& mnu,int bSelec)
  {
     int n;
     char *cad;
     int item;
     string description;

     description.reserve(2000);

     description = "";
     
     item=*(mnu.data2);
       
     gotoxy(4,mnu.nFila);
     if (bSelec==4) return 0; 
     
     if (bSelec)
        textbackground(RED);
     else
        textbackground(BLACK);          
     textcolor(WHITE);   
        
     cad=mnu.data;
     cad=cad+mnu.nItem*20; //15 it's the size of an item of the menu
     cprintf(cad);
     textbackground(BLACK);
     if (!bSelec) return 0;
     
     for (n=5;n<=24;n++)
       {
       	 gotoxy(25,n);
       	 cprintf("%53s"," ");
       }	     
     n=strlen(cad);
     while (n++<20) {cprintf(" ");};
          
     switch(mnu.nItem)
       {
       	 case 0:
       	    description+=desc_brief(you.inv[item]);
       	    break;
       	 case 1:
       	    description+=describe_weapon(you.inv[item],1);
       	    break;
       	 //when wielding a weapon   
       	 case 2:
       	   description+=desc_damage(you.inv[item]);
       	   break;
       	 case 3:
       	   description+=desc_hit(you.inv[item]);
       	   break;
       	 //when unarmed  
       	 case 4:
       	   description+=desc_dam_unarmed();  
       	   break;
       	 case 5:
       	   description+=desc_unarmed();
       	   break;  
       	 //second attack  
       	 case 6:
       	   description+=desc_dam_2_attack();  
       	   break;
       	 case 7:
       	   description+=desc_hit_2_attack();  
       	   break;
       }	
     


    print_description(description,79-30,30,5);

  }


//Menu for the options to describe 
void desc_weapon(int weap)
 {

   struct tMenu mnu;
   int nWeap;
   char items[][20]=
     {
       "a - Brief",
       "b - Description",
       "c - Damage",
       "d - To Hit",
       "e - Dam. unarmed",
       "f - Hit  unarmed",
       "g - Dam. 2 attack",
       "h - To Hit 2 attack"
    };
   nWeap=weap;  
   mnu.nFilaMax=23;
   mnu.nFilaIni=5;
   mnu.nMaxItems=7;
   mnu.print_item=show_desc_weap;
   mnu.handle_key=key_desc_weap;
   mnu.data=(char*)items;
   mnu.data2=(char*) &nWeap;

   textbackground(BLACK);   
   clrscr();
   textcolor(MAGENTA);
   Box(1,1,80,3);textcolor(WHITE);
   gotoxy(5,2);cprintf("Describe Weapon");
   item_name(you.inv[weap], DESC_INVENTORY_EQUIP, str_pass );
   textcolor(YELLOW);gotoxy(25,2);cprintf(str_pass);
   textcolor(MAGENTA);Box(1,4,24,24);
   menu(mnu);      	
 }
 
 
//Ask the weapon to describe 	
void cmd_desc_wep()
 {
   int item_slot = prompt_invent_item( "Examine which weapon?", OBJ_WEAPONS );
   if (item_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

   if (you.inv[item_slot].base_type!=OBJ_WEAPONS && you.inv[item_slot].base_type!=OBJ_STAVES)
     {
       mpr("Not a weapon nor a stave.",MSGCH_WARN);
       return;	
     }	 
     
   #ifdef DOS_TERM
    char buffer[4800];

    window(1, 1, 80, 25);
    gettext(1, 1, 80, 25, buffer);
   #endif	

   desc_weapon(item_slot);
   redraw_screen();
   
     #ifdef DOS_TERM
      puttext(1, 1, 80, 25, buffer);
   #endif 
 }
 


void spoiler_gen_prop_weap()
 {
   FILE *fich,*fich2;
   int n;
   char cad[]="%-20s%2d    %2d   %2d   %3d   %-5s %-2s   %s\n";
   char hands[7];
   char skill[20];
   char stat[2];
   int nhands;
   int nstat;
   item_def item;
   
   item.base_type=OBJ_WEAPONS;
   strcpy(item.inscription,"");
   item.plus=0;
   item.plus2=0;
   item.quantity=1;
   item.special=0;
   item.flags = 0;
   int str_weight;
   
   //fich=fopen("weapons.spl","wt");
   fich2=fopen("data/weapons.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_WEAPONS);
   //fprintf(fich,"WEAPON              DAMAGE HIT SPEED MASS HANDS STAT SKILL\n");
   //fprintf(fich,"-------------------------------------------------------------------\n");
   mpr("spoiler weapons");
   more();
   for (n=0;n<NUM_WEAPONS;n++)
     {
     	item.sub_type=n;
     	item_name(item, DESC_PLAIN,str_pass);
     	switch (hands_reqd_for_weapon(item.base_type, item.sub_type))
        {
        case HANDS_ONE_HANDED:
            strcpy(hands,"1");
            nhands=1;
            break;
        case HANDS_ONE_OR_TWO_HANDED:
            strcpy(hands,"1/2");
            nhands=3;
            break;
        case HANDS_TWO_HANDED:
            strcpy(hands,"2");
            nhands=2;
            break;
        }
        
        switch (item.sub_type)
        {
        case WPN_SLING:
            strcpy(skill, "sling");
            break;
        case WPN_BOW:
            strcpy(skill, "bow");
            break;
        case WPN_HAND_CROSSBOW:
        case WPN_CROSSBOW:
            strcpy(skill, "crossbow");
            break;
        case WPN_BLOWGUN:
            strcpy(skill, "dart");
            break;
        default:
            // Melee weapons
            switch (weapon_skill(item.base_type, item.sub_type))
            {
            case SK_SHORT_BLADES:
                strcpy(skill, "short_blades");
                break;
            case SK_LONG_SWORDS:
                strcpy(skill, "long_swords");
                break;
            case SK_AXES:
                strcpy(skill, "axes");
                break;
            case SK_MACES_FLAILS:
                strcpy(skill, "maces_flails");
                break;
            case SK_POLEARMS:
                strcpy(skill, "polearms");
                break;
            case SK_STAVES:
                strcpy(skill, "staves");
                break;
            default:
                strcpy(skill, "bug");
                break;
            }
        }
        
        str_weight = weapon_str_weight( item.base_type, item.sub_type );

        if (str_weight >= 8)
            {strcpy(stat,"S+");nstat=2;}
        else if (str_weight > 5)
            {strcpy(stat,"S");nstat=1;}
        else if (str_weight <= 2)
            {strcpy(stat,"D+");nstat=4;}
        else if (str_weight < 5)
            {strcpy(stat,"D");nstat=3;}
            
     	//fprintf(fich,cad,str_pass,
     	//                 property( item, PWPN_DAMAGE ),
        //                 property( item, PWPN_HIT ), 
        //                 property( item, PWPN_SPEED ),                         
        //                 mass_item( item ),
        //                 hands,
        //                 stat,
        //                 skill
        //                 );
       
       
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"DG:%d\n",property( item, PWPN_DAMAGE ));
       fprintf(fich2,"HT:%d\n",property( item, PWPN_HIT ));
       fprintf(fich2,"SP:%d\n",property( item, PWPN_SPEED ));
       fprintf(fich2,"WG:%d\n",mass_item(item));
       fprintf(fich2,"HN:%d\n",nhands);
       fprintf(fich2,"ST:%s\n",stat);
       fprintf(fich2,"GD:%d\n",item_value(item,id,-1));
       fprintf(fich2,"DC:%s\n",description_weapon(item).c_str());
       fprintf(fich2,"SK:%s\n\n\n",skill);

       
       
     }
   //fclose(fich);  
   fclose(fich2);
   mpr("spoiler weapons 2");
   more();
 }	               
 
 
void spoiler_gen_prop_armour()
 {
   FILE *fich,*fich2;
   int n;
   char cad[]="%-30s%3d    %3d         %3d\n";
   char hands[7];
   char skill[20];
   char stat[2];
   int nhands;
   int nstat;
   item_def item;
   
   item.base_type=OBJ_ARMOUR;
   strcpy(item.inscription,"");
   item.plus=0;
   item.plus2=0;
   item.quantity=1;
   item.special=0;
   item.flags = 0;
   int str_weight;
   
   //fich=fopen("doc/tables/armour.spl","wt");
   fich2=fopen("data/armour.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_ARMOURS);
   //fprintf(fich,"ARMOUR                       AC   EVASION       MASS\n");
   //fprintf(fich,"----------------------------------------------------\n");
   
   for (n=0;n<NUM_ARMOURS;n++)
     {
     	item.sub_type=n;
     	item_name(item, DESC_PLAIN,str_pass);
            
     	//fprintf(fich,cad,str_pass,
     	//                 property( item, PARM_AC ),
        //                 property( item, PARM_EVASION ), 
        //                 mass_item( item )
        //                 );

       
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"AC:%d\n",property( item, PARM_AC ));
       fprintf(fich2,"EV:%d\n",property( item, PARM_EVASION ));
       fprintf(fich2,"WG:%d\n",mass_item(item));
       fprintf(fich2,"GD:%d\n",item_value(item,id,-1));
       fprintf(fich2,"DC:%s\n\n\n",description_armour(item).c_str());
     }
   //fclose(fich);  
   fclose(fich2);
 }	 
 
  
 void spoiler_gen_prop_missil()
 {
   FILE *fich,*fich2;
   int n;
   char cad[]="%-30s%3d    %3d           %4d\n";
   char hands[7];
   char skill[20];
   char stat[2];
   int nhands;
   int nstat;
   item_def item;
   
   item.base_type=OBJ_MISSILES;
   strcpy(item.inscription,"");
   item.plus=0;
   item.plus2=0;
   item.quantity=1;
   item.special=0;
   item.flags = 0;
   int str_weight;
   
   //fich=fopen("doc/tables/missil.spl","wt");
   fich2=fopen("data/missil.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_MISSILES);
   //fprintf(fich,"MISSIL                        HIT   DAMAGE         MASS\n");
   //fprintf(fich,"-------------------------------------------------------\n");
   for (n=0;n<NUM_MISSILES;n++)
     {
     	item.sub_type=n;
     	item_name(item, DESC_PLAIN,str_pass);
     	//fprintf(fich,cad,str_pass,
     	//                 property( item, PWPN_DAMAGE ),
        //                 property( item, PWPN_HIT ), 
        //                 mass_item( item )
        //                 );

       
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"DG:%d\n",property( item, PWPN_DAMAGE ));
       fprintf(fich2,"HT:%d\n",property( item, PWPN_HIT ));
       fprintf(fich2,"WG:%d\n",mass_item(item));
       fprintf(fich2,"GD:%d\n",item_value(item,id,-1));
       fprintf(fich2,"DC:%s\n\n\n",describe_ammo(item).c_str());
     }
   //fclose(fich);  
   fclose(fich2);
 }	 
 
 
 
 void spoiler_gen_prop_potions()
 {
   FILE *fich,*fich2;
   int n;
 
   item_def item;
   
   item.base_type=OBJ_POTIONS;
   strcpy(item.inscription,"");
   item.plus=0;
   item.plus2=0;
   item.quantity=1;
   item.special=0;
   item.flags = ISFLAG_IDENT_MASK;
   
   
   
   fich2=fopen("data/potions.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_POTIONS);

   for (n=0;n<NUM_POTIONS;n++)
     {
     	item.sub_type=n;
     	id[IDTYPE_POTIONS][n]=ID_KNOWN_TYPE;
     	item_name(item, DESC_PLAIN,str_pass);
     	
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"GD:%d\n",item_value(item,id,-1));
       fprintf(fich2,"DC:%s\n\n\n",describe_potion( item).c_str());
     }
   fclose(fich2);
 }	 
 
 
 
 
 void spoiler_gen_prop_wands()
 {
   FILE *fich,*fich2;
   int n;
 
   item_def item;
   
   item.base_type=OBJ_WANDS;
   strcpy(item.inscription,"");
   item.plus=0;
   item.plus2=0;
   item.quantity=1;
   item.special=0;
   item.flags = ISFLAG_IDENT_MASK;
   
   
   
   fich2=fopen("data/wands.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_WANDS);

   for (n=0;n<NUM_WANDS;n++)
     {
     	item.sub_type=n;
     	id[IDTYPE_WANDS][n]=ID_KNOWN_TYPE;
     	item_name(item, DESC_PLAIN,str_pass);
     	
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"GD:%d\n",item_value(item,id,-1));
       fprintf(fich2,"DC:%s\n\n\n",describe_stick( item).c_str());
     }
   fclose(fich2);
 }
 
  
 
  void spoiler_gen_prop_jew()
 {
   FILE *fich,*fich2;
   int n;
 
   item_def item;
   
   item.base_type=OBJ_JEWELLERY;
   strcpy(item.inscription,"");
   item.plus=0;
   item.plus2=0;
   item.quantity=1;
   item.special=0;
   item.flags = ISFLAG_IDENT_MASK;
   
   
   
   fich2=fopen("data/jews.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_JEWELLERY);
         
   for (n=0;n<NUM_JEWELLERY;n++)
     {
     	item.sub_type=n;
     	id[IDTYPE_JEWELLERY][n]=ID_KNOWN_TYPE;
     	item_name(item, DESC_PLAIN,str_pass);
     	
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"GD:%d\n",item_value(item,id,true,false));
       fprintf(fich2,"DC:%s\n\n\n",describe_jewellery( item, 1 ).c_str());            
     }
   fclose(fich2);
 }         
 
 
 void spoiler_gen_prop_its()
 {
   FILE *fich,*fich2;
   int n;
 
   item_def item;
   
   item.base_type=OBJ_STAVES;
   strcpy(item.inscription,"");
   item.plus=0;
   item.plus2=0;
   item.quantity=1;
   item.special=0;
   item.flags = ISFLAG_IDENT_MASK;
   
   
   
   fich2=fopen("data/staves.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_STAVES);
         
   for (n=0;n<NUM_STAVES;n++)
     {
     	item.sub_type=n;
     	//id[IDTYPE_JEWELLERY][n]=ID_KNOWN_TYPE;
     	item_name(item, DESC_PLAIN,str_pass);
     	
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"GD:%d\n",item_value(item,id,true,false));
       fprintf(fich2,"DC:%s\n\n\n",describe_staff( item).c_str());            
     }
   fclose(fich2);
   
   item.base_type=OBJ_FOOD;
   
   fich2=fopen("data/food.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_FOODS);
         
   for (n=0;n<NUM_FOODS;n++)
     {
     	item.sub_type=n;
     	//id[IDTYPE_JEWELLERY][n]=ID_KNOWN_TYPE;
     	item_name(item, DESC_PLAIN,str_pass);
     	
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"GD:%d\n",item_value(item,id,true,false));
       fprintf(fich2,"DC:%s\n\n\n",describe_food( item).c_str());            
     }
   fclose(fich2);
   
   item.base_type=OBJ_MISCELLANY;
   
   fich2=fopen("data/misc.txt","wt");
   fprintf(fich2,"NI:%d\n",NUM_MISCELLANY);
         
   for (n=0;n<NUM_MISCELLANY;n++)
     {
     	item.sub_type=n;
     	//id[IDTYPE_JEWELLERY][n]=ID_KNOWN_TYPE;
     	item_name(item, DESC_PLAIN,str_pass);
     	
       fprintf(fich2,"ID:%d\n",n);                  
       fprintf(fich2,"NM:%s\n",str_pass);
       fprintf(fich2,"GD:%d\n",item_value(item,id,true,false));
       fprintf(fich2,"DC:%s\n\n\n",describe_misc_item( item ).c_str());            
     }
   fclose(fich2);
   
 }              
 
