#pragma once

#include "cm.h"

#define  OPTO_START_FRAME      0x7E
#define  OPTO_END_FRAME        0x7D
#define  OPTO_ESCAPE           0x7C
#define  OPTO_STUFFED_BIT      0x20

#define  OPTO_INT_RX           0x01
#define  OPTO_INT_TX           0x02
#define  OPTO_INT_PIPE         0x04
#define  OPTO_INT_ALL          0x08

#define  OPTO_OK               0x00000000
#define  OPTO_ERROR            0x80000001
#define  OPTO_ERR_MSG_NULL     0x80000002
#define  OPTO_ERR_LEN_NULL     0x80000004
#define  OPTO_ERR_LEN_MAX      0x80000008
#define  OPTO_ERR_FRAMING      0x80000010
#define  OPTO_ERR_OVERRUN      0x80000020
#define  OPTO_ERR_PARITY       0x80000040
#define  OPTO_ERR_TX_DROP      0x80000080
#define  OPTO_ERR_RX_DROP      0x80000100
#define  OPTO_ERR_CRC          0x80000200
#define  OPTO_ERR_OPEN         0x80000400

#define  OPTO_OP_START         0x00000001
#define  OPTO_OP_STOP          0x00000002
#define  OPTO_OP_BYPASS        0x00000004

#define  OPTO_MSGLEN_UINT8     512
#define  OPTO_MSGLEN_UINT32    (OPTO_MSGLEN_UINT8 >> 2)

#define  OPTO_PIPELEN_UINT8    1024

#define  OPTO_TX_BUFFER_LEN    2048
#define  OPTO_RX_BUFFER_LEN    2048

#define  OPTO_TX_QUE           8
#define  OPTO_RX_QUE           8

#define  OPTO_TX_SLOTS        (OPTO_TX_BUFFER_LEN / OPTO_MSGLEN_UINT8)
#define  OPTO_RX_SLOTS        (OPTO_RX_BUFFER_LEN / OPTO_MSGLEN_UINT8)

// Interrupt Register
typedef union _opto_int_reg_t {
   struct {
      uint32_t tx   : 1;  // opto_INT(0)
      uint32_t rx   : 1;  // opto_INT(1)
      uint32_t pipe : 1;  // opto_INT(2)
      uint32_t      : 29;
   } b;
   uint32_t i;
} opto_int_reg_t, *popto_int_reg_t;

// Control Register
typedef union _opto_ctl_reg_t {
   struct {
      uint32_t tx_head        : 2;  // opto_CONTROL(1:0)
      uint32_t rx_tail        : 2;  // opto_CONTROL(3:2)
      uint32_t                : 21; // opto_CONTROL(24:4)
      uint32_t pipe_int       : 1;  // opto_CONTROL(25)
      uint32_t dma_req        : 1;  // opto_CONTROL(26)
      uint32_t rx_int         : 1;  // opto_CONTROL(27)
      uint32_t tx_int         : 1;  // opto_CONTROL(28)
      uint32_t pipe_run       : 1;  // opto_CONTROL(29)
      uint32_t opto_run       : 1;  // opto_CONTROL(30)
      uint32_t enable         : 1;  // opto_CONTROL(31)
   } b;
   uint32_t i;
} opto_ctl_reg_t, *popto_ctl_reg_t;

// Status Register
typedef union _opto_ctl_sta_t {
   struct {
      uint32_t tail_addr      : 16; // opto_STATUS(15:0)
      uint32_t rx_head        : 2;  // opto_STATUS(17:16)
      uint32_t tx_tail        : 2;  // opto_STATUS(19:18)
      uint32_t                : 9;  // opto_STATUS(28:20)
      uint32_t rx_rdy         : 1;  // opto_STATUS(29)
      uint32_t tx_rdy         : 1;  // opto_STATUS(30)
      uint32_t tx_busy        : 1;  // opto_STATUS(31)
   } b;
   uint32_t i;
} opto_sta_reg_t, *popto_sta_reg_t;

// All Registers
typedef struct _opto_regs_t {
   uint32_t       ctl;
   uint32_t       version;
   uint32_t       test_bit;
   uint32_t       irq;
   uint32_t       sta;
   uint32_t       addr_beg;
   uint32_t       addr_end;
   uint32_t       pktcnt;
   uint32_t       unused[504];
   uint32_t       rx_buf[512];
   uint32_t       tx_buf[512];
   uint32_t       pipe[512];
} opto_regs_t, *popto_regs_t;

// Transmit Queue
typedef struct _opto_txq_t {
   uint32_t     *buf[OPTO_TX_QUE];
   uint8_t       tail;
   uint8_t       head;
   uint8_t       slots;
} opto_txq_t, *popto_txq_t;

// Receive Queue
typedef struct _opto_rxq_t {
   uint32_t     *buf[OPTO_RX_QUE];
   uint8_t       tail;
   uint8_t       head;
   uint8_t       slots;
} opto_rxq_t, *popto_rxq_t;

uint32_t  opto_init(uint32_t baudrate, uint8_t port);
void      opto_isr(void *arg);
void      opto_intack(uint8_t int_type);
void      opto_tx(pcm_msg_t msg);
void      opto_cmio(uint8_t op_code, pcm_msg_t msg);
void      opto_msgtx(void);
void      opto_pipe(uint32_t opcode, uint32_t addr_beg, uint32_t addr_end, uint32_t pktcnt);
uint32_t  opto_version(void);

