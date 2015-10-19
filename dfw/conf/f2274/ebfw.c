/* 
 *     ebfw : check functionality of ptfw.ez430u.
 *
 *                   written by skimu@mac.com
 *
 *     ../../../dfwsh and ../ez430u/mon are host side utilities.
 *
 */
#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <io.h>

#include "uif.h"
#include "uif_macros.h"
#include "datalink.h"
#include "misc.h"

void run(void)           __attribute__ ((noreturn));
void run_echo_back(void) __attribute__ ((noreturn));

void run_echo_back()
{
  while (1) 
    putchar(getchar());
}

static void wait_for_nl()    {while (getchar() != '\n') ;}

void run_cmd()
{
  while (1) {
    switch (getchar()) {
    case '\n': printf("*\n");       break;
    case 'o':  
      wait_for_nl(); 
      mode_LED_off();
      printf("LED OFF\n");
      break;
    case 'O':
      wait_for_nl(); 
      mode_LED_on();
      printf("LED ON\n");
      break;
    default:  wait_for_nl(); printf("*\n");
    }
  }
}

int main()
{
  WDTCTL = (WDTPW|WDTHOLD);

  init_datalink();
  init_uif();
  led_message("-.-");

  run_echo_back();
  //run_cmd();
}

/* EOF */
