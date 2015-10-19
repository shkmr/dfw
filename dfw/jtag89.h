#ifndef JTAG89_H
#define JTAG89_H
/* 
 * Constants for the JTAG instruction register (IR, requires LSB first).
 * The MSB has been interchanged with LSB due to use of the same shifting
 * function as used for the JTAG data register (DR, requires MSB first).
 */
#define JTAG_ID                    0x89

#define IR_CNTRL_SIG_16BIT         0xC8   // 0x13 original values
#define IR_CNTRL_SIG_CAPTURE       0x28   // 0x14
#define IR_CNTRL_SIG_RELEASE       0xA8   // 0x15

#define IR_PREPARE_BLOW            0x44   // 0x22
#define IR_EX_BLOW                 0x24   // 0x24

#define IR_DATA_16BIT              0x82   // 0x41
#define IR_DATA_CAPTURE            0x42   // 0x42 XXX guessed
#define IR_DATA_QUICK              0xC2   // 0x43

#define IR_DATA_PSA                0x22   // 0x44
#define IR_SHIFT_OUT_PSA           0x62   // 0x46

#define IR_ADDR_16BIT              0xC1   // 0x83
#define IR_ADDR_CAPTURE            0x21   // 0x84
#define IR_DATA_TO_ADDR            0xA1   // 0x85

#define IR_BYPASS                  0xFF   // 0xFF

/* 
   JTAG Control Signal Register for MSP430 1xx/2xx/4xx (Table 8-5 of SLAU265E)

   15: (N/A)       Always write 0
   14: SWITCH      Enables TDO output as TDI input (1=JTAG, 0=Nomal operation)
   13: TAGFUNCSAT  Sets flash module into JTAG access mode       (1=CPU, 0=JTAG)
   12: Release     Selects control source of the RW and BYTE bits (1=CPU, 0=JTAG)

   11: POR         Perform POR if 1 is written
   10: TCE1        Put CPU under JTAG control (1=CPU under JTAG, 0=CPU free runing)
    9: TCE         Indicate CPU syncronization (1=Synced, 0=Not Synced)
    8: (N/A)       Always write 0

    7: INSTR_LOAD  (R only, 1=CPU is instr fetch state)
    6: (N/A)       Always write 0
    5: (N/A)       Always write 0  (CPUOFF ???)
    4: BYTE        1=Byte Access

    3: HALT_JTAG   Sets CPU into a controlled halt state (1=CPU stopped)
    2: (N/A)       Always write 0  (NMI ???)
    1: (N/A)       Always write 0
    0: R/W         1=Read, 0=Write
  
    writing
    0x2C01 = .010 11r. r..0 0..1  (POR)
    0x3401 = .011 01r. r..0 0..1  (set pc)
    0x2408 = .010 01r. r..0 1..0  (word write)
    0x2418 = .010 01r. r..1 1..0  (byte write)
    0x2401 = .010 01r. r..0 0..1  (release cpu)
    0x2409 = .010 01r. r..0 1..1  (halt, word read)
    0x2419 = .010 01r. r..1 1..1  (byte read)

    reading
    0x2400 = .010 010. 0..0 0..0  (cpu running)
    0x2481 = .010 010. 1..0 0..1  (cpu running, instr fetch)
    0x2681 = .010 011. 1..0 0..1  (cpu stopped, instr fetch)
    0x2601 = .010 011. 0..0 0..1  (after release)
    0x2081 = .010 000. 1..0 0..1  (after continue)
    0x2689 = .010 011. 1..0 1..1  (after halt)
*/
#define SIG_NA_BIT15               0x8000   /* BIT 15 */
#define SIG_SWITCH                 0x4000   /* BIT 14 */
#define SIG_TAGFUNCSAT             0x2000   /* BIT 13 */
#define SIG_RELEASE_LBYTE          0x1000   /* BIT 12 */

#define SIG_POW                    0x0800   /* BIT 11 */
#define SIG_TCE1                   0x0400   /* BIT 10 */
#define SIG_TCE                    0x0200   /* BIT  9 */
#define SIG_NA_BIT8                0x0100   /* BIT  8 */

#define SIG_INSTR_LOAD             0x0080   /* BIT  7 */
#define SIG_NA_BIT6                0x0040   /* BIT  6 */
#define SIG_CPUOFF                 0x0020   /* BIT  5 XXX */
#define SIG_BYTE                   0x0010   /* BIT  4 */

#define SIG_HALT_JTAG              0x0008   /* BIT  3 */
#define SIG_NA_BIT2                0x0004   /* BIT  2 */
#define SIG_NA_BIT1                0x0002   /* BIT  1 */
#define SIG_READ                   0x0001   /* BIT  0 */

#endif /* JTAG89_H */

