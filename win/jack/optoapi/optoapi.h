#ifdef OPTO_EXPORTS
#define OPTO_API __declspec(dllexport)
#else
#define OPTO_API __declspec(dllimport)
#endif

#pragma once

// Device Info
typedef struct _opto_dev_info_t {
   uint32_t    flags;
   uint32_t    type;
   uint32_t    id;
   uint32_t    locid;
   char        serial[16];
   char        desc[64];
   void       *handle;
} opto_dev_info_t, *popto_dev_info_t;

#define  OPTO_VID              0x0403
#define  OPTO_PID              0x6001
#define  OPTO_VID_PID_STR      "VID_0403&PID_6001"
#define  OPTO_VID_PID          0x04036001

#define  OPTO_VID_ALT          0x0403
#define  OPTO_PID_ALT          0x6014
#define  OPTO_VID_PID_STR_ALT  "VID_0403&PID_6014"
#define  OPTO_VID_PID_ALT      0x04036014

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
#define  OPTO_POOL_SLOTS       256
#define  OPTO_PIPELEN_UINT8    1024
#define  OPTO_PACKET_CNT       32
#define  OPTO_BLOCK_LEN        (OPTO_PACKET_CNT * OPTO_PIPELEN_UINT8)

#define  OPTO_MAX_DEVICES      8

OPTO_API uint32_t opto_query(opto_dev_info_t **devinfo);
OPTO_API int32_t  opto_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port);
OPTO_API void     opto_tx(pcm_msg_t msg);
OPTO_API void     opto_cmio(uint8_t op_code, pcm_msg_t msg);
OPTO_API void     opto_head(void);
OPTO_API void     opto_rev(uint32_t *librev, uint32_t *sysrev, uint32_t *apirev);
OPTO_API void     opto_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat);
OPTO_API void     opto_final(void);

