/*										*
 * main.h	Defines for cross referencing between				*
 *		Firmware and BMLS-(Linux)-Driver				*
 *										*/
#ifndef _MAIN_H_
#define _MAIN_H_

#define	REQUEST_LINE		0
#define REQUEST_SOF		1	// used to sync data0/1-toggle on reopen bulk pipe
#define REQUEST_SON		2	// used to sync data0/1-toggle on reopen bulk pipe
#define REQUEST_BAUD		3
#define REQUEST_LCR		4
#define REQUEST_FCR		5
#define REQUEST_RTS		6
#define REQUEST_DTR		7
#define REQUEST_BREAK		8
#define REQUEST_CRTSCTS		9


#define BaudRate1200            0x001
#define BaudRate2400            0x002
#define BaudRate4800            0x003
#define BaudRate7200            0x004
#define BaudRate9600            0x005
#define BaudRate14400           0x006
#define BaudRate19200           0x007
#define BaudRate38400           0x008
#define BaudRate57600           0x009
#define BaudRate115200          0x00a
#define BaudRate230400          0x00b
#define BaudRate460800          0x00c
#define BaudRate921600          0x00d


#define	BAUD_BASE	923077

#define ULCR_OVR 0x1
#define ULCR_PTE 0x2
#define ULCR_FRE 0x4
#define ULCR_BRK 0x8 

#define MCR_CTS 0x1
#define MCR_DSR 0x2
#define MCR_CD  0x4
#define MCR_RI  0x8

#endif /* _MAIN_H_ */
