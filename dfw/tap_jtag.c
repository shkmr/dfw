/*    tap_jtag.c : Four wire JTAG functions.
 *
 *    Derived from reference code associated with,
 *    Markus Koesler, Franz Graf, and Zack Albus,
 *    ``Programming a Flash-Based MSP430 Using JTAG
 *    Interface'', Application Report SLAA149A,
 *    Texas Instruments, December 2005,
 *    and,
 *    ``MSP430 Memory Programming User's Guide''
 *    SLAU265E, Texas Instruments, Revsed January 2010.
 *
 *    Please find disclaimer at the end of this file.
 */
#include <stdint.h>

#include "uif.h"
#include "uif_macros.h"
#include "misc.h"

#define SetTCLK()        set_TDI()
#define ClrTCLK()        clr_TDI()
#define StoreTCLK()      tst_TDI()
#define RestoreTCLK(x)   (x == 0 ? clr_TDI(): set_TDI())

void    jtag_SET_TCLK()  {set_TDI();}
void    jtag_CLR_TCLK()  {clr_TDI();}

#define SetTMS()         set_TMS()
#define ClrTMS()         clr_TMS()
#define SetTDI()         set_TDI()
#define ClrTDI()         clr_TDI()
#define SetTCK()         set_TCK()
#define ClrTCK()         clr_TCK()
#define ScanTDO()        tst_TDO()

static uint32_t jtag_AllShifts(int bits, uint32_t data)
{   
  int        tclk = StoreTCLK();  // Store TCLK state;
  uint32_t   TDOword  = 0;
  uint32_t   MSB      = 0;
  int        i;

  switch (bits) {
  case 8:  MSB = 0x00000080;    break;
  case 16: MSB = 0x00008000;    break;
  case 20: MSB = 0x00080000;    break;
  case 32: MSB = 0x80000000;    break;
  default: 
    // this is an unsupported format, function will just return 0
    // XXX we should call panic();
    return 0;
  }    
  for (i = bits; i > 0; i--) {
    if ((data & MSB) == 0)
      ClrTDI(); 
    else
      SetTDI();
    data <<= 1;
    if (i == 1)  {
	SetTMS();           // Last bit requires TMS=1
    }
    ClrTCK();
    SetTCK();
    TDOword <<= 1;
    if (ScanTDO() != 0) {
      TDOword++;	
    }
  }
  RestoreTCLK(tclk);
  
  // JTAG FSM = Exit-DR
  ClrTCK();
  SetTCK();
  // JTAG FSM = Update-DR
  ClrTMS();
  ClrTCK();
  SetTCK();
  // JTAG FSM = Run-Test/Idle
  if (bits == 20)
    TDOword = ((TDOword&0x0f)<<16) + (TDOword>>4);

  return TDOword;
}

uint16_t jtag_DR_SHIFT16(uint16_t data)
{
  // JTAG FSM state = Run-Test/Idle
  SetTMS();
  ClrTCK();
  SetTCK();

  // JTAG FSM state = Select DR-Scan
  ClrTMS();
  ClrTCK();
  SetTCK();

  // JTAG FSM state = Capture-DR
  ClrTCK();
  SetTCK();
  
  // JTAG FSM state = Shift-DR, Shift in TDI (16-bit)

  return jtag_AllShifts(16, data);

  // JTAG FSM state = Run-Test/Idle
}

uint32_t jtag_DR_SHIFT20(uint32_t address)
{
  // JTAG FSM state = Run-Test/Idle
  SetTMS();
  ClrTCK();
  SetTCK();

  // JTAG FSM state = Select DR-Scan
  ClrTMS();
  ClrTCK();
  SetTCK();
  // JTAG FSM state = Capture-DR
  ClrTCK();
  SetTCK();

  // JTAG FSM state = Shift-DR, Shift in TDI (16-bit)
  return jtag_AllShifts(20, address);
  // JTAG FSM state = Run-Test/Idle
}

uint16_t jtag_IR_SHIFT(uint16_t instruction)
{
  // JTAG FSM state = Run-Test/Idle
  SetTMS();
  ClrTCK();
  SetTCK();
  // JTAG FSM state = Select DR-Scan
  ClrTCK();
  SetTCK();

  // JTAG FSM state = Select IR-Scan
  ClrTMS();
  ClrTCK();
  SetTCK();
  // JTAG FSM state = Capture-IR
  ClrTCK();
  SetTCK();

  // JTAG FSM state = Shift-IR, Shift in TDI (8-bit)
  return jtag_AllShifts(8, instruction);
  // JTAG FSM state = Run-Test/Idle
}

void jtag_RESET_TAP()
{
  int    i;

  // process TDI first to settle fuse current
  SetTDI();
  SetTMS();
  SetTCK();

  // Now fuse is checked, Reset JTAG FSM
  for (i = 6; i > 0; i--) {
    ClrTCK();
    SetTCK();
  }
  // JTAG FSM is now in Test-Logic-Reset
  ClrTCK();
  ClrTMS();
  SetTCK();
  ClrTCK();    
  SetTCK();
  ClrTCK();    
//SetTMS();
  // JTAG FSM is now in Run-Test/IDLE
}

uint16_t jtag_ATTACH_TAP()
{
  uif_attach_jtag();
  jtag_RESET_TAP();  // reset TAP state machine -> Run-Test/Idle
  return jtag_IR_SHIFT(0xff); /* IR_BYPASS */
}

void jtag_DETACH_TAP()
{
  uif_detach_jtag();
  msleep(10);
}

void jtag_TCLK_STROBES(int n)
{
  volatile int i;
  
  /*
   *   body cycles >= 400KHz
   *   FET430UIF MCLK= 8MHz -> 20 cycles,
   *   eZ430U    MCLK=12MHz -> 30 cycles.
   *   see below compiled assembly code
   */
  for (i = n; i > 0; i--)  {
    SetTCLK();
    TCLK_STROBES_NOPS();     /* defined in uif_macros.h */
    ClrTCLK();
   }
}

#if 0
.L43:
        bis.b   #llo(2), &0x0031        5
/* #APP */
        nop                             1 or 11
/* #NOAPP */
        bic.b   #llo(2),&0x0031         5
        add     #llo(-1), @r1           5
        cmp     #llo(1), @r1            5
        jge     .L43                    2
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
