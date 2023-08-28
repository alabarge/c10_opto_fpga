#pragma once

#include "cm.h"

// Device Info
typedef struct _lan_dev_info_t {
   uint32_t    flags;
   uint32_t    type;
   uint32_t    id;
   uint32_t    locid;
   char        serial[256];
   char        desc[256];
   void       *handle;
} lan_dev_info_t, *plan_dev_info_t;

#define  LAN_OK               0x00000000
#define  LAN_ERROR            0x80000001
#define  LAN_ERR_MSG_NULL     0x80000002
#define  LAN_ERR_LEN_NULL     0x80000004
#define  LAN_ERR_LEN_MAX      0x80000008
#define  LAN_ERR_FRAMING      0x80000010
#define  LAN_ERR_OVERRUN      0x80000020
#define  LAN_ERR_PARITY       0x80000040
#define  LAN_ERR_TX_DROP      0x80000080
#define  LAN_ERR_RX_DROP      0x80000100
#define  LAN_ERR_CRC          0x80000200
#define  LAN_ERR_OPEN         0x80000400
#define  LAN_ERR_RESP         0x80000800
#define  LAN_ERR_THREAD       0x80001000
#define  LAN_ERR_INFO         0x80002000
#define  LAN_ERR_DEV          0x80004000
#define  LAN_ERR_DEV_CNT      0x80008000
#define  LAN_ERR_POOL         0x80010000

#define  LAN_MSGLEN_UINT8     512
#define  LAN_MSGLEN_UINT32    (LAN_MSGLEN_UINT8 >> 2)

#define  LAN_FRAME_CNT        32
#define  LAN_POOL_SLOTS       32
#define  LAN_PIPELEN_UINT8    1024
#define  LAN_PIPE_CNT         32
#define  LAN_BLOCK_LEN        (LAN_PIPELEN_UINT8 * LAN_PIPE_CNT)
#define  LAN_PIPE_POOL        (LAN_POOL_SLOTS * LAN_BLOCK_LEN)

#define  LAN_CHAN_CTL         1
#define  LAN_MAX_DEVICES      16
#define  LAN_THREAD_TIMEOUT   50
#define  LAN_SNAPLEN          65536
#define  LAN_READ_TO          100

uint32_t  lan_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port,
                   uint16_t cm_udp_port, uint8_t *macip);
void      lan_tx(pcm_msg_t msg);
void      lan_cmio(uint8_t op_code, pcm_msg_t msg);
void      lan_head(void);
uint16_t  lan_crc(uint16_t *msg, uint32_t len);
void      lan_final(void);

