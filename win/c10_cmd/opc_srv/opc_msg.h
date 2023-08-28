#pragma once

#include "fw_cfg.h"

// OPC Return Errors
enum OPC_ERR {
      OPC_OK,
      OPC_ERR_BAD_MSG_ID,
      OPC_ERR_BAD_IO_ID,
      OPC_ERR_BAD_FLAG,
      OPC_ERR_NULL_PTR,
      OPC_ERR_DATA_RANGE,
      OPC_ERR_ADDR_RANGE,
      OPC_ERR_NO_DATA,
      OPC_ERR_PARMS_NULL,
      OPC_ERR_MSG_LEN_MAX,
      OPC_ERR_MSG_NULL,
      OPC_ERR_EVENT_ID,
      OPC_ERR_PARMS_RANGE,
      OPC_ERR_MSG_COUNT,
      OPC_ERR_FILE_CHECKSUM,
      OPC_ERR_FILE_ERROR,
};

// OPC Error String Table
typedef struct _opc_error_table_t {
   uint8_t  errID;
   char     errStr[CFG_MAX_ERR_STR_LEN];
} opc_error_table_t, *popc_error_table_t;

// ===========================================================================
//
// OPC SERVER MESSAGES
#define OPC_NULL_MSG        0x00
#define OPC_RUN_REQ         0x01
#define OPC_RUN_RESP        0x02
#define OPC_ERROR_REQ       0x3E
#define OPC_ERROR_RESP      0x3F
#define OPC_INT_IND         0x40
#define OPC_RUN_IND         0x41
#define OPC_STEP_IND        0x42

// OPC SERVER FLAGS
#define OPC_NO_FLAGS        0x00
#define OPC_RUN_START       0x01
#define OPC_RUN_HALT        0x02
#define OPC_STEP_PIPE       0x01
#define OPC_STEP_DONE       0x02

// OPERATION CODES
#define OPC_CMD_DAQ         1

// ===========================================================================
//
// OPC SERVER MESSAGE DATA DEFINITIONS

// RUN REQUEST/RESPONSE MESSAGE COMPLETE
typedef struct {
   cm_hdr_t        h;
   msg_parms_t     p;
} opc_run_msg_t, *popc_run_msg_t;

// DONE INDICATION MESSAGE BODY
typedef struct {
   uint32_t        opcode;
   uint32_t        status;
   uint32_t        stamp;
} opc_done_body_t, * popc_done_body_t;

// DONE INDICATION MESSAGE COMPLETE
typedef struct {
   cm_hdr_t        h;
   msg_parms_t     p;
   daq_done_body_t b;
} opc_done_ind_msg_t, *popc_done_ind_msg_t;
