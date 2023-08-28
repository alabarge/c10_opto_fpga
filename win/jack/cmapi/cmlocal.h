#pragma once

#include "cm_const.h"
#include "cm_msg.h"
#include "cmapi.h"
#include "msg_str.h"
#include "timer.h"

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
   void          *member;
   uint32_t      (*msg)(pcm_msg_t msg, void *member);
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
   HANDLE            q_thread;
   DWORD             q_tid;
   CRITICAL_SECTION  q_mutex;
   CONDITION_VARIABLE q_cv;
   uint8_t           q_head;
   uint8_t           q_tail;
   uint8_t           q_msg_cnt;
   uint8_t           flags;
   FILE              *log;
   CHRTimer          timer;
   uint32_t          last_us;
   uint32_t          alloc;
   HANDLE            q_timer;
   HANDLE            h_timer;
   cmio_t            log_cb;
   cm_timer_t        tim[CM_MAX_TIMERS];
   cm_pipe_con_t     pipe[CM_MAX_PIPES];
   cm_port_t         port[CM_MAX_PORTS + 1];
   cm_obj_t          obj[CM_MAX_OBJS + 1];
   cm_rt_rec_t       rt[CM_MAX_ROUTES + 1];
} cm_t, *pcm_t;
