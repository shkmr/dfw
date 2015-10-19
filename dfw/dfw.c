/* 
 *      D/FW : Debug Firmware for FET430UIF/eZ430U
 *
 *                        written by skimu@mac.com
 */
#include <stdint.h>
#include <signal.h>
#include <io.h>

#include "uif.h"
#include "tap.h"
#include "datalink.h"
#include "eeprom.h"
#include "target.h"
#include "misc.h"

void hardware_self_test()
{
  eeprom_firmware_check();
}

void run(void) __attribute__ ((noreturn)); /* in cmds.c */

int main()
{
  WDTCTL = (WDTPW|WDTHOLD);

  tap_set_jtag();
  target_set_msp430();
  init_datalink();
  init_uif();
  led_message("-.-");
  hardware_self_test();
  run();
}

/* EOF */
