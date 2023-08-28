#ifdef UDP_EXPORTS
#define UDP_API __declspec(dllexport)
#else
#define UDP_API __declspec(dllimport)
#endif

#pragma once

// Device Info
typedef struct _udp_dev_info_t {
   uint32_t    flags;
   uint32_t    type;
   uint32_t    id;
   uint32_t    locid;
   char        serial[256];
   char        desc[256];
   void       *handle;
} udp_dev_info_t, *pudp_dev_info_t;

#define  UDP_OK               0x00000000
#define  UDP_ERROR            0x80000001
#define  UDP_ERR_MSG_NULL     0x80000002
#define  UDP_ERR_LEN_NULL     0x80000004
#define  UDP_ERR_LEN_MAX      0x80000008
#define  UDP_ERR_FRAMING      0x80000010
#define  UDP_ERR_OVERRUN      0x80000020
#define  UDP_ERR_PARITY       0x80000040
#define  UDP_ERR_TX_DROP      0x80000080
#define  UDP_ERR_RX_DROP      0x80000100
#define  UDP_ERR_CRC          0x80000200
#define  UDP_ERR_OPEN         0x80000400
#define  UDP_ERR_RESP         0x80000800
#define  UDP_ERR_THREAD       0x80001000
#define  UDP_ERR_INFO         0x80002000
#define  UDP_ERR_DEV          0x80004000
#define  UDP_ERR_DEV_CNT      0x80008000
#define  UDP_ERR_POOL         0x80010000
#define  UDP_ERR_WSA          0x80020000
#define  UDP_ERR_SOCKET       0x80040000

#define  UDP_MSGLEN_UINT8     512
#define  UDP_MSGLEN_UINT32    (UDP_MSGLEN_UINT8 >> 2)

#define  UDP_FRAME_CNT        32
#define  UDP_POOL_SLOTS       32
#define  UDP_PIPELEN_UINT8    1024
#define  UDP_BLOCK_LEN        (UDP_PIPELEN_UINT8 * UDP_FRAME_CNT)
#define  UDP_PIPE_POOL        (UDP_POOL_SLOTS * UDP_BLOCK_LEN)

#define  UDP_CTL_MSG_LEN      518
#define  UDP_PIPE_MSG_LEN     1030

#define  UDP_MAX_DEVICES      16

UDP_API uint32_t udp_query(udp_dev_info_t **devinfo);
UDP_API int32_t  udp_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port,
                 uint16_t cm_udp_port, uint8_t *macip);
UDP_API void     udp_tx(pcm_msg_t msg);
UDP_API void     udp_cmio(uint8_t op_code, pcm_msg_t msg);
UDP_API void     udp_head(void);
UDP_API void     udp_rev(char **librev, uint32_t *sysrev, uint32_t *apirev);
UDP_API void     udp_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat);
uint16_t         udp_crc(uint16_t *msg, uint32_t len);
UDP_API void     udp_final(void);

