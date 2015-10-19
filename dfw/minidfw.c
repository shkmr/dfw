/* 
 *      Mini D/FW : update EEPROM and wait for firmware update.
 *
 *                        written by skimu@mac.com
 */
#include <stdint.h>
#include <stdio.h>
#include <io.h>

#include "uif.h"
#include "eeprom.h"
#include "datalink.h"

void run(void)           __attribute__ ((noreturn)); 
int  main(void)          __attribute__ ((noreturn));
void enter_updater(void) __attribute__ ((noreturn));

static void wait_for_nl()    {while (getchar() != '\n') ;}

void run()
{
  while (1) {
    switch (getchar()) {
    case '\n': printf("*\n");       break;
    case 'b':  
      wait_for_nl(); 
      enter_updater(); 
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
  eeprom_firmware_check();
  run();
}

/* EOF */
