#ifndef _TRANSLATE_
#define _TRANSLATE_

//////////////////////////////////////////
// Name of objects
//////////////////////////////////////////
struct tpTransNameObject
  {
    char sing[30];     //when singular
    char plural[30];   //when plural        
  };
  
  
typedef  struct tpTransNameObject TTransNameObject;

 
  
extern TTransNameObject TRANSLATE_NAME_OBJS[];
  
#endif   