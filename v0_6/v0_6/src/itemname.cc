/*
 *  File:       itemname.cc
 *  Summary:    Misc functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <4>      9/09/99        BWR             Added hands_required function
 *      <3>      6/13/99        BWR             Upped the base AC for heavy armour
 *      <2>      5/20/99        BWR             Extended screen lines support
 *      <1>      -/--/--        LRH             Created
 */

#include "AppHdr.h"
#include "itemname.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "mon-util.h"
#include "randart.h"
#include "skills2.h"
#include "stuff.h"
#include "wpn-misc.h"
#include "view.h"

#ifdef MACROS
#include "macro.h"
#endif

#include "menus.h" //alex:19.11.01
#include "options.h" //alex:29.11.01
#include "objects.h" //alex:11.12.01
#include "uniddesc.h"
#include "translate.h"

void select_items(int opci,struct tMenu& mnu_i,int bOnlyShow=0); //alex:19.11.01

char id[4][50];
int  prop[4][50][3];
FixedArray < int, 20, 50 > mss;

bool is_a_vowel(unsigned char let);
static char item_name_2( const item_def &item, char glog[80] );

char reduce(unsigned char reducee);
char retbit(char sed);
char retvow(char sed);

// Some convenient functions to hide the bit operations and create 
// an interface layer between the code and the data in case this 
// gets changed again. -- bwr
bool item_cursed( const item_def &item )
{
    return (item.flags & ISFLAG_CURSED);
}

bool item_uncursed( const item_def &item )
{
    return !(item.flags & ISFLAG_CURSED);
}

bool item_known_cursed( const item_def &item )
{
    return ((item.flags & ISFLAG_KNOW_CURSE) && (item.flags & ISFLAG_CURSED));
}

bool item_known_uncursed( const item_def &item )
{
    return ((item.flags & ISFLAG_KNOW_CURSE) && !(item.flags & ISFLAG_CURSED));
}

bool fully_identified( const item_def &item )
{
    return ((item.flags & ISFLAG_IDENT_MASK) == ISFLAG_IDENT_MASK);
}

bool item_ident( const item_def &item, unsigned long flags )
{
    return (item.flags & flags);
}

bool item_not_ident( const item_def &item, unsigned long flags )
{
    return ( !(item.flags & flags) );
}

void do_curse_item( item_def &item )
{
    item.flags |= ISFLAG_CURSED;
}

void do_uncurse_item( item_def &item )
{
    item.flags &= (~ISFLAG_CURSED);
}

void set_ident_flags( item_def &item, unsigned long flags )
{
    item.flags |= flags;
}

void unset_ident_flags( item_def &item, unsigned long flags )
{
    item.flags &= (~flags);
}

// These six functions might seem silly, but they provide a nice layer
// for later changes to these systems. -- bwr
unsigned long get_equip_race( const item_def &item )
{
    return (item.flags & ISFLAG_RACIAL_MASK);
}

unsigned long get_equip_desc( const item_def &item )
{
    return (item.flags & ISFLAG_COSMETIC_MASK);
}

bool cmp_equip_race( const item_def &item, unsigned long val )
{
    return (get_equip_race( item ) == val);
}

bool cmp_equip_desc( const item_def &item, unsigned long val )
{
    return (get_equip_desc( item ) == val);
}

void set_equip_race( item_def &item, unsigned long flags )
{
    ASSERT( (flags & ~ISFLAG_RACIAL_MASK) == 0 );

    item.flags &= ~ISFLAG_RACIAL_MASK; // delete previous
    item.flags |= flags;
}

void set_equip_desc( item_def &item, unsigned long flags )
{
    ASSERT( (flags & ~ISFLAG_COSMETIC_MASK) == 0 );

    item.flags &= ~ISFLAG_COSMETIC_MASK; // delete previous
    item.flags |= flags;
}

short get_helmet_type( const item_def &item )
{
    ASSERT( item.base_type == OBJ_ARMOUR && item.sub_type == ARM_HELMET );

    return (item.plus2 & THELM_TYPE_MASK);
}

short get_helmet_desc( const item_def &item )
{
    ASSERT( item.base_type == OBJ_ARMOUR && item.sub_type == ARM_HELMET );

    return (item.plus2 & THELM_DESC_MASK);
}

void set_helmet_type( item_def &item, short type )
{
    ASSERT( (type & ~THELM_TYPE_MASK) == 0 );
    ASSERT( item.base_type == OBJ_ARMOUR && item.sub_type == ARM_HELMET );

    item.plus2 &= ~THELM_TYPE_MASK; 
    item.plus2 |= type;
}

void set_helmet_desc( item_def &item, short type )
{
    ASSERT( (type & ~THELM_DESC_MASK) == 0 );
    ASSERT( item.base_type == OBJ_ARMOUR && item.sub_type == ARM_HELMET );

    item.plus2 &= ~THELM_DESC_MASK; 
    item.plus2 |= type;
}

void set_helmet_random_desc( item_def &item )
{
    ASSERT( item.base_type == OBJ_ARMOUR && item.sub_type == ARM_HELMET );

    item.plus2 &= ~THELM_DESC_MASK; 
    item.plus2 |= (random2(8) << 8);
}

bool cmp_helmet_type( const item_def &item, short val )
{
    ASSERT( item.base_type == OBJ_ARMOUR && item.sub_type == ARM_HELMET );

    return (get_helmet_type( item ) == val);
}

bool cmp_helmet_desc( const item_def &item, short val )
{
    ASSERT( item.base_type == OBJ_ARMOUR && item.sub_type == ARM_HELMET );

    return (get_helmet_desc( item ) == val);
}

bool set_item_ego_type( item_def &item, int item_type, int ego_type )
{
    if (item.base_type == item_type 
        && !is_random_artefact( item ) 
        && !is_fixed_artefact( item ))
    {
        item.special = ego_type;
        return (true);
    }

    return (false);
}

int get_weapon_brand( const item_def &item )
{
    // Weapon ego types are "brands", so we do the randart lookup here.
    
    // Staves "brands" handled specially
    if (item.base_type != OBJ_WEAPONS)
        return (SPWPN_NORMAL);

    if (is_fixed_artefact( item ))
    {
        switch (item.special)
        {
        case SPWPN_SWORD_OF_CEREBOV:
            return (SPWPN_FLAMING);

        case SPWPN_STAFF_OF_OLGREB:
            return (SPWPN_VENOM);

        case SPWPN_VAMPIRES_TOOTH:
            return (SPWPN_VAMPIRICISM);

        default:
            return (SPWPN_NORMAL);
        }
    }
    else if (is_random_artefact( item ))
    {
        return (randart_wpn_property( item, RAP_BRAND ));
    }

    return (item.special);
}

int get_ammo_brand( const item_def &item )
{
    // no artefact arrows yet -- bwr
    if (item.base_type != OBJ_MISSILES || is_random_artefact( item ))
        return (SPMSL_NORMAL);

    return (item.special);
}

int get_armour_ego_type( const item_def &item )
{
    // artefact armours have no ego type, must look up powers separately
    if (item.base_type != OBJ_ARMOUR || is_random_artefact( item ))
        return (SPARM_NORMAL);

    return (item.special);
}

bool item_is_rod( const item_def &item )
{
    return (item.base_type == OBJ_STAVES 
            && item.sub_type >= STAFF_SMITING && item.sub_type < STAFF_AIR);
}

bool item_is_staff( const item_def &item )
{
    // Isn't De Morgan's law wonderful. -- bwr
    return (item.base_type == OBJ_STAVES 
            && (item.sub_type < STAFF_SMITING || item.sub_type >= STAFF_AIR));
}

// it_name() and in_name() are now somewhat obsolete now that itemname
// takes item_def, so consider them depricated.
void it_name(int itn, char des, char str_pass[80])
{
    item_name( mitm[itn], des, str_pass );
}                               // end it_name()


void in_name(int inn, char des, char str_pass[80])
{
    item_name( you.inv[inn], des, str_pass );
}                               // end in_name()

// quant_name is usful since it prints out a different number of items
// than the item actually contains.
void quant_name(const item_def &item, int quant, char des, char str_pass[80])
{
    // item_name now requires a "real" item, so we'll mangle a tmp
    item_def tmp = item;
    tmp.quantity = quant;

    item_name( tmp, des, str_pass );
}                               // end quant_name()


void ParseName(char *name,const item_def &item,char *parse)
 {
      char buf[80];
      buf[0]=0;
      if (strcmp(parse,"P1")==0)
        {               
          if (item_ident( item, ISFLAG_KNOW_PLUSES ))  sprintf(buf,"%d",item.plus);            
        }    
      else  if (strcmp(parse,"PP1")==0)
        {
          if (item_ident( item, ISFLAG_KNOW_PLUSES ))  sprintf(buf,"(%d)",item.plus);            	
        }	   
      else if (strcmp(parse,"P2")==0)
        {               
            if (item_ident( item, ISFLAG_KNOW_PLUSES )) 
               sprintf(buf,"%d",item.plus2);                                   
        } 
      else if (strcmp(parse,"PP")==0)
        {               
            if (item_ident( item, ISFLAG_KNOW_PLUSES )) 
               {
                  if (item.base_type==OBJ_ARMOUR) 
                     sprintf(buf,"%d",item.plus);            
                  else
                     sprintf(buf,"%d,%d",item.plus,item.plus2);            
               }      
            else
              {
                 // Now that we can have "glowing elven" weapons, it's 
                 // probably a good idea to cut out the descriptive
                 // term once it's become obsolete. -- bwr                
                 switch (get_equip_desc( item ))
                   {
                      case ISFLAG_RUNED:
                        strcat(buf, "runed");
                        break;
                      case ISFLAG_GLOWING:
                        strcat(buf, "glowing");                                 
                        break;
                   }     
              }   
        } 
      else if (strcmp(parse,"RACE")==0)
        {
            switch (get_equip_race( item ))
                {
                  case ISFLAG_ORCISH:
                     strcat(buf, "orcish");
                     break;
                  case ISFLAG_ELVEN:
                     strcat(buf, "elven");
                     break;
                  case ISFLAG_DWARVEN:
                     strcat(buf, "dwarven ");
                     break;
                }     
        }  
      else if (strcmp(parse,"NAME")==0)
        {
           if (item.quantity>1)
              strcat(buf,TRANSLATE_NAME_OBJS[item.base_type].plural);
           else   
              strcat(buf,TRANSLATE_NAME_OBJS[item.base_type].sing);
        }
      else if (strcmp(parse,"MANUAL")==0)
           strcat(buf,skill_name(item.plus));  	
      else
        {
            clrscr();
            printf("Error parsing item name: Type: %d SubType:%d %s -> %s unknow",item.base_type,item.sub_type,name,parse);
            exit(0);
        }      
      strcat(name,buf);  
 }     

void NameItem(const item_def &item,char *name)
{
   char *cad;
   char *p;
   int n;
   char *str;
   
   if (IsIdent(item))
       str=aObjs[item.base_type][item.sub_type].name;
   else
       str=uniddesc(item.base_type,item.sub_type);
   
   name[0]=0;
   for (;str[0];)
     {
       cad=strstr(str,"$:");
       if (cad==NULL)
         {           
           strcat(name,str);
           return;
         }
       n=cad-str;
       str[n]=0; 
       strcat(name,str);
       str[n]='$';
       cad++;cad++;
       p=cad;
       p=strstr(cad,":$");
       if (p==NULL)
        {
         strcat(name,cad);return;
        }
       n=p-cad;
       cad[n]=0;
       ParseName(name,item,cad);
       cad[n]=':';
       p++;p++;
       str=p;
     }
}     

char item_name( const item_def &item, char descrip, char glag[80] )
{
    const int item_clas = item.base_type;
    const int item_typ = item.sub_type;
    const int it_quant = item.quantity;
    
    char itm_name[60] = "";
    char tmp_quant[5];

    item_name_2( item, itm_name );

    strcpy(glag, "");

    if (descrip == DESC_INVENTORY_EQUIP || descrip == DESC_INVENTORY) 
    {
        if (item.x == -1 && item.y == -1)
        {
            // actually in inventory
            char letter = index_to_letter( item.link );
            glag[0] = letter;
            glag[1] = '\0';

            strcat( glag, " - " );
        }
        else
        {
            // not in inventory
            descrip = DESC_CAP_A;
        }
    }

    if (item_clas == OBJ_ORBS
        || (item_ident( item, ISFLAG_KNOW_TYPE )
            && ((item_clas == OBJ_MISCELLANY
                    && item_typ == MISC_HORN_OF_GERYON)
                || (is_fixed_artefact( item )
                || (is_random_artefact( item ))))))
    {
        // artefacts always get "the" unless we just want the plain name
        switch (descrip)
        {
        case DESC_CAP_A:
        case DESC_CAP_YOUR:
        case DESC_CAP_THE:
            strcat(glag, "The ");
            break;
        case DESC_NOCAP_A:
        case DESC_NOCAP_YOUR:
        case DESC_NOCAP_THE:
        case DESC_NOCAP_ITS:
        case DESC_INVENTORY_EQUIP:
        case DESC_INVENTORY:
            strcat(glag, "the ");
            break;
        default:
        case DESC_PLAIN:
            break;
        }
    }
    else if (it_quant > 1)
    {
        switch (descrip)
        {
        case DESC_CAP_THE:
            strcat(glag, "The ");
            break;
        case DESC_NOCAP_THE:
            strcat(glag, "the ");
            break;
        case DESC_CAP_A:
        case DESC_NOCAP_A:
        case DESC_INVENTORY_EQUIP:
        case DESC_INVENTORY:
            break;
        case DESC_CAP_YOUR:
            strcat(glag, "Your ");
            break;
        case DESC_NOCAP_YOUR:
            strcat(glag, "your ");
            break;
        case DESC_NOCAP_ITS:
            strcat(glag, "its ");
            break;
        case DESC_PLAIN:
        default:
            break;
        }

        itoa(it_quant, tmp_quant, 10);
        strcat(glag, tmp_quant);
        strcat(glag, " ");
    }
    else
    {
        switch (descrip)
        {
        case DESC_CAP_THE:
            strcat(glag, "The ");
            break;
        case DESC_NOCAP_THE:
            strcat(glag, "the ");
            break;
        case DESC_CAP_A:
            strcat(glag, "A");

            if (itm_name[0] == 'a' || itm_name[0] == 'e' || itm_name[0] == 'i'
                || itm_name[0] == 'o' || itm_name[0] == 'u')
            {
                strcat(glag, "n");
            }

            strcat(glag, " ");
            break;              // A/An

        case DESC_NOCAP_A:
        case DESC_INVENTORY_EQUIP:
        case DESC_INVENTORY:
            strcat(glag, "a");

            if (itm_name[0] == 'a' || itm_name[0] == 'e' || itm_name[0] == 'i'
                || itm_name[0] == 'o' || itm_name[0] == 'u')
            {
                strcat(glag, "n");
            }

            strcat(glag, " ");
            break;              // a/an

        case DESC_CAP_YOUR:
            strcat(glag, "Your ");
            break;
        case DESC_NOCAP_YOUR:
            strcat(glag, "your ");
            break;
        case DESC_NOCAP_ITS:
            strcat(glag, "its ");
            break;
        case DESC_PLAIN:
        default:
            break;
        }
    }                           // end of else

    strcat(glag, itm_name);

    if (descrip == DESC_INVENTORY_EQUIP && item.x == -1 && item.y == -1)
    {
        ASSERT( item.link != -1 );

        if (item.link == you.equip[EQ_WEAPON])
        {
            if (you.inv[ you.equip[EQ_WEAPON] ].base_type == OBJ_WEAPONS
                || item_is_staff( you.inv[ you.equip[EQ_WEAPON] ] ))
            {
                strcat( glag, " (weapon)" );
            }
            else
            {
                strcat( glag, " (in hand)" );
            }
        }
        else if (item.link == you.equip[EQ_CLOAK]
                || item.link == you.equip[EQ_HELMET]
                || item.link == you.equip[EQ_GLOVES]
                || item.link == you.equip[EQ_BOOTS]
                || item.link == you.equip[EQ_SHIELD]
                || item.link == you.equip[EQ_BODY_ARMOUR])
        {
            strcat( glag, " (worn)" );
        }
        else if (item.link == you.equip[EQ_LEFT_RING])
        {
            strcat( glag, " (left hand)" );
        }
        else if (item.link == you.equip[EQ_RIGHT_RING])
        {
            strcat( glag, " (right hand)" );
        }
        else if (item.link == you.equip[EQ_AMULET])
        {
            strcat( glag, " (around neck)" );
        }
        //Ax-Crawl slots
        else if (item.link == you.equip[EQ_QUIVER])
        {
            strcat( glag, " (quiver)" );
        } 
    }
    
    //add inscription {alex:29.11.01}
    if (xtraOps.ax_show_inscriptions)
       {
        if (strlen(item.inscription)>0)
           {
             strcat(glag,"{");
             strcat(glag,item.inscription);
             strcat(glag,"}");
            }  
       }     
       
    return (1);
}                               // end item_name()


static char item_name_2( const item_def &item, char glog[80] )
{
    const int item_clas = item.base_type;
    const int item_typ = item.sub_type;
    const int it_plus = item.plus;
    const int item_plus2 = item.plus2;
    const int it_quant = item.quantity;

    char tmp_quant[5];
    char str_pass2[80];
    int  brand;
    unsigned char sparm;

    strcpy(glog, "");
    
    //in AX-Crawl all items can be cursed
    if (item_ident( item, ISFLAG_KNOW_CURSE ))
        {
            // We don't bother printing "uncursed" if the item is identified
            // for pluses (it's state should be obvious), this is so that
            // the weapon name is kept short (there isn't a lot of room
            // for the name on the main screen).  If you're going to change
            // this behaviour, *please* make it so that there is an option
            // that maintains this behaviour. -- bwr
            if (item_cursed( item ))
                strcat(glog, "cursed ");
            else if (Options.show_uncursed 
                    && item_not_ident( item, ISFLAG_KNOW_PLUSES ))
            {
                strcat(glog, "uncursed ");
            }
         }
    
    if (is_random_artefact( item ))
        {
            strcat( glog, randart_name(item) );
            return 1;
        }

    NameItem(item,str_pass2);
    strcat(glog,str_pass2);         
             
    switch(item.base_type)
    {
    case OBJ_MISSILES:        
        brand = get_ammo_brand( item );  
        if (brand == SPMSL_POISONED || brand == SPMSL_POISONED_II )
        {
            strcat(glog, "poisoned ");
        }

        if (item_ident( item, ISFLAG_KNOW_PLUSES ))
        {
            if (it_plus >= 0)
                strcat(glog, "+");

            itoa( it_plus, tmp_quant, 10 );

            strcat(glog, tmp_quant);
            strcat(glog, " ");
        }

        if (get_equip_race( item ))
        {
            int dwpn = get_equip_race( item );

            strcat(glog, (dwpn == ISFLAG_ORCISH) ? "orcish " :
                   (dwpn == ISFLAG_ELVEN) ? "elven " :
                   (dwpn == ISFLAG_DWARVEN) ? "dwarven " : "buggy ");
        }

        strcat(glog, aObjs[OBJ_MISSILES][item_typ].name);
        if (it_quant > 1) strcat(glog, "s");
        
        if (item_ident( item, ISFLAG_KNOW_TYPE ))
          {
            strcat(glog, (brand == SPMSL_NORMAL)           ? "" :
                         (brand == SPMSL_POISONED
                           || brand == SPMSL_POISONED_II)  ? "" :
                         (brand == SPMSL_FLAME)  ? " of flame"  :
                         (brand == SPMSL_ICE)    ? " of ice" :
                         //Ax-Crawl {alex:18.02.02}
                         (brand == SPMSL_SLOW)   ? " of slowing" : 
                         (brand == SPMSL_PARALYSE)   ? " of paralyse" :
                         (brand == SPMSL_ICEBALL)   ? " of iceball" :
                         (brand == SPMSL_FIREBALL)   ? " of fireball" :
                         (brand == SPMSL_CONFUSE)   ? " of confuse" :
                         (brand == SPMSL_BACKLITE)   ? " of backlight" :
                         (brand == SPMSL_SLAYING)   ? " of slaying" :
                                                         " of bugginess");
           }
        break;

    case OBJ_ARMOUR:
        if (item_ident( item, ISFLAG_KNOW_PLUSES ))
        {
            if (it_plus >= 0)
                strcat(glog, "+");

            itoa( it_plus, tmp_quant, 10 );

            strcat(glog, tmp_quant);
            strcat(glog, " ");
        }

        if (item_typ == ARM_GLOVES
            || (item_typ == ARM_BOOTS && item_plus2 == TBOOT_BOOTS))
        {
            strcat(glog, "pair of ");
        }

        if (is_random_artefact( item ))
        {
            strcat(glog, randart_armour_name(item));
            break;
        }

        if (item_typ != ARM_HELMET)
        {
            // Now that we can have "glowing elven" armour, it's 
            // probably a good idea to cut out the descriptive
            // term once it's become obsolete. -- bwr
            if (item_not_ident( item, ISFLAG_KNOW_PLUSES ))
            {
                switch (get_equip_desc( item ))
                {
                case ISFLAG_EMBROIDERED_SHINY:
                    if (item_typ == ARM_ROBE || item_typ == ARM_CLOAK
                        || item_typ == ARM_GLOVES || item_typ == ARM_BOOTS)
                    {
                        strcat(glog, "embroidered ");
                    }
                    else if (item_typ != ARM_LEATHER_ARMOUR)
                        strcat(glog, "shiny ");
                    break;

                case ISFLAG_RUNED:
                    strcat(glog, "runed ");
                    break;
                case ISFLAG_GLOWING:
                    strcat(glog, "glowing ");
                    break;
                }
            }

            // always give racial description (has game effects)
            switch (get_equip_race( item ))
            {
            case ISFLAG_ELVEN:
                strcat(glog, "elven ");
                break;
            case ISFLAG_DWARVEN:
                strcat(glog, "dwarven ");
                break;
            case ISFLAG_ORCISH:
                strcat(glog, "orcish ");
                break;
            }               // end switch
        }

        standard_name_armour( item, str_pass2 );  // in randart.cc
        strcat(glog, str_pass2);

        sparm = get_armour_ego_type( item );

        if (item_ident( item, ISFLAG_KNOW_TYPE ) && sparm != SPARM_NORMAL)
        {
            strcat(glog, " of ");

            strcat(glog, (sparm == SPARM_RUNNING) ? "running" :
                   (sparm == SPARM_FIRE_RESISTANCE) ? "fire resistance" :
                   (sparm == SPARM_COLD_RESISTANCE) ? "cold resistance" :
                   (sparm == SPARM_POISON_RESISTANCE) ? "poison resistance" :
                   (sparm == SPARM_SEE_INVISIBLE) ? "see invisible" :
                   (sparm == SPARM_DARKNESS) ? "darkness" :
                   (sparm == SPARM_STRENGTH) ? "strength" :
                   (sparm == SPARM_DEXTERITY) ? "dexterity" :
                   (sparm == SPARM_INTELLIGENCE) ? "intelligence" :
                   (sparm == SPARM_PONDEROUSNESS) ? "ponderousness" :
                   (sparm == SPARM_LEVITATION) ? "levitation" :
                   (sparm == SPARM_MAGIC_RESISTANCE) ? "magic resistance" :
                   (sparm == SPARM_PROTECTION) ? "protection" :
                   (sparm == SPARM_STEALTH) ? "stealth" :
                   (sparm == SPARM_RESISTANCE) ? "resistance" :
                   (sparm == SPARM_POSITIVE_ENERGY) ? "positive energy" :
                   (sparm == SPARM_ARCHMAGI) ? "the Archmagi" :
                   (sparm == SPARM_PRESERVATION) ? "preservation" :
                   //Ax-Crawl {18.02.02}
                   (sparm == SPARM_REFLECTION) ? "magic beams reflection"
                                                   : "bugginess");
        }
        break;


    // NB: adding another food type == must set for carnivorous chars
    // (Kobolds and mutants)
    case OBJ_FOOD:
       if (item_typ!=FOOD_CHUNK)
         strcat(glog,aObjs[OBJ_FOOD][item_typ].name);          
       else
         { 
            char gmo_n2[40];

            moname( it_plus, true, DESC_PLAIN, gmo_n2 );

            if (item.special < 100)
                strcat(glog, "rotting ");

            strcat(glog, "chunk");

            if (it_quant > 1)
                strcat(glog, "s");

            strcat(glog, " of ");
            strcat(glog, gmo_n2);
            strcat(glog, " flesh");
            break;

          }

        if (item_typ == FOOD_ROYAL_JELLY || item_typ == FOOD_STRAWBERRY
            || item_typ == FOOD_BEEF_JERKY)
            strcat(glog, (it_quant > 1) ? "ie" : "y");
        break;

    // compacted 15 Apr 2000 {dlb}:
    case OBJ_SCROLLS:
        strcat(glog, "scroll");
        strcat(glog, (it_quant == 1) ? " " : "s ");

        if (id[ IDTYPE_SCROLLS ][item_typ] == ID_KNOWN_TYPE
            || item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            strcat(glog, "of ");
            strcat(glog,aObjs[OBJ_SCROLLS][item_typ].name);
        }
        else
        {
            strcat(glog, "labeled ");

            make_name( item.special, it_plus, item_clas, 2, str_pass );
            strcat(glog, str_pass);

            if (id[ IDTYPE_SCROLLS ][item_typ] == ID_TRIED_TYPE)
            {
                strcat( glog, " {tried}" );
            }
        }
        break;

   

    // compacted 15 Apr 2000 {dlb}:
    case OBJ_MISCELLANY:
        switch (item_typ)
        {
        case MISC_RUNE_OF_ZOT:
            strcat( glog, (it_plus == RUNE_DIS)          ? "iron" :
                          (it_plus == RUNE_GEHENNA)      ? "obsidian" :
                          (it_plus == RUNE_COCYTUS)      ? "icy" :
                          (it_plus == RUNE_TARTARUS)     ? "bone" :
                          (it_plus == RUNE_SLIME_PITS)   ? "slimy" :
                          (it_plus == RUNE_VAULTS)       ? "silver" :
                          (it_plus == RUNE_SNAKE_PIT)    ? "serpentine" :
                          (it_plus == RUNE_ELVEN_HALLS)  ? "elven" :
                          (it_plus == RUNE_TOMB)         ? "golden" :
                          (it_plus == RUNE_SWAMP)        ? "decaying" :
 
                          // pandemonium and abyss runes:
                          (it_plus == RUNE_DEMONIC)      ? "demonic" :
                          (it_plus == RUNE_ABYSSAL)      ? "abyssal" :

                          // special pandemonium runes:
                          (it_plus == RUNE_MNOLEG)       ? "glowing" :
                          (it_plus == RUNE_LOM_LOBON)    ? "magical" :
                          (it_plus == RUNE_CEREBOV)      ? "fiery" :
                          (it_plus == RUNE_GLOORX_VLOQ)  ? "dark"
                                                         : "buggy" );

            strcat(glog, " ");
            strcat(glog, "rune");

            if (it_quant > 1)
                strcat(glog, "s");

            if (item_ident( item, ISFLAG_KNOW_TYPE ))
                strcat(glog, " of Zot");
            break;
        default:
            strcat(glog,aObjs[OBJ_MISCELLANY][item_typ].name);
            break;        
        }    
        break;



    // rearranged 15 Apr 2000 {dlb}:
    case OBJ_ORBS:
        strcpy(glog, "Orb of ");
        strcat(glog, (item_typ == ORB_ZOT) ? "Zot" :
/* ******************************************************************
                     (item_typ ==  1)      ? "Zug" :
                     (item_typ ==  2)      ? "Xob" :
                     (item_typ ==  3)      ? "Ix" :
                     (item_typ ==  4)      ? "Xug" :
                     (item_typ ==  5)      ? "Zob" :
                     (item_typ ==  6)      ? "Ik" :
                     (item_typ ==  7)      ? "Grolp" :
                     (item_typ ==  8)      ? "fo brO ehT" :
                     (item_typ ==  9)      ? "Plob" :
                     (item_typ == 10)      ? "Zuggle-Glob" :
                     (item_typ == 11)      ? "Zin" :
                     (item_typ == 12)      ? "Qexigok" :
                     (item_typ == 13)      ? "Bujuk" :
                     (item_typ == 14)      ? "Uhen Tiquritu" :
                     (item_typ == 15)      ? "Idohoxom Sovuf" :
                     (item_typ == 16)      ? "Voc Vocilicoso" :
                     (item_typ == 17)      ? "Chanuaxydiviha" :
                     (item_typ == 18)      ? "Ihexodox" :
                     (item_typ == 19)      ? "Rynok Pol" :
                     (item_typ == 20)      ? "Nemelex" :
                     (item_typ == 21)      ? "Sif Muna" :
                     (item_typ == 22)      ? "Okawaru" :
                     (item_typ == 23)      ? "Kikubaaqudgha" :
****************************************************************** */
               "Bugginess");    // change back to "Zot" if source of problems cannot be found {dlb}
        break;

    case OBJ_GOLD:
        strcat(glog, "gold piece");
        break;

    // still not implemented, yet:
    case OBJ_GEMSTONES:
        break;

    // rearranged 15 Apr 2000 {dlb}:
    case OBJ_CORPSES:
        if (item_typ == CORPSE_BODY && item.special < 100)
            strcat(glog, "rotting ");

        char gmo_n[40];

        moname(it_plus, true, DESC_PLAIN, gmo_n);

        strcat(glog, gmo_n);
        strcat(glog, " ");
        strcat(glog, (item_typ == CORPSE_BODY) ? "corpse" :
               (item_typ == CORPSE_SKELETON) ? "skeleton" : "corpse bug");
        break;

    default:
        strcat(glog, "!");
    }                           // end of switch?

    // debugging output -- oops, I probably block it above ... dang! {dlb}
    if (strlen(glog) < 3)
    {
        char ugug[4];

        strcat(glog, "questionable item (cl:");
        itoa(item_clas, ugug, 10);
        strcat(glog, ugug);
        strcat(glog, ",ty:");
        itoa(item_typ, ugug, 10);
        strcat(glog, ugug);
        strcat(glog, ",pl:");
        itoa(it_plus, ugug, 10);
        strcat(glog, ugug);
        strcat(glog, ",pl2:");
        itoa(item_plus2, ugug, 10);
        strcat(glog, ugug);
        strcat(glog, ",sp:");
        itoa(item.special, ugug, 10);
        strcat(glog, ugug);
        strcat(glog, ",qu:");
        itoa(it_quant, ugug, 10);
        strcat(glog, ugug);
        strcat(glog, ")");
    }

    // hackish {dlb}
    if (it_quant > 1
        && item_clas != OBJ_MISSILES
        && item_clas != OBJ_SCROLLS
        && item_clas != OBJ_POTIONS
        && item_clas != OBJ_MISCELLANY
        && (item_clas != OBJ_FOOD || item_typ != FOOD_CHUNK))
    {
        strcat(glog, "s");
    }

    return 1;
}                               // end item_name_2()

void save_id(char identy[4][50])
{
    char x = 0, jx = 0;

    for (x = 0; x < 4; x++)
    {
        for (jx = 0; jx < 50; jx++)
        {
            identy[x][jx] = id[x][jx];
        }
    }
}                               // end save_id()


void set_ident_type( char cla, char ty, char setting )
{
    int n;	
    // don't allow tried to overwrite known
    if (setting == ID_TRIED_TYPE && get_ident_type( cla, ty ) == ID_KNOWN_TYPE)
        return;
   
    aObjs[cla][ty].id = setting;  
}                               // end set_ident_type()

//Modified by: Alex, 19.11.01
char get_ident_type(char cla, char ty)
{
    char n;    	
    n=aObjs[cla][ty].id;
    //auto-pick and not auto-pick are known type
    if (n>=ID_NO_PICK) 
       return ID_KNOWN_TYPE;
    else
       return n;   
}

int IsIdent(const item_def &item)
  {
     return get_ident_type(item.base_type,item.sub_type) == ID_KNOWN_TYPE || item_ident( item, ISFLAG_KNOW_TYPE );
  }   

//Returns 1 if item can be picked	        
//  
//Alex: 22.11.01
int get_can_pick(char cla,char ty)
  {
   char n;
   int isgood;
   n=get_ident_type(cla,ty);
   //don't autopickup unknown items
   if (n==ID_UNKNOWN_TYPE) return 0;
   isgood=aObjs[cla][ty].isgood;
   
   //don't autopickup if known, option for skip bad item, and the item is bad
   if (!isgood)
        switch(cla)
          { 
          	case OBJ_POTIONS:     	  
          	  if (xtraOps.ax_no_pick_bad_potions)
          	     return 0;
          	case OBJ_SCROLLS:
          	  if (xtraOps.ax_no_pick_bad_scrolls)
          	     return 0;
          	case OBJ_JEWELLERY:
          	  if (xtraOps.ax_no_pick_bad_jewellery)
          	     return 0;           	  
          }	


 //unkown items, know items not marked as no_pick and tried items
 //can be picked     
   return (n!=ID_NO_PICK);
  }	
  
int property( const item_def &item, int prop_type )
{
    switch (item.base_type)
    {
    case OBJ_ARMOUR:
    case OBJ_WEAPONS:
    case OBJ_MISSILES:
        return (prop[ item.base_type ][ item.sub_type ][ prop_type ]);

    case OBJ_STAVES:
        if (item_is_staff( item ))
            return (prop[ OBJ_WEAPONS ][ WPN_QUARTERSTAFF ][ prop_type ]);   
        else if (prop_type == PWPN_SPEED) // item is rod
            return (10);                  // extra protection against speed 0
        else 
            return (0);

    default:
        return (0);
    }
}

int mass_item( const item_def &item )
{
    int unit_mass = 0;

    if (item.base_type == OBJ_GOLD)
    {
        unit_mass = 0;
    }
    else if (item.base_type == OBJ_CORPSES)
    {
        unit_mass = mons_weight( item.plus );

        if (item.sub_type == CORPSE_SKELETON)
            unit_mass /= 2;
    }
    else
    {
        	unit_mass = aObjs[ item.base_type ][ item.sub_type ].mass;
    }

    return (unit_mass > 0 ? unit_mass : 0);
}

// must be certain that you are passing the subtype
// to an OBJ_ARMOUR and nothing else, or as they say,
// "Bad Things Will Happen" {dlb}:
bool hide2armour( unsigned char *which_subtype )
{
    switch (*which_subtype)
    {
    case ARM_DRAGON_HIDE:
        *which_subtype = ARM_DRAGON_ARMOUR;
        return true;
    case ARM_TROLL_HIDE:
        *which_subtype = ARM_TROLL_LEATHER_ARMOUR;
        return true;
    case ARM_ICE_DRAGON_HIDE:
        *which_subtype = ARM_ICE_DRAGON_ARMOUR;
        return true;
    case ARM_MOTTLED_DRAGON_HIDE:
        *which_subtype = ARM_MOTTLED_DRAGON_ARMOUR;
        return true;
    case ARM_STORM_DRAGON_HIDE:
        *which_subtype = ARM_STORM_DRAGON_ARMOUR;
        return true;
    case ARM_GOLD_DRAGON_HIDE:
        *which_subtype = ARM_GOLD_DRAGON_ARMOUR;
        return true;
    case ARM_SWAMP_DRAGON_HIDE:
        *which_subtype = ARM_SWAMP_DRAGON_ARMOUR;
        return true;
    case ARM_STEAM_DRAGON_HIDE:
        *which_subtype = ARM_STEAM_DRAGON_ARMOUR;
        return true;
    default:
        return false;
    }
}                               // end hide2armour()


void make_name(unsigned char var1, unsigned char var2, unsigned char var3,
               char ncase, char str_pass[50])
{
    char name[30] = "";
    char glag[30] = "";
    FixedVector < unsigned char, 15 > numb;
    char len;
    char i = 0;
    char nexty = 0;
    char j = 0;
    char igo = 0;

    int x = 0;

    numb[0] = var1 * var2;
    numb[1] = var1 * var3;
    numb[2] = var2 * var3;
    numb[3] = var1 * var2 * var3;
    numb[4] = var1 + var2;
    numb[5] = var2 + var3;
    numb[6] = var1 * var2 + var3;
    numb[7] = var1 * var3 + var2;
    numb[8] = var2 * var3 + var1;
    numb[9] = var1 * var2 * var3 - var1;
    numb[10] = var1 + var2 + var2;
    numb[11] = var2 + var3 * var1;
    numb[12] = var1 * var2 * var3;
    numb[13] = var1 * var3 * var1;
    numb[14] = var2 * var3 * var3;

    for (i = 0; i < 15; i++)
    {
        while (numb[i] >= 25)
        {
            numb[i] -= 25;
        }
    }

    j = numb[6];

    len = reduce(numb[reduce(numb[11]) / 2]);

    while (len < 5 && j < 10)
    {
        len += 1 + reduce(1 + numb[j]);
        j++;
    }

    while (len > 14)
    {
        len -= 8;
    }

    nexty = retbit(numb[4]);

    char k = 0;

    j = 0;

    for (i = 0; i < len; i++)
    {
        j++;

        if (j >= 15)
        {
            j = 0;

            k++;

            if (k > 9)
                break;
        }

        if (nexty == 1 || (i > 0 && !is_a_vowel(name[i])))
        {
            name[i] = retvow(numb[j]);
            if ((i == 0 || i == len - 1) && name[i] == 32)
            {
                i--;
                continue;
            }
        }
        else
        {
            if (numb[i / 2] <= 1 && i > 3 && is_a_vowel(name[i]))
                goto two_letter;
            else
                name[i] = numb[j];

          hello:
            igo++;
        }

        if ((nexty == 0 && is_a_vowel(name[i]))
            || (nexty == 1 && !is_a_vowel(name[i])))
        {
            if (nexty == 1 && i > 0 && !is_a_vowel(name[i - 1]))
                i--;

            i--;
            continue;
        }

        if (!is_a_vowel(name[i]))
            nexty = 1;
        else
            nexty = 0;

        x++;
    }

    switch (ncase)
    {
    case 2:
        for (i = 0; i < len + 1; i++)
        {
            if (i > 3 && name[i] == 0 && name[i + 1] == 0)
            {
                name[i] = 0;
                if (name[i - 1] == 32)
                    name[i - 1] = 0;
                break;
            }
            if (name[i] != 32 && name[i] < 30)
                name[i] += 65;
            if (name[i] > 96)
                name[i] -= 32;
        }
        break;

    case 3:
        for (i = 0; i < len + 0; i++)
        {
            if (i != 0 && name[i] >= 65 && name[i] < 97)
            {
                if (name[i - 1] == 32)
                    name[i] += 32;
            }

            if (name[i] > 97)
            {
                if (i == 0 || name[i - 1] == 32)
                    name[i] -= 32;
            }

            if (name[i] < 30)
            {
                if (i == 0 || (name[i] != 32 && name[i - 1] == 32))
                    name[i] += 65;
                else
                    name[i] += 97;
            }
        }
        break;

    case 0:
        for (i = 0; i < len; i++)
        {
            if (name[i] != 32 && name[i] < 30)
                name[i] += 97;
        }
        break;

    case 1:
        name[i] += 65;

        for (i = 1; i < len; i++)
        {
            if (name[i] != 32 && name[i] < 30)
                name[i] += 97;  //97;
        }
        break;
    }

    strcpy(glag, name);

    if (strlen(glag) == 0)
        strcpy(glag, "Plog");

    strcpy(str_pass, glag);
    return;

  two_letter:
    if (nexty == 1)
        goto hello;

    if (!is_a_vowel(name[i - 1]))
        goto hello;

    i++;

    switch (i * (retbit(j) + 1))
    {
    case 0:
        strcat(name, "sh");
        break;
    case 1:
        strcat(name, "ch");
        break;
    case 2:
        strcat(name, "tz");
        break;
    case 3:
        strcat(name, "ts");
        break;
    case 4:
        strcat(name, "cs");
        break;
    case 5:
        strcat(name, "cz");
        break;
    case 6:
        strcat(name, "xt");
        break;
    case 7:
        strcat(name, "xk");
        break;
    case 8:
        strcat(name, "kl");
        break;
    case 9:
        strcat(name, "cl");
        break;
    case 10:
        strcat(name, "fr");
        break;
    case 11:
        strcat(name, "sh");
        break;
    case 12:
        strcat(name, "ch");
        break;
    case 13:
        strcat(name, "gh");
        break;
    case 14:
        strcat(name, "pr");
        break;
    case 15:
        strcat(name, "tr");
        break;
    case 16:
        strcat(name, "mn");
        break;
    case 17:
        strcat(name, "ph");
        break;
    case 18:
        strcat(name, "pn");
        break;
    case 19:
        strcat(name, "cv");
        break;
    case 20:
        strcat(name, "zx");
        break;
    case 21:
        strcat(name, "xz");
        break;
    case 23:
        strcat(name, "dd");
        break;
    case 24:
        strcat(name, "tt");
        break;
    case 25:
        strcat(name, "ll");
        break;
        //case 26: strcat(name, "sh"); break;
        //case 12: strcat(name, "sh"); break;
        //case 13: strcat(name, "sh"); break;
    default:
        i--;
        goto hello;
    }

    x += 2;

    goto hello;
}                               // end make_name()


char reduce(unsigned char reducee)
{
    while (reducee >= 26)
    {
        reducee -= 26;
    }

    return reducee;
}                               // end reduce()

bool is_a_vowel(unsigned char let)
{
    return (let == 0 || let == 4 || let == 8 || let == 14 || let == 20
            || let == 24 || let == 32);
}                               // end is_a_vowel()

char retvow(char sed)
{

    while (sed > 6)
        sed -= 6;

    switch (sed)
    {
    case 0:
        return 0;
    case 1:
        return 4;
    case 2:
        return 8;
    case 3:
        return 14;
    case 4:
        return 20;
    case 5:
        return 24;
    case 6:
        return 32;
    }

    return 0;
}                               // end retvow()

char retbit(char sed)
{
    return (sed % 2);
}                               // end retbit()


//------------------------------------------------------------------------------
//Menu system to show knows items
//
//Alex: 19.11.01
//--------------------------------------------------------------------------------
struct tpIdType
  {
    int base_type;
    int sub_type;	
  };	

typedef struct tpIdType TIDType;
TIDType aID[50];
                
                
//highlight on/off for the current item 
//of the left menu (the name of the object class)
int show_item_class(struct tMenu& mnu,int bSelec)
  {
     int n;
     char *cad;

     gotoxy(3,mnu.nFila);     
     if (bSelec==4) 
       cad=NULL;
     else 
        {  
          
          cad=(char *)mnu.data+(mnu.nItem*20);
          
          textcolor(WHITE);
          if (bSelec)
            {
              textbackground(RED);           
            }   
          else
            {
              textbackground(BLACK);
            }                        
         cprintf(cad);
       }
         
     for (n=strlen(cad)+1;n<17;n++) cprintf(" ");
     textbackground(BLACK);
     if (bSelec==1) select_items(mnu.nItem,mnu,1); 
  }  


int show_item_knw(struct tMenu& mnu,int bSelec)
  {
     int n;
     int *cad;
     char *cad2;
     char ch;
        
     gotoxy(24,mnu.nFila);        
     for (n=25;n<=79;n++) cprintf(" ");    
     
     if (bSelec!=4) 
        {   
          //highlight on/off for the current item of the menu
          cad2=(char *)mnu.data+(mnu.nItem*51);
          gotoxy(24,mnu.nFila);
          if (bSelec)
            {
               textcolor(WHITE);cprintf("*");
               textcolor(CYAN);           
            }   
          else
            {
               textcolor(WHITE);cprintf(" ");
            }    
          cprintf(cad2);
          gotoxy(72,mnu.nFila);
          if (bSelec)           
             textcolor(YELLOW);
          else 
             textcolor(WHITE);
          
         
          if (get_can_pick(aID[mnu.nItem].base_type,aID[mnu.nItem].sub_type))
             cprintf("Y");
          else      
             cprintf("N"); 
          gotoxy(72,mnu.nFila);   
        } 
               
  }   	


int choose_item_key(struct tMenu& mnu,int c)
 {
   
   switch(c)
     {
       case 27:
          return -1;
       case 'a':
       case 'A':
          return 0;
       case 'W':
       case 'w':
          return 1;
       case 's':
       case 'S':
          return 2;
       case 'J':
       case 'j':
          return 3;
       case 'P':
       case 'p':
          return 4;              
       case 75:
          if (bKeySpecial==1) return -1;
          return -2;      
       case 'Y':
       case 'y':          
          set_ident_type(aID[mnu.nItem].base_type,aID[mnu.nItem].sub_type,ID_PICK);
          show_item_knw(mnu,1);
          return -2;
       case 'N':
       case 'n':
          //gotoxy(1,1);          
          set_ident_type(aID[mnu.nItem].base_type,aID[mnu.nItem].sub_type,ID_NO_PICK);
          //cprintf("%d %d %d %d %d %d",
          //   mnu.nItem,
          //   id[aID[mnu.nItem].base_type][aID[mnu.nItem].sub_type],
          //   aID[mnu.nItem].base_type,aID[mnu.nItem].sub_type,
          //   get_can_pick(aID[mnu.nItem].base_type,aID[mnu.nItem].sub_type),
          //   get_id(aID[mnu.nItem].base_type,aID[mnu.nItem].sub_type));
          
          show_item_knw(mnu,1);
          return -2;
       default:
          
          return -2;
     }	
   return -2;  		
 } 	

 
//show the items of a the active object class  
void select_items(int opci,struct tMenu& mnu_i,int bOnlyShow=0)
  {
    int n,nIni,nFin,m,max;
    int ft;
    struct tMenu mnu;   
    char data[221][51];
    
    mnu.data=(char *)data;
    mnu.data2=NULL;
    
    // item_name now requires a "real" item, so we'll create a tmp
    item_def tmp = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 };    
    
      {
          mnu.nMaxItems=-1;
          if (opci==0)
            {
              nIni=0;
              nFin=3;
            }  
          else
            {
              nIni=opci-1;
              nFin=opci-1;
            }    
          for (m=nIni;m<=nFin;m++)
           {
           	switch (m)
                  { 
                    case IDTYPE_WANDS:
                       ft = OBJ_WANDS;
                       max = NUM_WANDS;
                       break;
                    case IDTYPE_SCROLLS:
                       ft = OBJ_SCROLLS;
                       max = NUM_SCROLLS;
                       break;
                    case IDTYPE_JEWELLERY:
                       ft = OBJ_JEWELLERY;
                       max = NUM_JEWELLERY;
                       break;
                    case IDTYPE_POTIONS:
                       ft = OBJ_POTIONS;
                       max = NUM_POTIONS;
                       break;
      
             }
             
             tmp.base_type=ft;  
             for (n=0;n<max;n++)
               {               	  
                  if (get_ident_type(ft,n)==ID_KNOWN_TYPE)                   
                     {                     	                     
                        tmp.sub_type=n;                    
                        item_name( tmp, DESC_PLAIN, str_pass );
                        
                        mnu.nMaxItems++;
                        aID[mnu.nMaxItems].base_type=ft;
                        aID[mnu.nMaxItems].sub_type=n;
                	      strncpy(data[mnu.nMaxItems],str_pass,50);                	
                	//sprintf(data[mnu.nMaxItems],"%d %d %d %d %d",m,n,&id[m][n]-&id[0][0],data2[mnu.nMaxItems],&data2);
                      }	
               }	 	
           }          
           
          
          int nItem;
          mnu.nFilaMax=23;
          mnu.nFilaIni=5;
          mnu.print_item=show_item_knw;
          mnu.handle_key=choose_item_key;    
          ClearScreen(24,5,79,23);
          if (bOnlyShow) 
             {  
                nItem=mnu_i.nItem;
                show_items(mnu);                
             }
          else
             {
                nItem=menu(mnu);
             }       
          switch(nItem)
            {
               case -1:
                  return;
               case -2:
                  break;
               default:   
                  show_item_class(mnu_i,2);
                  opci=nItem;
                  mnu_i.nItem=opci;  
                  mnu_i.nFila=mnu_i.nFilaIni+opci;
                  show_item_class(mnu_i,3);      
                  break;
            }    
      }
  } 
  


 
int choose_class_item_key(struct tMenu& mnu,int c)
 {
   int n;
   switch(c)
     {
       case 27:
          return 1;
       case 13:
          select_items(mnu.nItem,mnu);   
          return -2;
       case 'a':
       case 'A':
          n=0;break;
       case 'W':
       case 'w':
          n=1;break;
       case 's':
       case 'S':
          n=2;break;
       case 'J':
       case 'j':
          n=3;break;
       case 'P':
       case 'p':
          n=4;break;
       case 77:
          if (bKeySpecial==1) 
             {
             	select_items(mnu.nItem,mnu);
             } 	
          return -2;   
       default:
          return -2;
     }
   select_items(n,mnu,1);     	
   return -2;  		
 } 	   	  
  
unsigned char check_item_knowledge(void)
{
    char st_pass[60];
    unsigned char i,j,inv_count;

#ifdef DOS_TERM
    char buffer[4800];

    window(1, 1, 80, 25);
    gettext(1, 1, 80, 25, buffer);
#endif


    strcpy(st_pass, "");   

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 30; j++)
        {
            if (id[i][j])
                inv_count++;
        }
    }

    if (inv_count == 0)
    {
        mpr("You don't recognise anything yet!");
        if (getch() == 0)
            getch();
        return 27;
    }
    
    clrscr();
    struct tMenu mnu;
    char data[][20]=
      {
        "A - All",                      
        "W - Wands",
        "S - Scrolls",
        "J - Jewellery",
        "P - Potions"  
      } ;
    int nItem;
    int opcion=0;
    mnu.nFilaMax=23;
    mnu.nFilaIni=7;
    mnu.print_item=show_item_class;
    mnu.handle_key=choose_class_item_key;
    mnu.data=(char *)data;
    mnu.data2=(char *)opcion;  
    mnu.nMaxItems=4;
    
    
        
    
        textbackground(BLACK);
        textcolor(WHITE);    
        clrscr();
        Box(1,1,21,3);
        Box(23,1,80,3);
        textcolor(YELLOW);
        gotoxy(3,2);cprintf("Types of Objects");
        
        textcolor(WHITE);
        gotoxy(40,2);cprintf("Name");
        gotoxy(72,2);cprintf("A.Pick");
        textbackground(BLACK);
        
        textcolor(WHITE);       
        Box(1,4,21,24);
        Box(23,4,80,24);
         
        nItem=menu(mnu);
        textcolor(WHITE);
        textbackground(BLACK);

    redraw_screen();

#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif
}
