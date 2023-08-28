#pragma once

#define  LIBSP_START_FRAME      0x7E
#define  LIBSP_END_FRAME        0x7D
#define  LIBSP_ESCAPE           0x7C
#define  LIBSP_STUFFED_BIT      0x20

#define  LIBSP_RX_TIMEOUT       100
#define  LIBSP_TX_TIMEOUT       100
#define  LIBSP_THREAD_TIMEOUT   50
#define  LIBSP_RX_Q_SIZE        1024
#define  LIBSP_TX_Q_SIZE        1024

#define  LIBSP_EPID_NONE        0x00
#define  LIBSP_EPID_NEXT        0x20
#define  LIBSP_EPID_CTL         0x40
#define  LIBSP_EPID_PIPE        0x80

#define  LIBSP_TX_QUE           8
#define  LIBSP_RX_QUE           1

#define  LIBSP_RX_IDLE          0
#define  LIBSP_RX_TYPE          1
#define  LIBSP_RX_MSG           2
#define  LIBSP_RX_MSG_ESC       3
#define  LIBSP_RX_PIPE          4
#define  LIBSP_RX_PIPE_ESC      5

// receive queue
typedef struct _libsp_rxq_t {
	uint8_t       state;
	uint16_t      count;
	uint8_t       slotid;
	uint8_t       type;
	pcmq_t        slot;
	pcm_msg_t     msg;
} libsp_rxq_t,    *plibsp_rxq_t;

// transmit queue
typedef struct _libsp_txq_t {
	uint8_t       state;
	uint8_t       head;
	uint8_t       tail;
	uint8_t      *buf[LIBSP_TX_QUE];
	uint8_t       slots;
	uint16_t      len[LIBSP_TX_QUE];
	uint16_t      n;
} libsp_txq_t,    *plibsp_txq_t;

