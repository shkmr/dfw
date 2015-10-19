/* 
 *     D/FW main for F2274
 *                   written by skimu@mac.com
 */
#include <stdint.h>
#include <signal.h>
#include <io.h>

#include "uif.h"
#include "tap.h"
#include "datalink.h"
#include "target.h"
#include "misc.h"

void run(void) __attribute__ ((noreturn)); /* in cmds.c */

int main()
{
  WDTCTL = (WDTPW|WDTHOLD);

  tap_set_jtag();
  target_set_msp430();
  init_datalink();
  init_uif();
  //uif_usart_com();
  led_message("-.-");
  run();
}

/* EOF */
