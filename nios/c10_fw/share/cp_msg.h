#pragma once

#include "fw_cfg.h"

// CP Return Errors
enum CP_ERR {
      CP_OK,
      CP_ERR_BAD_MSG_ID,
      CP_ERR_BAD_IO_ID,
      CP_ERR_BAD_FLAG,
      CP_ERR_NULL_PTR,
      CP_ERR_DATA_RANGE,
      CP_ERR_ADDR_RANGE,
      CP_ERR_NO_DATA,
      CP_ERR_PARMS_NULL,
      CP_ERR_MSG_LEN_MAX,
      CP_ERR_MSG_NULL,
      CP_ERR_EVENT_ID,
      CP_ERR_PARMS_RANGE,
      CP_ERR_MSG_COUNT,
      CP_ERR_FILE_CHECKSUM,
      CP_ERR_FILE_ERROR,
};

// CP Error String Table
typedef struct _cp_error_table_t {
   uint8_t  errID;
   char           errStr[CFG_MAX_ERR_STR_LEN];
} cp_error_table_t, *pcp_error_table_t;

// ===========================================================================
//
// CP SERVER MESSAGES
#define CP_NULL_MSG        0x00
#define CP_VER_REQ         0x01
#define CP_VER_RESP        0x02
#define CP_MEM_REQ         0x03
#define CP_MEM_RESP        0x04
#define CP_TRACE_REQ       0x05
#define CP_TRACE_RESP      0x06
#define CP_RESET_REQ       0x07
#define CP_RESET_RESP      0x08
#define CP_BLOCK_REQ       0x09
#define CP_BLOCK_RESP      0x0A
#define CP_XL345_REQ       0x0B
#define CP_XL345_RESP      0x0C
#define CP_EEPROM_REQ      0x0D
#define CP_EEPROM_RESP     0x0E
#define CP_RPC_REQ         0x0F
#define CP_RPC_RESP        0x10
#define CP_STREAM_REQ      0x11
#define CP_STREAM_RESP     0x12
#define CP_PING_REQ        0x13
#define CP_PING_RESP       0x14
#define CP_ERROR_REQ       0x3E
#define CP_ERROR_RESP      0x3F
#define CP_INT_IND         0x40
#define CP_STATUS_IND      0x41
#define CP_XL345_IND       0x42
#define CP_PING_IND        0x43

// CP SERVER FLAGS
#define CP_NO_FLAGS        0x00
#define CP_MEM_RD          0x01
#define CP_MEM_WR          0x02
#define CP_BLOCK_RD        0x01
#define CP_BLOCK_WR        0x02
#define CP_TRACE_SET       0x01
#define CP_TRACE_GET       0x02
#define CP_RESET_HW        0x01
#define CP_RESET_SW        0x02
#define CP_RESET_CFG       0x04
#define CP_XL345_RUN       0x10
#define CP_XL345_STOP      0x20

// CP INTERRUPT INDICATIONS
#define CP_IND_XL345       0x01
#define CP_IND_PING        0x02

#define CP_BLOCK_MAX       256
#define CP_XL345_LEN       6
#define CP_XL345_MARK      25
#define CP_XL345_RATE      0x0F
#define CP_XL345_SENSE     0x03

// ===========================================================================
//
// CH SERVER MESSAGE DATA DEFINITIONS

// VERSION RESPONSE MESSAGE BODY
typedef struct {
   uint32_t    fw_ver;
   uint32_t    sysid;
   uint32_t    stamp_epoch;
   uint32_t    stamp_date;
   uint32_t    stamp_time;
   uint8_t     vhdl[8];
   uint32_t    trace;
   uint32_t    feature;
   uint32_t    debug;
} cp_ver_body_t, *pcp_ver_body_t;

// VERSION REQUEST/RESPONSE MESSAGE COMPLETE
typedef struct {
   cm_hdr_t      h;
   msg_parms_t   p;
   cp_ver_body_t b;
} cp_ver_msg_t, *pcp_ver_msg_t;

// MEMORY RD/WR REQUEST/RESPONSE MESSAGE BODY
typedef struct {
   uint32_t    address;
   uint32_t    value;
   uint32_t    type;
} cp_mem_body_t, *pcp_mem_body_t;

// MEMORY REQUEST/RESPONSE MESSAGE COMPLETE
typedef struct {
   cm_hdr_t      h;
   msg_parms_t   p;
   cp_mem_body_t b;
} cp_mem_msg_t, *pcp_mem_msg_t;

// TRACE REQUEST/RESPONSE MESSAGE BODY
typedef struct {
   uint32_t    trace;
   uint32_t    debug;
} cp_trace_body_t, * pcp_trace_body_t;

// TRACE REQUEST/RESPONSE MESSAGE COMPLETE
typedef struct {
   cm_hdr_t         h;
   msg_parms_t      p;
   cp_trace_body_t  b;
} cp_trace_msg_t, *pcp_trace_msg_t;

// BLOCK MEMORY READ/WRITE REQ/RESP MESSAGE BODY
typedef struct {
   uint32_t    index;
   uint32_t    type;
   uint32_t    address;
   uint32_t    length;
   uint8_t     data[CP_BLOCK_MAX];
} cp_block_body_t, *pcp_block_body_t;

// BLOCK MEMORY READ/WRITE REQ/RESP MESSAGE COMPLETE
typedef struct {
   cm_hdr_t         h;
   msg_parms_t      p;
   cp_block_body_t  b;
} cp_block_msg_t, *pcp_block_msg_t;

// PING REQ/RESP MESSAGE COMPLETE
typedef struct {
   cm_hdr_t         h;
   msg_parms_t      p;
} cp_ping_msg_t, *pcp_ping_msg_t;

// PING INDICATION MESSAGE COMPLETE
typedef struct {
   cm_hdr_t             h;
   msg_parms_t          p;
} cp_ping_ind_msg_t, *pcp_ping_ind_msg_t;

// XL345 RUN MESSAGE BODY
typedef struct {
   uint32_t    sense;
   uint32_t    rate;
} cp_xl345_run_body_t, *pcp_xl345_run_body_t;

// XL345 RUN REQUEST/RESPONSE MESSAGE COMPLETE
typedef struct {
   cm_hdr_t            h;
   msg_parms_t         p;
   cp_xl345_run_body_t b;
} cp_xl345_msg_t, *pcp_xl345_msg_t;

// XL345 INDICATION MESSAGE BODY
typedef struct {
   uint8_t     recid;
   uint8_t     flags;
   uint8_t     status;
   uint8_t     unused;
   uint32_t    seqid;
   uint32_t    sys_time;
   uint8_t     data[CP_XL345_LEN * CP_XL345_MARK];
} cp_xl345_body_t, * pcp_xl345_body_t;

// XL345 INDICATION MESSAGE COMPLETE
typedef struct {
   cm_hdr_t         h;
   msg_parms_t      p;
   cp_xl345_body_t  b;
} cp_xl345_ind_msg_t, *pcp_xl345_ind_msg_t;
