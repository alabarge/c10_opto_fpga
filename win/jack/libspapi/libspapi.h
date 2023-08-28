#ifdef LIBSP_EXPORTS
#define LIBSP_API __declspec(dllexport)
#else
#define LIBSP_API __declspec(dllimport)
#endif

#pragma once

// Device Info
typedef struct _libsp_dev_info_t {
   uint32_t    flags;
   uint32_t    type;
   uint32_t    id;
   uint32_t    locid;
   char        serial[16];
   char        desc[64];
   void       *handle;
} libsp_dev_info_t, *plibsp_dev_info_t;

#define  LIBSP_OK               0x00000000
#define  LIBSP_ERROR            0x80000001
#define  LIBSP_ERR_MSG_NULL     0x80000002
#define  LIBSP_ERR_LEN_NULL     0x80000004
#define  LIBSP_ERR_LEN_MAX      0x80000008
#define  LIBSP_ERR_FRAMING      0x80000010
#define  LIBSP_ERR_OVERRUN      0x80000020
#define  LIBSP_ERR_PARITY       0x80000040
#define  LIBSP_ERR_TX_DROP      0x80000080
#define  LIBSP_ERR_RX_DROP      0x80000100
#define  LIBSP_ERR_CRC          0x80000200
#define  LIBSP_ERR_OPEN         0x80000400
#define  LIBSP_ERR_RESP         0x80000800
#define  LIBSP_ERR_THREAD       0x80001000
#define  LIBSP_ERR_INFO         0x80002000
#define  LIBSP_ERR_DEV          0x80004000
#define  LIBSP_ERR_DEV_CNT      0x80008000
#define  LIBSP_ERR_POOL         0x80010000
#define  LIBSP_ERR_BAUDRATE     0x80020000

#define  LIBSP_MSGLEN_UINT8     512
#define  LIBSP_MSGLEN_UINT32    (LIBSP_MSGLEN_UINT8 >> 2)
#define  LIBSP_POOL_SLOTS       256
#define  LIBSP_PIPELEN_UINT8    1024
#define  LIBSP_PACKET_CNT       32
#define  LIBSP_BLOCK_LEN        (LIBSP_PACKET_CNT * LIBSP_PIPELEN_UINT8)

#define  LIBSP_MAX_DEVICES      256

LIBSP_API uint32_t libsp_query(libsp_dev_info_t **devinfo);
LIBSP_API int32_t  libsp_init(uint32_t baudrate, uint8_t cm_port, uint8_t libsp_port);
LIBSP_API void     libsp_tx(pcm_msg_t msg);
LIBSP_API void     libsp_cmio(uint8_t op_code, pcm_msg_t msg);
LIBSP_API void     libsp_head(void);
LIBSP_API void     libsp_rev(uint32_t *librev, uint32_t *sysrev, uint32_t *apirev);
LIBSP_API void     libsp_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat);
LIBSP_API void     libsp_final(void);

