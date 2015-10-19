#ifndef DATALINK_H
#define DATALINK_H

void   init_datalink();   /* initialize ring buffer */

int    putchar(int c);
int    getchar();
void   putstr(char *str);
void   gettoken(char *str, int maxchar);

#endif /* DATALINK_H */
