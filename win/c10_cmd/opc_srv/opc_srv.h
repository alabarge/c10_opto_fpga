#pragma once

#define  OPC_RX_QUE           8

#define  OPC_STATE_IDLE       0

#define  OPC_DAQ_STATE_IDLE   OPC_STATE_IDLE
#define  OPC_DAQ_STATE_INIT   1
#define  OPC_DAQ_STATE_RUN    2
#define  OPC_DAQ_STATE_DONE   3

#define  OPC_BLKS_PER_MSG     8 

#define  OPC_TMR_APP_TIMEOUT  0x60


// OPC Generic State Vector
typedef struct _opc_sv_t {
   int32_t     opcode;
   uint32_t    state;
   uint32_t    (*step)(void);
   uint32_t    status;
} opc_sv_t, *popc_sv_t;

// OPC Operation Table
typedef struct _opc_table_t {
   int32_t     opcode;
   uint32_t    (*step)(void);
   uint32_t    state;
} opc_table_t, *popc_table_t;

// OPC Service Data Structure
typedef struct _opc_t {
   uint8_t     srvid;
   uint8_t     handle;
   uint32_t    status;
   opc_sv_t    sv;
} opc_t, *popc_t;

// OPC DAQ State Vector, OPC_CMD_DAQ
typedef struct _opc_daq_sv_t {
   uint32_t    seqid;
   uint32_t    samcnt;
   uint32_t    opcmd;
   uint32_t    packets;
   uint32_t    adc_index;
   uint32_t    blklen;
   uint32_t    to_file;
   uint32_t    file_type;
   uint32_t    file_stamp;
   uint32_t    ramp;
   uint32_t    real;
   uint8_t     acq_done;
   uint8_t     dat_done;
   int32_t    *adc;
   FILE       *file;
   uint32_t    pkt_cnt;
   pcm_pipe_daq_t pipe;
} opc_daq_sv_t, *popc_daq_sv_t;

// Receive Queue
typedef struct _opc_rxq_t {
   uint32_t         *buf[OPC_RX_QUE];
   CRITICAL_SECTION  mutex;
   CONDITION_VARIABLE cv;
   DWORD             tid;
   HANDLE            thread;
   uint8_t           head;
   uint8_t           tail;
   uint8_t           slots;
} opc_rxq_t, *popc_rxq_t;

uint32_t opc_init(void);
uint32_t opc_msg(pcm_msg_t msg);
uint32_t opc_timer(pcm_msg_t msg);
uint32_t opc_tick(void);
uint32_t opc_qmsg(pcm_msg_t msg);
uint32_t opc_daq_state(void);
uint32_t opc_write_file(pcm_pipe_daq_t pipe);
void     opc_final(void);
