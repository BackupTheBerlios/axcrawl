/*
 *  File:       hiscores.h
 *  Summary:    Scorefile manipulation functions
 *  Written by: Gordon Lipford
 *
 *  Change History (most recent first):
 *
 *     <1>     16feb2001     GDL     Created
 */


#ifndef HISCORES_H
#define HISCORES_H

// last updated 16feb2001 {gdl}
/* ***********************************************************************
 * called from: ouch
 * *********************************************************************** */
void hiscores_new_entry(struct scorefile_entry &se);

// last updated 16feb2001 {gdl}
/* ***********************************************************************
 * called from: acr ouch
 * *********************************************************************** */
void hiscores_print_list(void);

// last updated 16feb2001 {gdl}
/* ***********************************************************************
 * called from: ouch hiscores
 * *********************************************************************** */
int hiscores_format_single(char *buffer, struct scorefile_entry &se);

/*
  hiscores_dump_list(char *filename);
  
  Dumps the score list to a printable file
  
  Returns 1 if any problem.
  Alex: 16.02.02
*/  
int hiscores_dump_list();
#endif  // HISCORES_H
