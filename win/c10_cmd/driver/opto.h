#pragma once

#include "cm.h"

#define  OPTO_SOF              0x7E
#define  OPTO_EOF              0x7D
#define  OPTO_ESC              0x7C
#define  OPTO_BIT              0x20

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
#define  OPTO_ERR_RESP         0x80000800
#define  OPTO_ERR_THREAD       0x80001000
#define  OPTO_ERR_INFO         0x80002000
#define  OPTO_ERR_DEV          0x80004000
#define  OPTO_ERR_DEV_CNT      0x80008000
#define  OPTO_ERR_POOL         0x80010000

#define  OPTO_MSGLEN_UINT8     512
#define  OPTO_MSGLEN_UINT32    (OPTO_MSGLEN_UINT8 >> 2)
#define  OPTO_PIPE_SLOTS       256
#define  OPTO_PIPELEN_UINT8    1024
#define  OPTO_PACKET_CNT       32
#define  OPTO_BLOCK_LEN        (OPTO_PACKET_CNT * OPTO_PIPELEN_UINT8)

#define  OPTO_MAX_DEVICES      16
#define  OPTO_RX_TIMEOUT       100
#define  OPTO_TX_TIMEOUT       100
#define  OPTO_THREAD_TIMEOUT   50

#define  OPTO_EPID_NONE        0x00
#define  OPTO_EPID_NEXT        0x20
#define  OPTO_EPID_CTL         0x40
#define  OPTO_EPID_PIPE        0x80

uint32_t  opto_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port);
void      opto_tx(pcm_msg_t msg);
void      opto_cmio(uint8_t op_code, pcm_msg_t msg);
void      opto_head(void);
void      opto_final(void);

