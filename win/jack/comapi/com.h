#pragma once

#define  COM_START_FRAME      0x7E
#define  COM_END_FRAME        0x7D
#define  COM_ESCAPE           0x7C
#define  COM_STUFFED_BIT      0x20

#define  COM_RX_TIMEOUT       100
#define  COM_TX_TIMEOUT       100
#define  COM_THREAD_TIMEOUT   50
#define  COM_RX_Q_SIZE        1024
#define  COM_TX_Q_SIZE        1024

#define  COM_EPID_NONE        0x00
#define  COM_EPID_NEXT        0x20
#define  COM_EPID_CTL         0x40
#define  COM_EPID_PIPE        0x80

#define  COM_TX_QUE           8
#define  COM_RX_QUE           1

#define  COM_RX_IDLE          0
#define  COM_RX_TYPE          1
#define  COM_RX_MSG           2
#define  COM_RX_MSG_ESC       3
#define  COM_RX_PIPE          4
#define  COM_RX_PIPE_ESC      5

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
	uint8_t      *buf[COM_TX_QUE];
	uint8_t       slots;
	uint16_t      len[COM_TX_QUE];
	uint16_t      n;
} com_txq_t,    *pcom_txq_t;

