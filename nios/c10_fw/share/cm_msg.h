#pragma once

// CM Message Return Errors
enum cm_msg_err {
      CM_MSG_OK,
      CM_MSG_ERROR
};

// CM Return Errors
enum cm_err {
      CM_OK,
      CM_ERR_BAD_MSG_ID,
      CM_ERR_NULL_PTR,
      CM_ERR_LEN_NULL,
      CM_ERR_PARMS_NULL,
      CM_ERR_LEN_MAX,
      CM_ERR_MSG_NULL,
      CM_ERR_CRC,
      CM_ERR_DEV_ID,
      CM_ERR_OBJ_ID,
      CM_ERR_MSGQ_EMPTY,
      CM_ERR_IF_MAX,
      CM_ERR_ROUTE,
      CM_ERR_TIMER_ID,
      CM_ERR_TASK_ID,
      CM_ERR_EVENT_ID,
      CM_ERR_MEDIA_NONE,
};

// ===========================================================================
//
// COMMUNICATIONS MANAGER (CM) MESSAGES
#define CM_NULL_MSG           0x00
#define CM_REG_REQ            0x01
#define CM_REG_RESP           0x02
#define CM_RPC_REQ            0x03
#define CM_RPC_RESP           0x04
#define CM_ECHO_REQ           0x05
#define CM_ECHO_RESP          0x06
#define CM_STATUS_REQ         0x07
#define CM_STATUS_RESP        0x08
#define CM_QUERY_REQ          0x09
#define CM_QUERY_RESP         0x0A
#define CM_ERROR_REQ          0x3E
#define CM_ERROR_RESP         0x3F
#define CM_START_IND          0x40
#define CM_REG_IND            0x41
#define CM_PING_IND           0x42

// CM FLAGS
#define CM_NO_FLAGS           0x00
#define CM_REG_OPEN           0x01
#define CM_REG_CLOSE          0x02

// ===========================================================================
//
// CM MESSAGE DATA DEFINITIONS

// CM Routing Table Record
typedef struct _cm_rt_rec_t {
   uint8_t     cmid;
   uint8_t     devid;
} cm_rt_rec_t, *pcm_rt_rec_t;

// REGISTER REQUEST/RESPONSE MESSAGE INFORMATION
typedef struct _cm_reg_body_t {
   uint32_t    status;
   uint8_t     rec_cnt;
   uint8_t     unused;
   cm_rt_rec_t rec[CM_MAX_ROUTES];
   char        device[CM_MAX_DEV_STR_LEN];
} cm_reg_body_t, *pcm_reg_body_t;

// REGISTER REQUEST/RESPONSE MESSAGE COMPLETE
typedef struct _cm_reg_msg_t {
   cm_hdr_t         h;
   msg_parms_t      p;
   cm_reg_body_t    b;
} cm_reg_msg_t, *pcm_reg_msg_t;

// STATUS REQUEST/RESPONSE MESSAGE BODY
typedef struct _cm_status_body_t {
   uint32_t    status;
   uint32_t    msg_cnt;
   uint16_t    crc_err;
   uint16_t    rt_err;
   uint16_t    q_err;
   uint16_t    reserved;
} cm_status_body_t, *pcm_status_body_t;

// QUERY REQUEST/RESPONSE MESSAGE BODY
typedef struct _cm_query_body_t {
   uint32_t    magic;
   uint32_t    sysid;
   uint32_t    stamp;
   uint8_t     devid;
   uint8_t     num_objs;
   uint8_t     num_cons;
   uint8_t     reserved;
} cm_query_body_t, *pcm_query_body_t;

// QUERY REQUEST/RESPONSE MESSAGE COMPLETE
typedef struct _cm_query_msg_t {
   cm_hdr_t         h;
   msg_parms_t      p;
   cm_query_body_t  b;
} cm_query_msg_t, * pcm_query_msg_t;
