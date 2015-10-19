/* Copyright (C) 2002 Chris Liechti and Steve Underwood
 
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
 
     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
     3. The name of the author may not be used to endorse or promote products
        derived from this software without specific prior written permission.
 
   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
   EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 
   Implementation of a dummy `skeleton' target for the GDB proxy server.
   
   Exported Data:
     skeletone_target            - target descriptor of the `skeleton' target 
  
   Imported Data:
     None     
  
   Static Data:
     skeleton_XXXX               - static data representing status and 
                                   parameters of the target 
  
   Global Functions:  
     None
  
   Static Functions:  
     skeleton_XXXX              - methods comprising the `skeleton' target.
                                  A description is in file gdbproxy.h
 
     skeleton_                  - local finctions
     skeleton_command
 
   $Id: target_skeleton.c,v 1.1.1.1 2006/04/10 01:57:25 skimu Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>

#include "gdbproxy.h"
#include "libdfw.h"

#if HAVE_GETOPT_H
#include <getopt.h>
#elif WIN32
#include "getopt.h"
#elif defined(__FreeBSD__)
#include "getopt.h"
#endif

/* Note: we are using prefix 'uifdfw' for static stuff in
   order to simplify debugging of the target code itself */

/* TODO: Put the correct values for the real target in these macros */
#define RP_UIFDFW_MIN_ADDRESS             0x0U
#define RP_UIFDFW_MAX_ADDRESS             0xFFFFU
#define RP_UIFDFW_REG_DATATYPE            uint16_t
#define RP_UIFDFW_REG_BYTES               (16*sizeof(uint16_t))
#define RP_UIFDFW_NUM_REGS                16
#define RP_UIFDFW_REGNUM_PC               0  /* Program counter reg. number */
#define RP_UIFDFW_REGNUM_SP               1  /* Stack pointer reg. number */
#define RP_UIFDFW_REGNUM_FP               4  /* Frame pointer reg. number */

#define RP_UIFDFW_MAX_BREAKPOINTS         10

/* Some example states a real target might support. */
#define RP_UIFDFW_TARGET_STATE_RUNNING                    0
#define RP_UIFDFW_TARGET_STATE_STOPPED                    1
#define RP_UIFDFW_TARGET_STATE_SINGLE_STEP_COMPLETE       2
#define RP_UIFDFW_TARGET_STATE_BREAKPOINT_HIT             3

/*
 * Target methods, static
 */
static void  uifdfw_help(const char *prog_name);
static int   uifdfw_open(int argc,
                           char * const argv[],
                           const char *prog_name,
                           log_func log_fn);
static void  uifdfw_close(void);
static int   uifdfw_connect(char *status_string,
                              size_t status_string_size,
                              int *can_restart);
static int   uifdfw_disconnect(void);
static void  uifdfw_kill(void);
static int   uifdfw_restart(void);
static void  uifdfw_stop(void);
static int   uifdfw_set_gen_thread(rp_thread_ref *thread);
static int   uifdfw_set_ctrl_thread(rp_thread_ref *thread);
static int   uifdfw_is_thread_alive(rp_thread_ref *thread, int *alive);
static int   uifdfw_read_registers(uint8_t *data_buf,
                                     uint8_t *avail_buf,
                                     size_t buf_size,
                                     size_t *read_size);
static int   uifdfw_write_registers(uint8_t *data_buf, size_t write_size);
static int   uifdfw_read_single_register(unsigned int reg_no,
                                           uint8_t *data_buf,
                                           uint8_t *avail_buf,
                                           size_t buf_size,
                                           size_t *read_size);
static int   uifdfw_write_single_register(unsigned int reg_no,
                                            uint8_t *data_buf,
                                            size_t write_size);
static int   uifdfw_read_mem(uint64_t addr,
                               uint8_t *data_buf,
                               size_t req_size,
                               size_t *actual_size);
static int   uifdfw_write_mem(uint64_t addr,
                                uint8_t *data_buf,
                                size_t req_sise);
static int   uifdfw_resume_from_current(int step, int sig);
static int   uifdfw_resume_from_addr(int step,
                                       int sig,
                                       uint64_t addr);
static int   uifdfw_go_waiting(int sig);
static int   uifdfw_wait_partial(int first,
                                   char *status_string,
                                   size_t status_string_len,
                                   out_func out,
                                   int *implemented,
                                   int *more);
static int   uifdfw_wait(char *status_string,
                           size_t status_string_len,
                           out_func out,
                           int *implemented);
static int   uifdfw_process_query(unsigned int *mask,
                                    rp_thread_ref *arg,
                                    rp_thread_info *info);
static int   uifdfw_list_query(int first,
                                 rp_thread_ref *arg,
                                 rp_thread_ref *result,
                                 size_t max_num,
                                 size_t *num,
                                 int *done);
static int   uifdfw_current_thread_query(rp_thread_ref *thread);
static int   uifdfw_offsets_query(uint64_t *text,
                                    uint64_t *data,
                                    uint64_t *bss);
static int   uifdfw_crc_query(uint64_t addr,
                                size_t len,
                                uint32_t *val);
static int   uifdfw_raw_query(char *in_buf,
                                char *out_buf,
                                size_t out_buf_size);
static int   uifdfw_remcmd(char *in_buf, out_func of, data_func df);
static int   uifdfw_add_break(int type, uint64_t addr, unsigned int len);
static int   uifdfw_remove_break(int type, uint64_t addr, unsigned int len);

//Table of remote commands following
static int uifdfw_rcmd_help(int argc, char *argv[], out_func of, data_func df);  //proto for table entry

static uint32_t crc32(uint8_t *buf, size_t len, uint32_t crc);

#define RCMD(name, hlp) {#name, uifdfw_rcmd_##name, hlp}  //table entry generation

//Table entry definition
typedef struct
{
    const char *name;                                   // command name
    int (*function)(int, char**, out_func, data_func);  // function to call
    const char *help;                                   // one line of help text
} RCMD_TABLE;

/*
 * Global target descriptor 
 */
rp_target uifdfw_target =
{
    NULL,      /* next */
    "uifdfw",
    "uifdfw FET430UIF/eZ430u with dfw firmware",
    uifdfw_help,
    uifdfw_open,
    uifdfw_close,
    uifdfw_connect,
    uifdfw_disconnect,
    uifdfw_kill,
    uifdfw_restart,
    uifdfw_stop,
    uifdfw_set_gen_thread,
    uifdfw_set_ctrl_thread,
    uifdfw_is_thread_alive,
    uifdfw_read_registers,
    uifdfw_write_registers,
    uifdfw_read_single_register,
    uifdfw_write_single_register,
    uifdfw_read_mem,
    uifdfw_write_mem,
    uifdfw_resume_from_current,
    uifdfw_resume_from_addr,
    uifdfw_go_waiting,
    uifdfw_wait_partial,
    uifdfw_wait,
    uifdfw_process_query,
    uifdfw_list_query,
    uifdfw_current_thread_query,
    uifdfw_offsets_query,
    uifdfw_crc_query,
    uifdfw_raw_query,
    uifdfw_remcmd,
    uifdfw_add_break,
    uifdfw_remove_break
};

struct uifdfw_status_s
{
    /* Start up parameters, set by uifdfw_open */
    log_func    log;
    int         is_open;

    /* Tell wait_xxx method the notion of whether or not
       previously called resume is supported */
    int         target_running;
    int         target_interrupted;
    RP_UIFDFW_REG_DATATYPE    registers[RP_UIFDFW_NUM_REGS];
    uint64_t                  breakpoints[RP_UIFDFW_MAX_BREAKPOINTS];
};

static struct uifdfw_status_s uifdfw_status =
{
    NULL,
    FALSE,
    FALSE,
    FALSE,
    {0},
    {0}
};

static int write_flash = 0;

/* Local functions */
static char *uifdfw_out_treg(char *in, unsigned int reg_no);
static int refresh_registers(void);

/* Target method */

#ifdef NDEBUG
#define DEBUG_OUT(...)
#else
static void DEBUG_OUT(const char *string,...)
{
    va_list args;
    va_start (args, string);
    fprintf (stderr, "debug: ");
    vfprintf (stderr, string, args);
    fprintf (stderr, "\n");
    va_end (args);
}
#endif

static void uifdfw_help(const char *prog_name)
{
    printf("This is GDB proxy for FET430UIF/eZ430U with dfw. Usage:\n\n");
    printf("  %s [options] %s [uifdfw-options]\n",
           prog_name,
           uifdfw_target.name);
    printf("\nOptions:\n\n");
    printf("  --debug              run %s in debug mode\n", prog_name);
    printf("  --help               `%s --help %s'  prints this message\n",
           prog_name,
           uifdfw_target.name);
    printf("  --port=PORT          use the specified TCP port\n");
    printf("\n");
    printf("uifdfw-options:\n\n");
    printf("  --c string           target-options string\n");
    printf("  --debug              set libdfw's debug flag\n");
    printf("\n");
    printf("target-options: (See dfw/cmd.c dfw/target_*.c for details)\n\n"
           "   MSP430    : (default) msp430x1xx, msp430x2xx, msp430x4xx\n"
           "   CPUXV1    :           msp430x2xx, msp430x4xx\n"
           "   CPUXV2    :           msp430x5xx, cc430x6xx \n"
           "   CC8051    :           cc111x, cc251x, etc.  \n"
           "\n"
           "   VCC xxxx  : sets VCC to XXXX mV (e.g., VCC 3300), FET430UIF only.\n"
           "\n"
           "   JTAG      : (default)\n"
           "   SBW       :          \n"
           "\n"
           "   FastFlash : (default)\n"
           "   SlowFlash :           for relatively old devices without FastFlash feature\n"
           "   LOCKA     : (default) locks INFO_A memory\n"
           "   UNLOCKA   :           unlocks INFO_A memory\n"
           "\n"
           "   FCTL addr : sets FCTL base address for CPUXV2 target.\n"
           "\n"
           "   FWS  n    : sets flash word size for CC8051 target.\n"
           "\n"
           );
    printf("Examples:\n\n");
    printf("   %s --port=2000 %s --c \"SBW\"\n", 
           prog_name, uifdfw_target.name);
    printf("   %s --port=2000 %s --c \"CPUXV1\"\n", 
           prog_name, uifdfw_target.name);
    printf("   %s --port=2000 %s --c \"CPUXV2 SBW\"\n", 
           prog_name, uifdfw_target.name);
    printf("   %s --port=2000 %s --c \"CPUXV2 SBW VCC 3300\" --debug\n\n",
           prog_name, uifdfw_target.name);

    return;
}

/* Target method */
static int uifdfw_open(int argc,
                         char * const argv[],
                         const char *prog_name,
                         log_func log_fn)
{
    char    *dfw_tap_config  =  NULL;
    char    *fet             =  NULL;
    int      libdfw_debug    =  0;

    static struct option long_options[] =
    {
        /* Options setting flag */
        {"debug",   0, 0, 1},
        {"c",       1, 0, 2},
        {"p",       1, 0, 3},
        {NULL,      0, 0, 0}
    };

    assert(!uifdfw_status.is_open);
    assert(prog_name != NULL);
    assert(log_fn != NULL);

    /* Set log */
    uifdfw_status.log = log_fn;

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_open()",
                        uifdfw_target.name);

    /* Process options */
    for (;;)
    {
        int c;
        int option_index;

        c = getopt_long(argc, argv, "+", long_options, &option_index);
        if (c == EOF)
            break;
        switch (c)
        {
        case 1:
            libdfw_debug = 1;
            break;
        case 2:
            dfw_tap_config = optarg;
            break;
        case 3:
            fet            = optarg;
            break;
        default:
            uifdfw_status.log(RP_VAL_LOGLEVEL_NOTICE,
                                "%s: Use `%s --help %s' to see a complete list of options",
                                uifdfw_target.name,
                                prog_name,
                                uifdfw_target.name);
            return RP_VAL_TARGETRET_ERR;
        }
    }

    if (optind != argc)
    {
        /* Bad number of arguments */
        uifdfw_status.log(RP_VAL_LOGLEVEL_ERR,
                            "%s: bad number of arguments",
                            uifdfw_target.name);
        uifdfw_target.help(prog_name);

        return RP_VAL_TARGETRET_ERR;
    }

    if (!uifdfw_status.is_open)
    {
        /* TODO: initialise the target interface */
        libdfw_set_debug_level(libdfw_debug);
        libdfw_init(fet);
        dfw_open_target(dfw_tap_config);
    }
    /* Set up initial default values */
    uifdfw_status.target_running = FALSE;
    uifdfw_status.target_interrupted = FALSE;
    memset (uifdfw_status.registers, 0, sizeof(uifdfw_status.registers));
    memset (uifdfw_status.breakpoints, 0, sizeof(uifdfw_status.breakpoints));

    uifdfw_status.is_open = TRUE;

    return RP_VAL_TARGETRET_OK;
}


/* Target method */
static void uifdfw_close(void)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_close()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    //dfw_close_target();
    libdfw_finish();

    uifdfw_status.is_open = FALSE;
}

/* Target method */
static int uifdfw_connect(char *status_string,
                            size_t status_string_len,
                            int *can_restart)
{
    char *cp;

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_connect()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    assert(status_string != NULL);
    assert(status_string_len >= 34);
    assert(can_restart != NULL);

    *can_restart = TRUE;

    /* Fill out the the status string */
    sprintf(status_string, "T%02d", RP_SIGNAL_ABORTED);

    if (refresh_registers())
        return RP_VAL_TARGETRET_ERR;
    
    cp = uifdfw_out_treg(&status_string[3], RP_UIFDFW_REGNUM_PC);
    cp = uifdfw_out_treg(cp, RP_UIFDFW_REGNUM_FP);

    return (cp != NULL)  ?  RP_VAL_TARGETRET_OK  :  RP_VAL_TARGETRET_ERR;
}

/* Target method */
static int uifdfw_disconnect(void)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_disconnect()",
                        uifdfw_target.name);

    return RP_VAL_TARGETRET_NOSUPP;
}

/* Target method */
static void uifdfw_kill(void)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_kill()",
                        uifdfw_target.name);

    dfw_close_target();
}

static int uifdfw_restart(void)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_restart()",
                        uifdfw_target.name);

    /* Just stop it. The actual restart will be done
       when connect is called again */
    uifdfw_stop();

    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static void uifdfw_stop(void)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_stop()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    dfw_target_stop_cpu();

    uifdfw_status.target_interrupted = TRUE;
}

static int uifdfw_set_gen_thread(rp_thread_ref *thread)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_set_gen_thread()",
                        uifdfw_target.name);

    return RP_VAL_TARGETRET_NOSUPP;
}

/* Target method */
static int uifdfw_set_ctrl_thread(rp_thread_ref *thread)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_set_ctrl_thread()",
                        uifdfw_target.name);

    return RP_VAL_TARGETRET_NOSUPP;
}

/* Target method */
static int uifdfw_is_thread_alive(rp_thread_ref *thread, int *alive)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_is_thread_alive()",
                        uifdfw_target.name);

    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_read_registers(uint8_t *data_buf,
                                 uint8_t *avail_buf,
                                 size_t buf_size,
                                 size_t *read_size)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_read_registers()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    assert(data_buf != NULL);
    assert(avail_buf != NULL);
    assert(buf_size >= RP_UIFDFW_REG_BYTES);
    assert(read_size != NULL);

    if (refresh_registers())
        return RP_VAL_TARGETRET_ERR;

    memcpy(data_buf, uifdfw_status.registers, RP_UIFDFW_REG_BYTES);
    memset(avail_buf, 1, RP_UIFDFW_REG_BYTES);
    *read_size = RP_UIFDFW_REG_BYTES;
    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_write_registers(uint8_t *buf, size_t write_size)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_write_registers()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    assert(buf != NULL);
    assert(write_size > 0);
    assert(write_size <= RP_UIFDFW_REG_BYTES);

    memcpy(uifdfw_status.registers, buf, write_size);

    {
        int n;
        for (n = 0; n < 16; n++)
            dfw_target_write_register(n, uifdfw_status.registers[n]);
    }

    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_read_single_register(unsigned int reg_no,
                                         uint8_t *data_buf,
                                         uint8_t *avail_buf,
                                         size_t buf_size,
                                         size_t *read_size)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_read_single_register()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    assert(data_buf != NULL);
    assert(avail_buf != NULL);
    assert(buf_size >= RP_UIFDFW_REG_BYTES);
    assert(read_size != NULL);

    if (reg_no < 0  ||  reg_no > RP_UIFDFW_NUM_REGS)
        return RP_VAL_TARGETRET_ERR;

    if (refresh_registers())
        return RP_VAL_TARGETRET_ERR;

    memcpy(data_buf, &uifdfw_status.registers[reg_no], sizeof(uifdfw_status.registers[reg_no]));
    memset(avail_buf, 1, sizeof(uifdfw_status.registers[reg_no]));
    *read_size = sizeof(uifdfw_status.registers[reg_no]);
    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_write_single_register(unsigned int reg_no,
                                          uint8_t *buf,
                                          size_t write_size)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_write_single_register(%d, 0x%X)",
                        uifdfw_target.name,
                        reg_no,
                        ((RP_UIFDFW_REG_DATATYPE *) buf)[0]);

    assert(uifdfw_status.is_open);

    assert(buf != NULL);
    assert(write_size == 2);

    if (reg_no < 0  ||  reg_no > RP_UIFDFW_NUM_REGS)
        return RP_VAL_TARGETRET_ERR;

    uifdfw_status.registers[reg_no] = ((RP_UIFDFW_REG_DATATYPE *) buf)[0];

    dfw_target_write_register(reg_no, uifdfw_status.registers[reg_no]);

    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_read_mem(uint64_t addr,
                             uint8_t *buf,
                             size_t req_size,
                             size_t *actual_size)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_read_mem(0x%llX, ptr, %d, ptr)",
                        uifdfw_target.name,
                        addr,
                        req_size);

    assert(uifdfw_status.is_open);

    assert(buf != NULL);
    assert(req_size > 0);
    assert(actual_size != NULL);

    if (addr > RP_UIFDFW_MAX_ADDRESS)
    {
        uifdfw_status.log(RP_VAL_LOGLEVEL_ERR,
                            "%s: bad address 0x%llx",
                            uifdfw_target.name,
                            addr);

        return RP_VAL_TARGETRET_ERR;
    }

    if (addr + req_size > RP_UIFDFW_MAX_ADDRESS + 1)
        *actual_size = RP_UIFDFW_MAX_ADDRESS + 1 - addr;
    else
        *actual_size = req_size;

    /* TODO: Read the required block of memory from the target. */
    {
        dfw_target_read_memory(addr, buf, *actual_size);
    }

    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_write_mem(uint64_t addr,
                              uint8_t *buf,
                              size_t write_size)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_write_mem(0x%llX, ptr, %d)",
                        uifdfw_target.name,
                        addr,
                        write_size);

    assert(uifdfw_status.is_open);
    assert(buf != NULL);

    /* GDB does zero length writes for some reason. Treat them harmlessly. */
    if (write_size == 0)
        return RP_VAL_TARGETRET_OK;

    if (addr > RP_UIFDFW_MAX_ADDRESS)
    {
        uifdfw_status.log(RP_VAL_LOGLEVEL_ERR,
                            "%s: bad address 0x%llx",
                            uifdfw_target.name,
                            addr);
        return RP_VAL_TARGETRET_ERR;
    }

    if ((addr + write_size - 1) > RP_UIFDFW_MAX_ADDRESS)
    {
        uifdfw_status.log(RP_VAL_LOGLEVEL_ERR,
                            "%s: bad address/write_size 0x%llx/0x%x",
                            uifdfw_target.name,
                            addr,
                            write_size);
        return RP_VAL_TARGETRET_ERR;
    }

    /* TODO: Write to the target. */
    {
        if (write_flash)
            dfw_target_write_flash(addr, buf, write_size);
        else
            dfw_target_write_memory(addr, buf, write_size);
    }

    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_resume_from_current(int step, int sig)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_resume_from_current(%s, %d)",
                        uifdfw_target.name,
                        (step)  ?  "step"  :  "run",
                        sig);

    assert(uifdfw_status.is_open);

    if (step) { 
       /* TODO: Single step the target */
        dfw_target_step_cpu(1);
    } else {
        /* TODO: Run the target to a breakpoint, or until we stop it. */;
        dfw_target_continue_cpu();
    }
    uifdfw_status.target_running = TRUE;
    uifdfw_status.target_interrupted = FALSE;
    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_resume_from_addr(int step, int sig, uint64_t addr)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_resume_from_addr(%s, %d, 0x%llX)",
                        uifdfw_target.name,
                        (step)  ?  "step"  :  "run",
                        sig,
                        addr);

    assert(uifdfw_status.is_open);

    uifdfw_status.registers[RP_UIFDFW_REGNUM_PC] = addr;

    /* TODO: Update the PC register in the target */
    dfw_target_write_register(RP_UIFDFW_REGNUM_PC,
                              uifdfw_status.registers[RP_UIFDFW_REGNUM_PC]);
    /* TODO: Run the target from the new PC address. */
    dfw_target_continue_cpu();

    uifdfw_status.target_running = TRUE;
    uifdfw_status.target_interrupted = FALSE;
    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_go_waiting(int sig)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_go_waiting()",
                        uifdfw_target.name);
    return RP_VAL_TARGETRET_NOSUPP;
}

/* Target method */
static int uifdfw_wait_partial(int first,
                                 char *status_string,
                                 size_t status_string_len,
                                 out_func of,
                                 int *implemented,
                                 int *more)
{
    int state;
    char *cp;
    int sig;

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_wait_partial()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    assert(status_string != NULL);
    assert(status_string_len >= 34);
    assert(of != NULL);
    assert(implemented != NULL);
    assert(more != NULL);

    *implemented = TRUE;

    if (!uifdfw_status.target_running)
        return RP_VAL_TARGETRET_NOSUPP;

#ifdef WIN32
    sleep((first)  ?  500  :  100);
#else
    usleep((first)  ?  500000  :  100000);
#endif
    /* TODO: Test the target state (i.e. running/stopped) without blocking */
    /* If the target only supports a blocking form of test return no support,
       and the blocking version of this test will be called instead. That is
       not so nice, as the system is less interactive using a blocking test. */
    if (dfw_target_is_cpu_stopped()) 
        state = RP_UIFDFW_TARGET_STATE_STOPPED;
    else
        state = RP_UIFDFW_TARGET_STATE_RUNNING;

    if (state == RP_UIFDFW_TARGET_STATE_RUNNING)
    {
        *more = TRUE;
        return RP_VAL_TARGETRET_OK;
    }

    switch (state)
    {
    case RP_UIFDFW_TARGET_STATE_STOPPED:
        if (uifdfw_status.target_interrupted)
            sig = RP_SIGNAL_INTERRUPT;
        else
            sig = RP_SIGNAL_ABORTED;
        break;
    case RP_UIFDFW_TARGET_STATE_RUNNING:
        *more = TRUE;
        return RP_VAL_TARGETRET_OK;
    case RP_UIFDFW_TARGET_STATE_SINGLE_STEP_COMPLETE:
        sig = RP_SIGNAL_BREAKPOINT;
        break;
    case RP_UIFDFW_TARGET_STATE_BREAKPOINT_HIT:
        sig = RP_SIGNAL_BREAKPOINT;
        break;
    default:
        uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                            "%s: unexpected state %d for the UIFDFW",
                            uifdfw_target.name,
                            state);
        sig = RP_SIGNAL_ABORTED;
        break;
    }
    /* Fill out the status string */
    sprintf(status_string, "T%02d", sig);

    if (refresh_registers())
        return RP_VAL_TARGETRET_ERR;
    
    cp = uifdfw_out_treg(&status_string[3], RP_UIFDFW_REGNUM_PC);
    cp = uifdfw_out_treg(cp, RP_UIFDFW_REGNUM_FP);

    *more = FALSE;

    return (cp != NULL)  ?  RP_VAL_TARGETRET_OK  :  RP_VAL_TARGETRET_ERR;
}

/* Target method */
static int uifdfw_wait(char *status_string,
                         size_t status_string_len,
                         out_func of,
                         int *implemented)
{
    int state;
    char *cp;
    int sig;

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_wait()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);
    assert(status_string != NULL);
    assert(status_string_len >= 34);
    assert(of != NULL);
    assert(implemented != NULL);

    *implemented = TRUE;

    if (!uifdfw_status.target_running)
        return RP_VAL_TARGETRET_NOSUPP;

    /* TODO: Wait for the target to stop */
    while (!dfw_target_is_cpu_stopped())  sleep(1);

    state = RP_UIFDFW_TARGET_STATE_STOPPED;

    switch (state)
    {
    case RP_UIFDFW_TARGET_STATE_STOPPED:
        sig = RP_SIGNAL_ABORTED;
        break;
    case RP_UIFDFW_TARGET_STATE_SINGLE_STEP_COMPLETE:
        sig = RP_SIGNAL_BREAKPOINT;
        break;
    case RP_UIFDFW_TARGET_STATE_BREAKPOINT_HIT:
        sig = RP_SIGNAL_BREAKPOINT;
        break;
    default:
        uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                            "%s: unexpected state %d for the UIFDFW",
                            uifdfw_target.name,
                            state);
        sig = RP_SIGNAL_ABORTED;
        break;
    }
    /* Fill out the status string */
    sprintf(status_string, "T%02d", sig);

    if (refresh_registers())
        return RP_VAL_TARGETRET_ERR;
    
    cp = uifdfw_out_treg(&status_string[3], RP_UIFDFW_REGNUM_PC);
    cp = uifdfw_out_treg(cp, RP_UIFDFW_REGNUM_FP);

    return (cp != NULL)  ?  RP_VAL_TARGETRET_OK  :  RP_VAL_TARGETRET_ERR;
}

/* Target method */
static int uifdfw_process_query(unsigned int *mask,
                                  rp_thread_ref *arg,
                                  rp_thread_info *info)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_process_query()",
                        uifdfw_target.name);
    /* TODO: Does your target support threads? Is so, implement this function.
       Otherwise just return no support. */
    return RP_VAL_TARGETRET_NOSUPP;
}

/* Target method */
static int uifdfw_list_query(int first,
                               rp_thread_ref *arg,
                               rp_thread_ref *result,
                               size_t max_num,
                               size_t *num,
                               int *done)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_list_query()",
                        uifdfw_target.name);
    /* TODO: Does your target support threads? Is so, implement this function.
       Otherwise just return no support. */
    return RP_VAL_TARGETRET_NOSUPP;
}

/* Target method */
static int uifdfw_current_thread_query(rp_thread_ref *thread)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_current_thread_query()",
                        uifdfw_target.name);
    /* TODO: Does your target support threads? Is so, implement this function.
       Otherwise just return no support. */
    return RP_VAL_TARGETRET_NOSUPP;
}

/* Target method */
static int uifdfw_offsets_query(uint64_t *text, uint64_t *data, uint64_t *bss)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_offsets_query()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    assert(text != NULL);
    assert(data != NULL);
    assert(bss != NULL);

    /* TODO: Is this what *your* target really needs? */
    *text = 0; 
    *data = 0;
    *bss = 0;
    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_crc_query(uint64_t addr, size_t len, uint32_t *val)
{
    uint8_t buf[1];

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_crc_query()",
                        uifdfw_target.name);

    assert(uifdfw_status.is_open);

    if (addr > RP_UIFDFW_MAX_ADDRESS  ||  addr + len > RP_UIFDFW_MAX_ADDRESS + 1)
    {
        uifdfw_status.log(RP_VAL_LOGLEVEL_ERR,
                            "%s: bad address 0x%llx",
                            uifdfw_target.name,
                            addr);

        return RP_VAL_TARGETRET_ERR;
    }

    /* TODO: Read the target memory,and use the crc32 routine to calculate
       the CRC value to be returned. */
    /* Note: The CRC can be calculated in chunks. The first call to crc32
       should set the current CRC value to all 1's, as this is the priming
       value for CRC32. Subsequent calls should set the current CRC to the
       value returned by the previous call, until all the data has been
       processed. */

    dfw_target_read_memory(addr, buf, len);

    *val = crc32(buf, sizeof(buf), 0xFFFFFFFF);

    return RP_VAL_TARGETRET_OK;
}

/* Target method */
static int uifdfw_raw_query(char *in_buf, char *out_buf, size_t out_buf_size)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_raw_query()",
                        uifdfw_target.name);

    return RP_VAL_TARGETRET_NOSUPP;
}

static int tohex(char *s, const char *t)
{
    int i;
    static char hex[] = "0123456789abcdef";

    i = 0;
    while (*t)
    {
        *s++ = hex[(*t >> 4) & 0x0f];
        *s++ = hex[*t & 0x0f];
        t++;
        i++;
    }
    *s = '\0';
    return i;
}

/* command: erase flash */
static int uifdfw_rcmd_erase(int argc, char *argv[], out_func of, data_func df)
{
    char buf[1000 + 1];

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_rcmd_erase()",
                        uifdfw_target.name);
    tohex(buf, "Erasing target flash - ");
    of(buf);

    /* TODO: perform the erase. */
    dfw_target_erase_flash(ERASE_MASS, 0xfff0);

    tohex(buf, " Erased OK\n");
    of(buf);
    return RP_VAL_TARGETRET_OK;
}

static int uifdfw_rcmd_flash(int argc, char *argv[], out_func of, data_func df)
{
    char buf[1000 + 1];

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_rcmd_flash()",
                        uifdfw_target.name);
    
    write_flash = 1;
    tohex(buf, " flash write enabled\n");
    of(buf);
    return RP_VAL_TARGETRET_OK;
}

static int uifdfw_rcmd_memory(int argc, char *argv[], out_func of, data_func df)
{
    char buf[1000 + 1];

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_rcmd_flash()",
                        uifdfw_target.name);
    
    write_flash = 0;
    tohex(buf, " flash write disabled\n");
    of(buf);
    return RP_VAL_TARGETRET_OK;
}

/* Table of commands */
static const RCMD_TABLE remote_commands[] =
{
    RCMD(help,      "This help text"),
    RCMD(erase,     "Erase target flash memory"),
    RCMD(flash,     "Prepair to write flash"),
    RCMD(memory,    "Prepair to write memory"),
    {0,0,0}     //sentinel, end of table marker
};

/* Help function, generate help text from command table */
static int uifdfw_rcmd_help(int argc, char *argv[], out_func of, data_func df)
{
    char buf[1000 + 1];
    char buf2[1000 + 1];
    int i = 0;

    tohex(buf, "Remote command help:\n");
    of(buf);
    for (i = 0;  remote_commands[i].name;  i++)
    {
#ifdef WIN32
        sprintf(buf2, "%-10s %s\n", remote_commands[i].name, remote_commands[i].help);
#else
        snprintf(buf2, 1000, "%-10s %s\n", remote_commands[i].name, remote_commands[i].help);
#endif
        tohex(buf, buf2);
        of(buf);
    }
    return RP_VAL_TARGETRET_OK;
}

/* Decode nibble */
static int remote_decode_nibble(const char *in, unsigned int *nibble)
{
    unsigned int nib;

    if ((nib = rp_hex_nibble(*in)) >= 0)
    {
        *nibble = nib;
        return  TRUE;
    }

    return  FALSE;
}


/* Decode byte */
static int remote_decode_byte(const char *in, unsigned int *byte)
{
    unsigned int ls_nibble;
    unsigned int ms_nibble;

    if (!remote_decode_nibble(in, &ms_nibble))
        return  FALSE;

    if (!remote_decode_nibble(in + 1, &ls_nibble))
        return  FALSE;

    *byte = (ms_nibble << 4) + ls_nibble;

    return  TRUE;
}


/* Target method */
#define MAXARGS 4
static int uifdfw_remcmd(char *in_buf, out_func of, data_func df)
{
    int count = 0;
    int i;
    char *args[MAXARGS];
    char *ptr;
    unsigned int ch;
    char buf[1000 + 1];
    char *s;

    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_remcmd()",
                        uifdfw_target.name);
    DEBUG_OUT("command '%s'", in_buf);

    if (strlen(in_buf))
    {
        /* There is something to process */
        /* TODO: Handle target specific commands, such as flash erase, JTAG
                 control, etc. */
        /* A single example "flash erase" command is partially implemented
           here as an example. */
        
        /* Turn the hex into ASCII */
        ptr = in_buf;
        s = buf;
        while (*ptr)
        {
            if (remote_decode_byte(ptr, &ch) == 0)
                return RP_VAL_TARGETRET_ERR;
            *s++ = ch;
            ptr += 2;
        }
        *s = '\0';
        DEBUG_OUT("command '%s'", buf);
        
        /* Split string into separate arguments */
        ptr = buf;
        args[count++] = ptr;
        while (*ptr)
        {
            /* Search to the end of the string */
            if (*ptr == ' ')
            {
                /* Space is the delimiter */
                *ptr = 0;
                if (count >= MAXARGS)
                    return RP_VAL_TARGETRET_ERR;
                args[count++] = ptr + 1;
            }
            ptr++;
        }
        /* Search the command table, and execute the function if found */
        DEBUG_OUT("executing target dependant command '%s'", args[0]);

        for (i = 0;  remote_commands[i].name;  i++)
        {
            if (strcmp(args[0], remote_commands[i].name) == 0)
                return remote_commands[i].function(count, args, of, df);
        }
        return RP_VAL_TARGETRET_NOSUPP;
    }
    return RP_VAL_TARGETRET_ERR;
}


/* Target method */
static int uifdfw_add_break(int type, uint64_t addr, unsigned int len)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_add_break(%d, 0x%llx, %d)",
                        uifdfw_target.name,
                        type,
                        addr,
                        len);
    /* TODO: Handle whichever types of breakpoint the target can support, and
       report no support for the others. */
    return RP_VAL_TARGETRET_NOSUPP;
}

/* Target method */
static int uifdfw_remove_break(int type, uint64_t addr, unsigned int len)
{
    uifdfw_status.log(RP_VAL_LOGLEVEL_DEBUG,
                        "%s: uifdfw_remove_break(%d, 0x%llx, %d)",
                        uifdfw_target.name,
                        type,
                        addr,
                        len);
    /* TODO: Handle whichever types of breakpoint the target can support, and
       report no support for the others. */
    return RP_VAL_TARGETRET_NOSUPP;
}

/* Output registers in the format suitable
   for TAAn:r...;n:r...;  format */
static char *uifdfw_out_treg(char *in, unsigned int reg_no)
{
    static const char hex[] = "0123456789abcdef";
    int16_t reg_val;

    if (in == NULL)
        return NULL;

    assert(reg_no < RP_UIFDFW_NUM_REGS);

    *in++ = hex[(reg_no >> 4) & 0x0f];
    *in++ = hex[reg_no & 0x0f];
    *in++ = ':';

    reg_val = uifdfw_status.registers[reg_no];
    /* The register goes into the buffer in little-endian order */
    *in++ = hex[(reg_val >> 4) & 0x0f];
    *in++ = hex[reg_val & 0x0f];
    *in++ = hex[(reg_val >> 12) & 0x0f];
    *in++ = hex[(reg_val >> 8) & 0x0f];
    *in++ = ';';
    *in   = '\0';

    return in;
}

/* Table used by the crc32 function to calcuate the checksum. */
static uint32_t crc32_table[256] =
{
    0,
    0
};

static uint32_t crc32(uint8_t *buf, size_t len, uint32_t crc)
{
    if (!crc32_table[1])
    {
        /* Initialize the CRC table and the decoding table. */
        int i;
        int j;
        unsigned int c;

        for (i = 0; i < 256; i++)
	{
	    for (c = i << 24, j = 8; j > 0; --j)
	        c = c & 0x80000000 ? (c << 1) ^ 0x04c11db7 : (c << 1);
	    crc32_table[i] = c;
	}
    }

    while (len--)
    {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
        buf++;
    }
    return crc;
}

static int refresh_registers(void)
{
    int i;
    uint32_t buf[16];

    dfw_target_read_registers((uint8_t *)buf, 64);

    if (0/* host endian does not match with MSP430 */)
        dfw_swab_uint32((uint8_t *)buf, 64);

    for (i = 0; i < 16; i++)
        uifdfw_status.registers[i] = buf[i];
    return  0;
}

/* 
Local Variables:
c-basic-offset: 4
End:
*/
