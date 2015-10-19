/* 
 *      Write minidfw into FET430UIF or eZ430U.
 *     
 *                        written by skimu@mac.com
 */
#include <stdint.h>
#include <stdio.h>
#include <io.h>

#include "uif.h"
#include "uif_macros.h"
#include "tap.h"
#include "datalink.h"
#include "target_msp430.h"
#include "misc.h"

#include "minidfw_lma.inc"

const uint8_t text_segment[] = {
#include "minidfw_text.inc"
};
const uint8_t data_segment[] = {
#include "minidfw_data.inc"
};
const uint8_t vect_segment[] = {
#include "minidfw_vect.inc"
};

static inline int button_pressed()
{
  return (P1IN&BIT2) == 0;
}

static inline int button_released()
{
  return (P1IN&BIT2) == BIT2;
}

void run(void)           __attribute__ ((noreturn)); 
int  main(void)          __attribute__ ((noreturn));
void enter_updater(void) __attribute__ ((noreturn));

static void wait_on_signal()
{
  uint32_t    i;

  while (1) {

    mode_LED_off();
    while (button_released()) ;
    mode_LED_on();

    for (i=0; button_pressed(); i++) {
      if (i > 1000000)  /* should be a few seconds */
        goto button_pressed_long_engouh;
    }

  }
 button_pressed_long_engouh:
  mode_LED_off();
  while (button_pressed()) ; /* wait for button released */
  return;
}

const int WRITE_SIZE = 16;

void write_flash(uint32_t addr, uint16_t *buf, uint16_t len)
{
  while (len > WRITE_SIZE) {
    mode_LED_flip();
    msp430_write_flash(addr, buf, WRITE_SIZE);
    addr += WRITE_SIZE*2;
    buf  += WRITE_SIZE;
    len  -= WRITE_SIZE;
  }
  mode_LED_flip();
  msp430_write_flash(addr, buf, len);
  mode_LED_off();
}

void run()
{
  if (sizeof(text_segment)   ==  0) panic();
  if (sizeof(text_segment)%2 !=  0) panic();
  if (sizeof(vect_segment)   != 32) panic();

  msp430_set_FastFlash();                    /* F1612 has FastFlash */
  if (msp430_get_device() != OK)    panic();
  if (msp430_device_id() != 0xf16c) panic(); /* Check if it is F1612 */

  led_message("--- -.-");                    /* OK: ready to write */
  
  wait_on_signal();

  led_message(". .");

  msp430_erase_flash(msp430_ERASE_MASS, 0xfff0);

#define u16(x) (x/2 + x%2)
  write_flash(text_lma, (uint16_t *)text_segment, u16(sizeof(text_segment)));
  write_flash(vect_lma, (uint16_t *)vect_segment, u16(sizeof(vect_segment)));
  if (sizeof(data_segment) > 0)
    write_flash(data_lma, (uint16_t *)data_segment, u16(sizeof(data_segment)));
#undef  u16

  led_message("..-. .. -.");                  /* FIN: finished */
  
  while (1) ;
}

int main()
{
  WDTCTL = (WDTPW|WDTHOLD);
  tap_set_jtag();
  //init_datalink();
  init_uif();

  P1DIR &= ~BIT2; /* button input               */
  P1OUT |=  BIT2; /* pullup resistor for button */
  P1REN |=  BIT2; /* pullup resistor enable     */

  run();
}

/* EOF */
