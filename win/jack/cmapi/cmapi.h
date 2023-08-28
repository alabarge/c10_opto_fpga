#ifdef CMAPI_EXPORTS
#define CM_API __declspec(dllexport)
#else
#define CM_API __declspec(dllimport)
#endif

#pragma once

#define WM_CM_MESSAGE         (WM_USER +  600)

#define CM_CALC_CRC           FALSE
#define CM_CHECK_CRC          TRUE

// Init Flags
#define CM_LOG_ENABLE         1
#define CM_LOG_DISABLE        2

#define CM_ERROR              0x80000000
#define CM_ERROR_NUM_PIPE     0x80000001
#define CM_ERROR_PIPE         0x80000002

// Message Types
#define CM_MSG_REQ            0x01
#define CM_MSG_RESP           0x02
#define CM_MSG_IND            0x03
#define CM_MSG_TIMER          0x04
#define CM_MSG_DEV_REQ        0x05
#define CM_MSG_DEV_RESP       0x06

// I/O Operation Codes
#define CM_IO_TX_ALLOC        0x01
#define CM_IO_TX              0x02
#define CM_IO_RX_ALLOC        0x03
#define CM_IO_RX              0x04
#define CM_IO_FREE            0x05

#define CM_MSGQ_SLOTS         128
#define CM_MSGQ_BUF_LEN       256

#define CM_TICKS_MS           100

// CM Port Callback Type Definition
typedef  void (*cmio_t)(uint8_t op_code, pcm_msg_t msg);

// CM Object Subscriptions
typedef struct _cm_sub_t {
   uint8_t       srvid;
   uint8_t       msgid;
   uint8_t       cmid;
} cm_sub_t, *pcm_sub_t;

// cm_send() Parameter Structure
typedef struct _cm_send_t {
   pcm_msg_t   msg;
   pcm_msg_t   req;
   pcm_pipe_t  pipe;
   msg_parms_t p;
   uint8_t    *msgbody;
   uint16_t    msglen;
   uint32_t    pipelen;
   uint8_t     dst_cmid;
   uint8_t     src_cmid;
   uint8_t     dst_devid;
   uint8_t     port;
} cm_send_t, *pcm_send_t;

// CM Message Queue
typedef struct _cmq_t {
   uint8_t     state;
   uint8_t     flags;
   uint8_t     reserved[2];
   uint16_t    size;
   uint16_t    msglen;
   uint32_t    raw[CM_MSGQ_BUF_LEN+4];
   uint32_t   *buf;
} cmq_t, *pcmq_t;

CM_API uint32_t   cm_init(cmio_t log_cb);
CM_API uint8_t    cm_register(uint8_t cmid, uint32_t (*msg)(pcm_msg_t msg, void *member),
                              void *member, cm_sub_t *cmsub);
CM_API uint32_t   cm_ioreg(cmio_t comio, uint8_t port, uint8_t media);
CM_API uint32_t   cm_send(uint8_t msg_type, pcm_send_t ps);
CM_API uint32_t   cm_route(pcm_msg_t msg);
CM_API pcmq_t     cm_alloc(void);
CM_API void       cm_free(pcm_msg_t msg);
CM_API uint32_t   cm_crc(pcm_msg_t msg, uint8_t crc_chk);
CM_API uint32_t   cm_msg(pcm_msg_t msg, void *member);
CM_API uint8_t    cm_get_handle(uint8_t cmid);
CM_API uint8_t    cm_id_exists(uint8_t cmid);
CM_API uint8_t    cm_get_devid(uint8_t cmid);
CM_API uint8_t    cm_get_sub(uint8_t h, uint8_t srvID, uint8_t msgid);
CM_API uint32_t   cm_timer_set(uint8_t timerid, uint8_t msgid, uint32_t time_ms,
                             uint8_t cmid, uint8_t srvid);
CM_API uint32_t   cm_timer_kill(uint8_t timerid, uint8_t cmid);
CM_API uint32_t   cm_tick(void);
CM_API void       cm_tx_drop(uint8_t op_code, pcm_msg_t msg);
CM_API uint32_t   cm_send_msg(uint8_t msg_type, pcm_msg_t msg, pcm_msg_t preq,
                              uint16_t msglen, uint8_t dst_cmid, uint8_t devid);
CM_API uint32_t   cm_send_reg_req(uint8_t devid, uint8_t port, uint8_t flags, uint8_t *device);
CM_API void       cm_local(uint8_t srvid, uint8_t msgid, uint8_t flags, uint8_t status);
CM_API uint32_t   cm_send_req(uint8_t srvid, uint8_t msgid, uint8_t srcid, uint8_t flags);
CM_API uint32_t   cm_pipe_reg(uint8_t cmid, uint8_t msgid, uint32_t mark, uint8_t dstdev);
CM_API uint32_t   cm_pipe_send(pcm_pipe_t pipe, uint32_t pipelen);
CM_API void       cm_qmsg(pcm_msg_t msg);
CM_API void       cm_log(pcm_msg_t msg, uint8_t flags);
CM_API void       cm_final(void);
