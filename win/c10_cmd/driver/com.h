#pragma once

#include "cm.h"

// Device Info
typedef struct _com_dev_info_t {
   uint32_t    flags;
   uint32_t    type;
   uint32_t    id;
   uint32_t    locid;
   char        serial[16];
   char        desc[64];
   void       *handle;
} com_dev_info_t, *pcom_dev_info_t;

#define  COM_START_FRAME      0x7E
#define  COM_END_FRAME        0x7D
#define  COM_ESCAPE           0x7C
#define  COM_STUFFED_BIT      0x20

#define  COM_OK               0x00000000
#define  COM_ERROR            0x80000001
#define  COM_ERR_MSG_NULL     0x80000002
#define  COM_ERR_LEN_NULL     0x80000004
#define  COM_ERR_LEN_MAX      0x80000008
#define  COM_ERR_FRAMING      0x80000010
#define  COM_ERR_OVERRUN      0x80000020
#define  COM_ERR_PARITY       0x80000040
#define  COM_ERR_TX_DROP      0x80000080
#define  COM_ERR_RX_DROP      0x80000100
#define  COM_ERR_CRC          0x80000200
#define  COM_ERR_OPEN         0x80000400
#define  COM_ERR_RESP         0x80000800
#define  COM_ERR_THREAD       0x80001000
#define  COM_ERR_INFO         0x80002000
#define  COM_ERR_DEV          0x80004000
#define  COM_ERR_DEV_CNT      0x80008000
#define  COM_ERR_POOL         0x80010000
#define  COM_ERR_BAUDRATE     0x80020000

#define  COM_MSGLEN_UINT8     512
#define  COM_MSGLEN_UINT32    (COM_MSGLEN_UINT8 >> 2)
#define  COM_POOL_SLOTS       256
#define  COM_PIPELEN_UINT8    1024
#define  COM_PACKET_CNT       4
#define  COM_BLOCK_LEN        (COM_PACKET_CNT * COM_PIPELEN_UINT8)

#define  COM_MAX_DEVICES      256
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

uint32_t  com_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port);
void      com_tx(pcm_msg_t msg);
void      com_cmio(uint8_t op_code, pcm_msg_t msg);
void      com_head(void);
void      com_final(void);

