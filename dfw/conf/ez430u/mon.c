/*  
 *  Interact with RF2500T via eZ430U/PTFW
 *
 *                         written by skimu@mac.com
 *
 *  Press ESC to exit from this program.
 */
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <err.h>

struct termios oldtio, newtio;
int           dfd;
int           tfd;

void myexit(int n)
{
  tcsetattr(tfd, TCSANOW, &oldtio);
  exit(n);
}

#define BUF_SIZE 16

read_message()
{
  char    buf[BUF_SIZE];
  int     n;

  n = read(dfd, buf, BUF_SIZE);
  write(tfd, buf, n);
}

send_message()
{
  char    buf[BUF_SIZE];
  int     n;

  n = read(tfd, buf, BUF_SIZE);
  if (buf[0] == '\x1b') myexit(0); /* ESC to exit */
  write(dfd, buf, n);
}

main(int c, char *v[])
{
  int       maxfd;
  fd_set    rfds;

  if (c < 2) {
    fprintf(stderr, "usage: %s /dev/cu.usbmodemXXX\n", v[0]);
    return 40;
  }
  if ((dfd = open(v[1],       O_RDWR|O_NONBLOCK)) < 0) err(1, v[1]);
  if ((tfd = open("/dev/tty", O_RDWR|O_NONBLOCK)) < 0) err(1, "/dev/tty");

  tcgetattr(tfd, &oldtio);
  memcpy(&newtio, &oldtio, sizeof(struct termios));
  cfmakeraw(&newtio);
  tcsetattr(tfd, TCSANOW, &newtio);
  err_set_exit(myexit);

  maxfd = (dfd > tfd ? dfd : tfd) + 1;
  FD_ZERO(&rfds);
  while (1) {
    FD_SET(dfd, &rfds);
    FD_SET(tfd, &rfds);
    select(maxfd, &rfds, NULL, NULL, NULL);
    if (FD_ISSET(dfd, &rfds)) read_message();
    if (FD_ISSET(tfd, &rfds)) send_message();
  }
}

/* EOF */
