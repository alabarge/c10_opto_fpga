#pragma once

#define  UART_START_FRAME      0x7E
#define  UART_END_FRAME        0x7D
#define  UART_ESCAPE           0x7C
#define  UART_STUFFED_BIT      0x20

#define  UART_RX_TIMEOUT       5
#define  UART_TX_TIMEOUT       5
#define  UART_THREAD_TIMEOUT   50
#define  UART_LATENCY          2

#define  UART_EPID_NONE        0x00
#define  UART_EPID_NEXT        0x20
#define  UART_EPID_CTL         0x40
#define  UART_EPID_PIPE        0x80

#define  UART_TX_QUE           8
#define  UART_RX_QUE           1

#define  UART_RX_IDLE          0
#define  UART_RX_TYPE          1
#define  UART_RX_MSG           2
#define  UART_RX_MSG_ESC       3
#define  UART_RX_PIPE          4
#define  UART_RX_PIPE_ESC      5

// receive queue
typedef struct _com_rxq_t {
   uint8_t       state;
   uint16_t      count;
   uint8_t       slotid;
   uint8_t       type;
   pcmq_t        slot;
   pcm_msg_t     msg;
} com_rxq_t,    *pcom_rxq_t;

// transmit queue
typedef struct _com_txq_t {
   uint8_t       state;
   uint8_t       head;
   uint8_t       tail;
   uint8_t      *buf[UART_TX_QUE];
   uint8_t       slots;
   uint16_t      len[UART_TX_QUE];
   uint16_t      n;
} com_txq_t,    *pcom_txq_t;
