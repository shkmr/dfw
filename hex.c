/*
  Intel MDS .hex file read and writer.

  Orion Sky Lawlor, olawlor@acm.org, 2003/8/3

  Modified by:
  Shigenobu Kimura, skimu@mac.com
    2006/01/20: hex_write(): return immediately if len <= 0
    2005/12/24: taken from pk2-1.34 which is covered by GPL,
                so I assume this file is also covered by GPL.
  Mark Rages, markrages@gmail.com, 2005/04/01
  Chen Xiao Fan, xiaofan@sg.pepperl-fuchs.com, 2005/08/12
  Jeff Post, j_post@pacbell.net, 2005/10/16
*/

#include <ctype.h>
#include "hex.h"

/* Begin writing a .hex file here */

void hex_write_begin(FILE *f)
{
	/* nothing to do, although you might set a type 02 or
	type 04 address marker here... */
}

typedef unsigned char byte;

/* Write this byte, updating checksum */

void hex_write_byte(FILE *f, unsigned int bi, byte *checksum)
{
  byte b = (byte)(0xffu & bi);
  
  fprintf(f, "%02X", (int) b); 
  *checksum -= b;
}

/* 
   Write this data to the .hex file at this address.
   Can write any number of bytes of data-- splits lines internally.
*/

void hex_write(FILE *f, unsigned int addr, unsigned int len, byte *data)
{
  int hex_bytes_max = 16;

  if (len <= 0)
    return;
  
  if (len <= hex_bytes_max) {

    unsigned int i;
    
    byte checksum = (byte) 0;
    fprintf(f, ":");
    hex_write_byte(f, len, &checksum);
    hex_write_byte(f, addr >> 8, &checksum);
    hex_write_byte(f, addr, &checksum);
    hex_write_byte(f, 0x00u, &checksum);
    
    for (i=0; i<len; i++)
      hex_write_byte(f, data[i], &checksum);
    
    hex_write_byte(f, checksum, &checksum);
    fprintf(f, "\n"); 

  } else { 
    /* len is too long-- split into smaller lines */

    while (len > 0) {
      int n = len;
      
      if (n > hex_bytes_max)
        n = hex_bytes_max;
      
      hex_write(f, addr, n, data);
      addr += n;
      len -= n;
      data += n;
    }
  }
}

/* Finish writing a .hex file here */

void hex_write_end(FILE *f)
{
  fprintf(f, ":00000001FF\n");	/* special end marker */
}

/* 
   Read a .hex file from here, sending the resulting address
   spans to this function. 
*/
const char *hex_read(FILE *f, hex_dest_fn fn, void *param)
{
  unsigned int addrbase16 = 0u; /* DOS-style "segment" of program */
  unsigned int addrbase32 = 0u; /* High 16 bits of program counter */

  if (f == NULL)
    return "could not open file";
  
  while (1) {
    /* 
       Read one line of the .hex file: 
       : <len> <addr hi> <addr lo> <type> [ < data > ] <checksum>
    */
    unsigned int addr, i, v, len, type, checksum = 0u;
#define  data_line_max  64u

    byte data[data_line_max + 1];
    
    /* Seek to start of next line */
    int c;

    while (EOF != (c = fgetc(f))) {
      
      if (c == ':')

        break; /* Start of next line */

      else if (!isspace(c)) 

        return "unexpected characters";

    }
    
    if (c == EOF)
      return NULL; /* Hit EOF */
    
    /* Read address and length of line */

    if (3 != fscanf(f, "%02x%04x%02x", &len, &addr, &type))
      return "unexpected start-of-line format";
    
    checksum += len;
    checksum += (addr + (addr >> 8));
    checksum += type;
    
    /* Read data and checksum for line: */

    if (len > data_line_max)
      return "line too long";
    
    for (i=0; i<len+1; i++) {
      if (1 != fscanf(f, "%02x", &v))
        return "unexpected data format";
      
      data[i] = v;
      checksum += v;
    }
    
    if (0 != (checksum & 0xffu))
      return "line checksum mismatch";
    
    /* Handle line */
    if (type == 0x00u) { 
 
     /* Regular data line-- pass data to user */

      (fn)(param, addrbase16 + addrbase32 + addr, len, data);

    } else if (type == 0x01u) { 

      /* End-of-file line-- exit happily */

      return NULL;

    } else if (type == 0x02u) { 

      /* latch bits 8-24 of PC */

      addrbase16 = addr << 8;

    }  else if (type == 0x04u) {

      /* latch bits 16-32 of PC */
      
      addrbase32 = addr << 16;
    } else 
      return "unrecognized line type"; 
  }
}

/* EOF */

