/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Control Panel (CP) Service Provider

   1.2 Functional Description

      This code implements the Service Provider functionality.

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
        7.1  cp_init()
        7.2  cp_msg()
        7.3  cp_timer()
        7.4  cp_tick()
        7.5  cp_qmsg()
        7.6  cp_thread()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// 4.2   External Data Structures

// 4.3   External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

// 6.2  Local Data Structures

   static   cp_t     cp  = {0};
   static   cp_rxq_t rxq = {0};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t cp_init(void) {

/* 7.1.1   Functional Description

   This function will initialize the Service Provider.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t      result = CFG_ERROR_OK;
   uint32_t      i;

// 7.1.5   Code

   // Initialize the Service
   // Provider Data Structure
   memset(&cp, 0, sizeof(cp_t));
   cp.srvid   = CM_ID_CP_SRV;

   // Initialize the RX Queue
   memset(&rxq, 0, sizeof(cp_rxq_t));
   for (i=0;i<CP_RX_QUE;i++) {
      rxq.buf[i] = NULL;
   }
   rxq.head  = 0;
   rxq.tail  = 0;
   rxq.slots = CP_RX_QUE;

   // Register this Service
   cp.handle = cm_register(cp.srvid, cp_qmsg, cp_timer, NULL);

   // Display Server ID
   if (gc.trace & CFG_TRACE_ID) {
      xlprint("%-13s srvid:handle %02X:%02X\n", "/dev/cp", cp.srvid, cp.handle);
   }

   return result;

} // end cp_init()


// ===========================================================================

// 7.2

uint32_t cp_msg(pcm_msg_t msg) {

/* 7.2.1   Functional Description

   This function receives all the incoming CM messages, including timer
   messages.

   7.2.2   Parameters:

   msg      The incoming message to be evaluated.

   7.2.3   Return Values:

   result   CP_OK

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    result = CP_OK;
   uint16_t    cm_msg  = MSG(msg->p.srvid, msg->p.msgid);
   uint32_t    i,j;
   uint32_t    address;

// 7.2.5   Code

   // Trace Entry
   if (gc.trace & CFG_TRACE_SERVER) {
      xlprint("cp_msg(), srvid:msgid:port = %02X:%02X:%02X\n",
         msg->p.srvid, msg->p.msgid, msg->h.port);
   }

   //
   // VERSION REQUEST MESSAGE
   //
   if (cm_msg == MSG(CM_ID_CP_SRV, CP_VER_REQ)) {
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pcp_ver_msg_t rsp = (pcp_ver_msg_t)slot->buf;
         rsp->p.srvid    = cp.srvid;
         rsp->p.msgid    = CP_VER_RESP;
         rsp->p.flags    = CP_NO_FLAGS;
         rsp->p.status   = CP_OK;
         rsp->b.fw_ver   = (BUILD_MAJOR << 24) | (BUILD_MINOR << 16) |
                           (BUILD_NUM   <<  8) |  BUILD_INC;
         rsp->b.sysid       = stamp_sysid();
         rsp->b.stamp_epoch = stamp_epoch();
         rsp->b.stamp_date  = stamp_date();
         rsp->b.stamp_time  = stamp_time();
         rsp->b.vhdl[0]     = com_hwver();
         rsp->b.vhdl[1]     = adc_version();
         rsp->b.vhdl[2]     = (stamp_version() >> 24);
         rsp->b.trace       = gc.trace;
         rsp->b.feature     = gc.feature;
         rsp->b.debug       = gc.debug;
         // Send the Response
         cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg, sizeof(cp_ver_msg_t), 0, 0);
      }
   }
   //
   // SET/GET TRACE REQUEST
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_TRACE_REQ)) {
      pcp_trace_msg_t req = (pcp_trace_msg_t)msg;
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pcp_trace_msg_t rsp = (pcp_trace_msg_t)slot->buf;
         rsp->p.srvid  = CM_ID_CP_SRV;
         rsp->p.msgid  = CP_TRACE_RESP;
         rsp->p.flags  = req->p.flags;
         rsp->p.status = CP_OK;
         // Determine if we're Setting the Flags
         if (req->p.flags == CP_TRACE_SET) {
            gc.debug   = req->b.debug;
            gc.trace   = req->b.trace;
         }
         rsp->b.debug  = gc.debug;
         rsp->b.trace  = gc.trace;
         // Send the Response
         cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg, sizeof(cp_trace_msg_t), 0, 0);
      }
   }
   //
   // READ/WRITE MEMORY REQUEST
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_MEM_REQ)) {
      pcp_mem_msg_t req = (pcp_mem_msg_t)msg;
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pcp_mem_msg_t rsp = (pcp_mem_msg_t)slot->buf;
         rsp->p.srvid     = CM_ID_CP_SRV;
         rsp->p.msgid     = CP_MEM_RESP;
         rsp->p.flags     = req->p.flags;
         rsp->p.status    = CP_OK;
         rsp->b.address   = req->b.address;
         rsp->b.value     = req->b.value;
         rsp->b.type      = req->b.type;
         address          = (uint32_t)req->b.address;
         // create the type pointers
         uint8_t  *addr8  = (uint8_t *)(address);
         uint16_t *addr16 = (uint16_t *)(address);
         uint32_t *addr32 = (uint32_t *)(address);
         // determine type
         switch(req->b.type) {
            case CFG_INT8U:
            case CFG_INT8S:
               if (req->p.flags & CP_MEM_WR) *addr8 = req->b.value;
               if (req->p.flags & CP_MEM_RD) rsp->b.value = *addr8;
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               if (req->p.flags & CP_MEM_WR) *addr16 = req->b.value;
               if (req->p.flags & CP_MEM_RD) rsp->b.value = *addr16;
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               if (req->p.flags & CP_MEM_WR) *addr32 = req->b.value;
               if (req->p.flags & CP_MEM_RD) rsp->b.value = *addr32;
               break;
            default:
               if (req->p.flags & CP_MEM_WR) *addr32 = req->b.value;
               if (req->p.flags & CP_MEM_RD) rsp->b.value = *addr32;
               break;
         }
         // Send the Response
         cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg, sizeof(cp_mem_msg_t), 0, 0);
      }
   }
   //
   // READ/WRITE BLOCK REQUEST
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_BLOCK_REQ)) {
      pcp_block_msg_t req = (pcp_block_msg_t)msg;
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pcp_block_msg_t rsp = (pcp_block_msg_t)slot->buf;
         rsp->p.srvid     = CM_ID_CP_SRV;
         rsp->p.msgid     = CP_BLOCK_RESP;
         rsp->p.flags     = req->p.flags;
         rsp->p.status    = CP_OK;
         rsp->b.index     = req->b.index;
         rsp->b.type      = req->b.type;
         rsp->b.address   = req->b.address;
         rsp->b.length    = req->b.length;
         address          = (uint32_t)req->b.address;
         // number of memory type to read/write
         j = rsp->b.length;
         // create the type pointers
         uint8_t  *addr8  = (uint8_t *)(address);
         uint16_t *addr16 = (uint16_t *)(address);
         uint32_t *addr32 = (uint32_t *)(address);
         uint8_t  *rsp8  = (uint8_t *)(rsp->b.data);
         uint16_t *rsp16 = (uint16_t *)(rsp->b.data);
         uint32_t *rsp32 = (uint32_t *)(rsp->b.data);
         uint8_t  *req8  = (uint8_t *)(req->b.data);
         uint16_t *req16 = (uint16_t *)(req->b.data);
         uint32_t *req32 = (uint32_t *)(req->b.data);
         // determine type
         switch(req->b.type) {
            case CFG_INT8U:
            case CFG_INT8S:
               if (req->p.flags & CP_MEM_WR) {
                  for (i=0;i<j;i++) {
                     *(addr8 + i) = *(req8 + i);
                  }
               }
               if (req->p.flags & CP_MEM_RD) {
                  for (i=0;i<j;i++) {
                     *(rsp8 + i) = *(addr8 + i);
                  }
               }
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               if (req->p.flags & CP_MEM_WR) {
                  for (i=0;i<j;i++) {
                     *(addr16 + i) = *(req16 + i);
                  }
               }
               if (req->p.flags & CP_MEM_RD) {
                  for (i=0;i<j;i++) {
                     *(rsp16 + i) = *(addr16 + i);
                  }
               }
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               if (req->p.flags & CP_MEM_WR) {
                  for (i=0;i<j;i++) {
                     *(addr32 + i) = *(req32 + i);
                  }
               }
               if (req->p.flags & CP_MEM_RD) {
                  for (i=0;i<j;i++) {
                     *(rsp32 + i) = *(addr32 + i);
                  }
               }
               break;
            default:
               if (req->p.flags & CP_MEM_WR) {
                  for (i=0;i<j;i++) {
                     *(addr32 + i) = *(req32 + i);
                  }
               }
               if (req->p.flags & CP_MEM_RD) {
                  for (i=0;i<j;i++) {
                     *(rsp32 + i) = *(addr32 + i);
                  }
               }
               break;
         }
         // send response, only include data if reading
         if (req->p.flags & CP_MEM_WR)
            cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg, sizeof(cp_block_msg_t) - CP_BLOCK_MAX, 0, 0);
         else
            cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg, sizeof(cp_block_msg_t), 0, 0);
      }
   }
   //
   // PING REQUEST
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_PING_REQ)) {
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pcp_ping_msg_t rsp = (pcp_ping_msg_t)slot->buf;
         rsp->p.srvid  = CM_ID_CP_SRV;
         rsp->p.msgid  = CP_PING_RESP;
         rsp->p.flags  = msg->p.flags;
         rsp->p.status = CP_OK;
         // reset ping timeout
         gc.ping_time = alt_timestamp();
         gc.ping_cnt  = 0;
         // Only respond if
         if (gc.status & CFG_STATUS_CONNECTED)
            cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg, sizeof(cp_ping_msg_t), 0, 0);
      }
   }
   //
   // RESET HARDWARE/SOFTWARE REQUEST, NO RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_RESET_REQ)) {
      // NOT USED
   }
   //
   // CP INTERRUPT INDICATION
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_INT_IND)) {
      // NOT USED
   }
   //
   // UNKNOWN MESSAGE
   //
   else {
      if (gc.trace & CFG_TRACE_ERROR) {
         // Unknown Message
         xlprint("cp_msg() Unknown Message, srvid:msgid = %02X:%02X\n",
               msg->p.srvid, msg->p.msgid);
         dump((uint8_t*)msg, 12, 0, 0);
      }
   }

   // Release the Slot
   cm_free(msg);

   return result;

} // end cp_msg()


// ===========================================================================

// 7.3

uint32_t cp_timer(pcm_msg_t msg) {

/* 7.3.1   Functional Description

   This function will handle the timer messages.

   7.3.2   Parameters:

   msg     The incoming timer message to be evaluated

   7.3.3   Return Values:

   result   CP_OK

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint32_t      result = CP_OK;

// 7.3.5   Code

   //
   // UNKNOWN TIMER
   //
   if (gc.trace & CFG_TRACE_ERROR) {
      // Unknown Timer Message
      xlprint("cp_msg() Unknown Timer Message, srvid:msgid = %02X:%02X\n",
            msg->p.srvid, msg->p.msgid);
      dump((uint8_t*)msg, 12, 0, 0);
   }

   return result;

} // end cp_timer()


// ===========================================================================

// 7.4

uint32_t cp_tick(void) {

/* 7.4.1   Functional Description

   This function will handle the period service tick.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   result   CP_OK

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint32_t    result = CP_OK;
   uint32_t    now = alt_timestamp();

// 7.4.5   Code

   // Ping Timeout Feature
   if (gc.feature & CFG_FEATURE_PING && gc.status & CFG_STATUS_CONNECTED) {
      gc.ping_cnt++;
      if (gc.ping_cnt >= CFG_PING_CYCLE) {
         gc.ping_cnt = 0;
         // Check for Ping Timeout and negate CFG_STATUS_CONNECTED
         if (((now - gc.ping_time) / SECONDS) > CFG_PING_TIMEOUT) {
            gc.status &= ~CFG_STATUS_CONNECTED;
            if (gc.trace & CFG_TRACE_ERROR) {
               xlprint("Ping Timeout, Forced Disconnect\n");
            }
         }
      }
   }

   return result;

} // end cp_tick()


// ===========================================================================

// 7.5

uint32_t cp_qmsg(pcm_msg_t msg) {

/* 7.5.1   Functional Description

   This routine will place the incoming message on the receive queue.

   7.5.2   Parameters:

   msg     CM Message to queue

   7.5.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

   uint32_t    result = CP_OK;

// 7.5.5   Code

   // Trace Entry
   if (gc.trace & CFG_TRACE_CM) {
      xlprint("cp_qmsg(), srvid:msgid:port:slot = %02X:%02X:%1X:%02X\n",
            msg->p.srvid, msg->p.msgid, msg->h.port, msg->h.slot);
   }

   // place in transmit queue
   if (msg != NULL) {
      rxq.buf[rxq.head] = (uint32_t *)msg;
      if (++rxq.head == rxq.slots) rxq.head = 0;
   }
   // silently drop
   else {
      if (++rxq.head == rxq.slots) rxq.head = 0;
   }

   return result;

} // end cp_qmsg()


// ===========================================================================

// 7.6

void cp_thread(void) {

/* 7.6.1   Functional Description

   This thread will provide a delivery service for CM messages from the queue,
   it is called from background during idle time.

   7.6.2   Parameters:

   NONE

   7.6.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.6.4   Data Structures

   pcm_msg_t   msg;

// 7.6.5   Code

   // clear previous message
   msg = NULL;

   if (rxq.head != rxq.tail) {
      // Validate message
      if (rxq.buf[rxq.tail] != NULL) {
         msg = (pcm_msg_t)rxq.buf[rxq.tail];
         if (++rxq.tail == rxq.slots) rxq.tail = 0;
      }
      // silently drop
      else {
         if (++rxq.tail == rxq.slots) rxq.tail = 0;
      }
   }

   // Deliver Message using this thread
   if (msg != NULL) {
      cp_msg(msg);
   }

} // end cp_thread()

