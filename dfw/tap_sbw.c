/*    tap_sbw.c : SpiByWire JTAG functions.
 *
 *    Derived from reference code associated with,
 *    ``MSP430 Memory Programming User's Guide''
 *    SLAU265E, Texas Instruments, Revsed January 2010.
 *
 *    Please find disclaimer at the end of this file.
 */
#include <stdint.h>
#include "uif.h"
#include "uif_macros.h"
#include "misc.h"

static uint8_t   tdo_bit;               
static uint8_t   TCLK_saved = 1;

//#define   nNOPS   { nop(); nop(); nop(); nop();  nop(); nop(); }
void nNOPS()  {nop(); nop();}
void TMSH()   {set_TDI();   nNOPS(); clr_TCK(); nNOPS(); set_TCK(); }
void TMSL()   {clr_TDI();   nNOPS(); clr_TCK(); nNOPS(); set_TCK(); }
void TMSLDH() {clr_TDI();   nNOPS(); clr_TCK(); nNOPS(); set_TDI(); set_TCK();}
void TDIH()   {set_TDI();   nNOPS(); clr_TCK(); nNOPS(); set_TCK(); }
void TDIL()   {clr_TDI();   nNOPS(); clr_TCK(); nNOPS(); set_TCK(); }
void TDOsbw() {clr_ENI2O(); nNOPS(); clr_TCK(); nNOPS(); set_TCK(); set_ENI2O(); }
void TDO_RD() {clr_ENI2O(); nNOPS(); clr_TCK(); nNOPS(); 
                        tdo_bit = tst_TDO(); set_TCK(); set_ENI2O();}

static void TMSL_TDIL()       { TMSL();  TDIL();  TDOsbw(); }
static void TMSH_TDIL()       { TMSH();  TDIL();  TDOsbw(); }
static void TMSL_TDIH()       { TMSL();  TDIH();  TDOsbw(); }
static void TMSH_TDIH()       { TMSH();  TDIH();  TDOsbw(); }
static void TMSL_TDIH_TDOrd() { TMSL();  TDIH();  TDO_RD(); }
static void TMSL_TDIL_TDOrd() { TMSL();  TDIL();  TDO_RD(); }
static void TMSH_TDIH_TDOrd() { TMSH();  TDIH();  TDO_RD(); }
static void TMSH_TDIL_TDOrd() { TMSH();  TDIL();  TDO_RD(); }

void sbw_CLR_TCLK()
{
  if (TCLK_saved)
    TMSLDH();
  else
    TMSL();
  
  clr_TDI(); TDIL(); TDOsbw();

  TCLK_saved = 0;
}

void sbw_SET_TCLK()
{
  if (TCLK_saved)
    TMSLDH();
  else
    TMSL();

  set_TDI(); TDIH(); TDOsbw();    //ExitTCLK
  TCLK_saved = 1;
}

/*-----------------------------------------------------------------------*/

static uint32_t sbw_AllShifts(int bits, uint32_t data)
{
  uint32_t    TDOword;
  uint32_t    MSB;
  int         i;

  switch (bits) {
  case  8: MSB = 0x00000080;  break;
  case 16: MSB = 0x00008000;  break;
  case 20: MSB = 0x00080000;  break;
  case 32: MSB = 0x80000000;  break;
  default: 
    // this is an unsupported format, function will just return 0
    // XXX we should call panic();
    return 0;
  }
  // shift in bits
  TDOword = 0;
  for (i = bits; i > 0; i--) {
    if (i == 1) {    // last bit requires TMS=1; TDO one bit before TDI
      if ((data & MSB) == 0)  
	TMSH_TDIL_TDOrd();
      else 
	TMSH_TDIH_TDOrd();
    } else {
      if ((data & MSB) == 0)
	TMSL_TDIL_TDOrd(); 
      else
	TMSL_TDIH_TDOrd();
    }
    data <<= 1;
    if (tdo_bit) TDOword++;
    if (i > 1)   TDOword <<= 1;
  }
  TMSH_TDIH();
  if (TCLK_saved)
    TMSL_TDIH();
  else
    TMSL_TDIL();

  if (bits == 20)
    TDOword = ((TDOword&0x0f)<<16) + (TDOword>>4);
  
  return TDOword;
}

uint16_t sbw_DR_SHIFT16(uint16_t data)
{
  // JTAG FSM state = Run-Test/Idle
  if (TCLK_saved) {
    TMSH_TDIH();
  } else {
    TMSH_TDIL();
  }
  // JTAG FSM state = Select DR-Scan
  TMSL_TDIH();
  // JTAG FSM state = Capture-DR
  TMSL_TDIH();

  return sbw_AllShifts(16, data);
}

uint32_t sbw_DR_SHIFT20(uint32_t data)
{
  // JTAG FSM state = Run-Test/Idle
  if (TCLK_saved) {
    TMSH_TDIH();
  } else {
    TMSH_TDIL();
  }
  // JTAG FSM state = Select DR-Scan
  TMSL_TDIH();
  // JTAG FSM state = Capture-DR
  TMSL_TDIH();
  return sbw_AllShifts(20, data);
}

uint16_t sbw_IR_SHIFT(uint16_t inst)
{
  // JTAG FSM state = Run-Test/Idle
  if (TCLK_saved) {
    TMSH_TDIH();
  } else {
    TMSH_TDIL();
  }
  // JTAG FSM state = Select DR-Scan
  TMSH_TDIH();
  
  // JTAG FSM state = Select IR-Scan
  TMSL_TDIH();
  // JTAG FSM state = Capture-IR
  TMSL_TDIH();

  return sbw_AllShifts(8, inst);
}

void sbw_RESET_TAP()
{
  int i;

  for (i = 6; i > 0; i--) TMSH_TDIH();

  TMSL_TDIH();   // now in Run/Test Idle
}

#define MAX_ENTRY_TRY 7

uint16_t sbw_ATTACH_TAP()
{
  uif_attach_sbw();
  sbw_RESET_TAP();
  return sbw_IR_SHIFT(0xff); /* IR_BYPASS */
}

void sbw_DETACH_TAP()
{
  uif_detach_sbw();
  msleep(10);
}

void sbw_TCLK_STROBES(int n)
{
  volatile int i;

  /*
   *   body cycles >= 400KHz
   *   FET430UIF MCLK= 8MHz -> 20 cycles,
   *   eZ430U    MCLK=12MHz -> 30 cycles.
   *   see below compiled assembly code
   */
  for (i = n; i > 0; i--)  {
    sbw_SET_TCLK();
    TCLK_STROBES_NOPS();      /* defined in uif_macros.h */
    sbw_CLR_TCLK();
    nop();
  }
}

#if 0
.L74:
        call    #sbw_SET_TCLK   ???
        call    #sbw_CLR_TCLK   ???
/* #APP */
        nop                      1
/* #NOAPP */
        add     #llo(-1), @r1    5
        cmp     #llo(1), @r1     5
        jge     .L74             2
.L76:
#endif

/* slau265d/JTAG Programming/Readme.txt :

THIS SOFTWARE IS PROVIDED "AS IS". TI MAKES NO WARRANTIES OR
REPRESENTATIONS, EITHER EXPRESS, IMPLIED OR STATUTORY, 
INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS 
FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
COMPLETENESS OF RESPONSES, RESULTS AND LACK OF NEGLIGENCE. 
TI DISCLAIMS ANY WARRANTY OF TITLE, QUIET ENJOYMENT, QUIET 
POSSESSION, AND NON-INFRINGEMENT OF ANY THIRD PARTY 
INTELLECTUAL PROPERTY RIGHTS WITH REGARD TO THE PROGRAM OR 
YOUR USE OF THE PROGRAM.

IN NO EVENT SHALL TI BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
CONSEQUENTIAL OR INDIRECT DAMAGES, HOWEVER CAUSED, ON ANY 
THEORY OF LIABILITY AND WHETHER OR NOT TI HAS BEEN ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGES, ARISING IN ANY WAY OUT 
OF THIS AGREEMENT, THE PROGRAM, OR YOUR USE OF THE PROGRAM. 
EXCLUDED DAMAGES INCLUDE, BUT ARE NOT LIMITED TO, COST OF 
REMOVAL OR REINSTALLATION, COMPUTER TIME, LABOR COSTS, LOSS 
OF GOODWILL, LOSS OF PROFITS, LOSS OF SAVINGS, OR LOSS OF 
USE OR INTERRUPTION OF BUSINESS. IN NO EVENT WILL TI'S 
AGGREGATE LIABILITY UNDER THIS AGREEMENT OR ARISING OUT OF 
YOUR USE OF THE PROGRAM EXCEED FIVE HUNDRED DOLLARS 
(U.S.$500).

Unless otherwise stated, the Program written and copyrighted 
by Texas Instruments is distributed as "freeware".  You may, 
only under TI's copyright in the Program, use and modify the 
Program without any charge or restriction.  You may 
distribute to third parties, provided that you transfer a 
copy of this license to the third party and the third party 
agrees to these terms by its first use of the Program. You 
must reproduce the copyright notice and any other legend of 
ownership on each copy or partial copy, of the Program.

You acknowledge and agree that the Program contains 
copyrighted material, trade secrets and other TI proprietary 
information and is protected by copyright laws, 
international copyright treaties, and trade secret laws, as 
well as other intellectual property laws.  To protect TI's 
rights in the Program, you agree not to decompile, reverse 
engineer, disassemble or otherwise translate any object code 
versions of the Program to a human-readable form.  You agree 
that in no event will you alter, remove or destroy any 
copyright notice included in the Program.  TI reserves all 
rights not specifically granted under this license. Except 
as specifically provided herein, nothing in this agreement 
shall be construed as conferring by implication, estoppel, 
or otherwise, upon you, any license or other right under any 
TI patents, copyrights or trade secrets.
*/
/* EOF */
