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
        
    switch (item_clas)
    {
    case OBJ_WEAPONS:        

        if (item_ident( item, ISFLAG_KNOW_PLUSES ))
        {
            if (it_plus == 0 && item_plus2 == 0)
            {
                strcat(glog, "+0 ");
            }
            else
            {
                if (it_plus >= 0)
                    strcat( glog, "+" );

                itoa( it_plus, tmp_quant, 10 );

                strcat( glog, tmp_quant );
                strcat( glog, "," );

                if (item_plus2 >= 0)
                    strcat(glog, "+");

                itoa( item_plus2, tmp_quant, 10 );

                strcat( glog, tmp_quant );
                strcat( glog, " " );
            }
        }

        if (is_random_artefact( item ))
        {
            strcat( glog, randart_name(item) );
            break;
        }

        if (is_fixed_artefact( item ))
        {
            if (item_ident( item, ISFLAG_KNOW_TYPE ))
            {
                strcat(glog,
                       (item.special == SPWPN_SINGING_SWORD) ? "Singing Sword" :
                       (item.special == SPWPN_WRATH_OF_TROG) ? "Wrath of Trog" :
                       (item.special == SPWPN_SCYTHE_OF_CURSES) ? "Scythe of Curses" :
                       (item.special == SPWPN_MACE_OF_VARIABILITY) ? "Mace of Variability" :
                       (item.special == SPWPN_GLAIVE_OF_PRUNE) ? "Glaive of Prune" :
                       (item.special == SPWPN_SCEPTRE_OF_TORMENT) ? "Sceptre of Torment" :
                       (item.special == SPWPN_SWORD_OF_ZONGULDROK) ? "Sword of Zonguldrok" :
                       (item.special == SPWPN_SWORD_OF_CEREBOV) ? "Sword of Cerebov" :
                       (item.special == SPWPN_STAFF_OF_DISPATER) ? "Staff of Dispater" :
                       (item.special == SPWPN_SCEPTRE_OF_ASMODEUS) ? "Sceptre of Asmodeus" :
                       (item.special == SPWPN_SWORD_OF_POWER) ? "Sword of Power" :
                       (item.special == SPWPN_KNIFE_OF_ACCURACY) ? "Knife of Accuracy" :
                       (item.special == SPWPN_STAFF_OF_OLGREB) ? "Staff of Olgreb" :
                       (item.special == SPWPN_VAMPIRES_TOOTH) ? "Vampire's Tooth" :
                       (item.special == SPWPN_STAFF_OF_WUCAD_MU) ? "Staff of Wucad Mu"
                                                   : "Brodale's Buggy Bola");
            }
            else
            {
                strcat(glog,
                       (item.special == SPWPN_SINGING_SWORD) ? "golden long sword" :
                       (item.special == SPWPN_WRATH_OF_TROG) ? "bloodstained battleaxe" :
                       (item.special == SPWPN_SCYTHE_OF_CURSES) ? "warped scythe" :
                       (item.special == SPWPN_MACE_OF_VARIABILITY) ? "shimmering mace" :
                       (item.special == SPWPN_GLAIVE_OF_PRUNE) ? "purple glaive" :
                       (item.special == SPWPN_SCEPTRE_OF_TORMENT) ? "jeweled golden mace" :
                       (item.special == SPWPN_SWORD_OF_ZONGULDROK) ? "bone long sword" :
                       (item.special == SPWPN_SWORD_OF_CEREBOV) ? "great serpentine sword" :
                       (item.special == SPWPN_STAFF_OF_DISPATER) ? "golden staff" :
                       (item.special == SPWPN_SCEPTRE_OF_ASMODEUS) ? "ruby sceptre" :
                       (item.special == SPWPN_SWORD_OF_POWER) ? "chunky great sword" :
                       (item.special == SPWPN_KNIFE_OF_ACCURACY) ? "thin dagger" :
                       (item.special == SPWPN_STAFF_OF_OLGREB) ? "green glowing staff" :
                       (item.special == SPWPN_VAMPIRES_TOOTH) ? "ivory dagger" :
                       (item.special == SPWPN_STAFF_OF_WUCAD_MU) ? "quarterstaff"
                                                           : "bola");
            }
            break;
        }

        // Now that we can have "glowing elven" weapons, it's 
        // probably a good idea to cut out the descriptive
        // term once it's become obsolete. -- bwr
        if (item_not_ident( item, ISFLAG_KNOW_PLUSES ))
        {
            switch (get_equip_desc( item ))
            {
            case ISFLAG_RUNED:
                strcat(glog, "runed ");
                break;
            case ISFLAG_GLOWING:
                strcat(glog, "glowing ");
                break;
            }
        } 

        // always give racial type (it does have game effects)
        switch (get_equip_race( item ))
        {
        case ISFLAG_ORCISH:
            strcat(glog, "orcish ");
            break;
        case ISFLAG_ELVEN:
            strcat(glog, "elven ");
            break;
        case ISFLAG_DWARVEN:
            strcat(glog, "dwarven ");
            break;
        }

        brand = get_weapon_brand( item );

        if (item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            if (brand == SPWPN_VAMPIRICISM)
                strcat(glog, "vampiric ");
        }                       // end if

        standard_name_weap(item_typ, str_pass2);
        
        strcat(glog, str_pass2);

        if (item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            switch (brand)
            {
            case SPWPN_NORMAL:
                break;
            case SPWPN_FLAMING:
                strcat(glog, " of flaming");
                break;
            case SPWPN_FREEZING:
                strcat(glog, " of freezing");
                break;
            case SPWPN_HOLY_WRATH:
                strcat(glog, " of holy wrath");
                break;
            case SPWPN_ELECTROCUTION:
                strcat(glog, " of electrocution");
                break;
            case SPWPN_ORC_SLAYING:
                strcat(glog, " of orc slaying");
                break;
            case SPWPN_VENOM:
                strcat(glog, " of venom");
                break;
            case SPWPN_PROTECTION:
                strcat(glog, " of protection");
                break;
            case SPWPN_DRAINING:
                strcat(glog, " of draining");
                break;
            case SPWPN_SPEED:
                strcat(glog, " of speed");
                break;
            case SPWPN_VORPAL:
                switch (damage_type(item_clas, item_typ))
                {
                case DVORP_CRUSHING:
                    strcat(glog, " of crushing");
                    break;
                case DVORP_SLICING:
                    strcat(glog, " of slicing");
                    break;
                case DVORP_PIERCING:
                    strcat(glog, " of piercing");
                    break;
                case DVORP_CHOPPING:
                    strcat(glog, " of chopping");
                    break;
                }
                break;

            case SPWPN_FLAME:
                strcat(glog, " of flame");
                break;          // bows/xbows

            case SPWPN_FROST:
                strcat(glog, " of frost");
                break;          // bows/xbows
                /* 13 - vamp */
            case SPWPN_DISRUPTION:
                strcat(glog, " of disruption");
                break;
            case SPWPN_PAIN:
                strcat(glog, " of pain");
                break;
            case SPWPN_DISTORTION:
                strcat(glog, " of distortion");
                break;

            case SPWPN_REACHING:
                strcat(glog, " of reaching");
                break;

                /* 25 - 29 are randarts */
                
            //AX-Crawl {25.02.02}
            case SPWPN_GOBLIN_SLAYING:
                strcat(glog, " of goblin slaying");
                break;         
            }
        }
        break;

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

    // compacted 15 Apr 2000 {dlb}:
    case OBJ_WANDS:
        if (id[ IDTYPE_WANDS ][item_typ] == ID_KNOWN_TYPE
            || item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            strcat(glog, "wand");
            if (it_quant > 1) strcat(glog, "s");            
            strcat(glog," of ");
            strcat(glog,aObjs[OBJ_WANDS][item_typ].name);
        }
        else
        {
            char primary = (item.special % 12);
            char secondary = (item.special / 12);

            strcat(glog, (secondary == 0) ? "" :        // hope this works {dlb}
                   (secondary == 1) ? "jeweled" :
                   (secondary == 2) ? "curved" :
                   (secondary == 3) ? "long" :
                   (secondary == 4) ? "short" :
                   (secondary == 5) ? "twisted" :
                   (secondary == 6) ? "crooked" :
                   (secondary == 7) ? "forked" :
                   (secondary == 8) ? "shiny" :
                   (secondary == 9) ? "blackened" :
                   (secondary == 10) ? "tapered" :
                   (secondary == 11) ? "glowing" :
                   (secondary == 12) ? "worn" :
                   (secondary == 13) ? "encrusted" :
                   (secondary == 14) ? "runed" :
                   (secondary == 15) ? "sharpened" : "buggily");

            if (secondary != 0)
                strcat(glog, " ");

            strcat(glog, (primary == 0) ? "iron" :
                   (primary == 1) ? "brass" :
                   (primary == 2) ? "bone" :
                   (primary == 3) ? "wooden" :
                   (primary == 4) ? "copper" :
                   (primary == 5) ? "gold" :
                   (primary == 6) ? "silver" :
                   (primary == 7) ? "bronze" :
                   (primary == 8) ? "ivory" :
                   (primary == 9) ? "glass" :
                   (primary == 10) ? "lead" :
                   (primary == 11) ? "plastic" : "buggy");

            strcat(glog, " wand");

            if (id[ IDTYPE_WANDS ][item_typ] == ID_TRIED_TYPE)
            {
                strcat( glog, " {tried}" );
            }
        }

        if (item_ident( item, ISFLAG_KNOW_PLUSES ))
        {
            strcat(glog, " (");
            itoa( it_plus, tmp_quant, 10 );
            strcat(glog, tmp_quant);
            strcat(glog, ")");
        }
        break;

    // compacted 15 Apr 2000 {dlb}:
    case OBJ_POTIONS:
        if (id[ IDTYPE_POTIONS ][item_typ] == ID_KNOWN_TYPE
            || item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            strcat(glog, "potion");
            strcat(glog, (it_quant == 1) ? " " : "s ");
            strcat(glog, "of ");
            strcat(glog,aObjs[OBJ_POTIONS][item_typ].name);
        }
        else
        {
            char primary = item.special / 14;
            char secondary = item.special % 14;

            strcat(glog, 
                   (primary ==  0) ? "" :
                   (primary ==  1) ? "bubbling " :
                   (primary ==  2) ? "lumpy " :
                   (primary ==  3) ? "fuming " :
                   (primary ==  4) ? "smoky " :
                   (primary ==  5) ? "fizzy " :
                   (primary ==  6) ? "glowing " :
                   (primary ==  7) ? "sedimented " :
                   (primary ==  8) ? "metallic " :
                   (primary ==  9) ? "murky " :
                   (primary == 10) ? "gluggy " :
                   (primary == 11) ? "viscous " :
                   (primary == 12) ? "oily " :
                   (primary == 13) ? "slimy " :
                   (primary == 14) ? "emulsified " : "buggy ");

            strcat(glog,
                   (secondary ==  0) ? "clear" :
                   (secondary ==  1) ? "blue" :
                   (secondary ==  2) ? "black" :
                   (secondary ==  3) ? "silvery" :
                   (secondary ==  4) ? "cyan" :
                   (secondary ==  5) ? "purple" :
                   (secondary ==  6) ? "orange" :
                   (secondary ==  7) ? "inky" :
                   (secondary ==  8) ? "red" :
                   (secondary ==  9) ? "yellow" :
                   (secondary == 10) ? "green" :
                   (secondary == 11) ? "brown" :
                   (secondary == 12) ? "pink" :
                   (secondary == 13) ? "white" : "buggy");

            strcat(glog, " potion");

            if (it_quant > 1) strcat(glog, "s");

            if (id[ IDTYPE_POTIONS ][item_typ] == ID_TRIED_TYPE)
            {
                strcat( glog, " {tried}" );
            }
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
            char str_pass[50];

            make_name( item.special, it_plus, item_clas, 2, str_pass );
            strcat(glog, str_pass);

            if (id[ IDTYPE_SCROLLS ][item_typ] == ID_TRIED_TYPE)
            {
                strcat( glog, " {tried}" );
            }
        }
        break;

    // compacted 15 Apr 2000 {dlb}: -- on hold ... what a mess!
    case OBJ_JEWELLERY:
        // not using {tried} here because there are some confusing 
        // issues to work out with how we want to handle jewellery 
        // artefacts and base type id. -- bwr

        if (item_ident( item, ISFLAG_KNOW_TYPE ) && is_random_artefact( item ))
        {
            strcat(glog, randart_ring_name(item));
            break;
        }

        if (id[ IDTYPE_JEWELLERY ][item_typ] == ID_KNOWN_TYPE
            || item_ident( item, ISFLAG_KNOW_TYPE ))
        {

            if (item_ident( item, ISFLAG_KNOW_PLUSES )
                && (item_typ == RING_PROTECTION || item_typ == RING_STRENGTH
                    || item_typ == RING_SLAYING || item_typ == RING_EVASION
                    || item_typ == RING_DEXTERITY
                    || item_typ == RING_INTELLIGENCE))
            {
                char gokh = it_plus;

                if (gokh >= 0)
                    strcat( glog, "+" );

                itoa( gokh, tmp_quant, 10 );
                strcat( glog, tmp_quant );

                if (item_typ == RING_SLAYING)
                {
                    strcat( glog, "," );

                    if (item_plus2 >= 0)
                        strcat(glog, "+");

                    itoa( item_plus2, tmp_quant, 10 );
                    strcat( glog, tmp_quant );
                }

                strcat(glog, " ");
            }
            
          strcat(glog,aObjs[OBJ_JEWELLERY][item_typ].name);
          
          break;
        }

        if (item_typ < AMU_RAGE)        // rings
        {
            if (is_random_artefact( item ))
            {
                strcat(glog, randart_ring_name(item));
                break;
            }

            switch (item.special / 13)       // secondary characteristic of ring
            {
            case 1:
                strcat(glog, "encrusted ");
                break;
            case 2:
                strcat(glog, "glowing ");
                break;
            case 3:
                strcat(glog, "tubular ");
                break;
            case 4:
                strcat(glog, "runed ");
                break;
            case 5:
                strcat(glog, "blackened ");
                break;
            case 6:
                strcat(glog, "scratched ");
                break;
            case 7:
                strcat(glog, "small ");
                break;
            case 8:
                strcat(glog, "large ");
                break;
            case 9:
                strcat(glog, "twisted ");
                break;
            case 10:
                strcat(glog, "shiny ");
                break;
            case 11:
                strcat(glog, "notched ");
                break;
            case 12:
                strcat(glog, "knobbly ");
                break;
            }

            switch (item.special % 13)
            {
            case 0:
                strcat(glog, "wooden ring");
                break;
            case 1:
                strcat(glog, "silver ring");
                break;
            case 2:
                strcat(glog, "golden ring");
                break;
            case 3:
                strcat(glog, "iron ring");
                break;
            case 4:
                strcat(glog, "steel ring");
                break;
            case 5:
                strcat(glog, "bronze ring");
                break;
            case 6:
                strcat(glog, "brass ring");
                break;
            case 7:
                strcat(glog, "copper ring");
                break;
            case 8:
                strcat(glog, "granite ring");
                break;
            case 9:
                strcat(glog, "ivory ring");
                break;
            case 10:
                strcat(glog, "bone ring");
                break;
            case 11:
                strcat(glog, "marble ring");
                break;
            case 12:
                strcat(glog, "jade ring");
                break;
            case 13:
                strcat(glog, "glass ring");
                break;
            }
        }                       // end of rings
        else                    // ie is an amulet
        {
            if (is_random_artefact( item ))
            {
                strcat(glog, randart_ring_name(item));
                break;
            }

            if (item.special > 13)
            {
                switch (item.special / 13)   // secondary characteristic of amulet
                {
                case 0:
                    strcat(glog, "dented ");
                    break;
                case 1:
                    strcat(glog, "square ");
                    break;
                case 2:
                    strcat(glog, "thick ");
                    break;
                case 3:
                    strcat(glog, "thin ");
                    break;
                case 4:
                    strcat(glog, "runed ");
                    break;
                case 5:
                    strcat(glog, "blackened ");
                    break;
                case 6:
                    strcat(glog, "glowing ");
                    break;
                case 7:
                    strcat(glog, "small ");
                    break;
                case 8:
                    strcat(glog, "large ");
                    break;
                case 9:
                    strcat(glog, "twisted ");
                    break;
                case 10:
                    strcat(glog, "tiny ");
                    break;
                case 11:
                    strcat(glog, "triangular ");
                    break;
                case 12:
                    strcat(glog, "lumpy ");
                    break;
                }
            }

            switch (item.special % 13)
            {
            case 0:
                strcat(glog, "zirconium amulet");
                break;
            case 1:
                strcat(glog, "sapphire amulet");
                break;
            case 2:
                strcat(glog, "golden amulet");
                break;
            case 3:
                strcat(glog, "emerald amulet");
                break;
            case 4:
                strcat(glog, "garnet amulet");
                break;
            case 5:
                strcat(glog, "bronze amulet");
                break;
            case 6:
                strcat(glog, "brass amulet");
                break;
            case 7:
                strcat(glog, "copper amulet");
                break;
            case 8:
                strcat(glog, "ruby amulet");
                break;
            case 9:
                strcat(glog, "ivory amulet");
                break;
            case 10:
                strcat(glog, "bone amulet");
                break;
            case 11:
                strcat(glog, "platinum amulet");
                break;
            case 12:
                strcat(glog, "jade amulet");
                break;
            case 13:
                strcat(glog, "plastic amulet");
                break;
            }
        }                       // end of amulets
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

    // compacted 15 Apr 2000 {dlb}:
    case OBJ_BOOKS:
        if (item_not_ident( item, ISFLAG_KNOW_TYPE ))
        {
            char primary = (item.special / 10);
            char secondary = (item.special % 10);

            strcat(glog, (primary == 0) ? "" :
                   (primary == 1) ? "chunky " :
                   (primary == 2) ? "thick " :
                   (primary == 3) ? "thin " :
                   (primary == 4) ? "wide " :
                   (primary == 5) ? "glowing " :
                   (primary == 6) ? "dog-eared " :
                   (primary == 7) ? "oblong " :
                   (primary == 8) ? "runed " :

                   // these last three were single spaces {dlb}
                   (primary == 9) ? "" :
                   (primary == 10) ? "" : (primary == 11) ? "" : "buggily ");

            strcat(glog, (secondary == 0) ? "paperback " :
                   (secondary == 1) ? "hardcover " :
                   (secondary == 2) ? "leatherbound " :
                   (secondary == 3) ? "metal-bound " :
                   (secondary == 4) ? "papyrus " :
                   // these two were single spaces, too {dlb}
                   (secondary == 5) ? "" :
                   (secondary == 6) ? "" : "buggy ");

            strcat(glog, "book");
        }
        else if (item_typ == BOOK_MANUAL)
        {
            strcat(glog, "manual of ");
            strcat(glog, skill_name(it_plus));
        }
        else if (item_typ == BOOK_NECRONOMICON)
            strcat(glog, "Necronomicon");
        else if (item_typ == BOOK_DESTRUCTION)
            strcat(glog, "tome of Destruction");
        else if (item_typ == BOOK_YOUNG_POISONERS)
            strcat(glog, "Young Poisoner's Handbook");
        else if (item_typ == BOOK_BEASTS)
            strcat(glog, "Monster Manual");
        else
        {
            strcat(glog, "book of ");
            strcat(glog, (item_typ == BOOK_MINOR_MAGIC_I
                          || item_typ == BOOK_MINOR_MAGIC_II
                          || item_typ == BOOK_MINOR_MAGIC_III) ? "Minor Magic" :
                          (item_typ == BOOK_CONJURATIONS_I
                          || item_typ == BOOK_CONJURATIONS_II) ? "Conjurations" :
                          (item_typ == BOOK_FLAMES) ? "Flames" :
                          (item_typ == BOOK_FROST) ? "Frost" :
                          (item_typ == BOOK_SUMMONINGS) ? "Summonings" :
                          (item_typ == BOOK_FIRE) ? "Fire" :
                          (item_typ == BOOK_ICE) ? "Ice" :
                          (item_typ == BOOK_SURVEYANCES) ? "Surveyances" :
                          (item_typ == BOOK_SPATIAL_TRANSLOCATIONS) ? "Spatial Translocations" :
                          (item_typ == BOOK_ENCHANTMENTS) ? "Enchantments" :
                          (item_typ == BOOK_TEMPESTS) ? "the Tempests" :
                          (item_typ == BOOK_DEATH) ? "Death" :
                          (item_typ == BOOK_HINDERANCE) ? "Hinderance" :
                          (item_typ == BOOK_CHANGES) ? "Changes" :
                          (item_typ == BOOK_TRANSFIGURATIONS) ? "Transfigurations" :
                          (item_typ == BOOK_PRACTICAL_MAGIC) ? "Practical Magic" :
                          (item_typ == BOOK_WAR_CHANTS) ? "War Chants" :
                          (item_typ == BOOK_CLOUDS) ? "Clouds" :
                          (item_typ == BOOK_HEALING) ? "Healing" :
                          (item_typ == BOOK_NECROMANCY) ? "Necromancy" :
                          (item_typ == BOOK_CALLINGS) ? "Callings" :
                          (item_typ == BOOK_CHARMS) ? "Charms" :
                          (item_typ == BOOK_DEMONOLOGY) ? "Demonology" :
                          (item_typ == BOOK_AIR) ? "Air" :
                          (item_typ == BOOK_SKY) ? "the Sky" :
                          (item_typ == BOOK_DIVINATIONS) ? "Divinations" :
                          (item_typ == BOOK_WARP) ? "the Warp" :
                          (item_typ == BOOK_ENVENOMATIONS) ? "Envenomations" :
                          (item_typ == BOOK_ANNIHILATIONS) ? "Annihilations" :
                          (item_typ == BOOK_UNLIFE) ? "Unlife" :
                          (item_typ == BOOK_CONTROL) ? "Control" :
                          (item_typ == BOOK_MUTATIONS) ? "Morphology" :
                          (item_typ == BOOK_TUKIMA) ? "Tukima" :
                          (item_typ == BOOK_GEOMANCY) ? "Geomancy" :
                          (item_typ == BOOK_EARTH) ? "the Earth" :
                          (item_typ == BOOK_WIZARDRY) ? "Wizardry" :
                          (item_typ == BOOK_POWER) ? "Power" :
                          (item_typ == BOOK_CANTRIPS) ? "Cantrips" :
                          (item_typ == BOOK_PARTY_TRICKS) ? "Party Tricks" :
                          (item_typ == BOOK_STALKING) ? "Stalking"
                                                           : "Bugginess");
        }
        break;

    // compacted 15 Apr 2000 {dlb}:
    case OBJ_STAVES:
        if (item_not_ident( item, ISFLAG_KNOW_TYPE ))
        {
            strcat(glog, (item.special == 0) ? "curved" :
                   (item.special == 1) ? "glowing" :
                   (item.special == 2) ? "thick" :
                   (item.special == 3) ? "thin" :
                   (item.special == 4) ? "long" :
                   (item.special == 5) ? "twisted" :
                   (item.special == 6) ? "jeweled" :
                   (item.special == 7) ? "runed" :
                   (item.special == 8) ? "smoking" :
                   (item.special == 9) ? "gnarled" :    // was "" {dlb}
                   (item.special == 10) ? "" :
                   (item.special == 11) ? "" :
                   (item.special == 12) ? "" :
                   (item.special == 13) ? "" :
                   (item.special == 14) ? "" :
                   (item.special == 15) ? "" :
                   (item.special == 16) ? "" :
                   (item.special == 17) ? "" :
                   (item.special == 18) ? "" :
                   (item.special == 19) ? "" :
                   (item.special == 20) ? "" :
                   (item.special == 21) ? "" :
                   (item.special == 22) ? "" :
                   (item.special == 23) ? "" :
                   (item.special == 24) ? "" :
                   (item.special == 25) ? "" :
                   (item.special == 26) ? "" :
                   (item.special == 27) ? "" :
                   (item.special == 28) ? "" :
                   (item.special == 29) ? "" : "buggy");
            strcat(glog, " ");
        }

        strcat( glog, (item_is_rod( item ) ? "rod" : "staff") );

        if (item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            strcat(glog, " of ");

            strcat(glog, (item_typ == STAFF_WIZARDRY) ? "wizardry" :
                   (item_typ == STAFF_POWER) ? "power" :
                   (item_typ == STAFF_FIRE) ? "fire" :
                   (item_typ == STAFF_COLD) ? "cold" :
                   (item_typ == STAFF_POISON) ? "poison" :
                   (item_typ == STAFF_ENERGY) ? "energy" :
                   (item_typ == STAFF_DEATH) ? "death" :
                   (item_typ == STAFF_CONJURATION) ? "conjuration" :
                   (item_typ == STAFF_ENCHANTMENT) ? "enchantment" :
                   (item_typ == STAFF_SMITING) ? "smiting" :
                   (item_typ == STAFF_STRIKING) ? "striking" :
                   (item_typ == STAFF_WARDING) ? "warding" :
                   (item_typ == STAFF_DISCOVERY) ? "discovery" :
                   (item_typ == STAFF_DEMONOLOGY) ? "demonology" :
                   (item_typ == STAFF_AIR) ? "air" :
                   (item_typ == STAFF_EARTH) ? "earth" :
                   (item_typ == STAFF_SUMMONING
                    || item_typ == STAFF_SPELL_SUMMONING) ? "summoning" :
                   (item_typ == STAFF_DESTRUCTION_I
                    || item_typ == STAFF_DESTRUCTION_II
                    || item_typ == STAFF_DESTRUCTION_III
                    || item_typ == STAFF_DESTRUCTION_IV) ? "destruction" :
                   (item_typ == STAFF_CHANNELING) ? "channeling"
                   : "bugginess");
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

void clear_ids(void)
{

    char i = 0, j = 0;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 50; j++)
        {
            id[i][j] = ID_UNKNOWN_TYPE;
        }
    }

}                               // end clear_ids()


void set_ident_type( char cla, char ty, char setting )
{
    int n;	
    // don't allow tried to overwrite known
    if (setting == ID_TRIED_TYPE && get_ident_type( cla, ty ) == ID_KNOWN_TYPE)
        return;

    switch (cla)
    {
    case OBJ_WANDS:
        n=IDTYPE_WANDS;
        break;

    case OBJ_SCROLLS:
        n=IDTYPE_SCROLLS;
        break;

    case OBJ_JEWELLERY:
        n=IDTYPE_JEWELLERY;
        break;

    case OBJ_POTIONS:
        n=IDTYPE_POTIONS;
        break;

    default:
        return;
    }
        
  //if item identified first time, mark as autopick  
  //Then, the item will be picked if their class is autopicked.
  //If don't do this, potions can be picked, but the subtype of potions
  //will be shown as no-picked in the item-knoledwe
  //Alex:10.12.01
  if (id[n][ty]<ID_NO_PICK && setting==ID_KNOWN_TYPE) setting=ID_PICK;
  id[n][ty] = setting;  
}                               // end set_ident_type()


//Return id[][] for the base_type ans sub_type of a object
//Alex:22.11.01
char get_id(char cla, char ty)
{
    char n;	
  
    switch (cla)
    {
    case OBJ_WANDS:
        n=id[ IDTYPE_WANDS ][ty];break;

    case OBJ_SCROLLS:
        n=id[ IDTYPE_SCROLLS ][ty];break;

    case OBJ_JEWELLERY:
        n=id[ IDTYPE_JEWELLERY ][ty];break;

    case OBJ_POTIONS:
        n=id[ IDTYPE_POTIONS ][ty];break;

    default:
        return (ID_UNKNOWN_TYPE);
    }
        
    
    return n;    
}

//Returns   ID_UNKNOWN_TYPE,ID_KNOWN_TYPE,ID_TRIED_TYPE
//
//If id[][] is ID_NO_PICK or ID_PICK returns ID_KNOWN_TYPE
//
//Modified by: Alex, 19.11.01
char get_ident_type(char cla, char ty)
 {
   char n;
   n=get_id(cla,ty);
   if (n>ID_TRIED_TYPE || n==ID_KNOWN_TYPE)  n=ID_KNOWN_TYPE; else n=ID_UNKNOWN_TYPE;	
   
   return n;
 }


//Returns 1 if item can be picked	
//Rules:
//  first   look if item is marked as autopick or no-pick
//
//  second  look if item is know
//          then see if is a bad item and option for not autopick bad items is active
//
//  else   all items can be caught if they aren't marked as no-pick
//          
//  
//Alex: 22.11.01
int get_can_pick(char cla,char ty)
  {
   char n;
   n=get_id(cla,ty);
   //don't pickup bad knows items
   if (n==ID_KNOWN_TYPE) return 1;
/*     {
        switch(cla)
          { 
          	case OBJ_POTIONS:     	  
          	  if (xtraOps.ax_no_pick_bad_potions && ty>=POT_FIRST_BAD)
          	     return 0;
          	case OBJ_SCROLLS:
          	  if (xtraOps.ax_no_pick_bad_scrolls && ty>=SCR_FIRST_BAD)
          	     return 0;
          	case OBJ_JEWELLERY:
          	  if (xtraOps.ax_no_pick_bad_jewellery  && ty>=JEW_FIRST_BAD)
          	     return 0;           	  
          }	
    }*/

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
        unit_mass = mss[ item.base_type ][ item.sub_type ];
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
