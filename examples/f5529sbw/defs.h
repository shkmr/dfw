#define BIT0                0x0001
#define BIT1                0x0002
#define BIT2                0x0004
#define BIT3                0x0008
#define BIT4                0x0010
#define BIT5                0x0020
#define BIT6                0x0040
#define BIT7                0x0080
#define BIT8                0x0100
#define BIT9                0x0200
#define BITA                0x0400
#define BITB                0x0800
#define BITC                0x1000
#define BITD                0x2000
#define BITE                0x4000
#define BITF                0x8000

#define C                   0x0001
#define Z                   0x0002
#define N                   0x0004
#define V                   0x0100
#define GIE                 0x0008
#define CPUOFF              0x0010
#define OSCOFF              0x0020
#define SCG0                0x0040
#define SCG1                0x0080

#ifdef __ASSEMBLER__ /* Begin #defines for assembler */
#define LPM0                CPUOFF
#define LPM1                SCG0+CPUOFF
#define LPM2                SCG1+CPUOFF
#define LPM3                SCG1+SCG0+CPUOFF
#define LPM4                SCG1+SCG0+OSCOFF+CPUOFF
#else /* Begin #defines for C */
#define LPM0_bits           CPUOFF
#define LPM1_bits           SCG0+CPUOFF
#define LPM2_bits           SCG1+CPUOFF
#define LPM3_bits           SCG1+SCG0+CPUOFF
#define LPM4_bits           SCG1+SCG0+OSCOFF+CPUOFF

#define LPM0      _BIS_SR(LPM0_bits) /* Enter Low Power Mode 0 */
#define LPM0_EXIT _BIC_SR_IRQ(LPM0_bits) /* Exit Low Power Mode 0 */
#define LPM1      _BIS_SR(LPM1_bits) /* Enter Low Power Mode 1 */
#define LPM1_EXIT _BIC_SR_IRQ(LPM1_bits) /* Exit Low Power Mode 1 */
#define LPM2      _BIS_SR(LPM2_bits) /* Enter Low Power Mode 2 */
#define LPM2_EXIT _BIC_SR_IRQ(LPM2_bits) /* Exit Low Power Mode 2 */
#define LPM3      _BIS_SR(LPM3_bits) /* Enter Low Power Mode 3 */
#define LPM3_EXIT _BIC_SR_IRQ(LPM3_bits) /* Exit Low Power Mode 3 */
#define LPM4      _BIS_SR(LPM4_bits) /* Enter Low Power Mode 4 */
#define LPM4_EXIT _BIC_SR_IRQ(LPM4_bits) /* Exit Low Power Mode 4 */
#endif /* End #defines for C */
