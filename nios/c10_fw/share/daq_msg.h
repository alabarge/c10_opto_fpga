#pragma once

#include "fw_cfg.h"

// Return Errors
enum DAQ_ERR {
      DAQ_OK,
      DAQ_ERR_BAD_MSG_ID,
      DAQ_ERR_BAD_IO_ID,
      DAQ_ERR_BAD_FLAG,
      DAQ_ERR_NULL_PTR,
      DAQ_ERR_DATA_RANGE,
      DAQ_ERR_NO_DATA,
      DAQ_ERR_PARMS_NULL,
      DAQ_ERR_MSG_LEN_MAX,
      DAQ_ERR_MSG_NULL,
      DAQ_ERR_EVENT_ID,
      DAQ_ERR_PARMS_RANGE,
      DAQ_ERR_MSG_COUNT,
      DAQ_ERR_FILE_CHECKSUM,
      DAQ_ERR_FILE_ERROR,
};

// Error String Table
typedef struct _daq_error_table_t {
   unsigned char  errID;
   char           errStr[CFG_MAX_ERR_STR_LEN];
} daq_error_table_t, *pdaq_error_table_t;

// ===========================================================================
//
// SERVER MESSAGES
#define DAQ_NULL_MSG        0x00
#define DAQ_RUN_REQ         0x01
#define DAQ_RUN_RESP        0x02
#define DAQ_DATA_REQ        0x03
#define DAQ_DATA_RESP       0x04
#define DAQ_ERROR_REQ       0x3E
#define DAQ_ERROR_RESP      0x3F
#define DAQ_INT_IND         0x40
#define DAQ_DONE_IND        0x41
#define DAQ_PKT_IND         0x42

// SERVER FLAGS
#define DAQ_NO_FLAGS        0x00

// INTERRUPT INDICATIONS
#define DAQ_INT_FLAG_PKT    0x01
#define DAQ_INT_FLAG_DONE   0x02
#define DAQ_INT_FLAG_PIPE   0x04

// RUN COMMAND FLAGS
#define DAQ_CMD_NONE       0x00000000
#define DAQ_CMD_FLAGS      0x000FFFFF
#define DAQ_CMD_RUN        0x00000001
#define DAQ_CMD_STOP       0x00000002
#define DAQ_CMD_SEND_IND   0x00000004
#define DAQ_CMD_UPDATE     0x00000008
#define DAQ_CMD_RAMP       0x00000010
#define DAQ_CMD_CH_SEL     0x00000F00
#define DAQ_CMD_CH_ALL     0x00001000
#define DAQ_CMD_SCAN       0x00002000
#define DAQ_CMD_HEAD       0x00004000

// Channels per ADC
#define DAQ_MAX_CH         8

// Maximum samples per pipe message
#define DAQ_MAX_LEN        496

// Single channel Samples per Pipe Message
#define DAQ_MAX_SAM        (DAQ_MAX_LEN / DAQ_MAX_CH)

// Pipe message pooling
#define DAQ_MAX_PIPE_RUN   32

// 2.5V / 2^12 (12-Bit ADC), Internal Reference
#define DAQ_LSB            (2.5 / 4096.0)

// ADC MAXIMUM SAMPLING RATE
#define DAQ_RATE_MAX       0x001E

// ADC MINIMUM SAMPLING RATE
#define DAQ_RATE_MIN       0x8000

// ===========================================================================
//
// SERVER MESSAGE DATA DEFINITIONS

// RUN MESSAGE BODY
typedef struct {
   uint32_t        opcode;
   uint32_t        packets;
} daq_run_body_t, *pdaq_run_body_t;

// RUN REQUEST/RESPONSE MESSAGE COMPLETE
typedef struct {
   cm_hdr_t        h;
   msg_parms_t     p;
   daq_run_body_t  b;
} daq_run_msg_t, *pdaq_run_msg_t;

// DONE INDICATION MESSAGE BODY
typedef struct {
   uint32_t        opcode;
   uint32_t        status;
   uint32_t        stamp;
} daq_done_body_t, * pdaq_done_body_t;

// DONE INDICATION MESSAGE COMPLETE
typedef struct {
   cm_hdr_t        h;
   msg_parms_t     p;
   daq_done_body_t b;
} daq_done_ind_msg_t, *pdaq_done_ind_msg_t;

// CM PIPE DAQ MESSAGE DATA STRUCTURE
// USED FOR FIXED 1024-BYTE DAQ PIPE MESSAGES
// SAME SIZE AS SLOTS IN WINDOWS DRIVER
typedef struct _cm_pipe_daq_t {
   uint8_t     dst_cmid;       // Destination CM ID
   uint8_t     msgid;          // Pipe Message ID, CM_PIPE_DAQ_DATA = 0x10
   uint8_t     port;           // Destination Port
   uint8_t     flags;          // Message Flags
   uint32_t    msglen;         // Message Length in 32-Bit words
   uint32_t    seqid;          // Sequence ID
   uint32_t    stamp;          // 32-Bit FPGA Clock Count
   uint32_t    stamp_us;       // 32-Bit Time Stamp in microseconds
   uint32_t    status;         // Current Machine Status
   uint32_t    rate;           // ADC rate
   uint32_t    magic;          // Magic Number
   uint16_t    samples[496];   // DAQ Samples
} cm_pipe_daq_t, *pcm_pipe_daq_t;
