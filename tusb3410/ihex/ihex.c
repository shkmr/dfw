/* Simple IntelHex to Binary translator			*
 * written by Bodo Rzany				*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define	buflen	256	// length of ascii input buffer

/* ihex-format is ':'<bytcnt:2><offset:4><rectyp:2><datbyt:2*bytcnt><chksum:2>'\n'	*
 * with offset:4 = <offs.hi:2><offs.lo:2> (!!!)						*
 * rectyp:  0=data, 1= eof, 2= extnd seg ads rec, 3= start seg ads rec,			*
 * 	4= extnd lin ads rec, 5= extnd start ads rec					*
 * chksum: 2's complement of sum from <bytcnt> to last <datbyt>				*/

unsigned int hexdig(char *buf) {
  char c = (*buf);
  if ( isdigit(c) ) return c-'0';
  return ((islower(c) ? toupper(c) : c)  - 'A' + 10);
}

unsigned int hexbyt(char *buf) {
  return (hexdig(buf) << 4) | hexdig(buf+1);
}

int ihex(char *filnam, unsigned char *binbuf, int binsiz) {
  FILE *ihexfile;
  char *ihexline;
  int linecount = 0;
  unsigned int recofs, maxofs, i;
  unsigned char reclen, rectyp, chksum;
  
  if ( (ihexfile = fopen(filnam, "r")) == NULL ) return -1;
  if ( (ihexline = malloc(buflen)) == NULL ) return -2;
  maxofs = 0;
  
  while ( !feof(ihexfile) ) {
    if ( fgets(ihexline, buflen, ihexfile) == NULL ) {
      printf("fgets returned NULL\n");
      return -3;
    }
    linecount++;
    if ( (*ihexline) != ':' ) {
      printf("missing ':' at line %i\n", linecount);
      return -4;
    }
    reclen = hexbyt(ihexline+1);	// bytcnt
    chksum = reclen;
    recofs = hexbyt(ihexline+3);	// offs.hi
    chksum += recofs;
    rectyp = hexbyt(ihexline+5);	// offs.lo
    chksum += rectyp;
    recofs = (recofs << 8) + rectyp;
    rectyp = hexbyt(ihexline+7);	// rectyp
    chksum += rectyp;

    switch (rectyp) {
    case 0:			// data record
      for ( i=0; i<reclen; i++ ) {
	if (recofs >= binsiz) {
	  printf("address offset too large at line %i\n", linecount);
	  return -1;
	}
	rectyp = hexbyt(ihexline+9+(2*i));
	chksum += rectyp;
	*(binbuf+recofs) = (unsigned char) rectyp;
	recofs += 1;
      }
      if (chksum) chksum = 256 - chksum;
      if ( chksum != hexbyt(ihexline+9+(2*reclen)) ) {
	printf("checksum error at line %i\n", linecount);
	return -1;
      }
      if ( maxofs < recofs ) maxofs = recofs;
      break;

    default:			// all other records
      for ( i=0; i<reclen; i++ ) chksum += hexbyt(ihexline+9+(2*i));
      if (chksum) chksum = 256 - chksum;
      if ( chksum != hexbyt(ihexline+9+(2*reclen)) ) {
	printf("checksum error at record type %i\n", rectyp);
	return -1;
      }
      if ( rectyp == 1 ) return maxofs;
      break;
    }
  }
  return 0;
}
