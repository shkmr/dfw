/*										*
 *	Eine Treiberlein-Firmware fuer das TUSB3410-'usb<->ser'-Interface	*
 * 										*
 * .. und falls noch einmal jemand fragen sollte, warum man ueberhaupt auf die	*
 * schraege Idee verfallen kann, einen 8bit-Prozessor in C zu programmieren:	*
 * Ein Blick auf den 8051/52 erklaert so Manches .. So etwas von krank koennen	*
 * sich Motorola/ARM/Sonstwas-Programmierer nun wirklich nicht vorstellen!	*
 *										*
 * Dies ist das 'Main'-Modul (mit der 'void main(void)'-Routine), das an die	*
 * erste Stelle gelinkt werden muﬂ.						*
 * Und weil C halt eine ganz besondere 'Sprache' ist, stehen auch alle		*
 * Interrupt-Service-Srtns in diesem Modul. Die Alternative waeren 'prototypes'	*
 * hier, und die eigentlichen Srtns woanders, aber das waere noch Bescheuerter.	*
 * Ohne diese Srtns (oder 'prototypes') hier zu definieren, werden die IRVecs	*
 * vom Linker schlicht nicht angelegt (natuerlich ohne 'error' oder 'warning').	*
 *										*/

#include <8052.h>
#include <stdio.h>
#include "tusb3410.h"
#include "ser.h"

extern tEDB xdata xEPIDsc[3];
extern tEDB xdata xEPODsc[3];

extern void SerInit(void);
extern void UsbInit(void);
extern void UsbReset(void);

extern bit fEP2Snd;
extern void usbEP2SndNxt();
/* Hier nun die IRQ-Srtns (siehe oben).						*/

// How to deal with the endpoint interrupts?					*
// During testing there were some 'undocumented features' found, though we just	*
// don't use these interrupts							*
void isr_usbiep1(void) {
  xEPIDsc[0].bEPBCTX = 0;
}

void isr_usbiep2(void) {
//  xEPIDsc[1].bEPBCTX = 0;

  xEPIDsc[1].bEPBCTX |= EPBCT_NAK;

//Griffin
  if ( fEP2Snd ) usbEP2SndNxt();
//  else bIEPCNF2 |= EPCNF_STALL;	// no more data
//  else xEPIDsc[1].bEPCNF |= EPCNF_STALL;	// no more data
}
void isr_usbiep3(void) {
  xEPIDsc[2].bEPBCTX = 0;
}
void isr_usboep1(void) {
  xEPODsc[0].bEPBCTX = 0;
}
void isr_usboep2(void) {
  xEPODsc[1].bEPBCTX = 0;
}
void isr_usboep3(void) {
  xEPODsc[2].bEPBCTX = 0;
}

// These interrupts might have to be used in future. We have to find an another	*
// workaround for the TUSB's suspend bug, because TUSB's initial state is not	*
// usable with the Linux USB subsystem. Just bend the SUS pin to ground for now.*
void isr_usbres(void) { bUSBSTA = USBSTA_RESR; }
void isr_usbsus(void) { bUSBSTA = USBSTA_SUSR; }
void isr_usbrst(void) {
  UsbReset();
  bUSBSTA = USBSTA_RSTR;
}

// to have a target for the many unused vectors ..				*
void isrnop(void) { }

// Another 'undocumented feature'. What is irq 1 used for?			*
void irq1(void) interrupt 1 {
  BYTE v = VECINT;
  UsbInit();
}

// The real one irg 0:	*
void irq0(void) interrupt 0 using 0 _naked {
  // Es gibt wohl keine vernuenftige Moeglichkeit, den Dispatcher in C zu	*
  // realisieren. Alle Versuche enden in viel zu langem Code oder aber sehr	*
  // 'annoying' warnings. Nun also direkt in Assembler!				*
  // Die Aufrufparameter bedeuten:	*
  // interrupt 0:	'externer IRQ' mit Vec auf 0x0003			*
  // using 0:		wir verwenden RegBank 0 (info an den compiler)		*
  // _naked:		wir wollen keinerlei Prae/Postamble vom Compiler!	*
  _asm ;
  ;
  ; // noch einige Worte zur INTEL-Scheisse:				*
  ; // Der Stack arbeitet falsch herum -> er 'growed' nach oben !!!	*
  ; // Push einer 2byte-Ads (oder ein CALL) sieht auf dem Stack so aus:	*
  ;
  ; //	sp(1) ->	n+2	ads.hi		*
  ; //			n+1	ads.lo		*
  ; //	sp(0) ->	n	???		*
  ;
  ; // Und weiter:
  ; // Als Stack steht maximal der RAM-Bereich von 0x30 bis 0xff (0x7f im 8051)	*
  ; // zur Verf¸gung. Da im unteren Bereich (ab 0x30) einige Arbeitsregister	*
  ; // angelegt werden muessen, wird der nutzbare Stack noch kleiner.		*
  ; // Der Priority Encoder im TUSB verwendet 8 IR-Priorities. Um jeweils alle	*
  ; // Regs zu pushen, werden je priority 13 Byte benˆtigt, was im 'worst case'	*
  ; // 104 Byte zusaetzlicher StackSpace belegt werden. Das reicht hin.		*
  ;
  ; // again, the following is for unINTELligent weasels only:			*
  ; // 'push' und 'pop' kann nur 'direkte' adss als target haben, d.i. die	*
  ; // 'lower 128 bytes of internal ram' und die 'special function registers'	*
  ; // (SFR) von 0x80 bis 0xff.
  push	acc	; // the direct address of the 'memory mapped' accumulator a	*
  push	b	;	// same for 'register' b	*
  push	dpl	;
  push	dph	;
  push	psw	;
  ;
  push	(0+0)	; // the 'direct address' of register 0 within 'bank' 0	*
  push	(0+1)	; // the 'direct address' of register 1 within 'bank' 0	*
  push	(0+2)	;			// a.s.o	*
  push	(0+3)	;
  push	(0+4)	;
  push	(0+5)	;
  push	(0+6)	;
  push	(0+7)	;
  ;
  ; // and again, the following is for unINTELligent weasels only:			*
  ; // Der mindestens ebenso INTELligente Texas-Priority-Vector-Controller haengt	*
  ; // offensichtlich am INTEL-irq0. Dieser ist zwar auf 'level triggered' gesetzt,	*
  ; // was aber nicht heisst, dass er damit vernuenftig funktioniert. Irgendwie muss	*
  ; // ein 'reti' oder ein access ( read oder write (?), INTEL widerspricht sich im	*
  ; // Handbuch hier selbst ..) passieren, damit ein neuer IRQ erkannt wird ?!??	*
  ; // Anyway:	*
  ; // Wir sind hier in der irq0-Routine, und von daher sollte uns die ganze Scheisse	*
  ; // eigentlich nicht k¸mmern. Leider aber gibt es irgendwelche unbekannten (weil	*
  ; // natuerlich nicht dokumentierten) Abhaengigkeiten zwischen dem INTEL- und dem	*
  ; // anscheinend von INTELligenz befruchteten Texas-Priority-Controller.
  ;
  mov	dptr, #VECINT	; get the actual irq vector (0bxxxxxxx0 !)
  movx	a, @dptr	;
  ;
1$:
  movx	@dptr, a	; write something to VECINT to update VECINT
  ;
  mov	dptr, #2$	; the return ads for the isrtn to be called
  push	dpl		; ret.lo ads to stack
  push	dph		; ret.hi ads to stack
  ;
  mov	dptr, #3$	; dptr -> dispatcher table
  anl	a, #0x7e	; a= vec with mapping from 0x80 to (unused) 0x00 !
  mov	b, a		;	save the actual vector
  inc	a		;	dptr+a -> dest.lo in dispatcher table
  movc	a, @a+dptr	; get dest.lo from dispatcher table
  push	acc		;	to stack
  mov	a, b		;	dptr+a -> dest.hi in dispatcher table
  movc	a, @a+dptr	; get dest.hi from dispatcher table
  push	acc		;	to stack
  ret			; -> go service routine (isrtab[vec])
  ;
2$:
  pop	(0+7)	;
  pop	(0+6)	;
  pop	(0+5)	;
  pop	(0+4)	;
  pop	(0+3)	;
  pop	(0+2)	;
  pop	(0+1)	;
  pop	(0+0)	;
  ;
  pop	psw	;
  pop	dph	;
  pop	dpl	;
  pop	b	;
  pop	acc	;
  ;
  reti		;
  ;
  // Die dispatcher table: Die originaeren C-Routinen beginnen mit 'underscore'	*
  // Weiterhin ist 0x80 -> 0x00 gemapped (siehe vor, im asm-code).		*
3$:
  .word	_isrnop	;		// 0x80	DMA1			*
  .word	_isrnop	;		// 0x82	unused			*
  .word	_isrnop	;		// 0x84	DMA3			*
  .word	_isrnop	;		// 0x86	unused			*
  .word	_isrnop	;		// 0x88	unused			*
  .word	_isrnop	;		// 0x8a	unused			*
  .word	_isrnop	;		// 0x8c	unused			*
  .word	_isrnop	;		// 0x8e	unused			*
  .word	_isrnop	;		// 0x10	unused			*
  .word	_isr_usboep1	;		// 0x12	Out EndP 1 transaction complete	*
  .word	_isr_usboep2	;		// 0x14	Out Endp 2 transaction complete	*
  .word	_isr_usboep3	;		// 0x16	Out Endp 3 transaction complete	*
  .word	_isrnop	;		// 0x18	unused			*
  .word	_isrnop	;		// 0x1a	unused			*
  .word	_isrnop	;		// 0x1c	unused			*
  .word	_isrnop	;		// 0x1e	unused			*
  .word	_isrnop	;		// 0x20	unused			*
  .word	_isr_usbiep1	;		// 0x22	Inp EndP 1 transaction complete	*
  .word	_isr_usbiep2	;		// 0x24	Inp Endp 2 transaction complete	*
  .word	_isr_usbiep3	;		// 0x26	Inp Endp 3 transaction complete	*
  .word	_isrnop	;		// 0x28	unused			*
  .word	_isrnop	;		// 0x2a	unused			*
  .word	_isrnop	;		// 0x2c	unused			*
  .word	_isrnop	;		// 0x2e	unused			*
  .word	_isrnop	;		// 0x30	STPOW reveived		*
  .word	_isr_setup	;	// 0x32	SETUP received		*
  .word	_isrnop	;		// 0x34	unused			*
  .word	_isrnop	;		// 0x36	unused			*
  .word	_isr_usbres	;	// 0x38	RESR interrupt		*
  .word	_isr_usbsus	;	// 0x3a	SUSR interrupt		*
  .word	_isr_usbrst	;	// 0x3c	RSTR interrupt		*
  .word	_isrnop	;		// 0x3e	unused			*
  .word	_isrnop	;		// 0x40	I2C TXE interrupt	*
  .word	_isrnop	;		// 0x42	I2C RXF interrupt	*
  .word	_isr_iep0	;		// 0x44	Inp EndP 0 transaction complete	*
  .word	_isr_oep0	;		// 0x46	Out EndP 0 transaction complete	*
  .word	_isrnop	;		// 0x48	unused			*
  .word	_isrnop	;		// 0x4a	unused			*
  .word	_isrnop	;		// 0x4c	unused			*
  .word	_isrnop	;		// 0x4e	unused			*
  .word	_isr_serls ;	// 0x50	UART Line Status, see below	*
  .word	_isr_serms ;	// 0x52	UART Modem Status, see below	*
;  .word	_isrnop	;		// 0x50	UART Line,  INTELligent -> unusable	*
;  .word	_isrnop	;		// 0x52	UART Modem, INTELligent -> unusable	*
  .word	_isrnop	;		// 0x54	unused			*
  .word	_isrnop	;		// 0x56	unused			*
  .word	_isrnop	;		// 0x58	unused			*
  .word	_isrnop	;		// 0x5a	unused			*
  .word	_isrnop	;		// 0x5c	unused			*
  .word	_isrnop	;		// 0x5e	unused			*
;  .word	_isr_serrx	;	// 0x60	UART RXF, see below	*
;  .word	_isr_sertx	;	// 0x62	UART TXE, see below	*
  .word	_isrnop	;		// 0x60	UART RXF, INTELligent -> unusable	*
  .word	_isrnop	;		// 0x62	UART TXE, INTELligent -> unusable	*
  .word	_isrnop	;		// 0x64	unused			*
  .word	_isrnop	;		// 0x66	unused			*
  .word	_isrnop	;		// 0x68	unused			*
  .word	_isrnop	;		// 0x6a	unused			*
  .word	_isrnop	;		// 0x6c	unused			*
  .word	_isrnop	;		// 0x6e	unused			*
  .word	_isrnop	;		// 0x70	unused			*
  .word	_isrnop	;		// 0x72	unused			*
  .word	_isrnop	;		// 0x74	unused			*
  .word	_isrnop	;		// 0x76	unused			*
  .word	_isrnop	;		// 0x78	unused			*
  .word	_isrnop	;		// 0x7a	unused			*
  .word	_isrnop	;		// 0x7c	unused			*
  .word	_isrnop	;		// 0x7e	unused			*
  ;
  _endasm	;
}


void main(void) {

  bUSBCTL = 0;	// disconnect from USB (should be done by bootloader)	*
  EA = 0;	// Interrupts should be disabled by bootloader ...	*
  		// We definitively disable them here again ...		*

  bWDCSR = 0x2a;	// this would disable the watchdog (tested!)	*

  // The following is for unINTELligent weasels only:				*
  // There is something that is called a 'watchdog' in the TUSB3410, but don't	*
  // think of it to be a real watchdog! This special kind of 'watchdog' derives	*
  // its clock from the USB-FramSync -> it runs only if the USB is 'connected'!	*
  // Don't expect any wise word of this silly behaviour in the so called	*
  // 'documentation'. Even worse: The bootcode from Texas Instruments  will not	*
  // work as proclaimed. There is no way to load the firmware to RAM without	*
  // executing it (this part of texas' code does not  work!).			*
  // That means that we have to find another way to program the i2c-chip.	*

  bWDCSR |= WDCSR_WDT;	// a single bone to the watchdog ..	*

  bDMACDR1 =	0;	// disable DMA1 (including interrupt)	*
  bDMACDR3 =	0;	// disable DMA3 (including interrupt)	*
  bUSBMSK =	0;	// disable USB interupts		*
  bMASK1 =	0;	// disable UART interrupts		*

  SerInit();
  
  // bWDCSR |= WDCSR_WDT;	// because usb is disconnected, the watchdog
                                // isn't hungry. See the comment above!
  UsbInit();

  //  IT0	= 0;		// use level triggering for irq0
  

//  _asm
//    clr p3.0	;	// enable dtr-reset of MSP430 (for bmls only)
//  _endasm	;

  EX0 = 1;		// enable irq0
  EA  = 1;		// global interrupt enable
  bUSBCTL = 0x80;	// and finally connect to the USB

  while ( 1 ) {
    bWDCSR |= WDCSR_WDT;	// a single bone to the watchdog ..
  }
}
