#ifndef UIF_H
#define UIF_H

void init_uif(void);

int  uif_set_vcct(int);

void uif_attach_jtag(void);
void uif_detach_jtag(void);

void uif_attach_sbw(void);
void uif_detach_sbw(void);

void uif_attach_cc8051(void);

void uif_reset_target(void);
void uif_assert_RST_3410(void);
void uif_negate_RST_3410(void);
void uif_reset_tusb3410(void);

void uif_usart_I2C(void);
void uif_usart_com(void);

#endif  /* UIF_H */
