/*										*
 * Ser-Interface-Routines							*
 * 										*/

#include <8052.h>
#include "tusb3410.h"
#include "usb.h"
#include "main.h"

//BYTE xdata myvec;
BYTE xdata MCR_STAT;

extern tEDB xdata xEPIDsc[3];
extern tEDB xdata xEPODsc[3];
extern void UsbReset(void);

extern void usbEP2Snd(void * vec, int len);

void isr_serls(void);
void isr_serms(void);

// this structure is used for ioctl transfers between firmware and linux	*
struct line {
  BYTE	len;
  BYTE	divl;
  BYTE	divh;
  BYTE	lcr;
  BYTE	mcr;
  BYTE	fcr;
  BYTE	lsr;
  BYTE	msr;
}	xdata	line;

void SerInit(void) {
  // The uart interrupts seem to be 'edge triggered' (this is very INTELligent)	*
  // and in addition rx and tx have only one common interrupt enable bit ...	*
  // How to overcome this whole shit?						*

  // We just don't use any uart interrupts, because a lot of undocumented	*
  // behaviour was observed when try to do concurrend dma transfers for usb.	*

  MCR_STAT=0;
	
  bMASK1 =	0;	// disable all interrupts (else dma won't work)	*

  bMCR1 =	1;	// initiate an uart reset (what does uart reset mean?)	*
  
  bXON1 =	17;	// just to define, we don't use it	*
  bXOFF1 =	19;	// just to define, we don't use it	*
  
  bDLL1 =	BaudRate460800_DLL;	//	9600			*
  bDLH1 =	BaudRate460800_DLH;	//	9600			*

  bLCR1 =	0x83;	// 8 bit, no par, fifo enable	*
			// dma transfers do not work with fifo disabled !!!	*
			// just another undocumented TUSB feature ...		*

  bFCRL1 =	0;	// no flow control, no rs485	*
  bMCR1 =	0;	// normal mode, rts/dtr clr	*

  bMASK1 |= MASK_MIE;
  bMASK1 |= MASK_SIE;
}

struct line xdata * SerialGet(tDEVICE_REQUEST xdata *req) {
  BYTE fun = req->bRequest;

  switch(fun) {
  case REQUEST_LINE:
    line.divl =	bDLL1;
    line.divh =	bDLH1;
    line.lcr =	bLCR1;
    line.mcr =	bMCR1 ^ 0x20;	// invert rts (as in SerialSet below!)	*
    line.fcr =	bFCRL1;
    line.lsr =	bLSR1;
    line.msr =	bMSR1;
    line.len = 7;
    
    return &line;
  default:
    return 0;
  }
}


void SetBaudRate(BYTE baudno){
 switch(baudno){
	case BaudRate1200:{
	    bDLH1 = BaudRate1200_DLH;
	    bDLL1 = BaudRate1200_DLL;
 	 return;
	}
	case BaudRate2400:{
	    bDLH1 = BaudRate2400_DLH;
	    bDLL1 = BaudRate2400_DLL;
 	 return;
	}
	case BaudRate4800:{
	    bDLH1 = BaudRate4800_DLH;
	    bDLL1 = BaudRate4800_DLL;
 	 return;
	}
	case BaudRate7200:{
	    bDLH1 = BaudRate7200_DLH;
	    bDLL1 = BaudRate7200_DLL;
 	 return;
	}
	case BaudRate9600:{
	    bDLH1 = BaudRate9600_DLH;
	    bDLL1 = BaudRate9600_DLL;
 	 return;
	}
	case BaudRate14400:{
	    bDLH1 = BaudRate14400_DLH;
	    bDLL1 = BaudRate14400_DLL;
 	 return;
	}
	case BaudRate19200:{
	    bDLH1 = BaudRate19200_DLH;
	    bDLL1 = BaudRate19200_DLL;
 	 return;
	}
	case BaudRate38400:{
	    bDLH1 = BaudRate38400_DLH;
	    bDLL1 = BaudRate38400_DLL;
 	 return;
	}
	case BaudRate57600:{
	    bDLH1 = BaudRate57600_DLH;
	    bDLL1 = BaudRate57600_DLL;
 	 return;
	}
	case BaudRate115200:{
	    bDLH1 = BaudRate115200_DLH;
	    bDLL1 = BaudRate115200_DLL;
 	 return;
	}
	case BaudRate230400:{
	    bDLH1 = BaudRate230400_DLH;
	    bDLL1 = BaudRate230400_DLL;
 	 return;
	}
	case BaudRate460800:{
	    bDLH1 = BaudRate460800_DLH;
	    bDLL1 = BaudRate460800_DLL;
 	 return;
	}
	case BaudRate921600:{
	    bDLH1 = BaudRate921600_DLH;
	    bDLL1 = BaudRate921600_DLL;
 	 return;
	}
 default: {
          bDLH1 = BaudRate9600_DLH;
	      bDLL1 = BaudRate9600_DLL;
 	 return;
	}//default
 }//switch
}
    
int SerialSet(tDEVICE_REQUEST xdata *req) {
  BYTE fun = req->bRequest;
  
  switch(fun) {
  case REQUEST_SOF:
//    bDMACDR3 = 0;
//    bDMACDR1 = 0;
    break;
  case REQUEST_SON:
    SerInit();
//    bDMACDR1 = 0x80 | 0x21 ;	// enable DMA, cont mode, no irq, OEP=1	*
//    bDMACDR3 = 0x80 | 0x21 ;	// enable DMA, cont mode, no irq, IEP=1	*
    break;
  case REQUEST_BAUD:
//    bDLH1 = req->bValueH;
  //  bDLL1 = req->bValueL;
	SetBaudRate(req->bValueL);
    break;
  case REQUEST_FCR:
//    bFCRL1 = req->bValueL;
    bFCRL1 = 0;				// force to no flow control !!!
    break;
  case REQUEST_LCR:
    bLCR1 = req->bValueL | 0x80;	// append 'enable fifo'	!!!
    break;
  case REQUEST_RTS:
    if ( !req->bValueL )		// must be inverted !!!
      bMCR1 |= MCR_RTS;
    else
      bMCR1 &= ~MCR_RTS;
    break;
  case REQUEST_DTR:
    if ( req->bValueL )
      bMCR1 |= MCR_DTR;
    else
      bMCR1 &= ~MCR_DTR;
    break;
  case REQUEST_BREAK:
    if ( req->bValueL )
      bLCR1 |= LCR_BRK;
    else
      bLCR1 &= ~LCR_BRK;
    break;
  case REQUEST_CRTSCTS:
   if ( req->bValueL ){
      bFCRL1 |= FCRL_CTS;
      bFCRL1 |= FCRL_RTS;
	}
    else {
      bFCRL1 &= ~FCRL_CTS;
      bFCRL1 &= ~FCRL_RTS;
	}
    break;
	
  default:
	isr_serms();
    return 1;
  }
  return 0;
}


void isr_serls(void) {	// line status
  BYTE xdata	myvec2[2];
  myvec2[1]=0;
  if (bLSR1 & 0x1) myvec2[0]=myvec2[0] | ULCR_OVR;
  if (bLSR1 & 0x2) myvec2[0]=myvec2[0] | ULCR_PTE;
  if (bLSR1 & 0x4) myvec2[0]=myvec2[0] | ULCR_FRE;
  if (bLSR1 & 0x8) myvec2[0]=myvec2[0] | ULCR_BRK;
  
  usbEP2Snd((BYTE*)&myvec2,2);
  
  if ( bLSR1 & 0x0f ) {
    bLSR1 |= 0x0f;		// clear any rx error		*
    //    bDMACSR3 |= 0x03;
    //    bDMACDR3 |= 0x80;		// restart dma			*
  }
}

void isr_serms(void) {	// modem status
  BYTE xdata	myvec2[2];
/*
	myvec2[0]=0;
        myvec2[1]=bMSR1;
        usbEP2Snd((BYTE*)&myvec2,2);
*/
	
 if (bMSR1 & 0x1) {
	 MCR_STAT = MCR_STAT ^ MCR_CTS;
	 bMSR1 |=0x1;
 }
 if (bMSR1 & 0x2) {
	 MCR_STAT = MCR_STAT ^ MCR_DSR;
	 bMSR1 |=0x2;
 }
 if (bMSR1 & 0x4) {
	 MCR_STAT = MCR_STAT ^ MCR_RI;
	 bMSR1 |=0x4;
 }
 if (bMSR1 & 0x8) {
	 MCR_STAT = MCR_STAT ^ MCR_CD;
	 bMSR1 |=0x8;
 }
	
 myvec2[0]=0;
 myvec2[1]=MCR_STAT;
// myvec2[1]=bMSR1;
 usbEP2Snd((BYTE*)&myvec2,2);
}


/* The following interrupt routines and standard i/o routines are commented out	*
 * because they conflict with the USB/DMA/UART system. No way to find out what	*
 * is really going on. Some hardware structures of the TUSB seem to be secret.	*
 *										*
 * We leave this code in place because it was helpfull during initial testing.	*
 * May be it can be reused in future.						*/


/*
 //Interrupt Service Routines, called from dispatcher in 'main.c'		*

void isr_serls(void) {	// line status
  if ( bLSR1 & 0x0f ) {
    bLSR1 |= 0x0f;		// clear any rx error		*
    bDMACSR3 |= 0x03;
    bDMACDR3 |= 0x80;		// restart dma			*
  }
}

void isr_serms(void) {	// modem status
}

void isr_sertx(void) {
  if ( !(txop == txip) ) {		// there is something to be sent	*
    bTDR1 = txbuf[txop];
    txop++;
  }
}

void isr_serrx(void) {
  *(rxbuf+rxip) = bRDR1;	// just queue to buffer	*
  rxip++;
  if ( rxip == rxop) rxip--;
}

 // Basic C Library Interface Routines						*

void putchar(char c) {
  if ( bMASK1 && 4 ) {	// if UART-Interrupts are enabled do:	*
    txbuf[txip] = c;	// just queue the character	*
    txip++;			//			*
    _asm
      push IE;		// if we would have a normal unINTELligent hardware,	*
      clr EA;		// we would (re)enable the tx interrupts an things	*
    _endasm;		// would work. But because we have INTELligent hardware	*
    if ( bLSR1 & 0x20 )	// the only way is to restart the transmitter if and	*
      isr_sertx();	// only if the tdone flag is set. And we have to do	*
    _asm pop IE; _endasm;	// this with disabled interrupts.			*
  }
  else {		// if UART-Interrupts are disabled do:	*
    while ( !(bLSR1 & 0x20) )	// wait for txdr empty		*
      bWDCSR |= WDCSR_WDT;	// with feeding the watchdog	*
    bTDR1 = c;
  }
}

char getchar(void) {
  char c;
  if ( bMASK1 && 4 ) {	// if UART-Interrupts are enabled do:	*
    if ( rxop == rxip ) return 0;
    c = *(rxbuf+rxop);
    rxop++;
    return c;
  }
  c = 0;		// if UART-Interrupts are disabled do:	*
  if ( bLSR1 & 0x10 )		// if UART has data ..	*
    c = bRDR1;			// .. do read this data	*
  return c;
}

// Local Subroutines		*

void wrchr(char c) {
  putchar(c);
}

void wrhexb(int n) {
  char c = n & 0x0f;
  c += ( c < 10 ) ? '0' : 'a'-10 ;
  putchar(c);
}

void wrhexw(int n) {
  wrhexb( n >> 4);
  wrhexb( n );
}

void wrstr(code char *s) {
  while ( *s ) putchar(*s++);
}
 */
