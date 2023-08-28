#ifdef FIFO_EXPORTS
#define FIFO_API __declspec(dllexport)
#else
#define FIFO_API __declspec(dllimport)
#endif

#pragma once

// Device Info
typedef struct _fifo_dev_info_t {
   uint32_t    flags;
   uint32_t    type;
   uint32_t    id;
   uint32_t    locid;
   char        serial[16];
   char        desc[64];
   void       *handle;
} fifo_dev_info_t, *pfifo_dev_info_t;

#define  FIFO_VID              0x0403
#define  FIFO_PID              0x6001
#define  FIFO_VID_PID_STR      "VID_0403&PID_6001"
#define  FIFO_VID_PID          0x04036001

#define  FIFO_VID_ALT          0x0403
#define  FIFO_PID_ALT          0x6014
#define  FIFO_VID_PID_STR_ALT  "VID_0403&PID_6014"
#define  FIFO_VID_PID_ALT      0x04036014

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
#define  FIFO_POOL_SLOTS       32
#define  FIFO_PIPELEN_UINT8    8192
#define  FIFO_BLOCK_LEN        (FIFO_PIPELEN_UINT8 * 4)
#define  FIFO_PIPE_BLKS        (FIFO_BLOCK_LEN / FIFO_PIPELEN_UINT8)
#define  FIFO_PIPE_POOL        (FIFO_POOL_SLOTS * FIFO_BLOCK_LEN)

FIFO_API uint32_t fifo_query(fifo_dev_info_t **devinfo);
FIFO_API int32_t  fifo_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port);
FIFO_API void     fifo_tx(pcm_msg_t msg);
FIFO_API void     fifo_cmio(uint8_t op_code, pcm_msg_t msg);
FIFO_API void     fifo_head(void);
FIFO_API void     fifo_rev(uint32_t *librev, uint32_t *sysrev, uint32_t *apirev);
FIFO_API void     fifo_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat);
FIFO_API void     fifo_final(void);

