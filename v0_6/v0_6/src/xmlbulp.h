/* ------------------------------------------------------------ ajps ---
 * This file contains #defined macros, function prototypes and data
 * structure definitions for use with the xmlbulp.c functions.
 *
 * Copyright (c) 2001 - 2002 Antony Sidwell
 * 
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 *
 * This source file is also available under the GNU GENERAL PUBLIC LICENCE.
 * --------------------------------------------------------------------- */

#ifndef INCLUDED_XMLBULP_H
#define INCLUDED_XMLBULP_H
            
#define _BULP_FILE 0
#define _BULP_MEM 1

#define TERM_DARK		0	/* 'd' */	/* 0,0,0 */
#define TERM_WHITE		1	/* 'w' */	/* 4,4,4 */
#define TERM_SLATE		2	/* 's' */	/* 2,2,2 */
#define TERM_ORANGE		3	/* 'o' */	/* 4,2,0 */
#define TERM_RED		4	/* 'r' */	/* 3,0,0 */
#define TERM_GREEN		5	/* 'g' */	/* 0,2,1 */
#define TERM_BLUE		6	/* 'b' */	/* 0,0,4 */
#define TERM_UMBER		7	/* 'u' */	/* 2,1,0 */
#define TERM_L_DARK		8	/* 'D' */	/* 1,1,1 */
#define TERM_L_WHITE	9	/* 'W' */	/* 3,3,3 */
#define TERM_VIOLET		10	/* 'v' */	/* 4,0,4 */
#define TERM_YELLOW		11	/* 'y' */	/* 4,4,0 */
#define TERM_L_RED		12	/* 'R' */	/* 4,0,0 */
#define TERM_L_GREEN	13	/* 'G' */	/* 0,4,0 */
#define TERM_L_BLUE		14	/* 'B' */	/* 0,4,4 */
#define TERM_L_UMBER	15	/* 'U' */	/* 3,2,1 */

#include "conio.h"
#include "strutil.h"

/* Signed/Unsigned 32 bit value */
#ifdef L64	/* 64 bit longs */
typedef signed int s32b;
typedef unsigned int u32b;
#else
typedef signed long s32b;
typedef unsigned long u32b;
#endif
typedef unsigned char byte;
typedef signed short s16b;
typedef unsigned short u16b;

typedef struct {
	int _bulp_type;
	void *_ptr;
	} BULP;

typedef struct {
	char *base;
	size_t size;
	u32b cur_pos;
	} _bulp_mem;

 
	
/* from bulp.c : */
      int bulp_stricmp (char *s1, char *s2);
    BULP *bulp_open    (char *filename, char *mode);
     bool bulp_close   (BULP *bulp_ptr);
      int bulp_getpos  (BULP *bulp_ptr, u32b *pos);
      int bulp_setpos  (BULP *bulp_ptr, u32b *pos);
      int bulp_seek    (BULP *bulp_ptr, s32b pos, int from);
   size_t bulp_read    (void *ptr, size_t size, size_t nobj, BULP *bulp_ptr);
      int bulp_feof    (BULP *bulp_ptr);
   size_t bulp_size    (BULP *bulp_ptr);
   
#define xmlbulp_skip_chunk(bptr, size) bulp_seek(bptr, size, SEEK_CUR)
             
/* from c.xmlbulp : */
      int xmlbulp_get_next_bit           (BULP *bptr, size_t *size);
      int xmlbulp_find_characters        (BULP *bptr, long int reasonable, int no_chars, ...);
      int xmlbulp_find_string            (BULP *bptr, char *string);
     char xmlbulp_get_unicode_charentity (char *buffer);
     char xmlbulp_get_named_charentity   (char *buffer);
      int xmlbulp_get_char_entity        (BULP *bptr, int size, char *buffer);
     bool xmlbulp_is_whitespace          (char character);
      int xmlbulp_get_text_chunk_st      (BULP *bptr, size_t size, u32b from, char *buffer, size_t bufflen);
      int xmlbulp_get_text_chunk         (BULP *bptr, size_t size, u32b from, char *buffer, size_t bufflen);
    char *xmlbulp_check_attr             (char *tag,char *attr);
      int xmlbulp_get_tag_and_type       (BULP *bptr, size_t size, char *buffer);
      int xmlbulp_get_CDATA_chunk        (BULP *bptr, size_t size, u32b from, char *buffer, size_t bufflen);


#define XMLBULP_MAX_SIZE -1
#define GETPOS_FAILED -1
#define TYPE_TAG 0     
#define TYPE_EOF 7
#define TYPE_TEXT 1
#define TYPE_DOCTYPE 2
#define TYPE_CDATA 3
#define TYPE_COMMENT 4   
#define TYPE_CHARENTITY 5
#define TYPE_ENTITYDEF 6
#define TAG_OPEN 0
#define TAG_CLOSE 1
#define TAG_COMPLETE 2
#define TAG_XML 3
#define TAG_FAILED 4
#define CHARENTITY_NAMED 1
#define CHARENTITY_UNICODE  2
#define CHARENTITY_NOT      0

#define CHARENTITY_MAXSIZE 30


	
void Term_putch(int x, int y, byte a, char c);
int Term_putstr(int x, int y, int n, byte a, cptr s);

typedef unsigned long huge;
int snprintf( char *str, size_t size, const char *format, ... ); //in libutil.cc

#include "z-mem.h"

#endif /* INCLUDED_XMLBULP_H */
