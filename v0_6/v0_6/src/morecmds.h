#ifndef _MORECMDS_
   #define _MORECMDS_
   
   void more_commands();
   
   //Dump current map
   void DumpMap(bool messg);

   //Extens the keyboard with a new keyset    
   //This keyset is the key '#' and a second key
   //When the user press '#', acr.cc calls 
   //this routine gets the next key 
   //an executes the command
   void extended_keyinput();
   
   
   //Screen to manage the equipment
   void Equipment();  
#endif   