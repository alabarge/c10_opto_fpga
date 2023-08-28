#pragma once

#include "cm.h"

#define  FIFO_OK               0x00000000
#define  FIFO_ERROR            0x80000001
#define  FIFO_ERR_MSG_NULL     0x80000002
#define  FIFO_ERR_LEN_NULL     0x80000004
#define  FIFO_ERR_LEN_MAX      0x80000008
#define  FIFO_ERR_FRAMING      0x80000010
#define  FIFO_ERR_OVERRUN      0x80000020
#define  FIFO_ERR_PARITY       0x80000040
#define  FIFO_ERR_TX_DROP      0x80000080
#define  FIFO_ERR_RX_DROP      0x80000100
#define  FIFO_ERR_CRC          0x80000200
#define  FIFO_ERR_OPEN         0x80000400
#define  FIFO_ERR_RESP         0x80000800
#define  FIFO_ERR_THREAD       0x80001000
#define  FIFO_ERR_INFO         0x80002000
#define  FIFO_ERR_DEV          0x80004000
#define  FIFO_ERR_DEV_CNT      0x80008000
#define  FIFO_ERR_POOL         0x80010000

#define  FIFO_MSGLEN_UINT8     512
#define  FIFO_MSGLEN_UINT32    (FIFO_MSGLEN_UINT8 >> 2)
#define  FIFO_PIPE_SLOTS       32
#define  FIFO_PIPELEN_UINT8    8192
#define  FIFO_PACKET_CNT       32
#define  FIFO_BLOCK_LEN        (FIFO_PIPELEN_UINT8 * 4)
#define  FIFO_PIPE_BLKS        (FIFO_BLOCK_LEN / FIFO_PIPELEN_UINT8)
#define  FIFO_PIPE_POOL        (FIFO_PIPE_SLOTS * FIFO_BLOCK_LEN)

#define  FIFO_MAX_DEVICES      16
#define  FIFO_RX_TIMEOUT       100
#define  FIFO_TX_TIMEOUT       100
#define  FIFO_THREAD_TIMEOUT   100
#define  FIFO_LATENCY          2

#define  FIFO_EPID_NONE        0x00
#define  FIFO_EPID_NEXT        0x20
#define  FIFO_EPID_CTL         0x40
#define  FIFO_EPID_PIPE        0x80
#define  FIFO_PIPE             0x84

uint32_t  fifo_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port);
void      fifo_tx(pcm_msg_t msg);
void      fifo_cmio(uint8_t op_code, pcm_msg_t msg);
void      fifo_head(void);
void      fifo_final(void);

