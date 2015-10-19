/*										*
 * USB-Interface-Routines							*
 * 										*/

#include <8052.h>
#include "tusb3410.h"
#include "usb.h"

extern int SerialSet(tDEVICE_REQUEST xdata *);
extern xdata * SerialGet(tDEVICE_REQUEST xdata *);

/*----------------------------------------------------------------------------+
| Internal Constant Definition                                                |
+----------------------------------------------------------------------------*/
#define NO_MORE_DATA              0xFFFF
#define USB_RETURN_DATA_LENGTH    4

#define INTR_PACKET_SIZE          0x40

/* Falls sich einer wundert:								*
 * Die folgenden Descriptoren sollten eigentlich die bereits irgendwo definierten	*
 * Strukturen sein. Nur: Aus irgendwelchen Gruenden lassen sich keine Strukturen	*
 * im 'code segment' anlegen. Deswegen hier Kopien der Strucs als 'byte arrays'.	*
 * INTELligent, nicht wahr?								*
 *											*
 * Und das mir sonst keiner Fragen stellt:						*
 * INTELligenterweise koennen weder DMA- noch USB-Controller diese Daten im		*
 * 'code segment' lesen. Sie werden daher beim 'init' in das ominoese 'xram' kopiert.	*
 * --- Also: Keine Fragen, Hauser!							*/

code BYTE UsbDevDsc[] = {
#if 1
#include "tusb3410_device_descriptor.inc"
#else
  sizeof(tDEVICE_DESCRIPTOR),	// Length of this descriptor (12h bytes)
  DESC_TYPE_DEVICE,		// Type code of this descriptor (01h)
    0x10,0x01,			// Release of USB spec (Rev 1.1)
    0xff,			// Device's base class code - vendor specific
    0,				// Device's sub class code
    0,				// Device's protocol type code
    EP0_PACKET_SIZE,		// End point 0's packet size = 8
    0x51,0x04,			// Vendor ID for device (lo,hi) TI=0x0451
//  0x14,0x14,			// Product ID for device (lo,hi)
//  0x10,0x34,			// Product ID for device (lo,hi)
//  0x30,0xf4,			// Product ID for device (lo,hi)
    0xef,0xbe,			// Product ID for device (lo,hi)
    0x01,0x00,			// Revision level of device (lo,hi: Rev=0.1)
    1,				// Index of manufacturer name string desc
    2,				// Index of product name string desc
    3,				// Index of serial number string desc
    2				// Number of configurations supported
#endif
};

/* Configuration Descriptor:  laut USB-Spec (gut im Text versteckt: USB2.0-S253/254)	*
 * sieht die Sache so aus:								*
 * cfg-dsc,ifa-dsc,epa-dsc[,epb-dsc[,ep...]][,ifa-dsc,epa-dsc[,epb-dsc[,ep...]]][,..]	*
 *											*/

BYTE code UsbFunDsc[] = {
#if 1
#include "tusb3410_config_descriptor.inc"
#else
  // Configuration  Descriptor (the one and only one!)		*
  sizeof(tCONFIG_DESCRIPTOR),	// bLength
  DESC_TYPE_CONFIG,		// bDescriptorType
    0xff, 0x00,			// wTotalLength -- size data will be inserted at copy time
    0x01,			// bNumInterfaces
    0x02,			// bConfigurationValue
    0x00,			// iConfiguration
    0x80,			// bmAttributes, bus bootcode
    0x32,			// Max. Power Consumption at 2mA unit
  // Interface Descriptor, size = 0x09
  sizeof(tINTERFACE_DESCRIPTOR),	// bLength
  DESC_TYPE_INTERFACE,		// bDescriptorType
    0x00,			// bInterfaceNumber
    0x00,			// bAlternateSetting
    3,				// bNumEndpoints
    0xFF,			// bInterfaceClass - vendor-specific
    0,				// bInterfaceSubClass, zero for hub
    0,				// bInterfaceProtocol
    0x00,			// iInterface
  // Endpoint Descriptor, size = 0x07 for OEP1
  sizeof(tENDPOINT_DESCRIPTOR),	// bLength
  DESC_TYPE_ENDPOINT,		// bDescriptorType
    0x01,			// bEndpointAddress; bit7=1 for IN, bits 3-0=1 for ep1
    EP_DESC_ATTR_TYPE_BULK,	// bmAttributes, bulk transfer
    0x40, 0x00,			// wMaxPacketSize, 64 bytes
    0x00,			// bInterval
  // Endpoint Descriptor, size = 0x07 for IEP1
  sizeof(tENDPOINT_DESCRIPTOR),	// bLength
  DESC_TYPE_ENDPOINT,		// bDescriptorType
    0x81,			// bEndpointAddress; bit7=1 for IN, bits 3-0=1 for ep1
    EP_DESC_ATTR_TYPE_BULK,	// bmAttributes, bulk transfer
    0x40, 0x00,			// wMaxPacketSize, 64 bytes
    0x00,			// bInterval
  // Endpoint Descriptor, size = 0x07 for IEP2
  sizeof(tENDPOINT_DESCRIPTOR),	// bLength
  DESC_TYPE_ENDPOINT,		// bDescriptorType
    0x82,			// bEndpointAddress; bit7=1 for IN, bits 3-0=1 for ep1
    EP_DESC_ATTR_TYPE_INT,	// bmAttributes, bulk transfer
//    0x01, 0x00,			// wMaxPacketSize, 1 byte
    0x40,0x00,
    0x00,			// bInterval
#endif
};

code BYTE UsbStrDsc[] = {
#if 1
#include "tusb3410_string_descriptor.inc"
#else
  // string index 0, language support
  4,
    3,
    0x09,0x04,				// 0x0409 for English
  // string index 1, manufacturer
  34,					// Length of this string descriptor
  DESC_TYPE_STRING,
    'U',0x00,'A',0x00,'R',0x00,'T',0x00,'2',0x00,'U',0x00,
    'S',0x00,'B',0x00,' ',0x00,'a',0x00,'d',0x00,'a',0x00,
    'p',0x00,'t',0x00,'e',0x00,'r',0x00,
  // string index 2, product
  42,					// Length of this string descriptor
  DESC_TYPE_STRING,
    'T',0x00,'U',0x00,'S',0x00,'B',0x00,'3',0x00,'4',0x00,'1',0x00,'0',0x00,' ',0x00,
    'B',0x00,'o',0x00,'o',0x00,'t',0x00,' ',0x00,
    'D',0x00,'e',0x00,'v',0x00,'i',0x00,'c',0x00,'e',0x00,
  // string index 3, serial number
  34,					// Length of this string descriptor
    DESC_TYPE_STRING,
    'B',0x00,'M',0x00,'L',0x00,'S',0x00,
    '0',0x00,'0',0x00,'0',0x00,'0',0x00,
    ' ',0x00,' ',0x00,' ',0x00,' ',0x00,
    ' ',0x00,' ',0x00,' ',0x00,' ',0x00
#endif
};

/*----------------------------------------------------------------------------+
| Internal Variables                                                          |
+----------------------------------------------------------------------------*/
//-----typ-----|-where-|-name-->


WORD		idata	wEP2SndCnt;     // Number of bytes left to send via EP0
xdata BYTE *	idata	wEP2SndPtr;	// pointer for InpEndPnt2-(snd)-tranfers
//xdata BYTE *	idata	sndbuf;	// pointer for InpEndPnt2-(snd)-tranfers
bit			fEP2Snd;	// set if Control Send Transfer active
BYTE xdata 	at 	0xfe00 xEP2SndBuff[64];

BYTE		idata	bActCfg;	// 0 if USB device is not configured
BYTE		idata	bActIfc;	// interface number

WORD		idata	wEP0SndCnt;     // Number of bytes left to send via EP0
WORD		idata	wEP0RcvCnt;     // Number of bytes left to recv via EP0
xdata BYTE *	idata	wEP0SndPtr;	// pointer for InpEndPnt0-(snd)-tranfers
xdata BYTE *	idata	wEP0RcvPtr;	// pointer for OutEndPnt0-(rcv)-tranfers

bit			fEP0Snd;	// set if Control Send Transfer active
bit			fEP0Rcv;	// set if Control Rcvr Transfer active

// Die folgenden Buffer werden direkt von der Hardware gelesen/beschrieben:	*

//	0xff60 .. 0xff7f	reserved
tEDB		xdata at 0xff48	xEPIDsc[3];
//	0xff20 .. 0xff47	reserved
tEDB		xdata at 0xff08	xEPODsc[3];
tDEVICE_REQUEST	xdata at 0xff00	tSetupPacket;
BYTE		xdata at 0xfef8	xEP0SndBuf[EP0_PACKET_SIZE];
BYTE		xdata at 0xfef0	xEP0RcvBuf[EP0_PACKET_SIZE];

//	0xfe00 .. 0xfeef	not used
BYTE		xdata at 0xfb00	xDMABuffers[0x300];	// 6*2*64 byte (6 Endpoints)

BYTE		xdata	xUsbDevDsc[sizeof(UsbDevDsc)];
BYTE		xdata	xUsbFunDsc[sizeof(UsbFunDsc)];
BYTE		xdata	xUsbStrDsc[sizeof(UsbStrDsc)];

BYTE		xdata	abUsbRequestReturnData[USB_RETURN_DATA_LENGTH];
BYTE xdata at 0xfe00 xDMABuff1[64];
/*----------------------------------------------------------------------------+
| General Subroutines                                                         |
+----------------------------------------------------------------------------*/





//----------------------------------------------------------------------------
void usbEP2SndNxt(void) {
  // Der InputEndPoint0 liefert Daten an den Host -> Send !	*
  // Diese Routine wird initial und dann aus dem irq aufgerufen	*
  BYTE i,siz= 0;
  fEP2Snd = 0;	// init: we don't want to be called on next send complete irq	*

    for ( i=0; i<65; i++ ) {
     xEP2SndBuff[i]=0;
    }
    
  if ( wEP2SndCnt ) {				// we have some data to send
    if ( wEP2SndCnt > EP_MAX_PACKET_SIZE ) {	// but more than will fit
      siz = EP_MAX_PACKET_SIZE;			// we do send with maximum size
      wEP2SndCnt -= EP_MAX_PACKET_SIZE;		// and do count this one
      fEP2Snd = 1;				// -> we have to cont on next irq
    } else {		// number of bytes fits: we do send all remaining bytes
      siz = (BYTE)wEP2SndCnt;
      wEP2SndCnt = 0;
    }
    for ( i=0; i<siz; i++ ) {
      xEP2SndBuff[i] = *wEP2SndPtr++;  
	}//for
  }
 xEPIDsc[1].bEPBCTX = siz;
}

//----------------------------------------------------------------------------
void usbEP2Snd(void * vec, int len) {
//  int req = ((WORD)tSetupPacket.bLengthH << 8) | tSetupPacket.bLengthL;
  int req = EP_MAX_PACKET_SIZE;
  wEP2SndPtr = vec;
  if ( req > len ) req = len;	// limit req-len to the len we actually have	*
  wEP2SndCnt = req;		// setup the transfer length
  // den Grund hierfuer muss ich mir noch einmal ueberlegen:
  xEPIDsc[1].bEPBCTX =0;		// for status stage
  usbEP2SndNxt();
}

//----------------------------------------------------------------------------
void usbEP0Stall(void) {
  bIEPCNFG0 |= EPCNF_STALL;
  bOEPCNFG0 |= EPCNF_STALL;
}

//----------------------------------------------------------------------------
void usbEP0SndNxt(void) {
  // Der InputEndPoint0 liefert Daten an den Host -> Send !	*
  // Diese Routine wird initial und dann aus dem irq aufgerufen	*
  BYTE i, siz= 0;
  fEP0Snd = 0;	// init: we don't want to be called on next send complete irq	*

  if ( wEP0SndCnt ) {				// we have some data to send
    if ( wEP0SndCnt > EP0_PACKET_SIZE ) {	// but more than will fit
      siz = EP0_PACKET_SIZE;			// we do send with maximum size
      wEP0SndCnt -= EP0_PACKET_SIZE;		// and do count this one
      fEP0Snd = 1;				// -> we have to cont on next irq
    } else {		// number of bytes fits: we do send all remaining bytes
      siz = (BYTE)wEP0SndCnt;
      wEP0SndCnt = 0;
    }
    for ( i=0; i<siz; i++ ) 
      xEP0SndBuf[i] = *wEP0SndPtr++;
  }
  bIEPBCNT0 = siz;	// clr NAK and setup byte count (may be 0!)
}

//----------------------------------------------------------------------------
void usbEP0Snd(void * vec, int len) {
  int req = ((WORD)tSetupPacket.bLengthH << 8) | tSetupPacket.bLengthL;
  wEP0SndPtr = vec;
  if ( req > len ) req = len;	// limit req-len to the len we actually have	*
  wEP0SndCnt = req;		// setup the transfer length
  // den Grund hierfuer muss ich mir noch einmal ueberlegen:
  bOEPBCNT0 = 0;		// for status stage
  usbEP0SndNxt();
}

//----------------------------------------------------------------------------
void usbEP0SndNul(void) {
  fEP0Snd	= 0;	// no completion call from send done irq
  wEP0SndCnt	= 0;	// 0 bytes to send
  bIEPBCNT0	= 0;	// enable ACK response with len=0 at hardware
}

//----------------------------------------------------------------------------
void usbEP0RcvNxt(void)
{
  bOEPCNFG0 |= EPCNF_STALL;
  // bootcode does not use this.
#if 0
  BYTE bIndex,bByte;

  bByte = bOEPBCNT0 & EPBCT_BYTECNT_MASK;

  if(wEP0RcvCnt >= (WORD)bByte){
    for(bIndex=0;bIndex<bByte;bIndex++)
      *wEP0RcvPtr++ = xEP0RcvBuf[bIndex];

    wEP0RcvCnt -= (WORD)bByte;

    // clear the NAK bit for next packet
    if(wEP0RcvCnt > 0){
      bOEPBCNT0 = 0;        
      fEP0Rcv = 1;
    }else {
      bOEPCNFG0 |= EPCNF_STALL;
      fEP0Rcv = 1;
    }
  }else{
    bOEPCNFG0 |= EPCNF_STALL;
    fEP0Rcv = 1;

  }
#endif
}

//----------------------------------------------------------------------------
/*
void usbEP0Rcv(BYTE * pbBuffer)
{

    wEP0RcvPtr = pbBuffer;

    wEP0RcvCnt = 
        (WORD)(tSetupPacket.bLengthH << 8) | (WORD)tSetupPacket.bLengthL;
    fEP0Rcv = 1;

    bOEPBCNT0 = 0;            

}
*/

//----------------------------------------------------------------------------
void usbClearEndpointFeature(void) {
  BYTE bEndpointNumber;

  // EP is from EP1 to EP7 while C language start from 0    
  bEndpointNumber = (tSetupPacket.bIndexL & EP_DESC_ADDR_EP_NUM);
  if(bEndpointNumber == 0x00)
    return usbEP0SndNul();
  bEndpointNumber--;    
  if(bEndpointNumber < MAX_ENDPOINT_NUMBER){
    if((tSetupPacket.bIndexL & EP_DESC_ADDR_DIR_IN) == EP_DESC_ADDR_DIR_IN){
      // input endpoint
      xEPIDsc[bEndpointNumber].bEPCNF &= ~(EPCNF_STALL | EPCNF_TOGLE);
    } else {
      // output endpoint
      xEPODsc[bEndpointNumber].bEPCNF &= ~(EPCNF_STALL | EPCNF_TOGLE);
    }
    usbEP0SndNul();
  }
  bWDCSR |= WDCSR_WDT;	// a single bone to the watchdog ..	*
}

//----------------------------------------------------------------------------
void usbGetConfiguration(void) {
  usbEP0Snd((BYTE *)&bActCfg, 1);
}

//----------------------------------------------------------------------------
void usbGetDeviceDescriptor(void) {
  usbEP0Snd((BYTE *)&xUsbDevDsc, sizeof(tDEVICE_DESCRIPTOR));
}

//----------------------------------------------------------------------------
void usbGetConfigurationDescriptor(void) {
  usbEP0Snd((BYTE *)&xUsbFunDsc, sizeof(UsbFunDsc));
}

//----------------------------------------------------------------------------
void usbGetStringDescriptor(void) {
  WORD i = 0;
  while(tSetupPacket.bValueL-- >  0x00) i += xUsbStrDsc[i];
  usbEP0Snd((BYTE *)&xUsbStrDsc[i], xUsbStrDsc[i]);
}

//----------------------------------------------------------------------------
void usbGetInterface(void) {
  // not fully supported, return one byte, zero
  abUsbRequestReturnData[0] = bActIfc;
  usbEP0Snd((BYTE *)&abUsbRequestReturnData[0], 1);
}

//----------------------------------------------------------------------------
void usbGetDeviceStatus(void) { 
  if((xUsbFunDsc[OFFSET_CONFIG_DESCRIPTOR_POWER] & 
      CFG_DESC_ATTR_SELF_POWERED) == CFG_DESC_ATTR_SELF_POWERED) 
    abUsbRequestReturnData[0] = DEVICE_STATUS_SELF_POWER;
  if((xUsbFunDsc[OFFSET_CONFIG_DESCRIPTOR_POWER] & 
      CFG_DESC_ATTR_REMOTE_WAKE) == CFG_DESC_ATTR_REMOTE_WAKE) 
    abUsbRequestReturnData[0] |= DEVICE_STATUS_REMOTE_WAKEUP;
    // Return self power status and remote wakeup status
    usbEP0Snd((BYTE *)&abUsbRequestReturnData[0], 2);
}

//----------------------------------------------------------------------------
void usbGetInterfaceStatus(void) {
  // check bIndexL for index number (not supported)
  // Return two zero bytes
  usbEP0Snd((BYTE *)&abUsbRequestReturnData[0], 2);
}

//----------------------------------------------------------------------------
void usbGetEndpointStatus(void) {
  BYTE bEndpointNumber;

  // Endpoint number is bIndexL
  bEndpointNumber = tSetupPacket.bIndexL & EP_DESC_ADDR_EP_NUM;
  
  if (bEndpointNumber == 0x00) {
    if ((tSetupPacket.bIndexL & EP_DESC_ADDR_DIR_IN) == EP_DESC_ADDR_DIR_IN) {
      // input endpoint 0
      abUsbRequestReturnData[0] = (BYTE)(bIEPCNFG0 & EPCNF_STALL);
    } else {
      // output endpoint 0
      abUsbRequestReturnData[0] = (BYTE)(bOEPCNFG0 & EPCNF_STALL);
    }
    abUsbRequestReturnData[0] = abUsbRequestReturnData[0] >> 3; // STALL is on bit 3
    bOEPBCNT0 = 0;                    // for status stage    
    usbEP0Snd((BYTE *)&abUsbRequestReturnData[0], 2);
  } else {
    bEndpointNumber--;
    // EP is from EP1 to EP7 while C language start from 0
    // Firmware should NOT response if specified endpoint is not supported. (charpter 8)
    if (bEndpointNumber < MAX_ENDPOINT_NUMBER) {
      if (tSetupPacket.bIndexL & EP_DESC_ADDR_DIR_IN) {
	// input endpoint
	abUsbRequestReturnData[0] = (BYTE)(xEPIDsc[bEndpointNumber].bEPCNF & EPCNF_STALL);
      } else {
	// output endpoint
	abUsbRequestReturnData[0] = (BYTE)(xEPODsc[bEndpointNumber].bEPCNF & EPCNF_STALL);
      }
    }   // no response if endpoint is not supported.
    abUsbRequestReturnData[0] = abUsbRequestReturnData[0] >> 3; // STALL is on bit 3        
    bOEPBCNT0 = 0;
    usbEP0Snd((BYTE *)&abUsbRequestReturnData[0], 2);
  } 
}

//----------------------------------------------------------------------------
void usbSetAddress(void) {
  bOEPCNFG0 |= EPCNF_STALL;	// control write without data stage
  // bValueL contains device address
  if ( tSetupPacket.bValueL < 128 ) {
    // hardware will update the address after status stage
    // therefore, firmware can set the address now.
    bFUNADR = tSetupPacket.bValueL;         
    usbEP0SndNul();
  }
  else usbEP0Stall();
}

//----------------------------------------------------------------------------
void usbSetConfiguration(void) {
  bOEPCNFG0 |= EPCNF_STALL;	// control write without data stage
  // configuration number is in bValueL
  // change the code if more than one configuration is supported
  bActCfg = tSetupPacket.bValueL;
  usbEP0SndNul();
}

//----------------------------------------------------------------------------
void usbSetDeviceFeature(void) {
  // stall because bootcode does not support
  usbEP0Stall();
    
  // bValueL contains feature selector
//    if(tSetupPacket.bValueL == FEATURE_REMOTE_WAKEUP){
//        xUsbFunDsc[OFFSET_CONFIG_DESCRIPTOR_POWER] 
//        |= CFG_DESC_ATTR_REMOTE_WAKE;
//        usbEP0SndNul();        
//    }else usbEP0Stall();
}


//----------------------------------------------------------------------------
void usbSetEndpointFeature(void) { 
  BYTE EPNum;
  bOEPCNFG0 |= EPCNF_STALL;	// control write without data stage

  // wValue contains feature selector
  // bIndexL contains endpoint number
  // Endpoint number is in low byte of wIndex
  if ( tSetupPacket.bValueL == FEATURE_ENDPOINT_STALL ) {
    EPNum = tSetupPacket.bIndexL & EP_DESC_ADDR_EP_NUM;
    if ( EPNum == 0x00 )
      usbEP0SndNul();	// do nothing for endpoint 0
    else {
      EPNum--;
      // Firmware should NOT response if specified endpoint is not supported. (charpter 8)        
      if ( EPNum < MAX_ENDPOINT_NUMBER ) {
	if ( tSetupPacket.bIndexL & EP_DESC_ADDR_DIR_IN ) {
	  // input endpoint
	  xEPIDsc[EPNum].bEPCNF |= EPCNF_STALL;
	} else {
	  // output endpoint
	  xEPODsc[EPNum].bEPCNF |= EPCNF_STALL;
	}
	usbEP0SndNul();                            
      } // no response if endpoint is not supported.
    }
  } else
    usbEP0Stall();
}

//----------------------------------------------------------------------------
void usbSetInterface(void) {
  bOEPCNFG0 |= EPCNF_STALL;	// control write without data stage
  // bValueL contains alternative setting
  // bIndexL contains interface number
  // change code if more than one interface is supported
  bActIfc = tSetupPacket.bIndexL;
  usbEP0SndNul();
}

//----------------------------------------------------------------------------
void usbInvalidRequest(void) {
  // check if setup overwrite is set
  // if set, do nothing since we might decode it wrong
  // setup packet buffer could be modified by hardware if another setup packet
  // was sent while we are deocding setup packet
  if ( (bUSBSTA & USBSTA_STPOW) == 0x00 ) usbEP0Stall();
}

void usbVendorSetRequest(void) {
  bOEPCNFG0 |= EPCNF_STALL;	// control write without data stage
  if ( SerialSet(&tSetupPacket) != 0 )
    usbEP0Stall();
  else
    usbEP0SndNul();
}

void usbVendorGetRequest(void) {
  BYTE xdata * vec;
  int len;
  if ( ( vec = (BYTE xdata *)SerialGet(&tSetupPacket)) ) {
    len = *(vec);
    usbEP0Snd(vec+1, len);
  }
  else
    usbEP0Stall();
}

void usbVendorRequest(void) {
  if ( tSetupPacket.bmRequestType & USB_REQ_TYPE_INPUT )
    usbVendorGetRequest();
  else
    usbVendorSetRequest();
}

/* CDC request */

struct cdc_line {
  BYTE DTERate0;
  BYTE DTERate1;
  BYTE DTERate2;
  BYTE DTERate3;
  BYTE CharFormat;      /* 0: 1 stop, 1: 1.5 stop, 2: 2 stop */
  BYTE ParityType;      /* 0: None, 1: Odd, 2: Even, 3: Mask, 4: Space */
  BYTE DataBits;        /* 5, 6, 7, 8 or 16 */
};

code struct cdc_line cdc_line = {
  460800, 460800>>8, 460800>>16, 460800>>24,
  0, 0, 8,
};

void usbGetLineCoding()       { usbEP0Snd(&cdc_line, 7); }
void usbSetLineCoding()       { bOEPCNFG0 |= EPCNF_STALL; usbEP0SndNul(); }
void usbSetControlLineState() { bOEPCNFG0 |= EPCNF_STALL; usbEP0SndNul(); }

/* ----------- */

BYTE BTOC(BYTE x) { return x > 9 ? 'A' + x - 0x0a : '0' + x; }

void usbSetSerNum()
{
  int i, j;

  i = xUsbDevDsc[16];  /* index of string descriptor for serial number string */
  if (i == 0) return;  /* zero means no serial number string */

  j = 0;
  while (i-- > 0)  j += xUsbStrDsc[j];
    
  i = xUsbStrDsc[j]; j += 2;  /* Length of string */
  if (i >= 32) {
    xUsbStrDsc[j] = BTOC(bSERNUM7 >> 4);     j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM7 & 0x0f);   j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM6 >> 4);     j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM6 & 0x0f);   j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM5 >> 4);     j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM5 & 0x0f);   j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM4 >> 4);     j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM4 & 0x0f);   j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM3 >> 4);     j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM3 & 0x0f);   j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM2 >> 4);     j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM2 & 0x0f);   j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM1 >> 4);     j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM1 & 0x0f);   j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM0 >> 4);     j += 2;
    xUsbStrDsc[j] = BTOC(bSERNUM0 & 0x0f);   j += 2;
  }
}


/*										*
 * Parsing of a setup packet:							*
 *										*
 * This structure just helps to define the entries within the following array:	*/
typedef struct _tspp {
  BYTE	bmRequestType;		// See bit definitions below
  BYTE	bRequest;		// See value definitions below
  BYTE	bValueL;		// Meaning varies with request type
  BYTE	bValueH;		// Meaning varies with request type
  BYTE	bIndexL;		// Meaning varies with request type
  BYTE	bIndexH;		// Meaning varies with request type
  BYTE	bLengthL;		// Number of bytes of data to transfer (LSByte)
  BYTE	bLengthH;		// Number of bytes of data to transfer (MSByte)
  BYTE	bCompareMask;		// MSB is bRequest, if set 1, bRequest should be matched
  void	(*pUsbFunction)(void);	// function pointer
} tspp, *ptspp;

/* This is the definition table which uses the above structure:			*/
code tspp tUsbReqDef[] = {
    //
    // the following listing are for vendor specific USB requests
    //
  {	// vendor specific requests
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE,
    0xff,                       // slect all vendor specific request
    0xff,0xff,
    0xff,0xff,
    0xff,0xff,
    0x80,&usbVendorRequest
  },
  {	// vendor specific requests
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE,
    0xff,                       // slect all vendor specific request
    0xff,0xff,
    0xff,0xff,
    0xff,0xff,
    0x80,&usbVendorRequest
  },

    //    
    // the following listing are for standard USB requests
    //    
    // clear device feature
//    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE, 
//    USB_REQ_CLEAR_FEATURE, 
//    FEATURE_REMOTE_WAKEUP,0x00,
//    0x00,0x00,
//    0x00,0x00,
//    0xff,&usbEP0Stall,

    // clear interface feature
//    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
//    USB_REQ_CLEAR_FEATURE,
//    0xff,0x00,
//    0xff,0x00,
//    0x00,0x00,
//    0xe7,&usbEP0Stall,

  {    // clear endpoint feature
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_ENDPOINT,
    USB_REQ_CLEAR_FEATURE,
    FEATURE_ENDPOINT_STALL,0x00,
    0xff,0x00,
    0x00,0x00,
    0xf7,&usbClearEndpointFeature
  },
  {	// get configuration
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
    USB_REQ_GET_CONFIGURATION,
    0x00,0x00,
    0x00,0x00,
    0x01,0x00,
    0xff,&usbGetConfiguration
  },
  {    // get device descriptor
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
    USB_REQ_GET_DESCRIPTOR,
    0xff,DESC_TYPE_DEVICE,                  // bValueL is index and bValueH is type
    0xff,0xff,
    0xff,0xff,
    0xd0,&usbGetDeviceDescriptor
  },
  {    // get configuration descriptor
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
    USB_REQ_GET_DESCRIPTOR,
    0xff,DESC_TYPE_CONFIG,                  // bValueL is index and bValueH is type
    0xff,0xff,
    0xff,0xff,
    0xd0,&usbGetConfigurationDescriptor
  },
  {    // get string descriptor
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
    USB_REQ_GET_DESCRIPTOR,
    0xff,DESC_TYPE_STRING,                  // bValueL is index and bValueH is type
    0xff,0xff,
    0xff,0xff,
    0xd0,&usbGetStringDescriptor
  },
  {    // get interface
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
    USB_REQ_GET_INTERFACE,
    0x00,0x00,
    0xff,0xff,
    0x01,0x00,
    0xf3,&usbGetInterface
  },
  {    // get device status
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
    USB_REQ_GET_STATUS,
    0x00,0x00,
    0x00,0x00,
    0x02,0x00,
    0xff,&usbGetDeviceStatus
  },
  {    // get interface status
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
    USB_REQ_GET_STATUS,
    0x00,0x00,
    0xff,0x00,
    0x02,0x00,
    0xf7,&usbGetInterfaceStatus
  },
  {    // get endpoint status
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_ENDPOINT,
    USB_REQ_GET_STATUS,
    0x00,0x00,
    0xff,0x00,
    0x02,0x00,
    0xf7,&usbGetEndpointStatus
  },
  {    // set address
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
    USB_REQ_SET_ADDRESS,
    0xff,0x00,
    0x00,0x00,
    0x00,0x00,
    0xdf,&usbSetAddress
  },
  {    // set configuration
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
    USB_REQ_SET_CONFIGURATION,
    0xff,0x00,
    0x00,0x00,
    0x00,0x00,
    0xdf,&usbSetConfiguration
  },

    // set descriptor (Bootcode stalls to this request)
//    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
//    USB_REQ_SET_DESCRIPTOR,
//    0xff,0xff,                      // descriptor type and descriptor index
//    0xff,0xff,                      // language ID
//    0xff,0xff,                      // desciprotr length
//    0xc0,&usbEP0Stall,

  {    // set device feature
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE,
    USB_REQ_SET_FEATURE,
    0xff,0x00,                      // feature selector
    0x00,0x00,
    0x00,0x00,
    0xdf,&usbSetDeviceFeature
  },

    // set interface feature
//    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
//    USB_REQ_SET_FEATURE,
//    0xff,0x00,
//    0xff,0x00,
//    0x00,0x00,
//    0xd7,&usbEP0Stall,

  {    // set endpoint feature
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_ENDPOINT,
    USB_REQ_SET_FEATURE,
    0xff,0x00,                      // feature selector
    0xff,0x00,                      // endpoint number <= 127 
    0x00,0x00,
    0xd7,&usbSetEndpointFeature
  },

  {    // set interface
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
    USB_REQ_SET_INTERFACE,
    0xff,0x00,                      // feature selector
    0xff,0x00,                      // interface number
    0x00,0x00,
    0xd7,&usbSetInterface
  },

    // synch frame (Bootcode stalls to this request)
//    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
//    USB_REQ_SYNCH_FRAME,
//    0x00,0x00,
//    0xff,0x00,                      // endpoint number
//    0x02,0x00,
//    0xf7,&usbEP0Stall,

  /* CDC request */
  {
    USB_REQ_TYPE_INPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
    USB_CDC_GET_LINE_CODING,
    0x00,0x00,
    0x00,0x00,
    0x07,0x00,
    0xff,&usbGetLineCoding,
  },

  {
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
    USB_CDC_SET_LINE_CODING,
    0x00,0x00,
    0x00,0x00,
    0x07,0x00,
    0xff,&usbSetLineCoding,
  },

  {
    USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
    USB_CDC_SET_CONTROL_LINE_STATE,
    0xff,0xff,
    0x00,0x00,
    0x00,0x00,
    0xcf,&usbSetControlLineState,
  },

  {    //
    // end of usb descriptor -- this one will be matched to any USB request
    //                          since bCompareMask is 0x00.
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0x00,&usbInvalidRequest
  }	// end of list
};

/* The parser routine that compares the request against the definition table above	*/
void usbSetupCompare(void) {
  BYTE bMask,bResult,i;
  BYTE *def, buf[8];
  BYTE typ, req;

  // copy setup packet to idata to speed up decoding
  for (i = 0; i < 8; i++)	buf[i] = *(pbEP0_SETUP_ADDRESS+i);

  // point to beginning of the matrix        
  def = (BYTE *)&tUsbReqDef[0];

  while ( 1 ) {
    typ = *def++;	// compare mask for bRequestType from table	*
    req = *def++;	// compare mask for bRequest from table		*

    // 'End of Table' and 'usb_req_type_vendor' are handled special:	*
    if ( ((typ == 0xff) && (req == 0xff)) ||		// 'End of Table'
       (buf[0] == (USB_REQ_TYPE_INPUT | USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE)) ||
       (buf[0] == (USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE)) ) {
      def -= 2;
      break;                
    }

    // on all other requests 'typ' and 'req' must match exactly:	*
    if ( (typ == buf[0]) && (req == buf[1]) ) {
      bResult = 0xc0;
      bMask   = 0x20;
      // first two bytes matched, compare the rest	*
      for (i = 2; i < 8; i++) {
	if ( buf[i] == *def++ ) bResult |= bMask;
	bMask = bMask >> 1;
      }
      // now we have the result				*
      if ( (*def & bResult) == *def ) {
	def -= 8;
	break;
      } else def += (sizeof(tspp)-8);
    } else def += (sizeof(tspp)-2);
  }

  // if another setup packet comes before we have the chance to process current
  // setup request, we return here without processing the request
  // this check is not necessary but still kept here to reduce response(or simulation) time
  if((bUSBSTA & USBSTA_STPOW) != 0x00) return;
    
  // now we found the match and jump to the function accordingly.
  ((ptspp)def)->pUsbFunction();
}



/*											*
 * Interrupt Service Routines, called from Interrupt Dispatcher located in main.c	*
 * Das TUSB3410-Handbuch gibt wenig her. Etwas mehr Info steht im 'slsa263a.pdf' (in	*
 * /tars/ti/usb), dem Handbuch zum TAS1020-Streaming-Controller.			*
 *											*/

void isr_setup(void) {	// setup packet received (at oep0)
  BYTE i;
  // Das SIR-Bit im USBCTL wird 'irgendwie' (genau!) von der USB-Hardware ausgewertet.	*
  // Wie empfohlen, setzen wir das Bit beim Entry in diesen Service und loeschen es	*
  // beim Exit:										*
  bUSBCTL |= USBCTL_SIR;	// set flag to hardware: 'setup service in progress'	*
  // Nun kann es angeblich passieren, dass der Host ein weiteres 'setup packet' sendet,	*
  // bevor die aktive 'setup transaction' (mit data/ack-stages) beendet ist. Weiss	*
  // irgendeine Sau, wie das gehen soll, wenn das Device immer nur NAKs liefert?	*
  do {
    // copy the MSB of bmRequestType to DIR bit of USBCTL
    if((tSetupPacket.bmRequestType & USB_REQ_TYPE_INPUT) == USB_REQ_TYPE_INPUT)
      bUSBCTL |= USBCTL_DIR;
    else
      bUSBCTL &= ~USBCTL_DIR;

    fEP0Snd = 0;
    fEP0Rcv = 0;

    // clear out return data buffer
    for(i=0;i<USB_RETURN_DATA_LENGTH;i++) abUsbRequestReturnData[i] = 0x00;

    // decode and process the request
    usbSetupCompare();
  } while ( bUSBSTA & USBSTA_STPOW );
  bUSBCTL &= ~USBCTL_SIR;	// clr flag to hardware: 'setup service in progress'	*
  bUSBSTA = USBSTA_SETUP;	// enable ACK to usb-host				*
}


//----------------------------------------------------------------------------
void isr_iep0(void) {	// CtlDatSnd (to host) complete
  // Der InputEndPoint0 sendet Daten an den USB-Host!		*
  bOEPBCNT0 = 0;			// will be set by the hardware      
  if ( fEP0Snd ) usbEP0SndNxt();
  else bIEPCNFG0 |= EPCNF_STALL;	// no more data
}

//----------------------------------------------------------------------------
void isr_oep0(void) {	// CtlDatRcv (from host) complete
  bIEPBCNT0 = 0;			// will be set by the hardware      
  if ( fEP0Rcv ) usbEP0RcvNxt();
  else bOEPCNFG0 |= EPCNF_STALL;	// no more data
}

//----------------------------------------------------------------------------
void UsbReset(void) {	// called from UsbInit and UsbResetRequest (irq) !	*
  BYTE i, a;

  wEP0SndCnt	= 0;
  wEP0RcvCnt	= 0;
  fEP0Snd	= 0;	// don't call send completion for control pipe
  fEP0Rcv	= 0;	// don't call rcvr completion for control pipe

  bDMACDR1 = 0;		// stop DMA on OEP1	*
  bDMACDR3 = 0;		// stop DMA on IEP1	*
  
  // Setup bulk endpoints for use by DMA and UBM	*
  a = (BYTE)((WORD)xDMABuffers >> 3 & 0x00ff);
  for ( i=0; i<3; i++ ) {
    xEPODsc[0].bEPCNF = 0;	// disable ubm		*
    xEPIDsc[0].bEPCNF = 0;	// disable ubm		*
    
    xEPODsc[i].bEPSIZXY	= 64;	// Output Buffer Size	*
    xEPIDsc[i].bEPSIZXY	= 64;	// Input Buffer Size	*
    
    xEPODsc[i].bEPBCTX	= 0;	// Output X-Cnt		*
    xEPODsc[i].bEPBCTY	= 0;	// Output Y-Cnt		*
    xEPIDsc[i].bEPBCTX	= 0x80;	// Input X-Cnt		*
    xEPIDsc[i].bEPBCTY	= 0x80;	// Input Y-Cnt		*
    
    xEPODsc[i].bEPBBAX	= a + (i*32);
    xEPODsc[i].bEPBBAY	= a + (i*32) +  8;
    xEPIDsc[i].bEPBBAX	= a + (i*32) + 16;
    xEPIDsc[i].bEPBBAY	= a + (i*32) + 24;
  }
  // enable endpoint 0 (ubm and interrupt) and setup initial stall condition	*
  // STALL on EP0 means that we are waiting for an initial setup packet!	*
  bIEPCNFG0 = EPCNF_USBIE | EPCNF_UBME | EPCNF_STALL;	// 8 byte data packet
  bOEPCNFG0 = EPCNF_USBIE | EPCNF_UBME | EPCNF_STALL;	// 8 byte data packet

  // Enable Endpoints 1 for use by DMA/UBM	*
  // also resyncs data0/1 toggle !		*
  xEPODsc[0].bEPCNF	= EPCNF_UBME | EPCNF_DBUF ; // | EPCNF_USBIE ;
  xEPIDsc[0].bEPCNF	= EPCNF_UBME | EPCNF_DBUF ; // | EPCNF_USBIE ;

//Griffin - May be, this enable my new endpoint :)
  xEPIDsc[1].bEPBBAX=(BYTE)((WORD)xEP2SndBuff >> 3 & 0x00ff);
  xEPIDsc[1].bEPCNF	= EPCNF_UBME | EPCNF_USBIE ;
  
  // Enable DMA between USB and UART	*
  bDMACSR1 = 0x80 | ( 5 << 2 );	// enable transaction timeout = 5 ms	*
  bDMACSR3 = 0x80 | ( 5 << 2 );	// enable transaction timeout = 5 ms	*
  bDMACDR1 = 0x80 | 0x21 ;	// enable DMA, cont mode, no irq, OEP=1	*
  bDMACDR3 = 0x80 | 0x21 ;	// enable DMA, cont mode, no irq, IEP=1	*
  
}

//----------------------------------------------------------------------------
#if 0
void UsbInit(void) {
  BYTE i;

  bUSBCTL	= 0;	// disconnect from USB
  bFUNADR	= 0;	// no device address

  bActCfg	= 0;	// device unconfigured
  bActIfc	= 0;
  
  // copy our descriptors from code to xram	*
  for ( i=0; i<sizeof(UsbDevDsc); i++ )
    xUsbDevDsc[i] = UsbDevDsc[i];
  for( i=0; i<sizeof(UsbFunDsc); i++ )
    xUsbFunDsc[i] = UsbFunDsc[i];
  xUsbFunDsc[2] = sizeof(UsbFunDsc);
  for( i=0; i<sizeof(UsbStrDsc); i++ )
    xUsbStrDsc[i] = UsbStrDsc[i];


  
  UsbReset();	// this routine is also used by an UsbResetRequest !	*

  // Enable the USB-specific Interrupts; SETUP, and RESET		*
  bUSBMSK =  USBMSK_SETUP | USBMSK_RSTR | USBMSK_SUSP | USBMSK_RESR;

}
#else
void UsbInit(void) {
  BYTE i;
  xdata  BYTE * idata buffadd;
  buffadd = (xdata BYTE *)OEP1_X_BUFFER_ADDRESS;
//  bUSBCTL	= 0;	// disconnect from USB
//  bFUNADR	= 0;	// no device address
  bActCfg	= buffadd[0];
  bActIfc	= buffadd[1];
  //bActCfg	= pbXBufferAddress[0];	// device unconfigured
  //bActIfc	= pbXBufferAddress[1];
  
  // copy our descriptors from code to xram	*
  for ( i=0; i<sizeof(UsbDevDsc); i++ )
    xUsbDevDsc[i] = UsbDevDsc[i];
  for( i=0; i<sizeof(UsbFunDsc); i++ )
    xUsbFunDsc[i] = UsbFunDsc[i];
  xUsbFunDsc[2] = sizeof(UsbFunDsc);
  for( i=0; i<sizeof(UsbStrDsc); i++ )
    xUsbStrDsc[i] = UsbStrDsc[i];

  usbSetSerNum();
  
  UsbReset();	// this routine is also used by an UsbResetRequest !	*

  // Enable the USB-specific Interrupts; SETUP, and RESET		*
  bUSBMSK =  USBMSK_SETUP | USBMSK_RSTR | USBMSK_SUSP | USBMSK_RESR;

}
#endif
