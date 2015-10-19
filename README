;;;
;;; D/FW  -- Debug Firmware for MSP-FET430UIF/eZ430U
;;;
;;;                                          skimu@mac.com
;;;  Revision history
;;;   0.19      May       7 2010   DFW for MSP430F2274.
;;;   0.18      May       3 2010   PTFW (Serial Pass-Through) for eZ430U.
;;;   0.17      May       1 2010   Improve compatibility with Linux.
;;;   0.16      April    16 2010   bufet: backup FET430UIF/eZ430U.
;;;   0.15      April     2 2010   added help messages
;;;   0.14      April     1 2010   CPUXv1 support
;;;   0.13      March    28 2010   CDC ACM firmware for TUSB3410
;;;   0.11      March    19 2010   Step execution support
;;;   0.9       March    14 2010   CPUXv2 support
;;;   0.7       February 23 2010   CC8051 support
;;;   0.2       February 7  2010   First release
;;;

----------- Introduction ----------------------------------------

This is an open source, experimental, and unofficial firmware for
MSP-FET430UIF/eZ430U debug interface and associated host side 
control programs.  Although it has been tested only on MacOSX,
host side programs are designed to work any modern UNIX operating 
systems (e.g. Linux, *BSD).

------------ Combinations known to work -------------------------

*** Host ***

   MacOSX (Darwin 10.2.0)
   Gentoo Linux

*** FET ***

   MSP-FET430UIF
   eZ430U(Rev2.0)
   MSP430F2274 (tested on RF2500T)

*** Target Devices ***

FET         Device  TAP    Target Board    
                        
FET430UIF   F169    JTAG   MSP-TS430PM64   
FET430UIF   F2274   JTAG   MSP-TS430DA38   
FET430UIF   F2274   SBW    MSP-TS430DA38   
FET430UIF   F2274   SBW    RF2500T         
FET430UIF   F1612   JTAG   eZ430U(Rev2.0)  
FET430UIF   F2618   JTAG   MSP-TS430PM64     (CPUXv1)
FET430UIF   F5529   JTAG   MSP-TS430PN80USB  (CPUXv2)
FET430UIF   F5529   SBW    MSP-TS430PN80USB  (CPUXv2)
FET430UIF   F6137   SBW    eZ430-Chronos     (CPUXv2)
eZ430U      F2012   SBW    eZ430-T2012
eZ430U      F2274   SBW    RF2500T         
eZ430U      F6137   SBW    eZ430-Chronos     (CPUXv2)
F2274       F1612   JTAG   eZ430U(Rev2.0)
F2274       F2274   SBW    RF2500T
FET430UIF   CC2511  CC8051 CC2511EM (USB dongle)

Target CC8051 supports Chipcon/TI's CC111x/CC251x/CC243x series
RF chip with 8051 MCU.  Theoretically, DFW should work for 
all of CC111x/CC251x series as is, but not sure for CC243x.

CC253x has similar interface and looks like upper compatible
to older devices, so there is a good chance that DFW can deal 
with CC253x as is, or with relatively minor modifications. (I 
haven't gone through the manual.)

F2274/DFW can handle everything FET430UIF/DFW can.
To use F2274/DFW, you need 3V serial interface to
connect host computer. You can use eZ430/PTFW for this purpose.

----------- Requirements ----------------------------------------

1) FET430UIF or eZ430U (target FET), which you don't miss much if 
   it get broken. (See below for notes on eZ430-F2013)

2) 4 wire JTAG programmer to write DFW into your FET. (mother FET).
   DFW has ability to upgrade itself from USB port, you only need 
   mother programmer just for the first time.

2') A target board with MSP430F2274. (eg. RF2500T)
    conf/f2274/README explains how to program your FET by RF2500T.
   
3) Host computer running UNIX operating system with USB modem support.

4) MSPGCC toolchain on your host computer. See below for tips
   on MSPGCC installation on MacOSX.

5) No USB device with vendor/product ID of 0x451/0xbeef.
   These are the default settings for DFW firmware.
   See below how to change USB IDs.

----------- Directories and files -------------------------------

dfw/         firmware source code for MSP-FET430UIF and eZ430U.
dfw/conf/*   build directory for each target configuration.

dfw/dfw.{fetuif,ez430u,ez430u1p1}.ihex
             pre-compiled debug firmware (DFW) for each FET.

dfw/ptfw.ez430u.ihex
             pre-compiled serial pass-through firmware (PTFW).
             (this is dedicated for pass-throgh only, no debug)

dmwt.c       host side program to program target device or
             update firmware of FET/DFW.

bufet.c      backup FET430UIF/eZ430 using FET/DFW.
bufetee/     Bit banging I2C interface to read/write EEPROM,
             used in bufet.c

dfwsh.c      shell like interface to debug DFW firmware.

gdbproxy/    customized version of gdbproxy, with uifdfw target.

examples/    usage examples for dmwt and gdbproxy.

tusb3410/    firmware source code for TUSB3410 (CDC ACM)
             derived from firmware/firmware-0.1.tar.gz in 
             http://sourceforge.net/projects/uticom/files
             You need sdcc and Gauche to compile.
             (see ``Changing USB ID'' section for details)

tusb3410_autoexec_firmware.inc
             complied binary image made from above directory.

boot3410.c   firmware uploader for TUSB3410
libusbcom/   USB communication library for boot3410.c

  TUSB3410 firmware can be used with TI's original firmware
  instead of TI's VCP software.

------------ Installation ---------------------------------------

1)  dfw.fetuif.ihex, dfw.ez430u.ihex and dfw.ez430u1p1.ihex in
    dfw directory is pre-compiled binary (in Intel HEX format) for 
    FET430UIF, eZ430U and eZ430U Rev1.1, respectively.  Write one 
    of these into your FET by mother FET. (See below for details.)

2)  D/FW firmware checks the content of EEPROM, and program it
    if necessary.  So it will take about 30 seconds before 
    it gets ready after the first power on.

3)  edit Makefile in this directory and correct your BFD library's
    location. 

4)  make will make dmwt command and other utilities.

5)  examples/*/Makefile contains examples how to use dmwt.

6)  (cd gdbproxy; ./configure && make)
    will make gdbproxy.

------------ Known bugs (less priority To-Do's) -----------------

1) Error recovery and safety checking are missing.

2) breakpoint is not supported.  I could not find any useful 
   documentation about f16x's breakpoint registers, nor f2xxx's
   EEM feature...
 
3) There are a lot of GCC/GDB compatibility issues.
3.1)  stack backtrace does not work with GDB-6.8 (7.0.x is OK).
3.2)  stack backtrace does not show correct arguments on CPUX
3.3)  cannot get local variables correctly (especially for binaries 
      compiled with -O0 option) for binaries compiled by gcc-3.2.3.
      For binaries compiled by gcc-4.4.3 looks OK.

4) support for target serial port at TUSB3410 on eZ430U, for 
   independent communications host<->FET and host<->target is missing.
   (this requires software serdes on TUSB3410, I think)
   ptfw.ez430u.ihex is dedicated serial port pass through firmware.
   (With ptfw, eZ430U becomes USB serial dongle.)

------------ Preparing your target FET for DFW ------------------

To make backup files for FET430UIF or eZ430, see ``Making backup
files'' section below.

a) FET430UIF
   0) You can get schematic and board layout diagrams from TI's 
      website (SLAU278)

   1) You can program F1612 through J5. Note that VBUS(pin7) is came
      from USB port, you have to supply 5V here.

   2) I don't connect VBUS (pin 7 of J5) because my mother FET's 
      revision was 1.3, (see note below) but provide power to target 
      FET from USB port using USB battery charger. (I am using one 
      came with my iPhone)

b) eZ430U
   0) You can get schematic and board layout diagrams from TI's 
      website (SLAU292)

   1) you have to make custom socket, or solder pins to TP1..TP7 
      holes of the FET board to get JTAG signal.  Note that pin name 
      order is inverted from usual JTAG socket. TDIO is assigned to TP7,
      corresponds to pin1 of JTAG 14pin socket.

   2) I soldered (since I do not have suitable contact), 
      but decided not to connect VCCT and use external
      USB charger to power eZ430U.

------------ dmwt usage examples ---------------------------------

Usage: dmwt [-vdf] [-p /dev/cu.usbmodemXXX] [-c TARGET_OPTIONS] 
            [-b name.txt from-addr] [object]

  ``object'' can be in either ELF, Intel HEX, or TITXT format.

  /dev/cu.usbmodemXXX might be /dev/ttyACMXXX or /dev/ttyXXX in some system.

   $ dmwt                               # will print help message
   $ dmwt -f dfw.fetuif.elf             # update firmware
   $ dmwt -c TARGET_OPTIONS a.out
   $ dmwt -p /dev/cu.usbmodem001 -c "SBW" a.out
   $ dmwt -vc "SBW" a.out               # verbose mode

   $ dmwt -c ""  a.out                  # program a.out with JTAG.
   $ dmwt -c "SBW"  a.out               # program a.out with SBW.
   $ dmwt -c "VCC 3000 UNLOCKA" a.out   # sets VCC 3.0V, unlock INFO_A
   $ dmwt -c "CPUXV1" a.out                   
   $ dmwt -c "CPUXV2" a.out                   
   $ dmwt -c "CC8051" main.hex

See examples/*/Makefile for more examples.

------------ gdbproxy command line examples ----------------------

   $ msp430-gdbproxy --help uifdfw
   $ msp430-gdbproxy --port=2000 uifdfw --c TARGET_OPTIONS
   $ msp430-gdbproxy --port=2000 uifdfw --c "CPUXV2" --debug

   $ msp430-gdbproxy --port=2000 uifdfw --c "SBW"         # normal msp430 with SBW
   $ msp430-gdbproxy --port=2000 uifdfw --c "CPUXV1"      # CPUXV1 with JTAG
   $ msp430-gdbproxy --port=2000 uifdfw --c "CPUXV2 SBW"  # CPUXV2 with SBW

------------ TARGET_OPTIONS --------------------------------------

   MSP430    : (default) msp430x1xx, msp430x2xx, msp430x4xx
   CPUXV1    :           msp430x2xx, msp430x4xx with CPUX
   CPUXV2    :           msp430x5xx, cc430x6xx 
   CC8051    :           cc111x, cc251x, etc.  

   VCC xxxx  : sets VCC to XXXX mV (e.g., VCC 3300), FET430UIF only.

   JTAG      : (default)
   SBW       :          

   FastFlash : (default)
   SlowFlash :           for relatively old devices without FastFlash feature
   LOCKA     : (default) locks INFO_A memory
   UNLOCKA   :           unlocks INFO_A memory

   FCTL addr : sets FCTL base address for CPUXV2 target.

   FWS  n    : sets flash word size for CC8051 target.

See cmd.c target_*.c for details.

------------ Making backup files for target FET -------------------

Usage: bufet [-vd] [-p /dev/cu.usbmodemXXX] [-c TARGET_OPTIONS] 
             [fet_eeprom.txt fet_flash.txt]

To make backup file:

1) You need DFW equipped 4wire JTAG FET (mother FET).
   (eZ430U is SBW only FET. eZ430U cannot be used for this purpose.)
2) Connect target FET430UIF or eZ430U to mother FET with JTAG
   cable, and provide power to target FET through USB cable.
3) Connect mother to the host computer with USB cable.
4) Then type, (assuming you are in the same directory as this
   README file)

   $ ./bufet

or

   $ mkdir FET_backup
   $ cd FET_backup
   $ ../bufet

bufet will create fet_eeprom.txt and fet_flash.txt in the current
directory.  Those files are in TITXT format.  After making backup
files, you might want to compress these files using titxt_compress 
command to save disk space and restore time.

To restore from backup file,

   $ ../bufet fet_eeprom.txt fet_flash.txt

Note that reading/writing eeprom is very slow.

------------ Notes on eZ430-F2013 (or eZ430U Rev. 1.1) -----------

Currently dfw known to work on eZ430U Rev. 2.0 only.
The FET comes with eZ430-F2013 (4pin header to target board),
is Rev. 1.1, which uses different port for RST_3410.
Precompiled hex file is included but not tested.

------------ Notes on MSP-FET430UIF Revisions --------------------

If you plan to use FET430UIF as mother FET, you need to check
your FET's revision before plug JTAG connector into your target's.

In Rev. 1.3, unused pins of JTAG target connector are grounded.
This will cause short between VBUS (pin7 of J5) and GND.
You have to put an adopter between JTAG cable and target FET,
to avoid this.

------------ Notes on MSPGCC Installation on MacOSX --------------

** Toolchain versions I am using. **

0.1)  In general, following page is very helpful.

      Building MSPGCC from Source in MSPGCC Wiki,

      http://sourceforge.net/apps/mediawiki/mspgcc/index.php?title=Building_MSPGCC_from_Source_Code

      I am using gcc-3.2.3, binutils-2.19, built by the procedure described
      in this page.

0.2)  As for gdb, gdb-6.8 somewhat did not work well for me.
      gdb-7.0 from mspgcc4 project, works (relatively) well.

      http://mspgcc4.sourceforge.net/

** Building toolchain on MacOSX **

1)  gcc-3.2.3 does not recognize i686-apple-darwin10.2.0 as a host.
    To work this around, put this entry in gcc/config.gcc like follows.

    ---8<------8<-- gcc/config.gcc ---8<-------8<---
i686-apple-darwin10.2.0)
        ;;
*)
        echo "Configuration $machine not supported" 1>&2
        exit 1
        ;;
    ---8<------8<------8<------8<-----8<-------8<---

2)  gcc gives a lot of warnings when compiling intl directory,
    of binutils, or gdb.  To avoid this. Put --disable-nls 
    option to ./configure, like follows,

    $ ./configure --prefix=/usr/local/mspgcc --target=msp430 --disable-nls
    
------------ Changing USB ID for DFW ---------------------------

DFW is using 0xbeef as USB product ID, which I found in SLAA276.
Since this product ID is used in Application Report, I assume TI
is not using this ID for official product, and it is allowed to
be used for this kind of non-commercial experimental purposes.  
For the same reason, I imagine, some of potential users of this
software had already been using this ID for their in-house USB
devices.

In case you want to change USB ID, you can do it with following 
steps, but you need Gauche scheme interpreter to generate USB 
descriptors and sdcc (Small Device C Compiler) to compile
tusb3410's firmware.

   1) Change VID/PID in tusb3410/Makefile
      
   2) (cd tusb3410; make)

   3) mv tusb3410_autoexec_firmware.inc tusb3410_autoexec_firmware.bak

   4) cp tusb3410/tusb3410_autoexec_firmware.inc .

   5) (cd dfw; make distclean; make dist)

   6) Now, new VID and PID are in hex files.

3) is optional.

------------ References -----------------------------------------

MSPGCC  http://mspgcc.sourceforge.net/     (MSP430 toolchain)

SLAU278 MSP430 Hardware Tools User's Guide (Schematics for FET430UIF)
SLAU292 eZ430-Chronos Development Tool User's Guide (Schematics for eZ430U)
SLAU176 eZ430-F2013 Development Tool User's Guide (Schematics for eZ430U Rev 1.1)
SLAA276 MSP430 USB Connectivity Using TUSB3410   (I2C reference code)
SLAU265 MSP430 Memory Programming User's Guide  (JTAG/SBW reference code)

SLLS519 TUSB3410, TUSB3410I Data Manual      (EEPROM header format, BSL)
uticom  http://sourceforge.net/projects/uticom/files (TUSB3410 firmware)
SDCC    http://sdcc.sourceforge.net/         (Compiler for TUSB3410)
Gauche  http://practical-scheme.net/gauche/  (for USB Descriptor tool)

;;; EOF
