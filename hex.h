/*
  Intel MDS .hex file read and writer.

  Orion Sky Lawlor, olawlor@acm.org, 2003/8/3

  Modified by:
  Shigenobu Kimura, skimu@mac.com, 2005/12/24
     taken from pk2-1.34 which is covered by GPL,
     so I assume this file is also covered by GPL.
  Mark Rages, markrages@gmail.com, 2005/04/01
  Chen Xiao Fan, xiaofan@sg.pepperl-fuchs.com, 2005/08/12
  Jeff Post, j_post@pacbell.net, 2005/09/30

*/
#ifndef __OSL_HEX_H
#define __OSL_HEX_H

#include <stdio.h>

/* Begin writing a .hex file here */

void hex_write_begin(FILE *f);

/*
  Write this data to this address. 
  Can write any number of bytes
  of data-- splits lines internally.
  WARNING: only low 16 bits of address supported.
*/
void hex_write(FILE *f, unsigned int addr, unsigned int len, 
               unsigned char *data);

/* Finish writing a .hex file here */

void hex_write_end(FILE *f);

/*
  A user-supplied function containing what to do 
  with the .hex spans once they're read in. 

*/
typedef void (*hex_dest_fn)(void *param, unsigned int addr, 
                            unsigned int len, unsigned char *data);

/*
  Read a .hex file from here, sending the resulting address
  spans to this function.   Returns NULL on success; 
  or an error description on failure.  Works with type 02
  and type 04 to supply full 32 bits of address.

*/
const char *hex_read(FILE *f, hex_dest_fn fn, void *param);

#endif
