#ifdef COM_EXPORTS
#define COM_API __declspec(dllexport)
#else
#define COM_API __declspec(dllimport)
#endif

#pragma once

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

COM_API uint32_t com_query(com_dev_info_t **devinfo);
COM_API int32_t  com_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port);
COM_API void     com_tx(pcm_msg_t msg);
COM_API void     com_cmio(uint8_t op_code, pcm_msg_t msg);
COM_API void     com_head(void);
COM_API void     com_rev(uint32_t *librev, uint32_t *sysrev, uint32_t *apirev);
COM_API void     com_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat);
COM_API void     com_final(void);

