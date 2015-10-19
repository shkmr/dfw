/*
 *  DFW host interface
 *
 */

#define DFW_OK       0
#define DFW_ERROR    1

void    libdfw_init(char *fet);
void    libdfw_finish(void);
void    libdfw_set_debug_level(int n);

#define DFW_TARGET_NONE            "NONE"
#define DFW_TARGET_MSP430          "MSP430"
#define DFW_TARGET_MSP430Xv1       "CPUXV1"
#define DFW_TARGET_MSP430Xv2       "CPUXV2"
#define DFW_TARGET_MSP430X         MSP430Xv1
#define DFW_TARGET_CPUX            MSP430Xv1
#define DFW_TARGET_CPUXv2          MSP430Xv2

#define DFW_TAP_NONE               "NONE"
#define DFW_TAP_JTAG               "JTAG"
#define DFW_TAP_SBW                "SBW"

#define ERASE_GLOB                 "GLOB"
#define ERASE_ALLMAIN              "ALLM"
#define ERASE_MASS                 "MASS"
#define ERASE_MAIN                 "MAIN"
#define ERASE_SGMT                 "SGMT"


int     dfw_open_target            (const char *options);
int     dfw_close_target           (void);

int     dfw_target_is_cpu_stopped  (void);
int     dfw_target_stop_cpu        (void);
int     dfw_target_continue_cpu    (void);
int     dfw_target_step_cpu        (int);

int     dfw_target_read_registers  (uint8_t *regbuf, uint32_t len);
int     dfw_target_write_register  (int n, uint32_t val);

void    dfw_swab_uint16            (void *buf, unsigned int len);
void    dfw_swab_uint32            (void *buf, unsigned int len);


int     dfw_target_erase_flash     (const char *mode, uint32_t addr);
int     dfw_target_read_memory     (uint32_t addr, uint8_t *dst, uint32_t len);
int     dfw_target_write_memory    (uint32_t addr, const uint8_t *src, uint32_t len);
int     dfw_target_write_flash     (uint32_t addr, const uint8_t *src, uint32_t len);

int     dfw_enter_updater          (void);
int     dfwup_erase_flash          (void);
int     dfwup_PUC                  (void);

int     dfwup_read_memory          (uint32_t addr, uint8_t *buf, uint32_t len);
#define dfwup_write_flash          dfw_target_write_flash

/* EOF */
