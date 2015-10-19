/* dfwsh.c : to debug dfw */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <termios.h>
#include <readline/readline.h>

enum {
  LINE_MAX = 128
};

static char dfw_buf[LINE_MAX];
static char *line_read = (char *)NULL;

/*   Read a string, and return a pointer to it.
 *   Returns NULL on EOF. 
 */
char *rl_gets ()
{

  if (line_read) {
    free(line_read);
    line_read = (char *)NULL;
  }
  line_read = readline ("(dfw) ");

  if (line_read && *line_read)
    add_history(line_read);

  return line_read;
}

int   dfd;

void dfw_puts(char *buf)
{
  int  n = strlen(buf);

  strcpy(dfw_buf, buf);
  dfw_buf[n] = '\n';
  write(dfd, dfw_buf, n + 1);
}

char *dfw_gets()
{
  char   *buf = dfw_buf;
  int     n, len;
  
  len = LINE_MAX;
  n =  0;
  while (len > 0) {
    n = read(dfd, buf, len);
    if (n > 0 && buf[n-1] == '\n') 
      break;
    buf += n;
    len -= n;
  }
  buf[n-1] = '\0';

  return dfw_buf;
}

int main(int c, char *v[])
{
  struct termios tio;

  if (c < 2) {
    fprintf(stderr, "usage: %s /dev/cu.usbmodemXXX\n", v[0]);
    return 40;
  }
  if ((dfd = open(v[1], O_RDWR)) < 0) err(1, v[1]);

  tcgetattr(dfd, &tio);
  cfmakeraw(&tio);
  if (tcsetattr(dfd, TCSANOW, &tio) < 0) 
    err(1, NULL);

  while (rl_gets() != NULL) {
    dfw_puts(line_read);
    puts(dfw_gets());
  }
  return 0;
}
