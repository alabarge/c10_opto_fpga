/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Fast Serial COM I/O Driver

   1.2 Functional Description

      The COM I/O Interface routines are contained in this module.

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
         7.1   com_init()
         7.2   com_thread()
         7.3   com_tx()
         7.4   com_cmio()
         7.5   com_head()
         7.6   com_final()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define COM_RESP          64
   #define COM_RETRIES       4

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static   DWORD WINAPI      com_thread(LPVOID data);

// 6.2  Local Data Structures

   static   uint8_t           m_cm_port  = CM_PORT_NONE;

   static   CRITICAL_SECTION  m_tx_mutex;
   static   HANDLE            m_thread;
   static   DWORD             m_tid;

   static   uint8_t          *m_pool = NULL;
   static   uint8_t          *m_nxt_pipe = NULL;
   static   uint8_t          *m_blk_pipe = NULL;
   static   uint8_t           m_end_thread = FALSE;
   static   uint32_t          m_pktcnt = 0;
   static   uint32_t          m_cmcnt = 0;
   static   uint32_t          m_dropcnt = 0;
   static   uint32_t          m_head = 0;

   static   uint32_t          m_sysid, m_stamp, m_cmdat;
   static   uint8_t           m_devid, m_numobjs, m_numcons;
   static   int32_t           m_librev, m_sysrev;

   static   struct sp_port    *m_com;
   static   struct sp_event_set  *m_event;

   static   UCHAR             m_query[] = {0x7E, 0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00, 0x7D};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t com_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port) {

/* 7.1.1   Functional Description

   The COM Interface is initialized in this routine.

   7.1.2   Parameters:

   baudrate  Serial Baud Rate
   cm_port   CM Port
   com_port  FTDI COM Port from FT_GetDeviceInfoList()

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    result = COM_OK;
   int32_t     sent, recv;
   UCHAR       resp[COM_RESP] = {0}, msg[COM_RESP] = {0};
   CHAR        com[8] = {0};
   uint8_t     retry = 0;
   int32_t     i,k;

// 7.1.5   Code

   if (baudrate == 0) result |= COM_ERR_BAUDRATE;

   if (result == COM_OK) {
      printf("\ncom_init() selected port = %d\n", com_port);
      // Open Port for Read/Write, 8N1
      sprintf_s(com, sizeof(com), "COM%d", com_port);
      if (sp_get_port_by_name(com, &m_com) == SP_OK) {
         result |= (sp_open(m_com, SP_MODE_READ_WRITE) == SP_OK) ? COM_OK : COM_ERR_OPEN;
         result |= (sp_set_baudrate(m_com, baudrate) == SP_OK)   ? COM_OK : COM_ERR_BAUDRATE;
         result |= (sp_set_bits(m_com, 8) == SP_OK)              ? COM_OK : COM_ERR_OPEN;
         result |= (sp_set_parity(m_com, SP_PARITY_NONE) == SP_OK) ? COM_OK : COM_ERR_OPEN;
         result |= (sp_set_stopbits(m_com, 1) == SP_OK)          ? COM_OK : COM_ERR_OPEN;
         result |= (sp_set_flowcontrol(m_com, SP_FLOWCONTROL_NONE) == SP_OK) ? COM_OK : COM_ERR_OPEN;
         result |= (sp_flush(m_com, SP_BUF_BOTH) == SP_OK)       ? COM_OK : COM_ERR_OPEN;
      }
      else
         result |= COM_ERR_OPEN;
   }
   
   // Device Opened
   if (result == COM_OK) {

      // Purge Queues
      sp_flush(m_com, SP_BUF_BOTH);

      while (retry < COM_RETRIES) {

         result |= COM_ERR_RESP;

         // Send CM_QUERY_REQ to validate connection
         sent = sp_blocking_write(m_com, (CHAR *)m_query, sizeof(m_query), 1000);
         Sleep(100);

         // Read Response
         recv = sp_blocking_read(m_com, (CHAR *)resp, sizeof(resp), 1000);

         // Validate
         if (recv == 0) result |= COM_ERR_RESP;

         // Check SOF & EOF
         if (resp[0] == COM_START_FRAME && resp[recv-1] == COM_END_FRAME) {
            // Remove HDLC Coding
            for (i=1,k=0;i<(recv-1);i++) {
               if (resp[i] == COM_ESCAPE) {
                  msg[k++] = resp[i+1] ^ COM_STUFFED_BIT;
                  i++;
               }
               else {
                  msg[k++] = resp[i];
               }
            }
            // Verify Magic Number
            if (msg[12] == 0x34 && msg[13] == 0x12 &&
                msg[14] == 0xAA && msg[15] == 0x55) {
               // Purge Queues
               sp_flush(m_com, SP_BUF_BOTH);
               // Record SysID
               m_sysid = (msg[19] << 24) | (msg[18] << 16) | (msg[17] << 8) | msg[16];
               // Record Timestamp
               m_stamp = (msg[23] << 24) | (msg[22] << 16) | (msg[21] << 8) | msg[20];
               // Record Device ID, etc.
               m_devid    = msg[24];
               m_numobjs  = msg[25];
               m_numcons  = msg[26];
               m_cmdat    = (m_devid << 24) | (m_numobjs << 16) | (m_numcons << 8);
               result = COM_OK;
               break;
            }
         }
         retry++;
      }
   }

   // OK to Continue
   if (result == COM_OK) {

      // Serial I/O Version and Build
      m_librev = SP_LIB_VERSION_CURRENT;
      m_sysrev = SP_LIB_VERSION_REVISION;

      // Init the Mutex's
      InitializeCriticalSection (&m_tx_mutex);

      // Update CM Port
      m_cm_port = cm_port;

      // Register the I/O Interface callback for CM
      cm_ioreg(com_cmio, m_cm_port, CM_MEDIA_COM);

      // Allocate Pipe Message Pool
      m_pool  = (uint8_t *)malloc(COM_POOL_SLOTS * COM_PIPELEN_UINT8);
      if (m_pool == NULL) result = COM_ERR_POOL;

      // Events
      sp_new_event_set(&m_event);
      sp_add_port_events(m_event, m_com, SP_EVENT_RX_READY);

      // Start the H/W Receive Thread
      m_end_thread = FALSE;
      if ((m_thread = CreateThread(NULL, 0, com_thread, NULL, 0, &m_tid)) == NULL) {
         result = COM_ERR_THREAD;
      }
      else {
         SetThreadPriority(&m_tid, THREAD_PRIORITY_NORMAL);
      }

       // Print Hardware Version to Serial Port
      if (gc.trace & WIN_TRACE_ID) {
         printf("Opened COM.%d for Messaging\n", com_port);
         printf("COM.%d : sysid:stamp:cm = %d:%d:%08X\n\n", com_port, m_sysid, m_stamp, m_cmdat);
      }
   }
   else {
      sp_close(m_com);
      sp_free_port(m_com);
   }

   if ((gc.trace & WIN_TRACE_ERROR) && result != COM_OK) {
      printf("com_init() Error : %08X\n", result);
   }

   return (result == COM_OK) ? WIN_ERROR_OK : WIN_ERROR_COM;

}  // end com_init()


// ===========================================================================

// 7.2

static DWORD WINAPI com_thread(LPVOID data) {


/* 7.2.1   Functional Description

   This thread will service the incoming characters from the COM serial
   interface.

   7.2.2   Parameters:

   data     Thread parameters

   7.2.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   DWORD       rx_bytes, i;
   uint8_t     raw[COM_PIPELEN_UINT8 * 2];
   uint8_t     *buf = NULL;
   uint32_t    head = 0;
   com_rxq_t   rxq = {0};

   pcm_pipe_daq_t  pipe;

// 7.2.5   Code

   if (gc.trace & WIN_TRACE_ID) {
      printf("com_thread() started, tid 0x%X\n", m_tid);
   }

   // Purge Queues
   sp_flush(m_com, SP_BUF_BOTH);

   // beginning of PIPE message circular buffer
   m_nxt_pipe  = m_pool;
   m_blk_pipe  = m_pool;
   m_head      = 0;

   m_pktcnt = 0;
   m_cmcnt  = 0;

   while (m_end_thread == FALSE) {
      rx_bytes = sp_input_waiting(m_com);
      if (rx_bytes == 0) sp_wait(m_event, 1000);
      rx_bytes = sp_input_waiting(m_com);
      //
      // HDLC DECODING
      //
      if (rx_bytes != 0 && m_end_thread == FALSE) {
         // prevent buffer overflow, account for encoding
         rx_bytes = (rx_bytes > (COM_PIPELEN_UINT8 * 2)) ? COM_PIPELEN_UINT8 * 2 : rx_bytes;
         memset(raw, 0, sizeof(raw));
         rx_bytes = sp_blocking_read(m_com, (CHAR *)raw, rx_bytes, 1000);
         if ((rx_bytes > 0) && (m_end_thread != TRUE)) {
            for (i=0;i<rx_bytes;i++) {
               // handle restart outside of switch
               if (raw[i] == COM_START_FRAME) {
                  rxq.state = COM_RX_IDLE;
                  rxq.count = 0;
                  if (rxq.slot != NULL) {
                     cm_free((pcm_msg_t)rxq.slot->buf);
                     rxq.slot = NULL;
                  }
               }
               switch (rxq.state) {
                  case COM_RX_IDLE :
                     // all messages begin with COM_START_FRAME
                     // drop all characters until start-of-frame
                     if (raw[i] == COM_START_FRAME) {
                        rxq.state = COM_RX_TYPE;
                     }
                     break;
                  case COM_RX_TYPE :
                     if (raw[i] != CM_ID_PIPE) {
                        // allocate slot from cmq
                        rxq.slot = cm_alloc();
                        if (rxq.slot != NULL) {
                           rxq.state  = COM_RX_MSG;
                           rxq.count  = 0;
                           rxq.msg = (pcm_msg_t)rxq.slot->buf;
                           // preserve q slot id
                           rxq.slotid = rxq.msg->h.slot;
                           buf = (uint8_t *)rxq.slot->buf;
                           buf[rxq.count++] = raw[i];
                        }
                        else {
                           // no room at the inn, drop the entire message
                           rxq.state = COM_RX_IDLE;
                           rxq.count = 0;
                           m_dropcnt++;
                        }
                     }
                     else {
                        rxq.state = COM_RX_PIPE;
                        m_nxt_pipe[rxq.count++] = raw[i];
                     }
                     break;
                  case COM_RX_MSG :
                     // unstuff next octet
                     if (raw[i] == COM_ESCAPE) {
                        rxq.state = COM_RX_MSG_ESC;
                     }
                     // end-of-frame
                     else if (raw[i] == COM_END_FRAME) {
                        // restore q slot id
                        rxq.msg->h.slot  = rxq.slotid;
                        rxq.slot->msglen = rxq.msg->h.msglen;
                        // validate message length
                        if (rxq.msg->h.msglen >= sizeof(cm_msg_t) &&
                            rxq.msg->h.msglen <= CM_MAX_MSG_INT8U) {
                           // queue the message
                           cm_qmsg((pcm_msg_t)rxq.slot->buf);
                           rxq.state = COM_RX_IDLE;
                           rxq.count = 0;
                           m_cmcnt++;
                           rxq.slot  = NULL;
                        }
                        // drop the mal-formed message
                        else {
                           cm_free((pcm_msg_t)rxq.slot->buf);
                           rxq.state = COM_RX_IDLE;
                           rxq.count = 0;
                           m_dropcnt++;
                           rxq.slot  = NULL;
                        }
                     }
                     else {
                        // store message octet
                        buf[rxq.count] = raw[i];
                        // buffer length exceeded, drop the entire message
                        if (++rxq.count > COM_PIPELEN_UINT8) {
                           rxq.state = COM_RX_IDLE;
                           rxq.count = 0;
                           m_dropcnt++;
                           if (rxq.slot != NULL) {
                              cm_free((pcm_msg_t)rxq.slot->buf);
                              rxq.slot = NULL;
                           }
                        }
                     }
                     break;
                  case COM_RX_MSG_ESC :
                     // unstuff octet
                     buf[rxq.count] = raw[i] ^ COM_STUFFED_BIT;
                     // buffer length exceeded, drop the entire message
                     if (++rxq.count > COM_PIPELEN_UINT8) {
                        rxq.state = COM_RX_IDLE;
                        rxq.count = 0;
                        m_dropcnt++;
                        if (rxq.slot != NULL) {
                           cm_free((pcm_msg_t)rxq.slot->buf);
                        }
                     }
                     else {
                        rxq.state = COM_RX_MSG;
                     }
                     break;
                  case COM_RX_PIPE :
                     // unstuff next octet
                     if (raw[i] == COM_ESCAPE) {
                        rxq.state = COM_RX_PIPE_ESC;
                     }
                     // end-of-frame
                     else if (raw[i] == COM_END_FRAME) {
                        pipe = (pcm_pipe_daq_t)m_nxt_pipe;
                        // packet arrival
                        pipe->stamp_us = (uint32_t)tmr_get_elapsed_microseconds();
                        // next slot in circular buffer
                        if (++m_head == COM_POOL_SLOTS) m_head = 0;
                        m_nxt_pipe = m_pool + (m_head * COM_PIPELEN_UINT8);
                        // last packet in block?
                        if (++m_pktcnt == COM_PACKET_CNT) {
                           m_pktcnt = 0;
                           // send pipe message
                           cm_pipe_send((pcm_pipe_t)m_blk_pipe, COM_BLOCK_LEN);
                           // record next start of block
                           m_blk_pipe = m_nxt_pipe;
                        }
                        rxq.state = COM_RX_IDLE;
                        rxq.count = 0;
                     }
                     else {
                        // store pipe octet
                        m_nxt_pipe[rxq.count] = raw[i];
                        // buffer length exceeded, drop the entire message
                        if (++rxq.count > COM_PIPELEN_UINT8) {
                           rxq.state = COM_RX_IDLE;
                           rxq.count = 0;
                           m_dropcnt++;
                        }
                     }
                     break;
                  case COM_RX_PIPE_ESC :
                     // unstuff octet
                     m_nxt_pipe[rxq.count] = raw[i] ^ COM_STUFFED_BIT;
                     // buffer length exceeded, drop the entire message
                     if (++rxq.count > COM_PIPELEN_UINT8) {
                        rxq.state = COM_RX_IDLE;
                        rxq.count = 0;
                        m_dropcnt++;
                     }
                     else {
                        rxq.state = COM_RX_PIPE;
                     }
                     break;

               }
            }
         }
      }
   }

   return 0;

} // end com_thread()


// ===========================================================================

// 7.3

void com_tx(pcm_msg_t msg) {

/* 7.3.1   Functional Description

   This routine will transmit the message. The tx_mutex is used to prevent
   mulitple threads from interferring with a single message transfer. HDLC
   coding is used to form a message packet.

   7.3.2   Parameters:

   msg     CM message to send.

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint8_t    *out = (uint8_t *)msg;
   uint8_t    *buf;
   uint16_t    i, j = 0;
   DWORD       bytes_left, bytes_sent;
   uint8_t     retry = 0;

// 7.3.5   Code

   // Trace Entry
   if (gc.trace & WIN_TRACE_UART) {
      printf("com_tx() srvid:msgid:msglen = %02X:%02X:%04X\n",
               msg->p.srvid, msg->p.msgid, msg->h.msglen);
      dump((uint8_t *)msg, msg->h.msglen, LIB_ASCII, 0);
   }

   // Enter Critical Section
   EnterCriticalSection(&m_tx_mutex);

   buf = (uint8_t *)calloc(COM_MSGLEN_UINT8 * 2, sizeof(uint8_t));

   buf[j++] = COM_START_FRAME;
   for (i=0;i<msg->h.msglen;i++) {
      if (out[i] == COM_START_FRAME || out[i] == COM_END_FRAME || out[i] == COM_ESCAPE) {
         buf[j++] = COM_ESCAPE;
         buf[j++] = out[i] ^ COM_STUFFED_BIT;
      }
      else {
         buf[j++] = out[i];
      }
   }
   buf[j++] = COM_END_FRAME;
   bytes_left = j;
   bytes_sent = sp_blocking_write(m_com, (CHAR *)buf, bytes_left, 1000);
   bytes_left -= bytes_sent;
   //retry
   while (bytes_left != 0 && retry < COM_RETRIES) {
      Sleep(1);
      bytes_sent = sp_blocking_write(m_com, (CHAR *)buf[j - bytes_left], bytes_left, 1000);
      bytes_left -= bytes_sent;
      retry++;
   }

   // release message
   cm_free(msg);

   free(buf);

   // Leave Critical Section
   LeaveCriticalSection(&m_tx_mutex);

} // end com_tx()


// ===========================================================================

// 7.4

void com_cmio(uint8_t op_code, pcm_msg_t msg) {

/* 7.4.1   Functional Description

   OPCODES

   CM_IO_TX : The transmit queue index will be incremented,
   this causes the top of the queue to be transmitted.

   7.4.2   Parameters:

   msg     Message Pointer
   opCode  CM_IO_TX

   7.4.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

// 7.4.5   Code

   if (gc.trace & WIN_TRACE_UART) {
      printf("com_cmio() op_code = %02X\n", op_code);
   }

   // transmit message
   com_tx(msg);

} // end com_cmio()


// ===========================================================================

// 7.5

void com_head(void) {

/* 7.5.1   Functional Description

   This routine will reset the pipe circular buffer pointers and
   associated counters.

   7.5.2   Parameters:

   NONE

   7.5.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

// 7.5.5   Code

   // reset pipe circular buffer
   m_nxt_pipe  = m_pool;
   m_blk_pipe  = m_pool;
   m_head      = 0;
   m_pktcnt    = 0;

} // end com_head()


// ===========================================================================

// 7.6

void com_final(void) {

/* 7.6.1   Functional Description

   This routine will clean-up any allocated resources.

   7.6.2   Parameters:

   NONE

   7.6.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.6.4   Data Structures

// 7.6.5   Code

   // Wake-up Thread and Cancel
   m_end_thread = TRUE;

   // Close COM
   sp_free_event_set(m_event);
   sp_close(m_com);
   sp_free_port(m_com);

   // Release Memory
   if (m_pool != NULL) free(m_pool);

   m_pool = NULL;

} // end com_final()




