#pragma once

#define  CP_OK                   0
#define  CP_RX_QUE               8

#define  CP_TMR_PING             0x60
#define  CP_TMR_PING_TIMEOUT     0x61

// CP Client Data Structure
typedef struct _cp_t {
   uint8_t           srvid;
   uint8_t           handle;
   pthread_t         tid;
} cp_t, *pcp_t;

// Receive Queue
typedef struct _cp_rxq_t {
   uint32_t         *buf[CP_RX_QUE];
   pthread_mutex_t   mutex;
   pthread_cond_t    cv;
   uint8_t           head;
   uint8_t           tail;
   uint8_t           slots;
} cp_rxq_t, *pcp_rxq_t;

uint32_t cp_init(void);
uint32_t cp_msg(pcm_msg_t msg);
uint32_t cp_timer(pcm_msg_t msg);
uint32_t cp_tick(void);
uint32_t cp_qmsg(pcm_msg_t msg);
void     cp_final(void);
