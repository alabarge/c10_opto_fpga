#ifdef UART_EXPORTS
#define UART_API __declspec(dllexport)
#else
#define UART_API __declspec(dllimport)
#endif

#pragma once

// Device Info
typedef struct _uart_dev_info_t {
   uint32_t    flags;
   uint32_t    type;
   uint32_t    id;
   uint32_t    locid;
   char        serial[16];
   char        desc[64];
   void       *handle;
} uart_dev_info_t, *puart_dev_info_t;

#define  UART_VID              0x0403
#define  UART_PID              0x6001
#define  UART_VID_PID_STR      "VID_0403&PID_6001"
#define  UART_VID_PID          0x04036001

#define  UART_VID_ALT          0x0403
#define  UART_PID_ALT          0x6014
#define  UART_VID_PID_STR_ALT  "VID_0403&PID_6014"
#define  UART_VID_PID_ALT      0x04036014

#define  UART_OK               0x00000000
#define  UART_ERROR            0x80000001
#define  UART_ERR_MSG_NULL     0x80000002
#define  UART_ERR_LEN_NULL     0x80000004
#define  UART_ERR_LEN_MAX      0x80000008
#define  UART_ERR_FRAMING      0x80000010
#define  UART_ERR_OVERRUN      0x80000020
#define  UART_ERR_PARITY       0x80000040
#define  UART_ERR_TX_DROP      0x80000080
#define  UART_ERR_RX_DROP      0x80000100
#define  UART_ERR_CRC          0x80000200
#define  UART_ERR_OPEN         0x80000400
#define  UART_ERR_RESP         0x80000800
#define  UART_ERR_THREAD       0x80001000
#define  UART_ERR_INFO         0x80002000
#define  UART_ERR_DEV          0x80004000
#define  UART_ERR_DEV_CNT      0x80008000
#define  UART_ERR_POOL         0x80010000

#define  UART_MSGLEN_UINT8     512
#define  UART_MSGLEN_UINT32    (UART_MSGLEN_UINT8 >> 2)
#define  UART_POOL_SLOTS       256
#define  UART_PIPELEN_UINT8    1024
#define  UART_PACKET_CNT       32
#define  UART_BLOCK_LEN        (UART_PACKET_CNT * UART_PIPELEN_UINT8)

#define  UART_MAX_DEVICES      8

UART_API uint32_t uart_query(uart_dev_info_t **devinfo);
UART_API int32_t  uart_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port);
UART_API void     uart_tx(pcm_msg_t msg);
UART_API void     uart_cmio(uint8_t op_code, pcm_msg_t msg);
UART_API void     uart_head(void);
UART_API void     uart_rev(uint32_t *librev, uint32_t *sysrev, uint32_t *apirev);
UART_API void     uart_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat);
UART_API void     uart_final(void);

