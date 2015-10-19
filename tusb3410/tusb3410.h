/*----------------------------------------------------------------------------+
|                                                                             |
|                             Texas Instruments                               |
|                 TUSB3410 Single Channel Serial Port Controller              |
|                                                                             |
|                             Bootcode Header File                            |
|                                                                             |
+-----------------------------------------------------------------------------+
|  Source: tusb3410.h, v 1.0 2001/02/07 12:32:30                              |
|                                                                             |
|  Release Notes: (none)                                                      |
|  Logs:                                                                      |
|                                                                             |
|  WHO     WHEN         WHAT                                                  |
|  ---     --------     ----------------------------------------------------- |
|  HMT     20010207     born                                                  |
|                                                                             |
+----------------------------------------------------------------------------*/

#ifndef _TUSB3410_H_
#define _TUSB3410_H_

typedef	unsigned char	BYTE;
typedef	unsigned int	WORD;


/*----------------------------------------------------------------------------+
| Include files (none)                                                        |
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
| Function Prototype (none)                                                   |
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
| Type Definition & Macro                                                     |
+----------------------------------------------------------------------------*/
// EDB Data Structure
typedef struct _tEDB
{
    BYTE    bEPCNF;             // Endpoint Configuration
    BYTE    bEPBBAX;            // Endpoint X Buffer Base Address
    BYTE    bEPBCTX;            // Endpoint X Buffer byte Count
    BYTE    bSPARE0;            // no used
    BYTE    bSPARE1;            // no used
    BYTE    bEPBBAY;            // Endpoint Y Buffer Base Address
    BYTE    bEPBCTY;            // Endpoint Y Buffer byte Count
    BYTE    bEPSIZXY;           // Endpoint XY Buffer Size
} tEDB, *tpEDB;

//-----------------------------------------------------------------------------
// register address definition
//-----------------------------------------------------------------------------

sfr bPCON       = 0x87;         // Power Control Register

// Power Control Register (@ SFR 87h)
// PCON            0x87    // sfr 0x87
#define PCON_IDL        0x01    // MCU idle bit
                                // 0: MCU NOT in idle, 1:MCU idle
#define PCON_GF0        0x04    // General purpose bit
#define PCON_GF1        0x08    // General purpose bit
#define PCON_SMOD       0x80    // Double baud rate control bit


// External Memory Pointer
// don't use this one because it is not efficient in the binary code.
// char *pbExternalRAM = (char *)0x010000;

#define pbExternalRAM  ((char xdata *)0x0000)       // use this for the future design
#define pbInternalROM  ((char code  *)0x0000)

// USB related Constant
#define MAX_ENDPOINT_NUMBER     0x03
#define EP0_MAX_PACKET_SIZE     0x08
#define EP0_PACKET_SIZE         0x08
#define EP_MAX_PACKET_SIZE      0x40
                                                                                  
#define IEP1_X_BUFFER_ADDRESS   0xF800  // Input  Endpoint 1 X Buffer Base-address
#define IEP1_Y_BUFFER_ADDRESS   0xF840  // Input  Endpoint 1 Y Buffer Base-address
#define OEP1_X_BUFFER_ADDRESS   0xF880  // Output Endpoint 1 X Buffer Base-address
#define OEP1_Y_BUFFER_ADDRESS   0xF8C0  // Output Endpoint 1 Y Buffer Base-address

#define IEP2_X_BUFFER_ADDRESS   0xF900  // Input  Endpoint 2 X Buffer Base-address
#define IEP2_Y_BUFFER_ADDRESS   0xF940  // Input  Endpoint 2 Y Buffer Base-address
#define OEP2_X_BUFFER_ADDRESS   0xF980  // Output Endpoint 2 X Buffer Base-address
#define OEP2_Y_BUFFER_ADDRESS   0xF9C0  // Output Endpoint 2 Y Buffer Base-address

#define IEP3_X_BUFFER_ADDRESS   0xFA00  // Input  Endpoint 3 X Buffer Base-address
#define IEP3_Y_BUFFER_ADDRESS   0xFA40  // Input  Endpoint 3 Y Buffer Base-address
#define OEP3_X_BUFFER_ADDRESS   0xFA80  // Output Endpoint 3 X Buffer Base-address
#define OEP3_Y_BUFFER_ADDRESS   0xFAC0  // Output Endpoint 3 Y Buffer Base-address

// Miscellaneous Registers
#define ROMS_SDW        0x01
#define ROMS_R0         0x02    // Revision Number R[3:0]
#define ROMS_R1         0x04
#define ROMS_R2         0x08
#define ROMS_R3         0x10
#define ROMS_S0         0x20    // Code Size S[1:0]
#define ROMS_S1         0x40    // 00: 4K, 01:8k, 10:16k, 11:32k
#define ROMS_ROA        0x80    // Code Space 0:in ROM, 1:in RAM

// EndPoint Desciptor Block
#define EPCNF_USBIE     0x04    // USB Interrupt on Transaction Completion. Set By MCU
                                // 0:No Interrupt, 1:Interrupt on completion
#define EPCNF_STALL     0x08    // USB Stall Condition Indication. Set by UBM
                                // 0: No Stall, 1:USB Install Condition
#define EPCNF_DBUF      0x10    // Double Buffer Enable. Set by MCU
                                // 0: Primary Buffer Only(x-buffer only), 1:Toggle Bit Selects Buffer
#define EPCNF_TOGLE     0x20    // USB Toggle bit. This bit reflects the toggle sequence bit of DATA0 and DATA1.
#define EPCNF_ISO       0x40    // ISO=0, Non Isochronous transfer. This bit must be cleared by MCU since only non isochronous transfer is supported.
#define EPCNF_UBME      0x80    // UBM Enable or Disable bit. Set or Clear by MCU.
                                // 0:UBM can't use this endpoint
                                // 1:UBM can use this endpoint
#define EPBCT_BYTECNT_MASK 0x7F // MASK for Buffer Byte Count
#define EPBCT_NAK       0x80    // NAK, 0:No Valid in buffer, 1:Valid packet in buffer

// Endpoint 0 Descriptor Registers
#define EPBCNT_NAK     0x80     // NAK bit
                                // 0:buffer contains valid data
                                // 1:buffer is empty

// USB Registers
#define USBSTA_STPOW    0x01    // Setup Overwrite Bit. Set by hardware when setup packet is received
                                // while there is already a packet in the setup buffer.
                                // 0:Nothing, 1:Setup Overwrite
#define USBSTA_WAKEUP   0x02    // Remote wakeup pin status
                                // 0:Nothing, 1:Remote wakeup request
#define USBSTA_SETUP    0x04    // Setup Transaction Received Bit. As long as SETUP is '1',
                                // IN and OUT on endpoint-0 will be NAKed regardless of their real NAK bits values.
#define USBSTA_UR1RI    0x08    // Uart 1 Ring Indicator
                                // 0: no ring coming, 1:ring coming
#define USBSTA_UR2RI    0x10    // Uart 2 Ring Indicator
                                // 0: no ring coming, 1:ring coming
#define USBSTA_RESR     0x20    // Function Resume Request Bit. 0:clear by MCU, 1:Function Resume is detected.
#define USBSTA_SUSR     0x40    // Function Suspended Request Bit. 0:clear by MCU, 1:Function Suspend is detected.
#define USBSTA_RSTR     0x80    // Function Reset Request Bit. This bit is set in response to a global or selective suspend condition.
                                // 0:clear by MCU, 1:Function reset is detected.

#define USBMSK_STPOW    0x01    // Setup Overwrite Interrupt Enable Bit
                                // 0: disable, 1:enable
#define USBMSK_WAKEUP   0x02    // Remote Wakeup Interrupt Enable Bit
                                // 0: disable, 1:enable
#define USBMSK_SETUP    0x04    // Setup Interrupt Enable Bit
                                // 0: disable, 1:enable
#define USBMSK_UR1RI    0x08    // UART 1 Ring Indicator Interrupt Enable Bit
                                // 0: disable, 1:enable
#define USBMSK_UR2RI    0x10    // UART 2 Ring Indicator Interrupt Enable Bit
                                // 0: disable, 1:enable
#define USBMSK_RESR     0x20    // Function Resume Interrupt Enable Bit
                                // 0: disable, 1:enable
#define USBMSK_SUSP     0x40    // Function Suspend Interrupt Enable Bit
                                // 0: disable, 1:enable
#define USBMSK_RSTR     0x80    // Function Reset Interrupt Enable Bit
                                // 0: disable, 1:enable

#define USBCTL_DIR      0x01    // USB traffic direction 0: USB out packet, 1:in packet (from TUSB5152 to Host)
#define USBCTL_SIR      0x02    // Setup interrupt status bit
                                // 0: SETUP interrupt is not served.
                                // 1: SETUP interrupt in progess
#define USBCTL_BS       0x04    // Bus/self powered bit  // read only
                                // 0: bus, 1:self
#define USBCTL_SCEN     0x08    // Smart Card Mode Enable
                                // 0: disable, 1:enable 
#define USBCTL_FRSTE    0x10    // Function Reset Condition Bit.
                                // This bit connects or disconnects the USB Function Reset from the MCU reset
                                // 0:not connect, 1:connect
#define USBCTL_RWUP     0x20    // Remote wakeup request
#define USBCTL_IREN     0x40    // IR Mode Enable
                                // 0: disable, 1:enable 
#define USBCTL_CONT     0x80    // Connect or Disconnect Bit
                                // 0:Upstream port is disconnected. Pull-up disabled
                                // 1:Upstream port is connected. Pull-up enabled

#define MODECNFG_TXCNTL 0x01    // Transmit Output Control
                                // 0: hardware, 1:firmware
#define MODECNFG_SOFTSW 0x02    // Soft switch
                                // 0: disable, 1:enable 
#define MODECNFG_CLKOUTEN 0x04  // Clock Output Enable bit
                                // 0: disable, 1:enable 
#define MODECNFG_CLKSLCT 0x08   // Clock Output Souce Select
                                // 0: UART baud out clock, 1: fixed 4Mhz free running clock

// DMA Control Registers
#define DMA_BASE_ADDRESS            0xFFE0  // all DMA register starts at this address

#define DMACDR_ENDPOINT_MASK        0x07    // Endpoint Select Mask
#define DMACDR_ENDPOINT1            0x01    // Select Endpoint 1
#define DMACDR_ENDPOINT2            0x02    // Select Endpoint 2
#define DMACDR_ENDPOINT3            0x03    // Select Endpoint 3
#define DMACDR_ENDPOINT4            0x04    // Select Endpoint 4
#define DMACDR_ENDPOINT5            0x05    // Select Endpoint 5
#define DMACDR_ENDPOINT6            0x06    // Select Endpoint 6
#define DMACDR_ENDPOINT7            0x07    // Select Endpoint 7
#define DMACDR_TR                   0x08    // DMA Direction (not used in UMP)
                                            // 0:out, 1:in
#define DMACDR_XY                   0x10    // XY Buffer Select (valid only when CNT=0)
                                            // 0:X buffer 1:Y buffer
#define DMACDR_CNT                  0x20    // DMA Continuous Transfer Control bit
                                            // 0:Burst Mode, 1:Continuos Mode
#define DMACDR_INE                  0x40    // DMA Interrupt Enable or Disable bit.
                                            // 0:disable, 1:enable
#define DMACDR_EN                   0x80    // DMA Channel Enable
                                            // 0:disable, 1:enable

#define DMACSR_OVRUN                0x01    // Overrun Condition Bit. Set by DMA and Cleared by MCU
                                            // 0: no overrun, 1:overrun
#define DMACSR_PPKT                 0x01    // Overrun Condition Bit. Set by DMA and Cleared by MCU
                                            // 0: no overrun(no partial packet) , 1:overrun
#define DMACSR_TXFT                 0x02    // Transfer Timeout Condition. Cleared by MCU
                                            // 0: no timeout, 1:timeout
#define DMACSR_TIMEOUT_MASK         0x7C    // Select Timeout Value
#define DMACSR_TEN                  0x80    // Transaction Timeout Conouter Enable or Disable Bit
                                            // 0:disable(no timeout) 1:enable
// UART
// Line Control Register
#define LCR_WL_MASK                 0x03    // Word Length Mask
#define LCR_WL_5BIT                 0x00    // 5bit work length
#define LCR_WL_6BIT                 0x01    // 6bit work length
#define LCR_WL_7BIT                 0x02    // 7bit work length
#define LCR_WL_8BIT                 0x03    // 8bit work length
#define LCR_STP                     0x40    // Stop Bits
                                            // 0:1 stop bit, 1:1.5 or 2 stop bits
#define LCR_PRTY                    0x08    // Parity Bit
                                            // 0:no parity, 1:parity bit is used.
#define LCR_EPRTY                   0x10    // Odd Even Parity Bit
                                            // 0:odd, 1:even
#define LCR_FPTY                    0x20    // Force Parity Bit Slect
                                            // 0:not forced, 1:parity bit forced
#define LCR_BRK                     0x40    // Break Controll Bit
                                            // 0:normal operation, 1:forces SOUT into break condition(logic 0)
#define LCR_FEN                     0x80    // FIFO Enable
                                            // 0: FIFO cleared and disable, 1: enable

#define FCRL_TXOF                   0x01    // Transmitter Xon Xoff flow control
                                            // 0:disable, 1:enable
#define FCRL_TXOA                   0x02    // Xon-on-any Xoff flow control

                                            // 0:disable, 1:enable
#define FCRL_CTS                    0x04    // Transmitter CTS* Flow Control Enable Bit
                                            // 0:disable, 1:enable
#define FCRL_DSR                    0x04    // Transmitter DSR* Flow Control Enable Bit
                                            // 0:disable, 1:enable
#define FCRL_RXOF                   0x10    // Receiver Xon Xoff flow control
                                            // 0:disable, 1:enable
#define FCRL_RTS                    0x20    // Receiver RTS* Flow Controller Enable Bit
                                            // 0:disable, 1:enable
#define FCRL_DTR                    0x40    // Receiver DTR* Flow Controller Enable Bit
                                            // 0:disable, 1:enable
#define FCRL_485E                   0x80    // RS485 enable bit
                                            // 0:normal, full duplex, 1:for RS485

#define MCR_URST                    0x01    // UART Soft Reset
                                            // 0:Mornal operation, 1:UART Reset
#define MCR_RCVE                    0x02    // receiver enable bit
                                            // 0:disable, 1:enable
#define MCR_LOOP                    0x04    // Normal Loopback Mode Select
                                            // 0:normal operation 1:enable loopback mode
//#define MCR_IEN                     0x08    // Global UART Interrupt Enable Bit
//                                            // 0:disable, 1:enable
#define MCR_DTR                     0x10    // Set DTR*
                                            // 0:set DTR* high, 1:set DTR* low
#define MCR_RTS                     0x20    // Set RTS*
                                            // 0:set RTS* high, 1:set RTS* low
#define MCR_LRI                     0x40    // Used in loop-back mode only.
                                            // 0: MSR[6]=0, 1:MSR[6]=1
#define MCR_LCD                     0x40    // Used in loop-back mode only.
                                            // 0: MSR[7]=0, 1:MSR[7]=1

#define LSR_OVR                     0x01    // Overrun Condition
                                            // 0:no overrun, 1:overrun
#define LSR_PTE                     0x02    // Parity Condition
                                            // 0:no parity error, 1:parity error
#define LSR_FRE                     0x04    // Framing Condition
                                            // 0:no frame error, 1:frame error
#define LSR_BRK                     0x08    // Break Condition
                                            // 0:no break condition, 1:break condition
#define LSR_RXF                     0x10    // Receiver Data Register Condition
                                            // 0:no data 1:has new byte coming in
#define LSR_TXE                     0x20    // Transmitter Data Register Condition
                                            // 0:not empty 1:empty
#define LSR_TMT                     0x40    // Receiver Timeout Indication
                                            // 0:not timeout 1:timeout

#define MSR_dCTS                    0x01    // CTS* State Changed
                                            // 0:no changed 1:changed
#define MSR_dDSR                    0x02    // DSR* State Changed
                                            // 0:no changed 1:changed
#define MSR_TRI                     0x04    // Trailing edge of the ring-indicator.
                                            // Indicate RI* pin changed from 0 to 1
                                            // 0:RI* is high, 1:RI* pin changed
#define MSR_dCD                     0x08    // CD* State Changed. Cleared by MCU Reading MSR
                                            // 0:no changed 1:changed
#define MSR_LCTS                    0x10    // During loopback mode, this reflects MCR[1]
                                            // 0:CTS* is low, 1:CTS* is high
#define MSR_LDSR                    0x20    // During loopback mode, this reflects MCR[0]
                                            // 0:LDSR is high, 1:LDSR is low
#define MSR_LRI                     0x40    // During loopback mode, this reflects MCR[2]
                                            // 0:RI* is high, 1:RI* is low
#define MSR_LCD                     0x80    // During loopback mode, this reflects MCR[3]
                                            // 0:CD* is high, 1:CD* is low

// Baud Rate
#define BaudRate1200_DLL            0x01
#define BaudRate1200_DLH            0x03
#define BaudRate2400_DLL            0x81
#define BaudRate2400_DLH            0x01
#define BaudRate4800_DLL            0xC0
#define BaudRate4800_DLH            0x00
#define BaudRate7200_DLL            0x80
#define BaudRate7200_DLH            0x00
#define BaudRate9600_DLL            0x60
#define BaudRate9600_DLH            0x00
#define BaudRate14400_DLL           0x40
#define BaudRate14400_DLH           0x00
#define BaudRate19200_DLL           0x30
#define BaudRate19200_DLH           0x00
#define BaudRate38400_DLL           0x18
#define BaudRate38400_DLH           0x00
#define BaudRate57600_DLL           0x10
#define BaudRate57600_DLH           0x00
#define BaudRate115200_DLL          0x08
#define BaudRate115200_DLH          0x00
#define BaudRate230400_DLL          0x04
#define BaudRate230400_DLH          0x00
#define BaudRate460800_DLL          0x02
#define BaudRate460800_DLH          0x00
#define BaudRate921600_DLL          0x01
#define BaudRate921600_DLH          0x00

#define MASK_MIE                    0x01    // Modem interrupt
#define MASK_SIE                    0x02    // status interrupt
#define MASK_TRIE                   0x04    // TxRx interrupt

#define VECINT_NO_INTERRUPT             0x00
#define VECINT_OUTPUT_ENDPOINT1         0x12
#define VECINT_OUTPUT_ENDPOINT2         0x14
#define VECINT_OUTPUT_ENDPOINT3         0x16
//#define VECINT_OUTPUT_ENDPOINT4         0x18
//#define VECINT_OUTPUT_ENDPOINT5         0x1A
//#define VECINT_OUTPUT_ENDPOINT6         0x1C
//#define VECINT_OUTPUT_ENDPOINT7         0x1E

#define VECINT_INPUT_ENDPOINT1          0x22
#define VECINT_INPUT_ENDPOINT2          0x24
#define VECINT_INPUT_ENDPOINT3          0x26

//#define VECINT_INPUT_ENDPOINT4          0x28
//#define VECINT_INPUT_ENDPOINT5          0x2A
//#define VECINT_INPUT_ENDPOINT6          0x2C
//#define VECINT_INPUT_ENDPOINT7          0x2E

#define VECINT_STPOW_PACKET_RECEIVED    0x30            // USBSTA
#define VECINT_SETUP_PACKET_RECEIVED    0x32            // USBSTA
#define VECINT_RESR_INTERRUPT           0x38            // USBSTA
#define VECINT_SUSR_INTERRUPT           0x3A            // USBSTA
#define VECINT_RSTR_INTERRUPT           0x3C            // USBSTA
#define VECINT_RWUP_INTERRUPT           0x3E            // USBSTA

#define VECINT_I2C_RXF_INTERRUPT        0x40            // I2CSTA
#define VECINT_I2C_TXE_INTERRUPT        0x42            // I2CSTA

#define VECINT_INPUT_ENDPOINT0          0x44
#define VECINT_OUTPUT_ENDPOINT0         0x46

#define VECINT_UART1_STATUS_INTERRUPT   0x50
#define VECINT_UART1_MODEM_INTERRUPT    0x52
//#define VECINT_UART2_STATUS_INTERRUPT   0x54
//#define VECINT_UART2_MODEM_INTERRUPT    0x56

#define VECINT_UART1_RXF_INTERRUPT      0x60
#define VECINT_UART1_TXE_INTERRUPT      0x62

#define VECINT_DMA1_INTERRUPT           0x80
#define VECINT_DMA3_INTERRUPT           0x84

// Watchdog timer
#define WDCSR_WDT           0x01        // reset timer
#define WDCSR_WDE           0x80        // enable bit


//I2C Registers
#define I2CSTA_SWR          0x01        // Stop Write Enable
                                        // 0:disable, 1:enable
#define I2CSTA_SRD          0x02        // Stop Read Enable
                                        // 0:disable, 1:enable
#define I2CSTA_TIE          0x04        // I2C Transmitter Empty Interrupt Enable
                                        // 0:disable, 1:enable
#define I2CSTA_TXE          0x08        // I2C Transmitter Empty
                                        // 0:full, 1:empty
#define I2CSTA_400K         0x10        // I2C Speed Select
                                        // 0:100kHz, 1:400kHz
#define I2CSTA_ERR          0x20        // Bus Error Condition
                                        // 0:no bus error, 1:bus error
#define I2CSTA_RIE          0x40        // I2C Receiver Ready Interrupt Enable
                                        // 0:disable, 1:enable
#define I2CSTA_RXF          0x80        // I2C Receiver Full
                                        // 0:empty, 1:full
#define I2CADR_READ         0x01        // Read Write Command Bit
                                        // 0:write, 1:read

//-----------------------------------------------------------------------------------------------
// register address definition
//-----------------------------------------------------------------------------------------------

// EndPoint Desciptor Block
// USB Data Buffer
#define bOEP0_BUFFER_ADDRESS    (* (char xdata *)0xFEF0)    // Output Endpoint 0 Buffer Base-address
#define bIEP0_BUFFER_ADDRESS    (* (char xdata *)0xFEF8)    // Input  Endpoint 0 Buffer Base-address
#define bEP0_SETUP_ADDRESS      (* (char xdata *)0xFF00)    // setup packet

#define pbOEP0_BUFFER_ADDRESS   ( (char xdata *)0xFEF0)    // Output Endpoint 0 Buffer Base-address
#define pbIEP0_BUFFER_ADDRESS   ( (char xdata *)0xFEF8)    // Input  Endpoint 0 Buffer Base-address
#define pbEP0_SETUP_ADDRESS     ( (char xdata *)0xFF00)    // setup packet


#define bOEPCNF1    (* (char xdata *)0xFF08)        // Output Endpoint 1 Configuration
#define bOEPCNF2    (* (char xdata *)0xFF10)        // Output Endpoint 2 Configuration
#define bOEPCNF3    (* (char xdata *)0xFF18)        // Output Endpoint 3 Configuration
//#define bOEPCNF4    (* (char xdata *)0xFF20)        // Output Endpoint 4 Configuration
//#define bOEPCNF5    (* (char xdata *)0xFF28)        // Output Endpoint 5 Configuration
//#define bOEPCNF6    (* (char xdata *)0xFF30)        // Output Endpoint 6 Configuration
//#define bOEPCNF7    (* (char xdata *)0xFF38)        // Output Endpoint 7 Configuration

#define bOEPBBAX1   (* (char xdata *)0xFF09)        // Output Endpoint 1 X-Buffer Base-address
#define bOEPBBAX2   (* (char xdata *)0xFF11)        // Output Endpoint 2 X-Buffer Base-address
#define bOEPBBAX3   (* (char xdata *)0xFF19)        // Output Endpoint 3 X-Buffer Base-address
//#define bOEPBBAX4   (* (char xdata *)0xFF21)        // Output Endpoint 4 X-Buffer Base-address
//#define bOEPBBAX5   (* (char xdata *)0xFF29)        // Output Endpoint 5 X-Buffer Base-address
//#define bOEPBBAX6   (* (char xdata *)0xFF31)        // Output Endpoint 6 X-Buffer Base-address
//#define bOEPBBAX7   (* (char xdata *)0xFF39)        // Output Endpoint 7 X-Buffer Base-address

#define bOEPBCTX1   (* (char xdata *)0xFF0A)        // Output Endpoint 1 X Byte Count
#define bOEPBCTX2   (* (char xdata *)0xFF12)        // Output Endpoint 2 X Byte Count

#define bOEPBCTX3   (* (char xdata *)0xFF1A)        // Output Endpoint 3 X Byte Count
//#define bOEPBCTX4   (* (char xdata *)0xFF22)        // Output Endpoint 4 X Byte Count
//#define bOEPBCTX5   (* (char xdata *)0xFF2A)        // Output Endpoint 5 X Byte Count
//#define bOEPBCTX6   (* (char xdata *)0xFF32)        // Output Endpoint 6 X Byte Count
//#define bOEPBCTX7   (* (char xdata *)0xFF3A)        // Output Endpoint 7 X Byte Count

#define bOEPBBAY1   (* (char xdata *)0xFF0D)        // Output Endpoint 1 Y-Buffer Base-address
#define bOEPBBAY2   (* (char xdata *)0xFF15)        // Output Endpoint 2 Y-Buffer Base-address
#define bOEPBBAY3   (* (char xdata *)0xFF1D)        // Output Endpoint 3 Y-Buffer Base-address
//#define bOEPBBAY4   (* (char xdata *)0xFF25)        // Output Endpoint 4 Y-Buffer Base-address
//#define bOEPBBAY5   (* (char xdata *)0xFF2D)        // Output Endpoint 5 Y-Buffer Base-address
//#define bOEPBBAY6   (* (char xdata *)0xFF35)        // Output Endpoint 6 Y-Buffer Base-address
//#define bOEPBBAY7   (* (char xdata *)0xFF3D)        // Output Endpoint 7 Y-Buffer Base-address

#define bOEPBCTY1   (* (char xdata *)0xFF0E)        // Output Endpoint 1 Y Byte Count
#define bOEPBCTY2   (* (char xdata *)0xFF16)        // Output Endpoint 2 Y Byte Count
#define bOEPBCTY3   (* (char xdata *)0xFF1E)        // Output Endpoint 3 Y Byte Count
//#define bOEPBCTY4   (* (char xdata *)0xFF26)        // Output Endpoint 4 Y Byte Count
//#define bOEPBCTY5   (* (char xdata *)0xFF2E)        // Output Endpoint 5 Y Byte Count
//#define bOEPBCTY6   (* (char xdata *)0xFF36)        // Output Endpoint 6 Y Byte Count
//#define bOEPBCTY7   (* (char xdata *)0xFF3E)        // Output Endpoint 7 Y Byte Count

#define bOEPSIZXY1  (* (char xdata *)0xFF0F)        // Output Endpoint 1 XY-Buffer Size
#define bOEPSIZXY2  (* (char xdata *)0xFF17)        // Output Endpoint 2 XY-Buffer Size
#define bOEPSIZXY3  (* (char xdata *)0xFF1F)        // Output Endpoint 3 XY-Buffer Size
//#define bOEPSIZXY4  (* (char xdata *)0xFF27)        // Output Endpoint 4 XY-Buffer Size
//#define bOEPSIZXY5  (* (char xdata *)0xFF2F)        // Output Endpoint 5 XY-Buffer Size
//#define bOEPSIZXY6  (* (char xdata *)0xFF37)        // Output Endpoint 6 XY-Buffer Size
//#define bOEPSIZXY7  (* (char xdata *)0xFF3F)        // Output Endpoint 7 XY-Buffer Size

#define bIEPCNF1    (* (char xdata *)0xFF48)        // Input Endpoint 1 Configuration
#define bIEPCNF2    (* (char xdata *)0xFF50)        // Input Endpoint 2 Configuration
#define bIEPCNF3    (* (char xdata *)0xFF58)        // Input Endpoint 3 Configuration
//#define bIEPCNF4    (* (char xdata *)0xFF60)        // Input Endpoint 4 Configuration
//#define bIEPCNF5    (* (char xdata *)0xFF68)        // Input Endpoint 5 Configuration
//#define bIEPCNF6    (* (char xdata *)0xFF70)        // Input Endpoint 6 Configuration
//#define bIEPCNF7    (* (char xdata *)0xFF78)        // Input Endpoint 7 Configuration

#define bIEPBBAX1   (* (char xdata *)0xFF49)        // Input Endpoint 1 X-Buffer Base-address
#define bIEPBBAX2   (* (char xdata *)0xFF51)        // Input Endpoint 2 X-Buffer Base-address
#define bIEPBBAX3   (* (char xdata *)0xFF59)        // Input Endpoint 3 X-Buffer Base-address
//#define bIEPBBAX4   (* (char xdata *)0xFF61)        // Input Endpoint 4 X-Buffer Base-address
//#define bIEPBBAX5   (* (char xdata *)0xFF69)        // Input Endpoint 5 X-Buffer Base-address
//#define bIEPBBAX6   (* (char xdata *)0xFF71)        // Input Endpoint 6 X-Buffer Base-address
//#define bIEPBBAX7   (* (char xdata *)0xFF79)        // Input Endpoint 7 X-Buffer Base-address

#define bIEPDCTX1   (* (char xdata *)0xFF4A)        // Input Endpoint 1 X Byte Count
#define bIEPDCTX2   (* (char xdata *)0xFF52)        // Input Endpoint 2 X Byte Count
#define bIEPDCTX3   (* (char xdata *)0xFF5A)        // Input Endpoint 3 X Byte Count
//#define bIEPDCTX4   (* (char xdata *)0xFF62)        // Input Endpoint 4 X Byte Count
//#define bIEPDCTX5   (* (char xdata *)0xFF6A)        // Input Endpoint 5 X Byte Count
//#define bIEPDCTX6   (* (char xdata *)0xFF72)        // Input Endpoint 6 X Byte Count
//#define bIEPDCTX7   (* (char xdata *)0xFF7A)        // Input Endpoint 7 X Byte Count
                    
#define bIEPBBAY1   (* (char xdata *)0xFF4D)        // Input Endpoint 1 Y-Buffer Base-address
#define bIEPBBAY2   (* (char xdata *)0xFF55)        // Input Endpoint 2 Y-Buffer Base-address
#define bIEPBBAY3   (* (char xdata *)0xFF5D)        // Input Endpoint 3 Y-Buffer Base-address
//#define bIEPBBAY4   (* (char xdata *)0xFF65)        // Input Endpoint 4 Y-Buffer Base-address
//#define bIEPBBAY5   (* (char xdata *)0xFF6D)        // Input Endpoint 5 Y-Buffer Base-address
//#define bIEPBBAY6   (* (char xdata *)0xFF75)        // Input Endpoint 6 Y-Buffer Base-address
//#define bIEPBBAY7   (* (char xdata *)0xFF7D)        // Input Endpoint 7 Y-Buffer Base-address
                    
#define bIEPDCTY1   (* (char xdata *)0xFF4E)        // Input Endpoint 1 Y Byte Count
#define bIEPDCTY2   (* (char xdata *)0xFF56)        // Input Endpoint 2 Y Byte Count
#define bIEPDCTY3   (* (char xdata *)0xFF5E)        // Input Endpoint 3 Y Byte Count
//#define bIEPDCTY4   (* (char xdata *)0xFF66)        // Input Endpoint 4 Y Byte Count
//#define bIEPDCTY5   (* (char xdata *)0xFF6E)        // Input Endpoint 5 Y Byte Count
//#define bIEPDCTY6   (* (char xdata *)0xFF76)        // Input Endpoint 6 Y Byte Count
//#define bIEPDCTY7   (* (char xdata *)0xFF7E)        // Input Endpoint 7 Y Byte Count

#define bIEPSIZXY1  (* (char xdata *)0xFF4F)        // Input Endpoint 1 XY-Buffer Size
#define bIEPSIZXY2  (* (char xdata *)0xFF57)        // Input Endpoint 2 XY-Buffer Size
#define bIEPSIZXY3  (* (char xdata *)0xFF5F)        // Input Endpoint 3 XY-Buffer Size
//#define bIEPSIZXY4  (* (char xdata *)0xFF67)        // Input Endpoint 4 XY-Buffer Size
//#define bIEPSIZXY5  (* (char xdata *)0xFF6F)        // Input Endpoint 5 XY-Buffer Size
//#define bIEPSIZXY6  (* (char xdata *)0xFF77)        // Input Endpoint 6 XY-Buffer Size
//#define bIEPSIZXY7  (* (char xdata *)0xFF7F)        // Input Endpoint 7 XY-Buffer Size

// Endpoint 0 Descriptor Registers
#define bIEPCNFG0   (* (char xdata *)0xFF80)        // Input Endpoint Configuration Register
#define bIEPBCNT0   (* (char xdata *)0xFF81)        // Input Endpoint 0 Byte Count
#define bOEPCNFG0   (* (char xdata *)0xFF82)        // Output Endpoint Configuration Register
#define bOEPBCNT0   (* (char xdata *)0xFF83)        // Output Endpoint 0 Byte Count

// Miscellaneous Registers
#define bROMS       (* (char xdata *)0xFF90)        // ROM Shadow Configuration Register
//#define bGLOBCTL    (* (char xdata *)0xFF91)        // Global Control Register

#define	VECINT	0xff92
#define bVECINT     (* (char xdata *)VECINT)        // Vector Interrupt Register
#define bWDCSR      (* (char xdata *)0xFF93)        // watchdog timer register


#define PUR3        (* (char xdata *)0xFF9E)        // GPIO Pull Up Register for Port 3

// UART Registers
#define bRDR1       (* (char xdata *)0xFFA0)        // UART1 Receiver Data Register
#define bTDR1       (* (char xdata *)0xFFA1)        // UART1 Transmitter Data Register
#define bLCR1       (* (char xdata *)0xFFA2)        // UART1 Line Control Register
#define bFCRL1      (* (char xdata *)0xFFA3)        // UART1 Flow Control Register
#define bMCR1       (* (char xdata *)0xFFA4)        // UART1 Modem Control Register
#define bLSR1       (* (char xdata *)0xFFA5)        // UART1 Line Status Register
#define bMSR1       (* (char xdata *)0xFFA6)        // UART1 Modem Status Register
#define bDLL1       (* (char xdata *)0xFFA7)        // UART1 Divisor Register Low-byte
#define bDLH1       (* (char xdata *)0xFFA8)        // UART1 Divisor Register High-byte
#define bXON1       (* (char xdata *)0xFFA9)        // UART1 Xon Register
#define bXOFF1      (* (char xdata *)0xFFAA)        // UART1 Xoff Register
#define bMASK1      (* (char xdata *)0xFFAB)        // UART1 Interrupt Mask Register

//#define bRDR2       (* (char xdata *)0xFFB0)        // UART2 Receiver Data Register
//#define bTDR2       (* (char xdata *)0xFFB1)        // UART2 Transmitter Data Register
//#define bLCR2       (* (char xdata *)0xFFB2)        // UART2 Line Control Register
//#define bFCRL2      (* (char xdata *)0xFFB3)        // UART2 Flow Control Register
//#define bMCR2       (* (char xdata *)0xFFB4)        // UART2 Modem Control Register
//#define bLSR2       (* (char xdata *)0xFFB5)        // UART2 Line Status Register
//#define bMSR2       (* (char xdata *)0xFFB6)        // UART2 Modem Status Register
//#define bDLL2       (* (char xdata *)0xFFB7)        // UART2 Divisor Register Low-byte
//#define bDLH2       (* (char xdata *)0xFFB8)        // UART2 Divisor Register High-byte
//#define bXON2       (* (char xdata *)0xFFB9)        // UART2 Xon Register
//#define bXOFF2      (* (char xdata *)0xFFBA)        // UART2 Xoff Register
//#define bMASK2      (* (char xdata *)0xFFBB)        // UART2 Interrupt Mask Register

// DMA registers
#define bDMACDR1    (* (char xdata *)0xFFE0)        // DMA Channel 1 Definition Register for UART 1 Transmitter
#define bDMACSR1    (* (char xdata *)0xFFE1)        // DMA Channel 1 Control & Status Register
//#define bDMACDR2    (* (char xdata *)0xFFE2)        // DMA Channel 2 Definition Register for UART 2 Transmitter
//#define bDMACSR2    (* (char xdata *)0xFFE3)        // DMA Channel 2 Control & Status Register
#define bDMACDR3    (* (char xdata *)0xFFE4)        // DMA Channel 3 Definition Register for UART 1 Receiver
#define bDMACSR3    (* (char xdata *)0xFFE5)        // DMA Channel 3 Control & Status Register
//#define bDMACDR4    (* (char xdata *)0xFFE6)        // DMA Channel 4 Definition Register for UART 2 Receiver
//#define bDMACSR4    (* (char xdata *)0xFFE7)        // DMA Channel 4 Control & Status Register

// Serial Number Registers
#define bSERNUM0    (* (unsigned char xdata *)0xFFE8)        // Serial Number Register
#define bSERNUM1    (* (unsigned char xdata *)0xFFE9)        // Serial Number Register
#define bSERNUM2    (* (unsigned char xdata *)0xFFEA)        // Serial Number Register
#define bSERNUM3    (* (unsigned char xdata *)0xFFEB)        // Serial Number Register
#define bSERNUM4    (* (unsigned char xdata *)0xFFEC)        // Serial Number Register
#define bSERNUM5    (* (unsigned char xdata *)0xFFED)        // Serial Number Register
#define bSERNUM6    (* (unsigned char xdata *)0xFFEE)        // Serial Number Register
#define bSERNUM7    (* (unsigned char xdata *)0xFFEF)        // Serial Number Register

//I2C Registers     
#define bI2CSTA     (* (char xdata *)0xFFF0)        // I2C Status and Control Register
#define bI2CDAO     (* (char xdata *)0xFFF1)        // I2C Data Out Register
#define bI2CDAI     (* (char xdata *)0xFFF2)        // I2C Data In Register
#define bI2CADR     (* (char xdata *)0xFFF3)        // I2C Address Register

// USB Registers
#define bDEVREVL    (* (char xdata *)0xFFF5)        // Device Revision Number
#define bDEVREVH    (* (char xdata *)0xFFF6)        // Device Revision Number
#define bDEVPIDL    (* (char xdata *)0xFFF7)        // Device PID
#define bDEVPIDH    (* (char xdata *)0xFFF8)        // Device PID
#define bDEVVIDL    (* (char xdata *)0xFFF9)        // Device VID
#define bDEVVIDH    (* (char xdata *)0xFFFA)        // Device VID
#define bMODECNFG   (* (char xdata *)0xFFFB)        // Mode configuration register
#define bUSBCTL     (* (char xdata *)0xFFFC)        // USB Control Register
#define bUSBMSK     (* (char xdata *)0xFFFD)        // USB Interrupt Mask Register
#define bUSBSTA     (* (char xdata *)0xFFFE)        // USB Status Register
#define bFUNADR     (* (char xdata *)0xFFFF)        // This register contains the device function address.

#endif /* _TUSB3410_H_ */
