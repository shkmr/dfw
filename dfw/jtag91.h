#ifndef JTAG91_H
#define JTAG91_H
/* 
 * Constants for the JTAG instruction register (IR, requires LSB first).
 * The MSB has been interchanged with LSB due to use of the same shifting
 * function as used for the JTAG data register (DR, requires MSB first).
 */
#define JTAG_ID                    0x91

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

/* additional instructions for JTAG_ID91 architectures */
#define IR_COREIP_ID               0xE8   // 0x17
#define IR_DEVICE_ID               0xE1   // 0x87

/* Instructions for the JTAG mailbox */
#define IR_JMB_EXCHANGE            0x86   // 0x61
#define IR_TEST_REG                0x54   // 0x2A

#define IR_BYPASS                  0xFF   // 0xFF

/* Constants for JTAG mailbox data exchange */
#define OUT1RDY                    0x0008
#define IN0RDY                     0x0001
#define JMB32B                     0x0010
#define OUTREQ                     0x0004
#define INREQ                      0x0001

/* 
   JTAG Control Signal Register for MSP430 5xx family (Table 8-6 of SLAU265E)

    INSTR_SEQ_NO1 INSTR_SEQ_NO0
      Instruction sequence number. Read only

    RELEASE_LBYTE1 RELEASE_LBYTE1
      Release control bits in low byte from JTAG control
      00 = All bits are controlled by JTAG if TCE1 is 1.
      01 = R/W and BYTE are released from JTAG control
      10 = R/W and HALT, INTREQ, BYTE are released from JTAG control
      11 = Reserved

    POR  controlls the POR signal, same as previous families

    TCE1 Establishes JTAG control over the CPU, same as previous families
    TCE0 Inticates CPU syncronization, same as previous families
   
    CPUSUSP   Suspend CPU. Write 1 to suspend. If reading is 0,
      pipeline is not empty yet. If reading is 1, pipeline is empty.

    INSTR_LOAD  same as previous families
    BYTE        same as previous families
    WAIT        wait signal. read only.
    READ        R/W 
*/
#define SIG_INSTR_SEQ_NO1          0x8000   /* BIT 15 */
#define SIG_INSTR_SEQ_NO0          0x4000   /* BIT 14 */
#define SIG_RELEASE_LBYTE1         0x2000   /* BIT 13 */
#define SIG_RELEASE_LBYTE0         0x1000   /* BIT 12 */

#define SIG_POR                    0x0800   /* BIT 11 */
#define SIG_TCE1                   0x0400   /* BIT 10 */
#define SIG_TCE0                   0x0200   /* BIT  9 */
#define SIG_CPUSUSP                0x0100   /* BIT  8 */

#define SIG_INSTR_LOAD             0x0080   /* BIT  7 */
#define SIG_NA_BIT6                0x0040   /* BIT  6 */
#define SIG_CPUOFF                 0x0020   /* BIT  5 XXX */
#define SIG_BYTE                   0x0010   /* BIT  4 */

#define SIG_WAIT                   0x0008   /* BIT  3 */
#define SIG_NA_BIT2                0x0004   /* BIT  2 */
#define SIG_NA_BIT1                0x0002   /* BIT  1 */
#define SIG_READ                   0x0001   /* BIT  0 */

#endif /* JTAG91_H */
