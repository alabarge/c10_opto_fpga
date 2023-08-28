/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

     Operation Code Service Provider

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
        7.1  opc_init()
        7.2  opc_msg()
        7.3  opc_timer()
        7.4  opc_tick()
        7.5  opc_qmsg()
        7.6  opc_thread()
        7.7  opc_daq_state()
        7.8  opc_write_file()
        7.9  opc_final()

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

   static   void *opc_thread(void *data);

// 6.2  Local Data Structures

   static   opc_t          opc = {0};

   // operation state machines
   static   opc_daq_sv_t   opc_daq = {0};

   // available operations
   static   opc_table_t    opc_table[] = {
               {OPC_CMD_DAQ,     opc_daq_state, OPC_DAQ_STATE_INIT}
   };

   static   opc_rxq_t      rxq = {{0}};

   // cm subscriptions
   static cm_sub_t subs[] = {
      {CM_ID_DAQ_SRV, DAQ_DONE_IND, CM_ID_DAQ_SRV},
      {CM_ID_NULL, 0, 0}
   };

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t opc_init(void) {

/* 7.1.1   Functional Description

   This function will initialize the Service Provider.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t      result = LIN_ERROR_OK;
   uint32_t      i;

// 7.1.5   Code

   // Initialize the Service
   // Provider Data Structure
   memset(&opc, 0, sizeof(opc));
   memset(&opc_daq,  0, sizeof(opc_daq));
   opc.srvid   = CM_ID_OPC_SRV;

   // Initialize the RX Queue
   memset(&rxq, 0, sizeof(opc_rxq_t));
   for (i=0;i<OPC_RX_QUE;i++) {
      rxq.buf[i] = NULL;
   }
   pthread_mutex_init(&rxq.mutex, NULL);
   pthread_cond_init(&rxq.cv, NULL);
   rxq.head  = 0;
   rxq.tail  = 0;
   rxq.slots = OPC_RX_QUE;

   // Allocate space for ADC Samples
   opc_daq.adc = (int32_t *)malloc(DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM * DAQ_MAX_CH * sizeof(int32_t));

   // Start the Message Delivery Thread
   if (pthread_create(&opc.tid, NULL, opc_thread, NULL)) {
      result = CFG_ERROR_OPC;
   }

   // Register this Service
   opc.handle = cm_register(opc.srvid, opc_qmsg, opc_timer, subs);

   // Display Server ID
   if (gc.trace & CFG_TRACE_ID) {
      printf("%-13s srvid:handle %02X:%02X\n", "/dev/opc", opc.srvid, opc.handle);
   }

   return result;

} // end opc_init()


// ===========================================================================

// 7.2

uint32_t opc_msg(pcm_msg_t msg) {

/* 7.2.1   Functional Description

   This function receives all the incoming CM messages, including timer
   messages.

   7.2.2   Parameters:

   msg     The incoming message to be evaluated.

   7.2.3   Return Values:

   result   OPC_OK

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    result = OPC_OK;
   uint32_t    i;
   uint16_t    cm_msg = MSG(msg->p.srvid, msg->p.msgid);

   pcm_pipe_daq_t pipe = (pcm_pipe_daq_t)msg;

// 7.4.5   Code

   //
   // CONTROL MESSAGE
   //
   if (msg->h.dst_cmid != CM_ID_PIPE) {
      // Trace Entry
      if (gc.trace & CFG_TRACE_SERVER) {
         printf("opc_msg(), srvid:msgid:flags:port = %02X:%02X:%02X:%02X\n",
                 msg->p.srvid, msg->p.msgid, msg->p.flags, msg->h.port);
      }
      //
      //    RUN REQUEST
      //
      if (cm_msg == MSG(CM_ID_OPC_SRV, OPC_RUN_REQ)) {
         // start application timeout timer, opcode dependant
         cm_timer_set(CM_TMR_ID2, OPC_TMR_APP_TIMEOUT, cc.opc_timeout,
               CM_ID_OPC_SRV, CM_ID_OPC_SRV);
         pcmq_t slot = cm_alloc();
         if (slot != NULL) {
            popc_run_msg_t rsp = (popc_run_msg_t)slot->buf;
            rsp->p.srvid    = opc.srvid;
            rsp->p.msgid    = OPC_RUN_RESP;
            rsp->p.flags    = OPC_NO_FLAGS;
            rsp->p.status   = OPC_OK;
            // start the OPC state machine
            if (msg->p.flags & OPC_RUN_START) {
               // set associated state machine for operation
               for (i=0;i<DIM(opc_table);i++) {
                  if (cc.opc_opcode == opc_table[i].opcode) {
                     opc.sv.step  = opc_table[i].step;
                     opc.sv.state = opc_table[i].state;
                     // first step of state machine
                     opc.sv.step();
                     break;
                  }
               }
               // exit if no valid operation code is defined
               if (i == DIM(opc_table)) {
                  printf("opc_msg() Fatal Error : Invalid Operation Code %d\n", cc.opc_opcode);
                  gc.error |= LIN_ERROR_OP_CODE;
                  gc.halt   = TRUE;
               }
            }
            // halt the OPC operation and shutdown the application
            else if (msg->p.flags & OPC_RUN_HALT) {
               // un-register for DAQ pipe messages
               cm_pipe_reg(CM_ID_OPC_SRV, 0, 0, CM_DEV_NULL);
               gc.error |= LIN_ERROR_HALT;
               gc.halt   = TRUE;
            }
            // Send the Response
            cm_send_msg(CM_MSG_RESP, (pcm_msg_t)rsp, msg, sizeof(opc_run_msg_t), 0, 0);
         }
      }
      //
      //    DAQ RUN RESPONSE
      //
      else if (cm_msg == MSG(CM_ID_DAQ_SRV, DAQ_RUN_RESP)) {
         pdaq_run_msg_t rsp = (pdaq_run_msg_t)msg;
         // issue step for state machine when stopping
         if (rsp->b.opcode & DAQ_CMD_STOP) {
            opc_daq.acq_done = TRUE;
            // issue step indication
            cm_local(CM_ID_OPC_SRV, OPC_STEP_IND, OPC_STEP_DONE, OPC_OK);
         }
      }
      //
      //    DAQ DONE INDICATION
      //
      else if (cm_msg == MSG(CM_ID_DAQ_SRV, DAQ_DONE_IND)) {
         // nop
      }
      //
      //    OPC STEP INDICATION
      //
      else if (cm_msg == MSG(CM_ID_OPC_SRV, OPC_STEP_IND)) {
         if (opc.sv.step != NULL) opc.sv.step();
      }
      //
      // UNKNOWN MESSAGE
      //
      else if (gc.trace & CFG_TRACE_ERROR) {
         // Unknown Message
         printf("opc_msg() Unknown Message, srvid:msgid = %02X:%02X\n",
               msg->p.srvid, msg->p.msgid);
         dump((uint8_t *)msg, 12, 0, 0);
      }

      // Release the Slot
      cm_free(msg);
   }

   //
   // DAQ PIPE MESSAGE
   //
   else if (pipe->dst_cmid == CM_ID_PIPE && pipe->msgid == CM_PIPE_DAQ_DATA) {
      // record pipe message
      opc_daq.pipe = pipe;
      // issue step indication
      cm_local(CM_ID_OPC_SRV, OPC_STEP_IND, OPC_STEP_PIPE, OPC_OK);
   }

   return result;

} // end opc_msg()


// ===========================================================================

// 7.3

uint32_t opc_timer(pcm_msg_t msg) {

/* 7.3.1   Functional Description

   This function will handle the timer messages.

   7.3.2   Parameters:

   pMsg    The incoming timer message to be evaluated

   7.3.3   Return Values:

   result   opc_OK

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint32_t    result = OPC_OK;
   uint16_t    cm_msg = MSG(msg->p.srvid, msg->p.msgid);

// 7.3.5   Code

   //
   //    APPLICATION TIMEOUT TIMER
   //
   if (cm_msg == MSG(CM_ID_OPC_SRV, OPC_TMR_APP_TIMEOUT)) {
      printf("opc_timer() Warning : Application Timeout, Forced Exit\n");
      // halt the application
      gc.error |= LIN_ERROR_APP_TIMEOUT;
      gc.halt   = TRUE;
   }
   //
   //    UNKNOWN TIMER
   //
   else if (gc.trace & CFG_TRACE_ERROR) {
      // Unknown Timer Message
      printf("opc_msg() Unknown Timer Message, srvid:msgid = %02X:%02X\n",
            msg->p.srvid, msg->p.msgid);
      dump((uint8_t *)msg, 12, 0, 0);
   }

   return result;

} // end opc_timer()


// ===========================================================================

// 7.4

uint32_t opc_tick(void) {

/* 7.4.1   Functional Description

   This function will handle the periodic service tick. This function runs
   in the WIN32 timer thread, no significant processing should be done here.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   result   OPC_OK

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint32_t    result = OPC_OK;

// 7.4.5   Code

   return result;

} // end opc_tick()


// ===========================================================================

// 7.5

uint32_t opc_qmsg(pcm_msg_t msg) {

/* 7.5.1   Functional Description

   This routine will place the incoming message on the receive queue.

   7.5.2   Parameters:

   msg     CM Message to queue

   7.5.3   Return Values:

   result   CM_OK

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

   uint32_t    result = CM_OK;

// 7.5.5   Code

   // validate message
   if (msg != NULL) {

      // Trace Entry
      if (gc.trace & LIN_TRACE_CM) {
         printf("opc_qmsg(), srvid:msgid:port:slot = %02X:%02X:%1X:%02X\n",
               msg->p.srvid, msg->p.msgid, msg->h.port, msg->h.slot);
      }

      // Lock the RXQ mutex
      pthread_mutex_lock(&rxq.mutex);

      // place in receive queue
      rxq.buf[rxq.head] = (uint32_t *)msg;
      if (++rxq.head == rxq.slots) rxq.head = 0;

      // Unlock the RXQ mutex
      pthread_mutex_unlock(&rxq.mutex);

      // signal the OPC thread
      pthread_cond_signal(&rxq.cv);

   }
   else {
      if (gc.trace & CFG_TRACE_ERROR) {
         printf("opc_qmsg() Null Pointer\n");
      }
   }

   return result;

} // end opc_qmsg()


// ===========================================================================

// 7.6

static void *opc_thread(void *data) {

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


   if (gc.trace & CFG_TRACE_ID) {
      printf("opc_thread() started, data:tid %08X:%lu\n", (uint32_t)data, syscall(SYS_gettid));
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
         opc_msg(msg);
      }

   }

   return 0;

} // end opc_thread()


// ===========================================================================

// 7.7

uint32_t opc_daq_state(void) {

/* 7.7.1   Functional Description

   This function will handle stepping the DAQ state machine for opcode
   OPC_CMD_DAQ.

   7.7.2   Parameters:

   NONE

   7.7.3   Return Values:

   result   OPC_OK

-----------------------------------------------------------------------------
*/

// 7.7.4   Data Structures

   uint32_t    result = OPC_OK;
   struct tm*  c_tm;
   time_t      time_now;
   cm_send_t   ps = {0};
   char        file[512] = {0};
   char        line[1024];
   char        build_time[64], build_date[64];

   pcm_pipe_daq_t pipe;

// 7.7.5   Code

   if (opc.sv.state != OPC_STATE_IDLE) {
      // switch on current state
      switch(opc.sv.state) {
         //
         // INIT DAQ STATE VECTOR
         //
         case OPC_DAQ_STATE_INIT :
            //
            // init state vector with CC parameters
            //
            opc.sv.state       = OPC_DAQ_STATE_RUN;
            opc_daq.seqid      = 0;
            opc_daq.samcnt     = 0;
            opc_daq.opcmd      = cc.daq_opcmd;
            opc_daq.packets    = cc.daq_packets;
            opc_daq.adc_index  = 0;
            opc_daq.blklen     = 0;
            opc_daq.to_file    = cc.daq_to_file;
            opc_daq.file_type  = cc.daq_file_type;
            opc_daq.file_stamp = cc.daq_file_stamp;
            opc_daq.ramp       = cc.daq_ramp;
            opc_daq.real       = cc.daq_real;
            opc_daq.acq_done   = FALSE;
            opc_daq.dat_done   = FALSE;
            opc_daq.pkt_cnt    = 0;
            opc_daq.file       = NULL;
            opc_daq.pipe       = NULL;
            // reset circular pipe buffer
            fifo_head();
            // register for DAQ pipe messages
            cm_pipe_reg(CM_ID_OPC_SRV, CM_PIPE_DAQ_DATA, 1, CM_DEV_WIN);
            //
            // file timestamp : YYYYMMDDHHMMSS_filename
            //
            time(&time_now);
            c_tm = localtime(&time_now);
            sprintf(build_date, "%04d%02d%02d", (c_tm->tm_year+1900), c_tm->tm_mon + 1, c_tm->tm_mday);
            sprintf(build_time,"%02d%02d%02d", c_tm->tm_hour, c_tm->tm_min, c_tm->tm_sec);
            if (opc_daq.file_stamp == 1) {
               sprintf(file, "%s_%s_%s", build_date, build_time, cc.daq_file);
            }
            else {
               sprintf(file, "%s", cc.daq_file);
            }
            //
            // open file for writing samples
            //
            if (opc_daq.to_file == 1) {
               if (opc_daq.file_type == 0) {
                  // plain text with labels
                  opc_daq.file = fopen(file, "wt");
                  // labels
                  if (opc_daq.file != NULL) {
                     sprintf(line, "%02d.%s.%02d %02d:%02d:%02d\n\n",
                        c_tm->tm_mday, gc.month[c_tm->tm_mon], (c_tm->tm_year+1900)-2000,
                        c_tm->tm_hour, c_tm->tm_min, c_tm->tm_sec );
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.opcmd        = 0x%08X\n", opc_daq.opcmd);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.to_file      = %d\n", opc_daq.to_file);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.file_type    = %d\n", opc_daq.file_type);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.file_stamp   = %d\n", opc_daq.file_stamp);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.ramp         = %d\n", opc_daq.ramp);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.real         = %d\n", opc_daq.real);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.packets      = %d\n", opc_daq.packets);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.file         = %s\n\n", file);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "time,ch1,ch2,ch3,ch4,ch5,ch6,ch7,ch8\n");
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                  }
               }
               else if (opc_daq.file_type == 1) {
                  // binary
                  opc_daq.file = fopen(file, "wb");
               }
               else if (opc_daq.file_type == 2) {
                  // csv text with labels
                  opc_daq.file = fopen(file, "wt");
                  // labels
                  if (opc_daq.file != NULL) {
                     sprintf(line, "%02d.%s.%02d %02d:%02d:%02d\n\n",
                        c_tm->tm_mday, gc.month[c_tm->tm_mon], (c_tm->tm_year+1900)-2000,
                        c_tm->tm_hour, c_tm->tm_min, c_tm->tm_sec );
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.opcmd        = 0x%08X\n", opc_daq.opcmd);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.to_file      = %d\n", opc_daq.to_file);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.file_type    = %d\n", opc_daq.file_type);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.file_stamp   = %d\n", opc_daq.file_stamp);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.ramp         = %d\n", opc_daq.ramp);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.real         = %d\n", opc_daq.real);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.packets      = %d\n", opc_daq.packets);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "opc_daq.file         = %s\n\n", file);
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                     sprintf(line, "time,ch1,ch2,ch3,ch4,ch5,ch6,ch7,ch8\n");
                     fwrite(line, sizeof(char), strlen(line), opc_daq.file);
                  }
               }
               // close application if file doesn't open
               if (opc_daq.file == NULL) {
                  printf("opc_daq_state() Fatal Error : ADC sample file did not Open, %s", file);
                  opc.sv.state = OPC_STATE_IDLE;
                  gc.error    |= LIN_ERROR_FILE;
                  gc.halt      = TRUE;
               }
            }
            //
            // okay to go
            //
            if (opc.sv.state == OPC_DAQ_STATE_RUN) {
               // issue DAQ run request using CC parameters
               pcmq_t slot = cm_alloc();
               if (slot != NULL) {
                  pdaq_run_msg_t msg = (pdaq_run_msg_t)slot->buf;
                  msg->p.srvid   = CM_ID_DAQ_SRV;
                  msg->p.msgid   = DAQ_RUN_REQ;
                  msg->p.flags   = DAQ_NO_FLAGS;
                  msg->p.status  = DAQ_OK;
                  msg->b.opcode  = opc_daq.opcmd;
                  msg->b.packets = opc_daq.packets;
                  ps.msg         = (pcm_msg_t)msg;
                  ps.dst_cmid    = CM_ID_DAQ_SRV;
                  ps.src_cmid    = CM_ID_OPC_SRV;
                  ps.msglen      = sizeof(daq_run_msg_t);
                  // Send the Request
                  result = cm_send(CM_MSG_REQ, &ps);
               }
            }
            break;
         //
         // PROCESS ACQUIRED SAMPLES
         //
         case OPC_DAQ_STATE_RUN :
            if ((pipe = opc_daq.pipe) != NULL) {
               // write to file
               if (opc_daq.to_file) opc_write_file(pipe);
               // track packets
               opc_daq.pkt_cnt += DAQ_MAX_PIPE_RUN;
               opc_daq.samcnt  += (DAQ_MAX_LEN * FIFO_PACKET_CNT);
               // Clear pipe message
               opc_daq.pipe = NULL;
               // All samples collected
               if (opc_daq.pkt_cnt == opc_daq.packets) {
                  opc_daq.dat_done = TRUE;
                  opc.sv.state  = OPC_DAQ_STATE_DONE;
                  // issue run request DAQ_CMD_STOP
                  pcmq_t slot = cm_alloc();
                  if (slot != NULL) {
                     pdaq_run_msg_t msg = (pdaq_run_msg_t)slot->buf;
                     msg->p.srvid   = CM_ID_DAQ_SRV;
                     msg->p.msgid   = DAQ_RUN_REQ;
                     msg->p.flags   = DAQ_NO_FLAGS;
                     msg->p.status  = DAQ_OK;
                     msg->b.opcode  = DAQ_CMD_STOP;
                     ps.msg         = (pcm_msg_t)msg;
                     ps.dst_cmid    = CM_ID_DAQ_SRV;
                     ps.src_cmid    = CM_ID_OPC_SRV;
                     ps.msglen      = sizeof(daq_run_msg_t);
                     // Send the Request
                     result = cm_send(CM_MSG_REQ, &ps);
                  }
               }
            }
            break;
         //
         // WAIT FOR DAQ COMPLETE
         //
         case OPC_DAQ_STATE_DONE :
            if (opc_daq.acq_done == TRUE && opc_daq.dat_done == TRUE) {
               opc.sv.state = OPC_STATE_IDLE;
               cm_send_reg_req(CM_DEV_DE0, CM_PORT_COM0, CM_REG_CLOSE, (uint8_t *)gc.dev_str);
               usleep(100*1000);
               gc.halt = TRUE;
            }
            break;
      }
   }

   return result;

} // end opc_daq_state()


// ===========================================================================

// 7.8

uint32_t opc_write_file(pcm_pipe_daq_t pipe) {

/* 7.8.1   Functional Description

   This routine will write the pipe message to the selected data file.

   7.8.2   Parameters:

   NONE

   7.8.3   Return Values:

   result   OPC_OK

-----------------------------------------------------------------------------
*/

// 7.8.4   Data Structures

   uint32_t    result = OPC_OK;
   uint32_t    i,j,l,m;
   int32_t     k;
   char        line[1024];
   float       samf[16];
   int32_t     sami[16];

// 7.8.5   Code

   //
   // Write to Binary File
   //
   if (opc_daq.file_type == 1 && opc_daq.file != NULL) {
      fwrite(opc_daq.pipe, 1, DAQ_MAX_PIPE_RUN * sizeof(cm_pipe_daq_t), opc_daq.file);
   }
   //
   // Write to Text File
   //
   else if (opc_daq.file_type == 0 && opc_daq.file != NULL) {
      // cycle over multiple 1K pipe messages
      for (i=0;i<DAQ_MAX_PIPE_RUN;i++) {
         // cycle over samples, packed in channel order
         for (k=0,m=0;k<DAQ_MAX_SAM;k++) {
            for (l=0;l<DAQ_MAX_CH;l++) {
               opc_daq.adc[(i*DAQ_MAX_LEN) + m++] = ((int32_t)pipe->samples[(k * DAQ_MAX_CH) + l]);
            }
         }
         for (l=0,j=0;l<DAQ_MAX_LEN;l+=DAQ_MAX_CH) {
            k = (i * DAQ_MAX_LEN) + l;
            if (opc_daq.real) {
               for (m=0;m<DAQ_MAX_CH;m++) samf[m] = (float)(opc_daq.adc[k+m] * DAQ_LSB);
               sprintf(line, "  %8d %8E %8E %8E %8E %8E %8E %8E %8E\n",
                     ((opc_daq.pkt_cnt + i) * DAQ_MAX_SAM) + j++,
                     samf[0],  samf[1],  samf[2],  samf[3],  samf[4],  samf[5],  samf[6],  samf[7]);
               fwrite(line, sizeof(char), strlen(line), opc_daq.file);
            }
            else {
               for (m=0;m<DAQ_MAX_CH;m++) sami[m] = opc_daq.adc[k+m];
               sprintf(line, "  %8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
                     ((opc_daq.pkt_cnt + i) * DAQ_MAX_SAM) + j++,
                     sami[0],  sami[1],  sami[2],  sami[3],  sami[4],  sami[5],  sami[6],  sami[7]);
               fwrite(line, sizeof(char), strlen(line), opc_daq.file);
            }
         }
         // next pipe message
         pipe = (pcm_pipe_daq_t)((uint8_t *)pipe + sizeof(cm_pipe_daq_t));
      }
   }
   //
   // Write to CSV File
   //
   else if (opc_daq.file_type == 2 && opc_daq.file != NULL) {
      // cycle over multiple 1K pipe messages
      for (i=0;i<DAQ_MAX_PIPE_RUN;i++) {
         // cycle over samples, packed in channel order
         for (k=0,m=0;k<DAQ_MAX_SAM;k++) {
            for (l=0;l<DAQ_MAX_CH;l++) {
               opc_daq.adc[(i*DAQ_MAX_LEN) + m++] = ((int32_t)pipe->samples[(k * DAQ_MAX_CH) + l]);
            }
         }
         for (l=0,j=0;l<DAQ_MAX_LEN;l+=DAQ_MAX_CH) {
            k = (i * DAQ_MAX_LEN) + l;
            if (opc_daq.real) {
               for (m=0;m<DAQ_MAX_CH;m++) samf[m] = (float)(opc_daq.adc[k+m] * DAQ_LSB);
               sprintf(line, "  %8d, %8E, %8E, %8E, %8E, %8E, %8E, %8E, %8E\n",
                     ((opc_daq.pkt_cnt + i) * DAQ_MAX_SAM) + j++,
                     samf[0],  samf[1],  samf[2],  samf[3],  samf[4],  samf[5],  samf[6],  samf[7]);
               fwrite(line, sizeof(char), strlen(line), opc_daq.file);
            }
            else {
               for (m=0;m<DAQ_MAX_CH;m++) sami[m] = opc_daq.adc[k+m];
               sprintf(line, "  %8d, %8d, %8d, %8d, %8d, %8d, %8d, %8d, %8d\n",
                     ((opc_daq.pkt_cnt + i) * DAQ_MAX_SAM) + j++,
                     sami[0],  sami[1],  sami[2],  sami[3],  sami[4],  sami[5],  sami[6],  sami[7]);
               fwrite(line, sizeof(char), strlen(line), opc_daq.file);
            }
         }
         // next pipe message
         pipe = (pcm_pipe_daq_t)((uint8_t *)pipe + sizeof(cm_pipe_daq_t));
      }
   }

   return result;

} // end opc_write_file()


// ===========================================================================

// 7.9

void opc_final(void) {

/* 7.9.1   Functional Description

   This routine will clean-up any allocated resources.

   7.9.2   Parameters:

   NONE

   7.9.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.9.4   Data Structures

// 7.9.5   Code

   // Cancel Thread
   pthread_cancel(opc.tid);
   pthread_join(opc.tid, NULL);

   // Free the ADC Sample buffer
   free(opc_daq.adc);

   // Close the ADC file
   if (opc_daq.file != NULL) fclose(opc_daq.file);

} // end opc_final()


