#pragma once

#include "main.h"
#include "cm_const.h"
#include "cm_msg.h"

#define CM_CALC_CRC           FALSE
#define CM_CHECK_CRC          TRUE

#define CM_MSGQ_SLOTS         128
#define CM_MSGQ_BUF_LEN       256

#define CM_OK                 0x00000000
#define CM_ERROR              0x80000001
#define CM_ERR_THREAD         0x80000002
#define CM_ERR_PIPE           0x80000004
#define CM_ERR_NUM_PIPE       0x80000008

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

// Message Q Buffer State
#define CM_Q_IDLE             0x00
#define CM_Q_ALLOC            0x01
#define CM_Q_DELIVER          0x02
#define CM_Q_BUSY             0x03

// Timers
#define CM_TMR_ID0            0x00
#define CM_TMR_ID1            0x01
#define CM_TMR_ID2            0x02
#define CM_TMR_ID3            0x03

// CM Port Callback Type Definition
typedef  void (*cmio_t)(uint8_t op_code, pcm_msg_t msg);

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

// CM Port Connection
typedef struct _cm_port_t {
   uint8_t     media;
   cmio_t      io;
} cm_port_t, *pcm_port_t;

// CM Pipe Connection
typedef struct _cm_pipe_con_t {
   uint8_t       cmid;
   uint8_t       msgid;
   uint8_t       dstdev;
   uint32_t      mark;
   uint32_t      count;
   uint8_t      *first;
   uint32_t      pipelen;
} cm_pipe_con_t, *pcm_pipe_con_t;

// CM Object Subscriptions
typedef struct _cm_sub_t {
   uint8_t       srvid;
   uint8_t       msgid;
   uint8_t       cmid;
} cm_sub_t, *pcm_sub_t;

// CM Timers
typedef struct _cm_timer_t {
   uint8_t       cmid;
   uint8_t       srvid;
   uint8_t       msgid;
   uint32_t      count;
   uint32_t      reload;
} cm_timer_t, *pcm_timer_t;

// CM Object
typedef struct _cm_obj_t {
   uint8_t       id;
   uint8_t       devid;
   pcm_sub_t     sub;
   uint32_t      (*msg)(pcm_msg_t msg);
   uint32_t      (*timer)(pcm_msg_t msg);
} cm_obj_t, *pcm_obj_t;

// Communications Manager
typedef struct _cm_t {
   uint8_t           id;
   uint8_t           handle;
   uint8_t           devid;
   uint8_t           num_objs;
   uint8_t           num_route;
   uint8_t           num_cons;
   uint8_t           num_pipes;
   uint8_t           seqid;
   pthread_t         tid;
   pthread_mutex_t   q_mutex;
   pthread_cond_t    q_cv;
   uint8_t           q_head;
   uint8_t           q_tail;
   uint8_t           q_msg_cnt;
   uint32_t          last_us;
   FILE              *log;
   pthread_mutex_t   log_mutex;
   cm_timer_t        tim[CM_MAX_TIMERS];
   cm_pipe_con_t     pipe[CM_MAX_PIPES];
   cm_port_t         port[CM_MAX_PORTS + 1];
   cm_obj_t          obj[CM_MAX_OBJS + 1];
   cm_rt_rec_t       rt[CM_MAX_ROUTES + 1];
} cm_t, *pcm_t;

uint32_t   cm_init(void);
uint8_t    cm_register(uint8_t cmid, uint32_t (*msg)(pcm_msg_t msg),
                       uint32_t (*timer)(pcm_msg_t msg), pcm_sub_t cmsub);
uint32_t   cm_ioreg(cmio_t comio, uint8_t port, uint8_t media);
uint32_t   cm_send(uint8_t msg_type, pcm_send_t ps);
uint32_t   cm_route(pcm_msg_t msg);
pcmq_t     cm_alloc(void);
void       cm_free(pcm_msg_t msg);
uint32_t   cm_crc(pcm_msg_t msg, uint8_t crc_chk);
uint32_t   cm_msg(pcm_msg_t msg);
uint8_t    cm_get_handle(uint8_t cmid);
uint8_t    cm_id_exists(uint8_t cmid);
uint8_t    cm_get_devid(uint8_t cmid);
uint8_t    cm_get_sub(uint8_t h, uint8_t srvID, uint8_t msgid);
uint32_t   cm_timer_set(uint8_t timerid, uint8_t msgid, uint32_t time_ms,
                        uint8_t cmid, uint8_t srvid);
uint32_t   cm_timer_kill(uint8_t timerid, uint8_t cmid);
uint32_t   cm_tick(void);
void       cm_tx_drop(uint8_t op_code, pcm_msg_t msg);
uint32_t   cm_send_msg(uint8_t msg_type, pcm_msg_t msg, pcm_msg_t preq,
                       uint16_t msglen, uint8_t dst_cmid, uint8_t devid);
uint32_t   cm_send_reg_req(uint8_t devid, uint8_t port, uint8_t flags, uint8_t *device);
void       cm_local(uint8_t srvid, uint8_t msgid, uint8_t flags, uint8_t status);
uint32_t   cm_send_req(uint8_t srvid, uint8_t msgid, uint8_t srcid, uint8_t flags);
uint32_t   cm_pipe_reg(uint8_t cmid, uint8_t msgid, uint32_t mark, uint8_t dstdev);
uint32_t   cm_pipe_send(pcm_pipe_t pipe, uint32_t pipelen);
void       cm_qmsg(pcm_msg_t msg);
void       cm_log(pcm_msg_t msg);
void       cm_timer_callback(size_t timer_id, void * user_data);
void       cm_final(void);

