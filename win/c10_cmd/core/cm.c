/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Communications Manager (CM)

   1.2 Functional Description

      Clients and servers use the communication manager to interact via
      message traffic. The routines that implement the CM protocol are
      contained herein.

   1.3 Specification/Design Reference

      See fw_cfg.h under the share directory.

   1.4 Module Test Specification Reference

      None

   1.5 Compilation Information

      See fw_cfg.h under the share directory.

   1.6 Notes

      NONE

   2  CONTENTS

      1 ABSTRACT
        1.1 Module Type
        1.2 Functional Description
        1.3 Specification/Design Reference
        1.4 Module Test Specification Reference
        1.5 Compilation Information
        1.6 Notes

      2 CONTENTS

      3 VOCABULARY

      4 EXTERNAL RESOURCES
        4.1  Include Files
        4.2  External Data Structures
        4.3  External Function Prototypes

      5 LOCAL CONSTANTS AND MACROS

      6 MODULE DATA STRUCTURES
        6.1  Local Function Prototypes
        6.2  Local Data Structures

      7 MODULE CODE
        7.1  cm_init()
        7.2  cm_register()
        7.3  cm_ioreg()
        7.4  cm_send()
        7.5  cm_alloc()
        7.6  cm_free()
        7.7  cm_crc()
        7.8  cm_msg()
        7.9  cm_get_handle()
        7.10 cm_id_exists()
        7.11 cm_get_devid()
        7.12 cm_get_sub()
        7.13 cm_timer_set()
        7.14 cm_timer_kill()
        7.15 cm_tick()
        7.16 cm_route()
        7.17 cm_tx_drop()
        7.18 cm_send_msg()
        7.19 cm_local()
        7.20 cm_send_req()
        7.21 cm_pipe_reg()
        7.22 cm_pipe_send()
        7.23 cm_send_reg_req()
        7.24 cm_qmsg()
        7.25 cm_thread()
        7.26 cm_log()
        7.27 cm_timer_callback()
        7.28 cm_final()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// 4.2   External Data Structures

   extern   gc_t  gc;
   extern   ci_t  ci;

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static DWORD WINAPI  cm_thread(LPVOID data);
   static VOID CALLBACK cm_timer_callback(PVOID lpParam, BOOLEAN TimerOrWaitFired);

// 6.2  Local Data Structures

   static   cm_t        cm = {0};
   static   cmq_t       cmq[CM_MSGQ_SLOTS] = {0};

   static uint8_t crc_array[] = {
      0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
      0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
      0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
      0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
      0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
      0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
      0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
      0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
      0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
      0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
      0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
      0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
      0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
      0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
      0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
      0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
      0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
      0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
      0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
      0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
      0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
      0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
      0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
      0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
      0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
      0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
      0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
      0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
      0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
      0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
      0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
      0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
   };

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t cm_init(void) {

/* 7.1.1   Functional Description

   This routine will initialize the Communications Manager.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   result   CM_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t   result = WIN_ERROR_OK;
   uint32_t   i;
   char       line[256];
   char       build_time[64], build_date[64];

   struct tm* c_tm;
   time_t time_now;

// 7.1.5   Code

   // Assign the CM Layer Object ID
   //
   // All CM Instances have the same CM ID,
   // they are distinguished only by the
   // Device ID where they reside.
   //
   cm.id    = CM_ID_INSTANCE;
   cm.devid = CM_DEV_WIN;
   cm.log   = NULL;

   // Initialize the CM Objects
   for (i=0;i<=CM_MAX_OBJS;i++) {
      cm.obj[i].id    = CM_ID_NULL;
      cm.obj[i].devid = cm.devid;
      cm.obj[i].msg   = NULL;
      cm.obj[i].timer = NULL;
      cm.obj[i].sub   = NULL;
   }

      // Initialize the CM Pipes
   for (i=0;i<CM_MAX_PIPES;i++) {
      cm.pipe[i].cmid = CM_ID_NULL;
      cm.pipe[i].msgid = CM_PIPE_NULL;
      cm.pipe[i].dstdev = CM_DEV_NULL;
      cm.pipe[i].mark = 0;
      cm.pipe[i].count = 0;
      cm.pipe[i].first = NULL;
      cm.pipe[i].pipelen = 0;
   }

   // Initialize the CM Timers
   for (i=0;i<CM_MAX_TIMERS;i++) {
      cm.tim[i].cmid   = CM_ID_NULL;
      cm.tim[i].srvid  = CM_ID_NULL;
      cm.tim[i].msgid  = 0;
      cm.tim[i].count  = 0;
      cm.tim[i].reload = 0;
   }

   // Initialize the CM Routing Table
   for (i=0;i<CM_MAX_ROUTES;i++) {
      cm.rt[i].cmid  = CM_ID_NULL;
      cm.rt[i].devid = CM_DEV_NULL;
   }

   // Initialize the CM Port Table
   for (i=0;i<CM_MAX_PORTS;i++) {
      cm.port[i].media = CM_MEDIA_NONE;
      cm.port[i].io    = cm_tx_drop;
   }

   // Init counters
   cm.num_objs    = 0;
   cm.num_route   = 0;
   cm.num_cons    = 0;
   cm.num_pipes   = 0;
   cm.seqid       = 0;
   cm.alloc       = 0;

   // Init Queue
   memset(&cmq, 0, sizeof(cmq));
   cm.q_head    = 0;
   cm.q_tail    = 0;
   cm.q_msg_cnt = 0;
   InitializeConditionVariable (&cm.q_cv);
   InitializeCriticalSection (&cm.q_mutex);
   for (i=0;i<CM_MSGQ_SLOTS;i++) {
      cmq[i].size  = CM_MSGQ_BUF_LEN;
      cmq[i].state = CM_Q_IDLE;
      // allow room for uint32_t length
      cmq[i].buf   = &cmq[i].raw[1];
   }

   // open file for logging
   if (gc.feature & WIN_FEATURE_LOG_TRAFFIC) {
      if ((cm.log = fopen("cm_traffic.txt", "wt")) == NULL) {
         printf("cm_init() Fatal Error : CM traffic log did not Open, %s",
                  "cm_traffic.txt");
         result |= WIN_ERROR_CM;
      }
      time(&time_now);
      c_tm = localtime(&time_now);
      sprintf(build_date,"%02d.%s.%02d", c_tm->tm_mday, gc.month[c_tm->tm_mon],
            (c_tm->tm_year+1900)-2000);
      sprintf(build_time,"%02d:%02d:%02d", c_tm->tm_hour, c_tm->tm_min, c_tm->tm_sec);
      sprintf(line, "%s %s \n\n", build_date, build_time);
      fwrite(line, sizeof(char), strlen(line), cm.log);
      sprintf(line, "< outbound, > inbound, * local, # timer, @ pipe\n\n");
      fwrite(line, sizeof(char), strlen(line), cm.log);
   }

   // Register this instance of the CM
   cm.handle = cm_register(cm.id, cm_msg, NULL, NULL);

   // Start the Message Delivery Thread
   if ((cm.q_thread = CreateThread(NULL, 0, cm_thread, NULL, 0, &cm.q_tid)) == NULL) {
      result = WIN_ERROR_CM;
   }

   // Start CM's Periodic Timer, every 100 milliseconds
   cm.q_timer = CreateTimerQueue();
   CreateTimerQueueTimer(&cm.h_timer, cm.q_timer, cm_timer_callback,
         NULL, 100, 100, 0);

   // Display CM Instance ID
   if (gc.trace & WIN_TRACE_ID) {
      printf("CM Instance, id:devid:handle = %02X:%02X:%02X\n", cm.id, cm.devid, cm.handle);
   }

   return result;

} // end cm_init()


// ===========================================================================

// 7.2

uint8_t cm_register(uint8_t cmid, uint32_t (*msg)(pcm_msg_t msg),
                    uint32_t (*timer)(pcm_msg_t msg), pcm_sub_t cmsub) {

/* 7.2.1   Functional Description

   This routine will register the calling object with CM.

   7.2.2   Parameters:

   cmid     CM ID used for registration.
   msg()    Message Callback routine
   timer()  Timer Callback routine
   cmsub    Pointer to subscriptions

   7.2.3   Return Values:

   handle   Small integer object handle

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint8_t    handle = 0;
   uint8_t    i;

// 7.2.5   Code

   // Place Object in CM List
   if (cm.num_objs < CM_MAX_OBJS) {
      // CM Identifier
      cm.obj[cm.num_objs].id    = cmid;
      // Device Identifier
      cm.obj[cm.num_objs].devid = cm.devid;
      // Record message callback
      cm.obj[cm.num_objs].msg   = msg;
      // Record timer callback
      cm.obj[cm.num_objs].timer = timer;
      // Record Subscriptions
      cm.obj[cm.num_objs].sub   = cmsub;
      // Store Handle and Next Object
      handle = cm.num_objs++;
   }

   // Place Object in Routing Table
   for (i=0;i<CM_MAX_ROUTES;i++) {
      if (cm.rt[i].cmid == CM_ID_NULL) {
         cm.rt[i].cmid  = cmid;
         cm.rt[i].devid = cm.devid;
         cm.num_route++;
         break;
      }
   }

   return handle;

} // end cm_register()


// ===========================================================================

// 7.3

uint32_t cm_ioreg(cmio_t comio, uint8_t port, uint8_t media) {

/* 7.3.1   Functional Description

   This routine will register the media transmit interface.

   7.3.2   Parameters:

   comio    Transmit Callback
   port     Com Port
   media    Media Type

   7.3.3   Return Values:

   result   CM_ERR_IF_MAX if Maximum Com interfaces exceeded
            CM_OK otherwise

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint32_t    result = CM_OK;

// 7.3.5   Code

   // Validate Media and Port Range
   if (media < CM_MAX_MEDIA && port < CM_MAX_PORTS) {
      // Register the Transmit Interface
      if (comio != NULL) {
         // Register the Callback
         cm.port[port].io    = comio;
         // Store the Media Type
         cm.port[port].media = media;
         // Update Connections
         cm.num_cons++;
      }
      // Un-register the Transmit Interface
      else {
         // Un-register the Callback
         cm.port[port].io    = cm_tx_drop;
         // Remove the Media Type
         cm.port[port].media = CM_MEDIA_NONE;
         // Update Connections
         cm.num_cons--;
      }
   }
   // Maximum Port or Media Type Exceeded
   else {
      result = CM_ERR_IF_MAX;
   }

   return result;

} // end cm_ioreg()


// ===========================================================================

// 7.4

uint32_t cm_send(uint8_t msg_type, pcm_send_t ps) {

/* 7.4.1   Functional Description

   For each message type, the parameters will be filled-in based on
   the incoming pS structure. The newly created message will then be placed in
   in the Media's transmit queue for delivery.

   7.4.2   Parameters:

   msg_type CM_MSG_REQ, CM_MSG_RESP, CM_MSG_IND or CM_MSG_TIMER
   ps       Parameter Structure

   7.4.3   Return Values:

   result      CM_OK, else CM_ERROR

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint32_t    result = CM_OK;

// 7.4.5   Code

   // Validate Message
   if (ps->msg != NULL) {

      // Trace Entry
      if (gc.trace & WIN_TRACE_CM) {
         printf("cm_send(), msg_type:srvid:msgid:port:msglen = %02X:%02X:%02X:%1X:%02X\n",
               msg_type, ps->msg->p.srvid, ps->msg->p.msgid,
               ps->msg->h.port, ps->msglen);
      }

      // Build the CM Message Header
      // Assign the CM Protocol, Event, Endian and Port
      ps->msg->h.proto  = CM_PROTO;
      ps->msg->h.event  = CM_EVENT_MSG;
      ps->msg->h.endian = CM_ENDIAN;

      //
      //  REQUEST
      //
      if (msg_type == CM_MSG_REQ) {
         // New Sequence ID
         ps->msg->h.seqid     = cm.seqid++;
         // Destination is a CM Server
         ps->msg->h.dst_cmid  = ps->dst_cmid;
         ps->msg->h.dst_devid = cm_get_devid(ps->dst_cmid);
         // Source
         ps->msg->h.src_cmid  = ps->src_cmid;
         ps->msg->h.src_devid = cm.devid;
         // Port Connection
         ps->msg->h.port      = CM_PORT_COM0;
      }
      //
      //  RESPONSE
      //
      else if (msg_type == CM_MSG_RESP && ps->req != NULL) {
         // Use the Request's Sequence ID
         ps->msg->h.seqid     = ps->req->h.seqid;
         // Destination from Request Msg
         ps->msg->h.dst_cmid  = ps->req->h.src_cmid;
         ps->msg->h.dst_devid = ps->req->h.src_devid;
         // Source from Request Msg
         ps->msg->h.src_cmid  = ps->req->h.dst_cmid;
         ps->msg->h.src_devid = ps->req->h.dst_devid;
         // Port Connection, from requester
         ps->msg->h.port      = ps->req->h.port;
      }
      //
      //  INDICATION
      //
      else if (msg_type == CM_MSG_IND) {
         // New Sequence ID
         ps->msg->h.seqid     = cm.seqid++;
         // Destination determines Routing
         ps->msg->h.dst_cmid  = CM_ID_BCAST;
         ps->msg->h.dst_devid = gc.winid;
         // Source from CM Object
         ps->msg->h.src_cmid  = ps->src_cmid;
         ps->msg->h.src_devid = cm.devid;
         // Port Connection, always COM0
         ps->msg->h.port      = CM_PORT_COM0;
      }
      //
      //  TIMER MESSAGE
      //
      else if (msg_type == CM_MSG_TIMER) {
         // Set Timer Event
         ps->msg->h.event     = CM_EVENT_TIMER;
         // New Sequence ID
         ps->msg->h.seqid     = cm.seqid++;
         // Destination is always local
         ps->msg->h.dst_cmid  = ps->dst_cmid;
         ps->msg->h.dst_devid = cm.devid;
         // Source from CM
         ps->msg->h.src_cmid  = ps->src_cmid;
         ps->msg->h.src_devid = cm.devid;
         // Port Connection, always COM0
         ps->msg->h.port      = CM_PORT_COM0;
      }
      //
      //  CM DEVICE REQUEST
      //
      else if (msg_type == CM_MSG_DEV_REQ) {
         // New Sequence ID
         ps->msg->h.seqid     = cm.seqid++;
         // Destination is a CM Server
         ps->msg->h.dst_cmid  = ps->dst_cmid;
         ps->msg->h.dst_devid = ps->dst_devid;
         // Source
         ps->msg->h.src_cmid  = ps->src_cmid;
         ps->msg->h.src_devid = cm.devid;
         // Port Connection
         ps->msg->h.port      = ps->port;
      }
      //
      //  CM DEVICE RESPONSE
      //
      else if (msg_type == CM_MSG_DEV_RESP) {
         // New Sequence ID
         ps->msg->h.seqid     = cm.seqid++;
         // Destination is a CM Server
         ps->msg->h.dst_cmid  = ps->dst_cmid;
         ps->msg->h.dst_devid = ps->dst_devid;
         // Source
         ps->msg->h.src_cmid  = ps->src_cmid;
         ps->msg->h.src_devid = cm.devid;
         // Port Connection
         ps->msg->h.port      = ps->port;
      }

      // Message Length in BYTES
      ps->msg->h.msglen = ps->msglen;

      // Compute the Message CRC
      cm_crc(ps->msg, CM_CALC_CRC);

      // queue the message
      cm_qmsg(ps->msg);

   }

   return result;

} // end cm_send()


// ===========================================================================

// 7.5

pcmq_t cm_alloc(void) {

/* 7.5.1   Functional Description

   This routine will allocate a queue slot from the local CM queue.
   All allocated messages are the same length.

   7.5.2   Parameters:

   NONE

   7.5.3   Return Values:

   pcmq_t   Pointer to allocated CM message queue slot

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

   pcmq_t      slot = NULL;
   uint32_t    i;
   pcm_msg_t   msg = NULL;

// 7.5.5   Code

   // Enter Critical Section
   EnterCriticalSection(&cm.q_mutex);

   // Find next available slot
   for (i=0;i<CM_MSGQ_SLOTS;i++) {
      if (cmq[cm.q_head].state == CM_Q_IDLE) {
         cmq[cm.q_head].state = CM_Q_ALLOC;
         slot = &cmq[cm.q_head];
         // clear the entire buffer
         memset(slot->raw, 0, sizeof(uint32_t) * CM_MSGQ_BUF_LEN);
         // account for uint32_t length at raw start
         msg = (pcm_msg_t)slot->buf;
         // used to retrieve slot from message
         msg->h.slot = cm.q_head;
         // in CM circular queue, so don't delete
         msg->h.keep = 1;
         if (++cm.q_head >= CM_MSGQ_SLOTS) cm.q_head = 0;
		 cm.alloc++;
         if (gc.trace & CFG_TRACE_CM) {
            printf("cm_malloc(), slotid = %02X\n", msg->h.slot);
         }
         break;
      }
      if (++cm.q_head >= CM_MSGQ_SLOTS) cm.q_head = 0;
   }

   // Leave Critical Section
   LeaveCriticalSection(&cm.q_mutex);

   if (gc.trace & WIN_TRACE_ERROR) {
      if (slot == NULL)
         printf("cm_alloc() No Queue Slots Available\n");
   }

   // Trace Exit
   if (gc.trace & WIN_TRACE_CM) {
      if (msg != NULL)
         printf("cm_malloc(), slotid = %02X\n", msg->h.slot);
   }

   return slot;

} // end cm_alloc()


// ===========================================================================

// 7.6

void cm_free(pcm_msg_t msg) {

/* 7.6.1   Functional Description

   This routine will release the slot in the queue.

   7.6.2   Parameters:

   msg     CM queue slot buffer, contains the slot id

   7.6.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.6.4   Data Structures

   pcmq_t   slot;

// 7.6.5   Code

   // Enter Critical Section
   EnterCriticalSection(&cm.q_mutex);

   // Release the Slot
   if (msg != NULL) {
      slot = &cmq[msg->h.slot];
      slot->state = CM_Q_IDLE;
	  cm.alloc--;
   }
   else {
      if (gc.trace & WIN_TRACE_ERROR) {
         printf("cm_free() Null Pointer\n");
      }
   }

   // Leave Critical Section
   LeaveCriticalSection(&cm.q_mutex);

} // end cm_free()


// ===========================================================================

// 7.7

uint32_t cm_crc(pcm_msg_t msg, uint8_t crc_chk) {

/* 7.7.1   Functional Description

   This routine will compute the messages PAYLOAD CRC-8 and place it at the
   appropriate location. If crcChk is TRUE, the CRC is validated against the
   message. The CRC-8 is computed using the polynomial x^8 + x^5 + x^4 + 1.
   For reference see Maxim's Application Note 27, "Understanding and Using
   Cyclic Redundancy Checks with Maxim iButton Products".

   NOTE: THE CM HEADER IS EXCLUDED FROM THE CRC-8 COMPUTATION.

   7.7.2   Parameters:

   msg      Pointer to Received Message
   crc_chk  CM_CHECK_CRC: Compute the messages checksum and compare
            CM_CALC_CRC:  Compute the messages checksum and place at msg->h.crc

   7.7.3   Return Values:

   result   CM_OK on success
            CM_ERR_CRC when CRC is invalid

-----------------------------------------------------------------------------
*/

// 7.7.4   Data Structures

   uint32_t    result = CM_OK;
   uint8_t     k = 0, data = 0;
   uint16_t    i;
   uint16_t    msglen = msg->h.msglen - sizeof(cm_hdr_t);
   uint8_t    *pin = (uint8_t *)msg;

// 7.7.5   Code

   // COMPUTE CRC-8
   if (crc_chk == CM_CALC_CRC) {
      // exclude CM header
      for (i=sizeof(cm_hdr_t);i<msglen;i++) {
         data = pin[i] ^ k;
         k = crc_array[data];
      }
      msg->h.crc8 = k;
   }
   // VERIFY CRC-8
   else {
      for (i=sizeof(cm_hdr_t);i<msglen;i++) {
         data = pin[i] ^ k;
         k = crc_array[data];
      }
      if (msg->h.crc8 != k) result = CM_ERR_CRC;
   }

   if (gc.trace & WIN_TRACE_ERROR && result == CM_ERR_CRC) {
      printf("cm_crc(), msg : %02X, calc : %02X, len : %d\n", msg->h.crc8, k, msglen);
      dump(pin, msglen, 0, 0);
   }

   return result;

} // end cm_crc()


// ===========================================================================

// 7.8

uint32_t cm_msg(pcm_msg_t msg) {

/* 7.8.1   Functional Description

   This routine will handle messages directed to this devices CM Instance.
   Any CM Message will clear the Ping Timer.

   7.8.2   Parameters:

   msg      Pointer to Received Message

   7.8.3   Return Values:

   result   CM_OK, else CM_ERROR

-----------------------------------------------------------------------------
*/

// 7.8.4   Data Structures

   uint32_t    result = CM_OK;
   uint8_t     i,j;

   uint8_t     devid  = msg->h.src_devid;
   uint16_t    port   = msg->h.port;
   uint16_t    cm_msg = MSG(msg->p.srvid, msg->p.msgid);
   cm_send_t   ps = {0};

// 7.8.5   Code

   // Trace Entry
   if (gc.trace & WIN_TRACE_SERVER) {
      printf("cm_msg(), srvid:msgid:port = %02X:%02X:%1X\n",
            msg->p.srvid, msg->p.msgid, msg->h.port);
   }

   //
   //    CM REGISTRATION REQUEST
   //
   if (cm_msg == MSG(CM_ID_INSTANCE, CM_REG_REQ)) {
      //
      // Open the Connection
      //
      if (msg->p.flags & CM_REG_OPEN) {
         pcm_reg_msg_t req = (pcm_reg_msg_t)msg;
         pcmq_t slot = cm_alloc();
         if (slot != NULL) {
            // remove existing devid routes
            for (i=0;i<CM_MAX_ROUTES;i++) {
               if (cm.rt[i].devid == devid) {
                  cm.rt[i].cmid  = CM_ID_NULL;
                  cm.rt[i].devid = CM_DEV_NULL;
                  cm.num_route--;
               }
            }
            // insert incoming routes
            if (req->b.rec_cnt < CM_MAX_ROUTES) {
               for (i=0;i<req->b.rec_cnt;i++) {
                  for (j=0;j<CM_MAX_ROUTES;j++) {
                     if (cm.rt[j].cmid == CM_ID_NULL) {
                        cm.rt[j].cmid  = req->b.rec[i].cmid;
                        cm.rt[j].devid = req->b.rec[i].devid;
                        cm.num_route++;
                        break;
                     }
                  }
               }
            }
            pcm_reg_msg_t rsp = (pcm_reg_msg_t)slot->buf;
            ps.msg         = (pcm_msg_t)rsp;
            ps.req         = (pcm_msg_t)msg;
            rsp->p.srvid   = CM_ID_INSTANCE;
            rsp->p.msgid   = CM_REG_RESP;
            rsp->p.flags   = CM_NO_FLAGS;
            rsp->p.status  = CM_OK;
            ps.dst_cmid    = CM_ID_INSTANCE;
            ps.src_cmid    = CM_ID_INSTANCE;
            ps.dst_devid   = devid;
            ps.port        = (uint8_t)port;
            // Fill out the Message Data with
            // the local CM Objects
            rsp->b.rec_cnt = 0;
            rsp->b.status  = 0;
            rsp->b.unused  = 0;
            // Cycle over all local CM Objects
            for (i=0;i<cm.num_objs;i++) {
               // Only Send up to CM_MAX_ROUTES
               if (rsp->b.rec_cnt == CM_MAX_ROUTES) break;
               // Add record
               rsp->b.rec[rsp->b.rec_cnt].cmid  = cm.obj[i].id;
               rsp->b.rec[rsp->b.rec_cnt].devid = cm.obj[i].devid;
               rsp->b.rec_cnt++;
            }
            // Device Name
            memcpy(rsp->b.device, gc.dev_str, strlen((char *)gc.dev_str));
            // Account for each record plus status and count, etc.
            ps.msglen = sizeof(cm_reg_msg_t);
            // Send the Request
            result = cm_send(CM_MSG_DEV_RESP, &ps);
            // Reset Ping Timeout
            time(&gc.ping_time);
            gc.ping_cnt = 0;
            // Machine status to Connected
            gc.status |= CFG_STATUS_CONNECTED;
            // report connection
            if (gc.trace & CFG_TRACE_ID) {
               printf("cm_msg() Connected to %s, port %d\n", req->b.device, req->h.port);
            }
         }
      }
      //
      // Close the Connection
      //
      else if (msg->p.flags & CM_REG_CLOSE) {
         pcm_reg_msg_t req = (pcm_reg_msg_t)msg;
         // Machine status to Disconnected
         gc.status &= ~CFG_STATUS_CONNECTED;
         // Reset Ping Timeout
         time(&gc.ping_time);
         gc.ping_cnt = 0;
         // report disconnection
         if (gc.trace & CFG_TRACE_ID) {
            printf("cm_msg() Disconnected from %s, port %d\n", req->b.device, msg->h.port);
         }
         // No Response
      }
   }
   //
   //    CM REGISTRATION RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_INSTANCE, CM_REG_RESP)) {
      pcm_reg_msg_t rsp = (pcm_reg_msg_t)msg;
      // remove existing devid routes
      for (i=0;i<CM_MAX_ROUTES;i++) {
         if (cm.rt[i].devid == devid) {
            cm.rt[i].cmid  = CM_ID_NULL;
            cm.rt[i].devid = CM_DEV_NULL;
            cm.num_route--;
         }
      }
      // insert incoming routes
      if (rsp->b.rec_cnt < CM_MAX_ROUTES) {
         for (i=0;i<rsp->b.rec_cnt;i++) {
            for (j=0;j<CM_MAX_ROUTES;j++) {
               if (cm.rt[j].cmid == CM_ID_NULL) {
                  cm.rt[j].cmid  = rsp->b.rec[i].cmid;
                  cm.rt[j].devid = rsp->b.rec[i].devid;
                  cm.num_route++;
                  break;
               }
            }
         }
      }
      // Reset Ping Timeout
      time(&gc.ping_time);
      gc.ping_cnt = 0;
      // Machine status to Connected
      gc.status |= CFG_STATUS_CONNECTED;
      // report connection
      if (gc.trace & CFG_TRACE_ID) {
         printf("cm_msg() Connected to %s, port %d\n", rsp->b.device, rsp->h.port);
      }
      // start ping request timer
      cm_timer_set(CM_TMR_ID0, CP_TMR_PING, 15000, CM_ID_CP_CLI, CM_ID_CP_CLI);
      // start ping timeout timer
      cm_timer_set(CM_TMR_ID1, CP_TMR_PING_TIMEOUT, 60000, CM_ID_CP_CLI, CM_ID_CP_CLI);
      // send version request on behalf of CP_CLI
      cm_send_req(CM_ID_CP_SRV, CP_VER_REQ, CM_ID_CP_CLI, 0);
   }
   //
   //    CM QUERY REQUEST
   //
   else if (cm_msg == MSG(CM_ID_INSTANCE, CM_QUERY_REQ)) {
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pcm_query_msg_t rsp = (pcm_query_msg_t)slot->buf;
         rsp->p.srvid      = CM_ID_INSTANCE;
         rsp->p.msgid      = CM_QUERY_RESP;
         rsp->p.flags      = CM_NO_FLAGS;
         // Return the Magic Number
         rsp->b.magic      = 0x55AA1234;
         // Return SysID and Time stamp
         rsp->b.sysid      = gc.sysid;
         rsp->b.stamp      = gc.sys_time;
         rsp->b.devid      = 0;
         rsp->b.num_objs   = 4;
         rsp->b.num_cons   = 2;
         rsp->b.reserved   = 0;
         // Send the Response
         cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg,
               sizeof(cm_query_msg_t), msg->h.src_cmid, msg->h.src_devid);
      }
   }
   //
   //    UNKNOWN MESSAGE
   //
   else {
      if (gc.trace & WIN_TRACE_ERROR) {
         printf("cm_msg() Unknown Message, msgid:srvid = %02X:%02X\n",
                  msg->p.msgid, msg->p.srvid);
         dump((uint8_t*)msg, 12, 0, 0);
      }
   }

   // Reset Ping Timeout
   time(&gc.ping_time);
   gc.ping_cnt = 0;

   // Free the Slot
   cm_free(msg);

   return result;

} // end cm_msg()


// ===========================================================================

// 7.9

uint8_t cm_get_handle(uint8_t cmid) {

/* 7.9.1   Functional Description

   This routine will return the handle for a particular cmid.

   7.9.2   Parameters:

   cmid     CM ID

   7.9.3   Return Values:

   result   handle

-----------------------------------------------------------------------------
*/

// 7.9.4   Data Structures

   uint32_t   i;

// 7.9.5   Code

   // Search for cmid in active Objects
   for (i=0;i<cm.num_objs;i++) {
      if (cm.obj[i].id == cmid) {
         return i;
      }
   }

   return CM_MAX_OBJS;

} // end cm_get_handle()


// ===========================================================================

// 7.10

uint8_t cm_id_exists(uint8_t cmid) {

/* 7.10.1   Functional Description

   This routine checks the existence for a particular cmid.

   7.10.2   Parameters:

   cmid     CM ID

   7.10.3   Return Values:

   result   TRUE if cmid exists
            FALSE otherwise

-----------------------------------------------------------------------------
*/

// 7.10.4   Data Structures

   uint32_t   i;
   uint8_t    result = FALSE;

// 7.10.5   Code

   for (i=0;i<cm.num_route;i++) {
      if (cm.rt[i].cmid == cmid) {
         result = TRUE;
         break;
      }
   }

   return result;

} // end cm_id_exists()


// ===========================================================================

// 7.11

uint8_t cm_get_devid(uint8_t cmid) {

/* 7.11.1   Functional Description

   This routine will return the devid for a particular cmid.

   7.11.2   Parameters:

   cmid     CM ID

   7.11.3   Return Values:

   result   devid

-----------------------------------------------------------------------------
*/

// 7.11.4   Data Structures

   uint32_t   i;

// 7.11.5   Code

   // First Look for Local Server
   for (i=0;i<cm.num_objs;i++) {
      if (cm.obj[i].id == cmid) {
         // Return Local Device
         return cm.devid;
      }
   }

   // Look for Remote Server
   for (i=0;i<cm.num_route;i++) {
      if (cm.rt[i].cmid == cmid) {
         return cm.rt[i].devid;
      }
   }

   return CM_DEV_NULL;

} // end cm_get_devid()


// ===========================================================================

// 7.12

uint8_t cm_get_sub(uint8_t h, uint8_t srvID, uint8_t msgid) {

/* 7.12.1   Functional Description

   This routine will search all the local objects for subscriptions to the
   srvid, msgid combination, if found the cmid of the object will be returned,
   otherwise CM_ID_NULL will be returned.

   7.12.2   Parameters:

   h        Object Handle
   srvID    CM Server ID
   msgid    Message ID

   7.12.3   Return Values:

   cmid     CM Object with subscription

-----------------------------------------------------------------------------
*/

// 7.12.4   Data Structures

   uint8_t    i;
   uint8_t    cmid = CM_ID_NULL;

// 7.12.5   Code

   if (cm.obj[h].sub != NULL) {
      // Cycle over all Object subscriptions
      // looking for a srvID:msgid match
      for (i=0;i<CM_MAX_SUBS;i++) {
         // Break when CM_ID_NULL
         if (cm.obj[h].sub[i].srvid == CM_ID_NULL) break;
         // Match srvID:msgid
         if ((cm.obj[h].sub[i].srvid == srvID) &&
             (cm.obj[h].sub[i].msgid == msgid)) {
            cmid = cm.obj[h].id;
            break;
         }
      }
   }

   return cmid;

} // end cm_get_sub()


// ===========================================================================

// 7.13

uint32_t cm_timer_set(uint8_t timerid, uint8_t msgid, uint32_t time_ms,
                      uint8_t cmid, uint8_t srvid) {

/* 7.13.1   Functional Description

   This routine will set the interval count of the timer associated
   with timerid. Timer resolution is based on WIN_TICKS_PER_SECOND.

   7.13.2   Parameters:

   timerid     Timer ID to set
   msgid       Timer message ID
   time_ms     Interval Time Out value in milliseconds
   cmid        CM ID
   srvid       Server ID

   7.13.3   Return Values:

   result   CM_OK, else CM_ERR_TIMER_ID

-----------------------------------------------------------------------------
*/

// 7.13.4   Data Structures

   uint32_t      result = CM_ERR_TIMER_ID;
   uint8_t       k = timerid;

// 7.13.5   Code

   // Validate Timer
   if (k < CM_MAX_TIMERS) {
      cm.tim[k].cmid   = cmid;
      cm.tim[k].srvid  = srvid;
      cm.tim[k].msgid  = msgid;
      cm.tim[k].count  = time_ms / WIN_TICKS_MS;
      cm.tim[k].reload = time_ms / WIN_TICKS_MS;
   }
   // Timer ID Error
   else {
      result = CM_ERR_TIMER_ID;
   }

   return result;

} // end cm_timer_set()


// ===========================================================================

// 7.14

uint32_t cm_timer_kill(uint8_t timerid, uint8_t cmid) {

/* 7.14.1   Functional Description

   This routine will kill the timer associated with timerid.

   7.14.2   Parameters:

   timerid     Timer ID to kill

   7.14.3   Return Values:

   result   CM_OK, else CM_ERR_TIMER_ID

-----------------------------------------------------------------------------
*/

// 7.14.4   Data Structures

   uint32_t      result = CM_ERR_TIMER_ID;
   uint8_t       k = timerid;

// 7.14.5   Code

   // Validate Timer
   if (k < CM_MAX_TIMERS) {
      cm.tim[k].cmid   = CM_MSG_NULL;
      cm.tim[k].srvid  = 0;
      cm.tim[k].msgid  = 0;
      cm.tim[k].count  = 0;
      cm.tim[k].reload = 0;
   }
   // Timer ID Error
   else {
      result = CM_ERR_TIMER_ID;
   }

   return result;

} // end cm_timer_kill()


// ===========================================================================

// 7.15

uint32_t cm_tick(void) {

/* 7.15.1   Functional Description

   This routine will look through the list of active timers and decrement
   their counters, if they expire, a timer message will be generated and
   sent to the associated CM object.

   Since each call to this routine will decrement the active timers, the
   resolution of each timer count is based on the frequency with which
   this call is made.

   This function runs in the WIN32 timer thread, no significant processing
   should be done here.

   7.15.2   Parameters:

   NONE

   7.15.3   Return Values:

   result   CM_OK, else CM_ERR

-----------------------------------------------------------------------------
*/

// 7.15.4   Data Structures

   uint32_t    result = CM_OK;
   uint32_t    i;
   cm_send_t   ps = {0};

// 7.15.5   Code

   //
   // Search for any active timers
   // and decrement their count, when they reach zero
   // send a timer event to the associated object.
   //
   // All timers reload automatically
   //
   // Cycle over all active timers
   for (i=0;i<CM_MAX_TIMERS;i++) {
      // break if CM_ID_NULL
      if (cm.tim[i].cmid == CM_ID_NULL) break;
      // Check for non-zero count
      if (cm.tim[i].count != 0) {
         cm.tim[i].count--;
         // Timer has expired
         if (cm.tim[i].count == 0) {
            // reload timer
            cm.tim[i].count = cm.tim[i].reload;
            memset(&ps, 0, sizeof(cm_send_t));
            pcmq_t slot = cm_alloc();
            if (slot != NULL) {
               pcm_msg_t msg = (pcm_msg_t)slot->buf;
               ps.msg = msg;
               msg->p.srvid  = cm.tim[i].srvid;
               msg->p.msgid  = cm.tim[i].msgid;
               msg->p.flags  = CM_NO_FLAGS;
               msg->p.status = CM_OK;
               ps.src_cmid   = cm.tim[i].cmid;
               ps.dst_cmid   = cm.tim[i].cmid;
               ps.dst_devid  = cm.devid;
               ps.msglen     = sizeof(cm_msg_t);
               // Send the Timer Message
               result = cm_send(CM_MSG_TIMER, &ps);
            }
         }
      }
   }

   return result;

} // end cm_tick()


// ===========================================================================

// 7.16

uint32_t cm_route(pcm_msg_t msg) {

/* 7.16.1   Functional Description

   This routine will call the destination objects receive message function.

   7.16.2   Parameters:

   msg     Pointer to Received Message

   7.16.3   Return Values:

   result   CM_OK on success
            CM_ERR_OBJ_ID_NULL when object ID is null

-----------------------------------------------------------------------------
*/

// 7.16.4   Data Structures

   uint32_t   result = CM_OK;
   uint8_t    cmid;

// 7.16.5   Code

   // Validate Message
   if (msg == NULL) {
      result = CM_ERR_ROUTE;
      if (gc.trace & WIN_TRACE_ERROR) {
         printf("cm_route() Error : NULL Message\n");
      }
      return result;
   }

   cmid  = msg->h.dst_cmid;

   // Trace the Message before Routing
   if (gc.trace & WIN_TRACE_ROUTE) {
      printf("cm_route() control : \n");
      printf("  dst_cmid:  %02X\n", msg->h.dst_cmid);
      printf("  src_cmid:  %02X\n", msg->h.src_cmid);
      printf("  dst_devid: %1X\n",  msg->h.dst_devid);
      printf("  src_devid: %1X\n",  msg->h.src_devid);
      printf("  seqid:     %02X\n", msg->h.seqid);
      printf("  endian:    %1X\n",  msg->h.endian);
      printf("  event:     %1X\n",  msg->h.event);
      printf("  keep:      %1X\n",  msg->h.keep);
      printf("  proto:     %1X\n",  msg->h.proto);
      printf("  crc8:      %02X\n", msg->h.crc8);
      printf("  slot:      %02X\n", msg->h.slot);
      printf("  port:      %1X\n",  msg->h.port);
      printf("  msglen:    %04X\n", msg->h.msglen);
   }

   // Validate CRC
   if (cm_crc(msg, CM_CHECK_CRC) != CM_OK) {
      result = CM_ERR_CRC;
      if (gc.trace & WIN_TRACE_ERROR) {
         printf("cm_route() Error : Invalid CRC\n");
      }
      cm_free(msg);
      return result;
   }

   // Validate Port
   if (cm.port[msg->h.port].media == CM_MEDIA_NONE) {
      result = CM_ERR_MEDIA_NONE;
      cm_free(msg);
      if (gc.trace & WIN_TRACE_ERROR) {
         printf("cm_route() Error : Media Port Invalid\n");
      }
      cm_free(msg);
      return result;
   }

   // Verify that the callback exists and
   // forward the message
   switch (msg->h.event) {
      case CM_EVENT_MSG:
         if (cm.obj[cm_get_handle(cmid)].msg != NULL)
            cm.obj[cm_get_handle(cmid)].msg(msg);
         else
            printf("cm_route() Error : CMID Invalid, CM_EVENT_MSG\n");
         break;
      case CM_EVENT_TIMER:
         if (cm.obj[cm_get_handle(cmid)].timer != NULL)
            cm.obj[cm_get_handle(cmid)].timer(msg);
         else
            printf("cm_route() Error : CMID Invalid, CM_EVENT_TIMER\n");
         break;
      default:
         result = CM_ERR_EVENT_ID;
         if (gc.trace & WIN_TRACE_ERROR) {
            printf("cm_route() Error : Invalid Event ID\n");
         }
         cm_free(msg);
         break;
   }

   return result;

} // end cm_route()


// ===========================================================================

// 7.17

void cm_tx_drop(uint8_t opCode, pcm_msg_t msg) {

/* 7.17.1   Functional Description

   This routine is the default IO callback function for the CM port
   connections.

   7.17.2   Parameters:

   opCode   Operation Code
   msg     CM Message

   7.17.3   Return Values:

   msg     Always NULL

-----------------------------------------------------------------------------
*/

// 7.17.4   Data Structures

// 7.17.5   Code

   // As the default port connection the message
   // is simply discarded silently

   cm_free(msg);

} // end cm_tx_drop()


// ===========================================================================

// 7.18

uint32_t cm_send_msg(uint8_t msg_type, pcm_msg_t msg, pcm_msg_t req,
                     uint16_t msglen, uint8_t dst_cmid, uint8_t devid) {

/* 7.18.1   Functional Description

   This function sends a Message to the Requester or Indication.

   7.18.2   Parameters:

   msg_type    Message Type
   msg         Out bound Response/Indication
   req         Incoming Request or NULL
   msglen      Length of response/indication
   dst_cmid    Destination CM ID
   devid       Destination Device ID if indication

   7.18.3   Return Values:

   result      CM_OK

-----------------------------------------------------------------------------
*/

// 7.18.4   Data Structures

   uint32_t    result = CM_OK;
   cm_send_t   ps = {0};

// 7.18.5   Code

   // fill-out cm_send_t
   ps.msg       = msg;
   ps.req       = req;
   ps.msglen    = msglen;
   ps.dst_cmid  = dst_cmid;
   ps.src_cmid  = msg->p.srvid;
   ps.dst_devid = devid;

   // Send the Message
   result = cm_send(msg_type, &ps);

   return result;

} // end cm_send_msg()


// ===========================================================================

// 7.19

void cm_local(uint8_t srvid, uint8_t msgid, uint8_t flags,
              uint8_t status) {

/* 7.19.1   Functional Description

   This function fills-in the indication header from the parameters
   and sends it to the identified LOCAL CM Client or Server. This routine may be
   called from an ISR. The message is written to the receive queue.

   Local messages will use the gc.comPort.

   7.19.2   Parameters:

   srvid       Server ID
   msgid       Message ID
   flags       Message Flags
   status      Message Status

   7.19.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.19.4   Data Structures

   pcm_msg_t   msg;
   pcmq_t      slot = NULL;

// 7.19.5   Code

   // Trace Entry
   if (gc.trace & WIN_TRACE_CM) {
      printf("cm_local(), srvid:msgid:flags:status = %02X:%02X:%02X:%02X\n",
               srvid, msgid, flags, status);
   }

   // Next Slot in Queue
   slot = cm_alloc();

   if (slot != NULL) {

      msg = (pcm_msg_t)slot->buf;

      // Build the CM Message Header
      // Assign the CM Protocol Type and Event ID
      msg->h.proto   = CM_PROTO;
      msg->h.event   = CM_EVENT_MSG;
      msg->h.endian  = CM_ENDIAN;
      msg->h.port    = CM_PORT_COM0;

      // New Sequence ID
      msg->h.seqid    = cm.seqid++;

      // Destination determines Routing
      msg->h.dst_cmid  = srvid;
      msg->h.dst_devid = gc.devid;

      // Source from CM Object
      msg->h.src_cmid  = srvid;
      msg->h.src_devid = gc.devid;

      // Message Parameters
      msg->p.srvid    = srvid;
      msg->p.msgid    = msgid;
      msg->p.flags    = flags;
      msg->p.status   = status;

      // Message Length in BYTES
      msg->h.msglen = sizeof(cm_msg_t);

      // Compute the Message CRC
      cm_crc(msg, CM_CALC_CRC);

      // queue the message
      cm_qmsg(msg);

      // Trace Indication
      if (gc.trace & WIN_TRACE_LOCAL) dump((uint8_t *)msg, msg->h.msglen, 0, 0);

   }

} // end cm_local()


// ===========================================================================

// 7.20

uint32_t cm_send_req(uint8_t srvid, uint8_t msgid, uint8_t srcid, uint8_t flags) {

/* 7.20.1   Functional Description

   This function sends a request message.

   7.20.2   Parameters:

   srvid       cm server
   msgid       message id
   srcid       cm source
   flags       flags
   status      status
   msgbody     message body, msgid dependent
   bodylen     body length

   7.20.3   Return Values:

   result      CM_OK

-----------------------------------------------------------------------------
*/

// 7.20.4   Data Structures

   uint32_t    result = CM_OK;
   cm_send_t   ps = {0};

// 7.20.5   Code

   pcmq_t slot = cm_alloc();
   if (slot != NULL) {
      pcm_msg_t msg = (pcm_msg_t)slot->buf;
      ps.msg = (pcm_msg_t)msg;
      msg->p.srvid  = srvid;
      msg->p.msgid  = msgid;
      msg->p.flags  = flags;
      msg->p.status = CM_OK;

      ps.dst_cmid   = srvid;
      ps.src_cmid   = srcid;

      ps.msglen     = sizeof(cm_msg_t);

      // Send the Request
      result = cm_send(CM_MSG_REQ, &ps);
   }

   return result;

} // end cm_send_req()


// ===========================================================================

// 7.21

uint32_t cm_pipe_reg(uint8_t cmid, uint8_t msgid, uint32_t mark, uint8_t dstdev) {

/* 7.21.1   Functional Description

   This routine will register the calling CM object with the associated PIPE
   message. If the dstDev is CM_DEV_NULL and the pipe exists (based on cmid) then
   the association will be deleted.

   7.21.2   Parameters:

   cmid     Destination CM ID for PIPE
   msgid    Associated Pipe msgid
   mark     Water mark for accumulating messages
   dstdev   Destination Device ID

   7.21.3   Return Values:

   result   CMAPI_OK for success
            CMAPI_ERR_NUM_PIPE CM_MAX_PIPES exceeded

-----------------------------------------------------------------------------
*/

// 7.21.4   Data Structures

   uint32_t   result = CM_OK;
   uint32_t   i;

// 7.21.5   Code

   //
   // DELETE EXISTING PIPE CONNECTION
   //
   if (dstdev == CM_DEV_NULL) {
      // Find existing pipe based on cmid
      for (i=0;i<CM_MAX_PIPES;i++) {
         if (cm.pipe[i].cmid == cmid) {
            // Delete pipe
            cm.pipe[i].cmid = CM_ID_NULL;
            cm.pipe[i].msgid = CM_PIPE_NULL;
            cm.pipe[i].dstdev = CM_DEV_NULL;
            cm.pipe[i].mark = 0;
            cm.pipe[i].count = 0;
            cm.pipe[i].first = NULL;
            cm.pipe[i].pipelen = 0;
            cm.num_pipes--;
         }
      }
   }
   //
   // ADD NEW PIPE CONNECTION
   //
   else if (cm.num_pipes != CM_MAX_PIPES && mark != 0) {
      // Find first Available
      for (i=0;i<CM_MAX_PIPES;i++) {
         if (cm.pipe[i].cmid == CM_ID_NULL) {
            // CM Identifier
            cm.pipe[i].cmid  = cmid;
            // Message Identifiers
            cm.pipe[i].msgid = msgid;
            // Destination Device ID
            cm.pipe[i].dstdev = dstdev;
            // Water Mark
            cm.pipe[i].mark = mark;
            // Counters
            cm.pipe[i].count = 0;
            cm.pipe[i].first = NULL;
            cm.pipe[i].pipelen = 0;
            // New Pipe Connection
            cm.num_pipes++;
            break;
         }
      }
      // No available connections
      if (i == CM_MAX_PIPES) result = CM_ERR_NUM_PIPE;
   }
   else
      result = CM_ERR_NUM_PIPE;

   return result;

} // end cm_pipe_reg()


// ===========================================================================

// 7.22

uint32_t cm_pipe_send(pcm_pipe_t pipe, uint32_t pipelen) {

/* 7.22.1   Functional Description

   This function delivers a pipe message directly, no routing.

   The CMAPI_MSG_PIPE is sent based on the cmapi_PipeReg() call, which
   associates the CM_ID_PIPE message ID with a particular cmid. The message
   is posted directly from this function.

   If the watermark feature is used for pipe messages, then they must
   be contiguous in memory. For the case of no cmid registered for the
   pipe message ID, it will be deleted if the CM_PIPE_FREE flag is set.


   7.22.2   Parameters:

   pipe        Pointer to CM_PIPE
   pipelen     Length in Bytes of this pipe Message

   7.22.3   Return Values:

   result      CMAPI_OK

-----------------------------------------------------------------------------
*/

// 7.22.4   Data Structures

   uint32_t    result = CM_OK;
   uint32_t    i;
   pcm_pipe_t  lead;

// 7.22.5   Code

   // Validate Pipe Message
   if (pipe->dst_cmid == CM_ID_PIPE) {

      // Find Pipe Message cmid Association
      for (i=0;i<CM_MAX_PIPES;i++) {
         // Bypass inactive connections
         if (cm.pipe[i].cmid == CM_ID_NULL) continue;
         // Registered Message?
         if (cm.pipe[i].msgid == pipe->msgid) {
            // Record First Message
            if (cm.pipe[i].count == 0) {
               cm.pipe[i].first = (uint8_t *)pipe;
               cm.pipe[i].pipelen = 0;
            }
            // Count Message
            cm.pipe[i].count++;
            cm.pipe[i].pipelen += pipelen;
            // Check the Water Mark
            if (cm.pipe[i].mark == cm.pipe[i].count) {
               lead = (pcm_pipe_t)cm.pipe[i].first;
               // Trace the Message
               if (gc.trace & WIN_TRACE_PIPE) {
                  printf("cm_pipe_send() : \n");
                  printf("  dst_cmid:  %02X\n", lead->dst_cmid);
                  printf("  msgid:     %02X\n", lead->msgid);
                  printf("  port:      %02X\n", lead->port);
                  printf("  flags:     %02X\n", lead->flags);
                  printf("  msglen:    %d\n",   lead->msglen);
                  printf("  seqid:     %08X\n", lead->seqid);
                  printf("  stamp:     %08X\n", lead->stamp);
                  printf("  stamp_us:  %08X\n", lead->stamp_us);
                  printf("  status:    %08X\n", lead->status);
                  printf("  rate:      %08X\n", lead->rate);
                  printf("  magic:     %08X\n", lead->magic);
               }
               // log message
               cm_log((pcm_msg_t)pipe);
               // Send the Pipe Message
               if (cm.obj[cm_get_handle(cm.pipe[i].cmid)].msg != NULL)
                  cm.obj[cm_get_handle(cm.pipe[i].cmid)].msg((pcm_msg_t)lead);
               cm.pipe[i].count = 0;
               cm.pipe[i].pipelen = 0;
               cm.pipe[i].first = NULL;
            }
         }
      }
   }
   else {
      result = CM_ERR_PIPE;
   }

   return result;

} // end cm_pipe_send()


// ===========================================================================

 // 7.23

uint32_t cm_send_reg_req(uint8_t devid, uint8_t port, uint8_t flags,
                         uint8_t *device) {

/* 7.23.1   Functional Description

   This function sends the CM_REG_REQ message to the connected CM at
   the device specified.

   7.23.2   Parameters:

   devid    CM Device Identifier
   port     CM Port Identifier
   flags    Message Flags, CM_REG_OPEN or CM_REG_CLOSE
   device   Requesting Device String ID

   7.23.3   Return Values:

   result   CM_OK, else CM_ERROR

-----------------------------------------------------------------------------
*/

// 7.23.4   Data Structures

   uint32_t      result = CM_OK;
   uint8_t       i;
   cm_send_t     ps = {0};

// 7.23.5   Code

   pcmq_t slot = cm_alloc();
   if (slot != NULL) {
      pcm_reg_msg_t msg = (pcm_reg_msg_t)slot->buf;
      ps.msg = (pcm_msg_t)msg;
      msg->p.srvid  = CM_ID_INSTANCE;
      msg->p.msgid  = CM_REG_REQ;
      msg->p.flags  = flags;
      msg->p.status = CM_OK;

      ps.dst_cmid   = CM_ID_INSTANCE;
      ps.src_cmid   = CM_ID_INSTANCE;
      ps.dst_devid  = devid;
      ps.port       = port;

      // Fill out the Message Data with
      // the local CM Objects
      msg->b.rec_cnt = 0;
      msg->b.status  = 0;
      msg->b.unused  = 0;
      // Cycle over all local CM Objects
      for (i=0;i<cm.num_objs;i++) {
         // Only Send up to CM_MAX_ROUTES
         if (msg->b.rec_cnt == CM_MAX_ROUTES) break;
         // Add record
         msg->b.rec[msg->b.rec_cnt].cmid  = cm.obj[i].id;
         msg->b.rec[msg->b.rec_cnt].devid = cm.obj[i].devid;
         msg->b.rec_cnt++;
      }

      // Device Name
      memcpy(msg->b.device, device, strlen((char *)device));

      // Account for each record plus status and count, etc.
      ps.msglen = sizeof(cm_reg_msg_t);

      // Send the Request
      result = cm_send(CM_MSG_DEV_REQ, &ps);
   }

   return result;

} // end cm_send_reg_req()


// ===========================================================================

// 7.24

void cm_qmsg(pcm_msg_t msg) {

/* 7.24.1   Functional Description

   This routine will place the incoming message on the delivery queue.

   7.24.2   Parameters:

   msg     CM Message to queue

   7.24.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.24.4   Data Structures

   pcmq_t   slot;

// 7.24.5   Code

   // validate message
   if (msg != NULL) {

      // Trace Entry
      if (gc.trace & WIN_TRACE_CM) {
         printf("cm_qmsg(), srvid:msgid:port:slot = %02X:%02X:%1X:%02X\n",
               msg->p.srvid, msg->p.msgid, msg->h.port, msg->h.slot);
      }

      // Trace the Message
      if (gc.trace & WIN_TRACE_CMQ) {
         printf("cm_qmsg() : \n");
         printf("  dst_cmid:  %02X\n", msg->h.dst_cmid);
         printf("  src_cmid:  %02X\n", msg->h.src_cmid);
         printf("  dst_devid: %1X\n",  msg->h.dst_devid);
         printf("  src_devid: %1X\n",  msg->h.src_devid);
         printf("  seqid:     %02X\n", msg->h.seqid);
         printf("  endian:    %1X\n",  msg->h.endian);
         printf("  event:     %1X\n",  msg->h.event);
         printf("  keep:      %1X\n",  msg->h.keep);
         printf("  proto:     %1X\n",  msg->h.proto);
         printf("  crc8:      %02X\n", msg->h.crc8);
         printf("  slot:      %02X\n", msg->h.slot);
         printf("  port:      %1X\n",  msg->h.port);
         printf("  msglen:    %04X\n", msg->h.msglen);
      }

      // Enter Critical Section
      EnterCriticalSection(&cm.q_mutex);

      // log message
      cm_log(msg);

      // Set delivery state
      slot = &cmq[msg->h.slot];
      slot->state = CM_Q_DELIVER;

      // q_msg_cnt cannot exceed number of slots,
      // something has stalled otherwise
      cm.q_msg_cnt = (cm.q_msg_cnt + 1) & (CM_MSGQ_SLOTS - 1);

      // Leave Critical Section
      LeaveCriticalSection(&cm.q_mutex);

      // signal the CM thread
      WakeConditionVariable (&cm.q_cv);

   }
   else {
      if (gc.trace & WIN_TRACE_ERROR) {
         printf("cm_qmsg() Null Pointer\n");
      }
   }

} // end cm_qmsg()


// ===========================================================================

// 7.25

static DWORD WINAPI cm_thread(LPVOID data) {

/* 7.25.1   Functional Description

   This thread will provide a delivery service for CM messages from the queue.

   7.25.2   Parameters:

   data     Thread parameters

   7.25.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.25.4   Data Structures

   pcmq_t      slot = NULL;
   uint32_t    i;
   pcm_msg_t   msg = NULL;

// 7.25.5   Code


   if (gc.trace & WIN_TRACE_ID) {
      printf("cm_thread() started, tid 0x%X\n", cm.q_tid);
   }

   // Message Delivery Loop
   while (1) {

      // Enter Critical Section
      EnterCriticalSection(&cm.q_mutex);

      // Wait on condition variable,
      // this unlocks the mutex while waiting
      while (cm.q_msg_cnt == 0) {
         SleepConditionVariableCS(&cm.q_cv, &cm.q_mutex, INFINITE);
      }

      // clear previous message
      slot = NULL;

      // Find Available Message
      if (cm.q_msg_cnt != 0) {
         // Find next available message
         for (i=0;i<CM_MSGQ_SLOTS;i++) {
            if (cmq[cm.q_tail].state == CM_Q_DELIVER) {
               cmq[cm.q_tail].state = CM_Q_BUSY;
               msg = (pcm_msg_t)cmq[cm.q_tail].buf;
               msg->h.slot = cm.q_tail;
               slot = &cmq[cm.q_tail];
               if (++cm.q_tail >= CM_MSGQ_SLOTS) cm.q_tail = 0;
               cm.q_msg_cnt--;
               break;
            }
            if (++cm.q_tail >= CM_MSGQ_SLOTS) cm.q_tail = 0;
         }
      }

      // Leave Critical Section
      LeaveCriticalSection(&cm.q_mutex);

      // Route Message
      if (slot != NULL) {
         // out bound traffic
         if (msg->h.dst_devid != cm.devid) {
            // transmit the message, based on port, use this thread
            cm.port[msg->h.port].io(CM_IO_TX, msg);
         }
         // indication, search for subscriber
         else if (msg->h.dst_cmid == CM_ID_BCAST) {
            for (i=0;i<cm.num_objs;i++) {
               msg->h.dst_cmid = cm_get_sub(i, msg->p.srvid, msg->p.msgid);
               if (msg->h.dst_cmid != CM_ID_NULL) {
                  cm_route(msg);
                  break;
               }
            }
            // drop message if no subscriber
            if (i == cm.num_objs) cm_free(msg);
         }
         // in bound traffic
         else if (msg->h.dst_devid == cm.devid) {
            // place message in service/client thread queue
            cm_route(msg);
         }
      }
   }

   return 0;

} // end _cm_thread()


// ===========================================================================

// 7.26

void cm_log(pcm_msg_t msg) {

/* 7.26.1   Functional Description

   This routine will write the message to the log file.

   7.26.2   Parameters:

   NONE

   7.26.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.26.4   Data Structures

   char    *dir;
   char     line[512], cat[64];
   uint16_t i,j;
   uint16_t len = msg->h.msglen;
   char    *msgid = "-", *cmid = "-";

   uint32_t now;
   double delta;

   pcm_pipe_t  pipe = (pcm_pipe_t)msg;

// 7.26.5   Code

    if (!(gc.feature & WIN_FEATURE_LOG_TRAFFIC) || cm.log == NULL) return;

   now   = (uint32_t)tmr_get_elapsed_microseconds();
   if (cm.last_us == 0) cm.last_us = now;
   delta = (double)(now - cm.last_us);
   if (delta != 0) delta = delta / (1E6);

   // Control Message
   if (msg->h.dst_cmid != CM_ID_PIPE) {
      uint8_t *in_u8 = (uint8_t *)msg;
      // traffic origin, local timer
      if (msg->h.event == CM_EVENT_TIMER) dir = "#";
      // traffic origin, local
      else if (msg->h.dst_devid == msg->h.src_devid) dir = "*";
      // inbound
      else if (msg->h.dst_devid == cm.devid)    dir = ">";
      // outbound
      else  dir = "<";
      // cm_hdr_t is 8 bytes
      sprintf(line, "%s\t000 : ", dir);
      for (i=0;i<8;i++) {
         sprintf(cat, "%02X", in_u8[i]);
         strcat(line, cat);
      }
      // find cmid and msgid strings
      for (i=0;i<gc.msg_table_len; i++) {
         if ((gc.msg_table[i].cmid  == msg->p.srvid) &&
             (gc.msg_table[i].msgid == msg->p.msgid)) {
            msgid = gc.msg_table[i].msg_str;
            cmid  = gc.msg_table[i].cmid_str;
         }
      }
      sprintf(cat, " %6d  %10.4lf  %s:%s\n", msg->h.msglen, delta, cmid, msgid);
      strcat(line, cat);
      fwrite(line, sizeof(char), strlen(line), cm.log);
      // msg_parms_t is 4 bytes + body
      if (msg->h.msglen < CM_MAX_MSG_INT8U &&
          msg->h.msglen > sizeof(cm_hdr_t)) {
         len -= 8;
         for (i=0;i<len;i+=8) {
            sprintf(line, "\t%03X : ", i+8);
            for (j=0;j<8;j++) {
               if ((i+j) >= (len))
                  strcat(line, "  ");
               else {
                  sprintf(cat, "%02X", in_u8[i+j+8]);
                  strcat(line, cat);
               }
            }
            strcat(line, "\n");
            fwrite(line, sizeof(char), strlen(line), cm.log);
         }
         sprintf(line, "\n");
         fwrite(line, sizeof(char), strlen(line), cm.log);
      }
   }
   // Pipe Message
   else {
      uint8_t  *in_u8   = (uint8_t *)pipe;
      uint8_t *in_pipe  = (uint8_t *)&pipe->msglen;
      //  8 bytes of pipe header
      sprintf(line, "%s\t000 : ", "@");
      for (i=0;i<8;i++) {
         sprintf(cat, "%02X", in_u8[i]);
         strcat(line, cat);
      }
      // find cmid and msgid strings
      for (i=0;i<gc.msg_table_len; i++) {
         if ((gc.msg_table[i].cmid  == msg->p.srvid) &&
             (gc.msg_table[i].msgid == msg->p.msgid)) {
            msgid = gc.msg_table[i].msg_str;
            cmid  = gc.msg_table[i].cmid_str;
         }
      }
      sprintf(cat, " %6d  %10.4lf  %s:%s\n", 1024, delta, cmid, msgid);
      strcat(line, cat);
      fwrite(line, sizeof(char), strlen(line), cm.log);
      // the next 48 bytes of pipe
      len = 48;
      for (i=0;i<len;i+=8) {
         sprintf(line, "\t%03X : ", i+8);
         for (j=0;j<8;j++) {
            if ((i+j) >= (len))
               strcat(line, "  ");
            else {
               sprintf(cat, "%02X", in_pipe[i+j+8]);
               strcat(line, cat);
            }
         }
         strcat(line, "\n");
         fwrite(line, sizeof(char), strlen(line), cm.log);
      }
      sprintf(line, "\n");
      fwrite(line, sizeof(char), strlen(line), cm.log);
   }

   cm.last_us = now;

} // end cm_log()


// ===========================================================================

// 7.27

VOID CALLBACK cm_timer_callback(PVOID lpParam, BOOLEAN TimerOrWaitFired) {

/* 7.27.1   Functional Description

   This is the CM timer callback function for handling CM
   periodic events.

   NOTE: Timing intervals are not precise for this callback.

   7.2.2   Parameters:

   lpParam
   TimerOrWaitFired

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.27.4   Data Structures

// 7.27.5   Code

   cm_tick();

} // end cm_timer_callback()


// ===========================================================================

// 7.28

void cm_final(void) {

/* 7.28.1   Functional Description

   This routine will clean-up any allocated resources.

   7.28.2   Parameters:

   NONE

   7.28.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.28.4   Data Structures

// 7.28.5   Code

   // Cancel Thread
   CloseHandle(cm.q_thread);

   // Delete Timer Queue
   DeleteTimerQueue(cm.q_timer);

   // close traffic log
   if (cm.log != NULL) fclose(cm.log);

} // end cm_final()




