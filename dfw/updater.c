#include <stdint.h>
#include <string.h>

void enter_updater()
{
  static const uint8_t updater[] = {
#include "updater/updater_object_array.inc"
  };
  memcpy((void*)0x1100, updater, sizeof(updater)); /* msp430f161x */
  asm("br #0x1100\n");
}
