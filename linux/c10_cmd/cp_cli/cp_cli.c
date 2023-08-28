/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Control Panel (CP) Client

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
        7.7  cp_final


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

   static   void *cp_thread(void *data);

// 6.2  Local Data Structures

   static   cp_t     cp  = {0};
   static   cp_rxq_t rxq = {{0}};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t cp_init(void) {

/* 7.1.1   Functional Description

   This function will initialize the Service Provider.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   result   LIN_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t      result = LIN_ERROR_OK;
   uint32_t      i;

// 7.1.5   Code

   // Initialize the Service
   // Provider Data Structure
   memset(&cp, 0, sizeof(cp_t));
   cp.srvid   = CM_ID_CP_CLI;

   // Initialize the RX Queue
   memset(&rxq, 0, sizeof(cp_rxq_t));
   for (i=0;i<CP_RX_QUE;i++) {
      rxq.buf[i] = NULL;
   }
   pthread_mutex_init(&rxq.mutex, NULL);
   pthread_cond_init(&rxq.cv, NULL);
   rxq.head  = 0;
   rxq.tail  = 0;
   rxq.slots = CP_RX_QUE;

   // Start the CP Thread
   if (pthread_create(&cp.tid, NULL, cp_thread, NULL)) {
      result = LIN_ERROR_CP;
   }

   // Register this Client
   cp.handle = cm_register(cp.srvid, cp_qmsg, cp_timer, NULL);

   // Display Server ID
   if (gc.trace & LIN_TRACE_ID) {
      printf("%-13s srvid:handle %02X:%02X\n", "/dev/cp", cp.srvid, cp.handle);
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

// 7.2.5   Code

   // Trace Entry
   if (gc.trace & LIN_TRACE_SERVER) {
      printf("cp_msg(), srvid:msgid:port = %02X:%02X:%02X\n",
         msg->p.srvid, msg->p.msgid, msg->h.port);
   }

   //
   //    VERSION RESPONSE
   //
   if (cm_msg == MSG(CM_ID_CP_SRV, CP_VER_RESP)) {
      pcp_ver_msg_t rsp = (pcp_ver_msg_t)msg;
      if (gc.trace & LIN_TRACE_ID) {
         printf("\n");
         printf("f/w ver     : %08X\n", rsp->b.fw_ver);
         printf("sysid       : %08X\n", rsp->b.sysid);
         printf("stamp_epoch : %08X\n", rsp->b.stamp_epoch);
         printf("stamp_date  : %08X\n", rsp->b.stamp_date);
         printf("stamp_time  : %08X\n", rsp->b.stamp_time);
         printf("fifo        : %02X\n", rsp->b.vhdl[0]);
         printf("adc         : %02X\n", rsp->b.vhdl[1]);
         printf("fpga        : %02X\n\n", rsp->b.vhdl[2]);
      }
      // send OPC run request
      cm_send_req(CM_ID_OPC_SRV, OPC_RUN_REQ, CM_ID_CP_CLI, OPC_RUN_START);
   }
   //
   //    PING RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_PING_RESP)) {
      // restart ping request timer
      cm_timer_set(CM_TMR_ID0, CP_TMR_PING, 15000, CM_ID_CP_CLI, CM_ID_CP_CLI);
      // restart ping timeout timer
      cm_timer_set(CM_TMR_ID1, CP_TMR_PING_TIMEOUT, 60000, CM_ID_CP_CLI, CM_ID_CP_CLI);
   }
   //
   //    OPC RUN RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_OPC_SRV, OPC_RUN_RESP)) {
      // drop response
   }
   //
   //    UNKNOWN MESSAGE
   //
   else if (gc.trace & LIN_TRACE_ERROR) {
      // Unknown Message
      printf("cp_msg() Warning : Unknown Message, srvid:msgid = %02X:%02X\n",
            msg->p.srvid, msg->p.msgid);
      dump((uint8_t*)msg, 12, 0, 0);
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

   uint32_t    result = CP_OK;
   uint16_t    cm_msg  = MSG(msg->p.srvid, msg->p.msgid);

// 7.3.5   Code

   // Trace Entry
   if (gc.trace & LIN_TRACE_TIMER) {
      printf("cp_msg(), srvid:msgid:port = %02X:%02X:%02X\n",
         msg->p.srvid, msg->p.msgid, msg->h.port);
   }

   //
   //    PING TIMER
   //
   if (cm_msg == MSG(CM_ID_CP_CLI, CP_TMR_PING)) {
      cm_send_req(CM_ID_CP_SRV, CP_PING_REQ, CM_ID_CP_CLI, 0);
   }
   //
   //    PING TIMEOUT TIMER
   //
   else if (cm_msg == MSG(CM_ID_CP_CLI, CP_TMR_PING_TIMEOUT)) {
      printf("cp_timer() Warning : Ping Timeout, Forced Application Exit\n");
      // halt the application
      gc.error |= LIN_ERROR_PING_TIMEOUT;
      gc.halt   = TRUE;
   }
   //
   //    UNKNOWN TIMER
   //
   else if (gc.trace & LIN_TRACE_ERROR) {
      // Unknown Timer Message
      printf("cp_timer() Warning : Unknown Timer Message, srvid:msgid = %02X:%02X\n",
            msg->p.srvid, msg->p.msgid);
      dump((uint8_t*)msg, 12, 0, 0);
   }

   // Release the Slot
   cm_free(msg);

   return result;

} // end cp_timer()


// ===========================================================================

// 7.4

uint32_t cp_tick(void) {

/* 7.4.1   Functional Description

   This function will handle the period service tick. This function runs
   in the WIN32 timer thread, no significant processing should be done here.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   result   CP_OK

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint32_t    result = CP_OK;

// 7.4.5   Code

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

   // validate message
   if (msg != NULL) {

      // Trace Entry
      if (gc.trace & LIN_TRACE_CM) {
         printf("cp_qmsg(), srvid:msgid:port:slot = %02X:%02X:%1X:%02X\n",
               msg->p.srvid, msg->p.msgid, msg->h.port, msg->h.slot);
      }

      // Lock the RXQ mutex
      pthread_mutex_lock(&rxq.mutex);

      // place in receive queue
      rxq.buf[rxq.head] = (uint32_t *)msg;
      if (++rxq.head == rxq.slots) rxq.head = 0;

      // Unlock the RXQ mutex
      pthread_mutex_unlock(&rxq.mutex);

      // signal the CP thread
      pthread_cond_signal(&rxq.cv);

   }
   else {
      if (gc.trace & LIN_TRACE_ERROR) {
         printf("cp_qmsg() Null Pointer\n");
      }
   }

   return result;

} // end cp_qmsg()


// ===========================================================================

// 7.6

static void *cp_thread(void *data) {

/* 7.6.1   Functional Description

   This thread will provide a delivery service for CM messages from the queue.

   7.6.2   Parameters:

   data     Thread parameters

   7.6.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.6.4   Data Structures

   pcm_msg_t   msg;

   struct timespec ts;

// 7.6.5   Code


   if (gc.trace & LIN_TRACE_ID) {
      printf("cp_thread() started, data:tid %08X:%lu\n", (uint32_t)data, syscall(SYS_gettid));
   }

   // Message Receive Loop
   while (1) {

      // Lock the RXQ mutex
      pthread_mutex_lock(&rxq.mutex);

      // Wait on condition variable,
      // this unlocks the mutex while waiting
      while (rxq.head == rxq.tail) {
         clock_gettime(CLOCK_REALTIME, &ts);
         ts.tv_sec += 1;
         pthread_cond_timedwait(&rxq.cv, &rxq.mutex, &ts);
         // Prevent arbitrary cancellation point
         pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
         pthread_testcancel();
         pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
      }

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

      // Unlock the RXQ mutex
      pthread_mutex_unlock(&rxq.mutex);

      // Deliver Message using this thread
      if (msg != NULL) {
         cp_msg(msg);
      }

   }

   return 0;

} // end cp_thread()


// ===========================================================================

// 7.7

void cp_final(void) {

/* 7.7.1   Functional Description

   This routine will clean-up any allocated resources.

   7.7.2   Parameters:

   NONE

   7.7.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.7.4   Data Structures

// 7.7.5   Code

   // Cancel CP Thread
   pthread_cancel(cp.tid);
   pthread_join(cp.tid, NULL);

} // end cp_final()




