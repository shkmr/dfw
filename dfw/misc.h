/* misc utils, defined in uif.c uif_fet.c uif_ez.c */

enum { 
  ERR     = 0,
  OK      = 1,
};

void msleep(int n);
void usleep(int n);
void led_message(char *s);
void panic(void) __attribute__ ((noreturn));

/* EOF */
