#pragma once

#define  DAQ_RX_QUE     8

// DAQ Service Provider Data Structure
typedef struct _daq_t {
   uint8_t     srvid;
   uint8_t     handle;
   uint32_t    status;
} daq_t, *pdaq_t;

// DAQ State Vector
typedef struct _daq_sv_t {
   uint32_t    seqid;
   uint32_t    samcnt;
   uint32_t    opcode;
   uint32_t    packets;
   uint32_t    adc_index;
   uint32_t    blklen;
   uint8_t     state;
} daq_sv_t, *pdaq_sv_t;

// Receive Queue
typedef struct _daq_rxq_t {
   uint32_t         *buf[DAQ_RX_QUE];
   uint8_t           head;
   uint8_t           tail;
   uint8_t           slots;
} daq_rxq_t, *pdaq_rxq_t;

uint32_t daq_init(void);
uint32_t daq_msg(pcm_msg_t msg);
uint32_t daq_timer(pcm_msg_t msg);
uint32_t daq_tick(void);
uint32_t daq_qmsg(pcm_msg_t msg);
void     daq_thread(void);
