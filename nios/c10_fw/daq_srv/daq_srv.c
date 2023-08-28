/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Channel Board Data Acquisition Service Provider

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
        7.1  daq_init()
        7.2  daq_msg()
        7.3  daq_timer()
        7.4  daq_tick()
        7.5  daq_qmsg()

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

   static   daq_t       daq = {0};
   static   daq_sv_t    sv  = {0};
   static   daq_rxq_t   rxq = {0};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t daq_init(void) {

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
   memset(&daq, 0, sizeof(daq));
   memset(&sv,  0, sizeof(sv));
   daq.srvid   = CM_ID_DAQ_SRV;

   // Initialize the RX Queue
   memset(&rxq, 0, sizeof(daq_rxq_t));
   for (i=0;i<DAQ_RX_QUE;i++) {
      rxq.buf[i] = NULL;
   }
   rxq.head  = 0;
   rxq.tail  = 0;
   rxq.slots = DAQ_RX_QUE;

   // Register this Service
   daq.handle = cm_register(daq.srvid, daq_qmsg, daq_timer, NULL);

   // Display Server ID
   if (gc.trace & CFG_TRACE_ID) {
      xlprint("%-13s srvid:handle %02X:%02X\n", "/dev/daq", daq.srvid, daq.handle);
   }

   return result;

} // end daq_init()


// ===========================================================================

// 7.2

uint32_t daq_msg(pcm_msg_t msg) {

/* 7.2.1   Functional Description

   This function receives all the incoming CM messages, including timer
   messages.

   7.2.2   Parameters:

   msg     The incoming message to be evaluated.

   7.2.3   Return Values:

   result   DAQ_OK

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    result = DAQ_OK;

   uint16_t    cm_msg = MSG(msg->p.srvid, msg->p.msgid);

// 7.4.5   Code

   // Trace Entry
   if (gc.trace & CFG_TRACE_SERVER) {
      xlprint("daq_msg(), srvid:msgid:flags:port = %02X:%02X:%02X:%02X\n",
              msg->p.srvid, msg->p.msgid, msg->p.flags, msg->h.port);
   }

   //
   // RUN REQUEST MESSAGE
   //
   if (cm_msg == MSG(CM_ID_DAQ_SRV, DAQ_RUN_REQ)) {
      pdaq_run_msg_t req = (pdaq_run_msg_t)msg;
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pdaq_run_msg_t rsp = (pdaq_run_msg_t)slot->buf;
         rsp->p.srvid    = CM_ID_DAQ_SRV;
         rsp->p.msgid    = DAQ_RUN_RESP;
         rsp->p.flags    = req->p.flags;
         rsp->p.status   = DAQ_OK;
         rsp->b.opcode   = req->b.opcode;
         rsp->b.packets  = req->b.packets;
         // Local Parameters
         sv.opcode        = req->b.opcode;
         sv.packets       = req->b.packets;
         // RUN State
         sv.state         = DAQH_STATE_RUN;
         sv.adc_index     = 0;
         sv.blklen        = ADC_POOL_CNT * sizeof(cm_pipe_daq_t);
         // Issue the H/W Run Command
         daq_hal_run(&sv);
         cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg, sizeof(daq_run_msg_t), 0, 0);
      }
   }
   //
   // DAQ INTERRUPT INDICATION
   //
   else if (cm_msg == MSG(CM_ID_DAQ_SRV, DAQ_INT_IND)) {
      // Packet Ready Indication, Only when not using Head/Tail from hardware
      if (msg->p.flags & DAQ_INT_FLAG_PKT) {
         // next location in circular memory
         if (++sv.adc_index > (ADC_FIFO_SPAN / (ADC_POOL_CNT * sizeof(cm_pipe_daq_t)))) {
            sv.adc_index = 0;
         }
      }
      // Transfer Done Indication, from FIFO, FTDI or COM
      else if (msg->p.flags & DAQ_INT_FLAG_PIPE) {
         // Create Done Indication
         pcmq_t slot = cm_alloc();
         if (slot != NULL) {
            pdaq_done_ind_msg_t ind = (pdaq_done_ind_msg_t)slot->buf;
            ind->p.srvid    = CM_ID_DAQ_SRV;
            ind->p.msgid    = DAQ_DONE_IND;
            ind->p.flags    = DAQ_NO_FLAGS;
            ind->p.status   = DAQ_OK;
            ind->b.opcode   = sv.opcode;
            ind->b.status   = daq.status;
            ind->b.stamp    = gc.sys_time;
            // Send the Indication
            cm_send_msg(CM_MSG_IND, (pcm_msg_t)ind, NULL, sizeof(daq_done_ind_msg_t),
                        CM_ID_BCAST, gc.winid);
         }
      }
      // ADC Done Indication, Ignore when using FIFO, FTDI or COM
      else if (msg->p.flags & DAQ_INT_FLAG_DONE) {
         // Create Done Indication
         pcmq_t slot = cm_alloc();
         if (slot != NULL) {
            pdaq_done_ind_msg_t ind = (pdaq_done_ind_msg_t)slot->buf;
            ind->p.srvid    = CM_ID_DAQ_SRV;
            ind->p.msgid    = DAQ_DONE_IND;
            ind->p.flags    = DAQ_NO_FLAGS;
            ind->p.status   = DAQ_OK;
            ind->b.opcode   = sv.opcode;
            ind->b.status   = daq.status;
            ind->b.stamp    = gc.sys_time;
            // Send the Indication
            cm_send_msg(CM_MSG_IND, (pcm_msg_t)ind, NULL, sizeof(daq_done_ind_msg_t),
                        CM_ID_BCAST, gc.winid);
         }
      }
   }
   //
   // UNKNOWN MESSAGE
   //
   else {
      if (gc.trace & CFG_TRACE_ERROR) {
         // Unknown Message
         xlprint("daq_msg() Unknown Message, srvid:msgid = %02X:%02X\n",
               msg->p.srvid, msg->p.msgid);
         dump((uint8_t *)msg, 12, 0, 0);
      }
   }

   // Release the Slot
   cm_free(msg);

   return result;

} // end daq_msg()


// ===========================================================================

// 7.3

uint32_t daq_timer(pcm_msg_t msg) {

/* 7.3.1   Functional Description

   This function will handle the timer messages.

   7.3.2   Parameters:

   pMsg    The incoming timer message to be evaluated

   7.3.3   Return Values:

   result   DAQ_OK

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint32_t      result = DAQ_OK;

// 7.3.5   Code

   //
   // UNKNOWN TIMER
   //
   if (gc.trace & CFG_TRACE_ERROR) {
      // Unknown Timer Message
      xlprint("daq_msg() Unknown Timer Message, srvid:msgid = %02X:%02X\n",
            msg->p.srvid, msg->p.msgid);
      dump((uint8_t *)msg, 12, 0, 0);
   }

   return result;

} // end daq_timer()


// ===========================================================================

// 7.4

uint32_t daq_tick(void) {

/* 7.4.1   Functional Description

   This function will handle the period service tick.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   result   DAQ_OK

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint32_t    result = DAQ_OK;

// 7.4.5   Code

   // Ping Timeout Feature
   if (gc.feature & CFG_FEATURE_PING) {
      // If DAQ is Running and we disconnect then Force Abort.
      if ((gc.status & CFG_STATUS_DAQ_RUN) &&
         ((gc.status & CFG_STATUS_CONNECTED) == 0)) {
         sv.opcode = DAQ_CMD_STOP;
         daq_hal_run(&sv);
         xlprint("daq_hal_run() Abort\n");
      }
   }

   return result;

} // end daq_tick()


// ===========================================================================

// 7.5

uint32_t daq_qmsg(pcm_msg_t msg) {

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
      xlprint("daq_qmsg(), srvid:msgid:port:slot = %02X:%02X:%1X:%02X\n",
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

} // end daq_qmsg()


// ===========================================================================

// 7.6

void daq_thread(void) {

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
   if (msg != NULL) daq_msg(msg);

} // end daq_thread()
