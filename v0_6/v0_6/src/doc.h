/*----------------------------------------------------------
doc.cc

Routines to show documentation stored
in ASCII files.

Alex: 17.09.01
------------------------------------------------------------*/
 
#ifndef _DOC_
#define _DOC_

//show the contents of a file
int show_file(char *name, int line=0, int mode=0700);
//called in acr.cc


//shows the file doc\help.hlp
void do_help();
//called in acr.cc

int ListFile(char *namefile);

#endif