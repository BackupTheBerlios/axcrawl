#ifndef _UNIDDESC_
#define _UNIDDESC_

struct tpUnIdDesc
 {
   char *descrip; //pointer to the description
 };

extern int NDescUnId[NUM_OBJECT_CLASSES];

typedef struct tpUnIdDesc TUnIdDesc;

//////////////////////////////////////////////////////////////
//load the text for descriptions of unidentified items
//////////////////////////////////////////////////////////////
void load_uniddesc(char *filename,int nWich);


//////////////////////////////////////////////////////////////
//returns the text for un-identified names of items
//////////////////////////////////////////////////////////////
char *uniddesc(int base_type,int sub_type);
char *uniddesc2(int base_type,int unid);

//////////////////////////////////////////////////////////////
//suffle the un-identified descriptions for an object type
//Each item gets an un-id. desccription in order they
//appear in aPtrDescUnId[]. It must be suffled
//in order to get differents un-id. descriptions in each game
//////////////////////////////////////////////////////////////
void shuffle(int base_type);

#endif


