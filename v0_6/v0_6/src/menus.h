/********************************************************
*
*  menus.h
*
*  Routines for drawig menus
*
*  Alex: 24.07.01
*
********************************************************/

#ifndef MENU_H
  #define MENU_H
  

  struct tMenu
  {
    int nFilaIni;  //first line available for the menu
    int nFilaMax;  //last file available for the menu
    int nFila;     //current row of the menu
    int nItem;     //current item of the menu
    int nMaxItems; //number of items
    
    //wich is the nItem shown in the first row
    //it's computed by the menu() routine
    int nItemFirstRow; 
    
    
    //Print the item mnu.nItem
    //bSelec=1 select mnu.nItem
    //bSelec=0 de-select mnu.nitem
    //bSelec=4 clear item
    int (*print_item)(struct tMenu& mnu,int bSelec);
    
    //To handle keyboard. "c" is the key that the 
    //user pressed
    int (*handle_key)(struct tMenu& mnu,int c);
    
    //pointers to user data
    char *data;     //normally text to show
    
    char *data2;    //codes for each item
                    //Ex: You can select the item "Text of the item"
                    //Selecting it means read some info of some fich
                    //and print it on the screen.
                    //The name of the fich is in data2 
                    
    int flags;  //extra user-flags                     
  };   
  
  
  //returns the number of the item selected or -1 if none  
  //
  //menu2() restore the screen. menu() not
  int menu2(struct tMenu& mnu,void (*scrn_update)(),int bRespectActualState=0);
  int menu(struct tMenu& mnu,int bRespectActualState=0);
  
  //Draws a box
  void Box(int x1,int y1,int x2,int y2);
  
  //read a fich an returns a pointer to the contents 
  char *read_desc_fich(char *nomfich);
  
  extern int bKeySpecial; //in menus.cc
  unsigned char gt_c(void);
  
  //show the items of a menu
  int show_items(struct tMenu mnu);
  
  //As show_item but don't reset mnu.nItem nor mnu.nFila
  int show_items2(struct tMenu mnu);
  
  //Clears a portion of the screen
  void ClearScreen(int nColIni,int nRowIni,int nColEnd,int nRowEnd,int colour=0);
  
  void AFormatScreen(char *Text,int nCol,int colour=MAGENTA);
#endif  
