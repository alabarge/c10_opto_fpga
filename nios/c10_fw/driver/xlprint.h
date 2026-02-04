#pragma once

// Status
#define  RS232_TRDY     0x0040

// Control
#define  RS232_IRRDY    0x0080

// Registers
#define  RS232_RXDATA   0x00
#define  RS232_TXDATA   0x04
#define  RS232_STATUS   0x08
#define  RS232_CONTROL  0x0C
#define  RS232_DIVISOR  0x10
#define  RS232_EOP      0x14

// All Registers
typedef struct _uart_regs_t {
   uint32_t       rx_dat;
   uint32_t       tx_dat;
   uint32_t       status;
   uint32_t       ctl;
   uint32_t       div;
   uint32_t       eop;
} uart_regs_t, *puart_regs_t;

int  xlprint(const char *format, ...);
int  xlprints(char *buf, const char *format, ...);
void xlprint_open(uint32_t devAddr);
void xlprint_isr(void *arg);
