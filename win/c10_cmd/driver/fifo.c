/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      FTDI FT245 Synchronous FIFO I/O Driver

   1.2 Functional Description

      This module provides an API to access the FIFO D2XX Driver.

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
         7.1   fifo_init()
         7.2   fifo_thread()
         7.3   fifo_tx()
         7.4   fifo_cmio()
         7.5   fifo_head()
         7.6   fifo_final()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"
#include "ftd2xx.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define FIFO_RETRIES       4

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static   DWORD WINAPI      fifo_thread(LPVOID data);

// 6.2  Local Data Structures

   static   uint8_t           m_cm_port  = CM_PORT_NONE;
   static   uint8_t           m_com_port = 0;

   static   HANDLE            m_thread;
   static   DWORD             m_tid;
   static   HANDLE            m_event = NULL;
   static   uint8_t          *m_pool = NULL;
   static   uint8_t          *m_nxt_pipe = NULL;
   static   uint8_t          *m_blk_pipe = NULL;
   static   uint8_t           m_end_thread = FALSE;
   static   uint32_t          m_blkcnt = 0;
   static   uint32_t          m_head = 0;

   static   uint8_t           m_txbuf[FIFO_MSGLEN_UINT8] = {0};
   static   uint8_t           m_rxbuf[FIFO_MSGLEN_UINT8] = {0};


   static   CRITICAL_SECTION  m_tx_mutex;

   static   uint32_t          m_sysid, m_stamp, m_cmdat;
   static   uint8_t           m_devid, m_numobjs, m_numcons;
   static   DWORD             m_librev, m_sysrev;

   static   HANDLE            m_fifo = NULL;

   static   UCHAR             m_query[] = {0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t fifo_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port) {

/* 7.1.1   Functional Description

   The FIFO Interface is initialized in this routine.

   7.1.2   Parameters:

   baudrate  Serial Baud Rate
   cm_port   CM Port
   com_port  FTDI COM Port from FT_GetDeviceInfoList()

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    result = FIFO_OK;
   FT_STATUS   status;
   DWORD       dev_cnt, sent, recv;
   uint8_t     retry = 0;
   UINT        i;

   FT_DEVICE_LIST_INFO_NODE dev_info[FIFO_MAX_DEVICES];

// 7.1.5   Code

   // close FIFO if opened
   if (m_fifo != NULL) FT_Close(m_fifo);
   m_fifo = NULL;

   // Update FTDI COM Port
   m_com_port = com_port;

   //
   // Open the Available Selected FTDI device
   //
   if (FT_CreateDeviceInfoList(&dev_cnt) == FT_OK) {
      if (dev_cnt <= FIFO_MAX_DEVICES) {
         // fill-out device info
         if (FT_GetDeviceInfoList(dev_info, &dev_cnt) != FT_OK) {
            result = FIFO_ERR_INFO;
         }
      }
      else {
         result = FIFO_ERR_DEV_CNT;
      }
   }
   else {
      result = FIFO_ERR_DEV;
   }

   // Okay to Go
   if (result == FIFO_OK) {
      if (gc.trace & WIN_TRACE_UART)
         printf("\nfifo_init() selected port = %d\n", m_com_port);
      // check for valid FIFO interface
      for (i=0;i<dev_cnt;i++) {
         if (dev_info[i].SerialNumber[0] == 'O' && dev_info[i].SerialNumber[1] == '2') {
            if (gc.trace & WIN_TRACE_UART) {
               printf("\nfifo_init() FIFO.%d :\n", i);
               printf("  flags  : %08X\n", dev_info[i].Flags);
               printf("  type   : %08X\n", dev_info[i].Type);
               printf("  id     : %08X\n", dev_info[i].ID);
               printf("  locid  : %08X\n", dev_info[i].LocId);
               printf("  serial : %s\n", dev_info[i].SerialNumber);
               printf("  desc   : %s\n\n", dev_info[i].Description);
            }

            // Open Selected Available port
            if (m_com_port == i) {
               status = FT_OpenEx((PVOID)dev_info[i].SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &m_fifo);
               // Configure Device characteristics
               if (status == FT_OK) {
                  status |= FT_ResetDevice(m_fifo);
                  status |= FT_Purge(m_fifo, FT_PURGE_RX | FT_PURGE_TX);
                  status |= FT_ResetDevice(m_fifo);
                  status |= FT_SetUSBParameters(m_fifo, 65536, 65536);
                  status |= FT_SetChars(m_fifo, FALSE, 0, FALSE, 0);
                  status |= FT_SetLatencyTimer(m_fifo, FIFO_LATENCY);
                  status |= FT_SetTimeouts(m_fifo, FIFO_RX_TIMEOUT, FIFO_TX_TIMEOUT);
                  // Set Sync 245 FIFO Mode
                  status |= FT_SetBitMode(m_fifo, 0x00, 0x40);
                  status |= FT_Purge(m_fifo, FT_PURGE_RX | FT_PURGE_TX);
                  // Check return status from FIFO
                  if (status != FT_OK) result |= FIFO_ERR_OPEN;
               }
               else
                  result = FIFO_ERR_OPEN;
               break;
            }
         }
      }
      if (dev_cnt == i) result = FIFO_ERR_DEV_CNT;
   }

   // Device Opened
   if (result == FIFO_OK && m_fifo != NULL) {

      // Empty the TX and RX Queues
      status |= FT_Purge(m_fifo, FT_PURGE_RX | FT_PURGE_TX);
      status |= FT_Read(m_fifo, m_rxbuf, FIFO_MSGLEN_UINT8, &recv);
      status |= FT_Read(m_fifo, m_rxbuf, FIFO_MSGLEN_UINT8, &recv);
      status |= FT_Read(m_fifo, m_rxbuf, FIFO_MSGLEN_UINT8, &recv);

      // Clear the TX & RX buffers
      memset(m_txbuf, 0, FIFO_MSGLEN_UINT8);
      memset(m_rxbuf, 0, FIFO_MSGLEN_UINT8);

      // Issue CM_QUERY_REQ multiple tries
      while (retry < FIFO_RETRIES) {

         result |= FIFO_ERR_RESP;

         // Send CM_QUERY_REQ to validate connection
         cm_crc((pcm_msg_t)&m_query[1], CM_CALC_CRC);
         memcpy(m_txbuf, m_query, sizeof(m_query));
         status |= FT_Write(m_fifo, m_txbuf, FIFO_MSGLEN_UINT8, &sent);

         // report message content
         if ((gc.trace & WIN_TRACE_UART)) {
            printf("fifo_init() tx msglen = %d\n", (int)sizeof(m_query));
            dump((uint8_t *)m_query, sizeof(m_query), 0, 0);
         }

         // If sent OK then check response
         if (status == FT_OK && sent == FIFO_MSGLEN_UINT8) {
            // Allow time for Response
            Sleep(50);
            // Read the Port
            status = FT_Read(m_fifo, m_rxbuf, FIFO_MSGLEN_UINT8, &recv);
            // report message content
            if ((gc.trace & WIN_TRACE_UART)) {
               printf("fifo_init() rx msglen = %d\n", recv);
               dump((uint8_t *)m_rxbuf, 28, 0, 0);
            }
            // Check Response
            if (status == FT_OK && recv == FIFO_MSGLEN_UINT8) {
               // Verify Magic Number
               if (m_rxbuf[12] == 0x34 && m_rxbuf[13] == 0x12 &&
                   m_rxbuf[14] == 0xAA && m_rxbuf[15] == 0x55) {
                  // Purge Queues
                  FT_Purge(m_fifo, FT_PURGE_RX | FT_PURGE_TX);
                  // Record SysID
                  m_sysid = (m_rxbuf[19] << 24) | (m_rxbuf[18] << 16) |
                            (m_rxbuf[17] << 8) | m_rxbuf[16];
                  // Record Timestamp
                  m_stamp = (m_rxbuf[23] << 24) | (m_rxbuf[22] << 16) |
                            (m_rxbuf[21] << 8) | m_rxbuf[20];
                  // Record Device ID, etc.
                  m_devid    = m_rxbuf[24];
                  m_numobjs  = m_rxbuf[25];
                  m_numcons  = m_rxbuf[26];
                  m_cmdat    = (m_devid << 24) | (m_numobjs << 16) | (m_numcons << 8);
                  result = FIFO_OK;
                  break;
               }
            }
         }
         retry++;
      }
   }

   // OK to Continue
   if (result == FIFO_OK && m_fifo != NULL) {

      FT_GetLibraryVersion(&m_librev);
      FT_GetDriverVersion(m_fifo, &m_sysrev);

      // Create or Reset Event for RX Sync
      if (m_event == NULL)
         m_event = CreateEvent(NULL, FALSE, FALSE, "");
      else
         ResetEvent(m_event);

      // Init the Mutex
      InitializeCriticalSection (&m_tx_mutex);

      // Update CM Port
      m_cm_port = cm_port;

      // Register the I/O Interface callback for CM
      cm_ioreg(fifo_cmio, m_cm_port, CM_MEDIA_FIFO);

      // Allocate Pipe Message Pool
      m_pool  = (uint8_t *)malloc(FIFO_PIPE_POOL);
      if (m_pool == NULL) result = FIFO_ERR_POOL;

      // Start the H/W Receive Thread
      m_end_thread = FALSE;
      if ((m_thread = CreateThread(NULL, 0, fifo_thread, NULL, 0, &m_tid)) == NULL) {
         result = FIFO_ERR_THREAD;
      }
      else {
         SetThreadPriority(&m_tid, THREAD_PRIORITY_NORMAL);
      }

       // Print Hardware Version to Serial Port
      if (gc.trace & WIN_TRACE_ID) {
         printf("Opened FIFO.%d (%s) for Messaging\n", m_com_port, dev_info[i].SerialNumber);
         printf("FIFO.%d : ftd2xx.lib:ftd2xx.sys = %08X:%08X\n", m_com_port, m_librev, m_sysrev);
         printf("FIFO.%d : sysid:stamp:cm = %d:%d:%08X\n\n", m_com_port, m_sysid, m_stamp, m_cmdat);
      }
   }

   if ((gc.trace & WIN_TRACE_ERROR) && result != FIFO_OK) {
      printf("fifo_init() Error : %08X\n", result);
   }

   return (result == FIFO_OK) ? WIN_ERROR_OK : WIN_ERROR_FIFO;

}  // end fifo_init()


// ===========================================================================

// 7.2

static DWORD WINAPI fifo_thread(LPVOID data) {


/* 7.2.1   Functional Description

   This thread will service the incoming characters from the FIFO serial
   interface.

   7.2.2   Parameters:

   data     Thread parameters

   7.2.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    rx_bytes = 0, j, k;
   uint32_t    bytes_left;
   FT_STATUS   status;
   uint8_t     slotid;
   uint32_t   *buf;
   uint16_t    msglen;
   pcmq_t      slot;
   pcm_msg_t   msg;

   pcm_pipe_daq_t  pipe;

// 7.2.5   Code

   if (gc.trace & WIN_TRACE_ID) {
      printf("fifo_thread() started, tid 0x%X\n", m_tid);
   }

   FT_Purge(m_fifo, FT_PURGE_RX | FT_PURGE_TX);
   FT_SetEventNotification(m_fifo, FT_EVENT_RXCHAR, m_event);

   // beginning of PIPE message circular buffer
   m_nxt_pipe  = m_pool;
   m_blk_pipe  = m_pool;
   m_head      = 0;
   m_blkcnt    = 0;

   while (m_end_thread == FALSE) {
      FT_GetQueueStatus(m_fifo, &rx_bytes);
      if (rx_bytes < FIFO_MSGLEN_UINT8)
         WaitForSingleObject(m_event, FIFO_THREAD_TIMEOUT);
      // Check for RX Characters
      FT_GetQueueStatus(m_fifo, &rx_bytes);
      while (rx_bytes >= FIFO_MSGLEN_UINT8) {
         status = FT_Read(m_fifo, m_rxbuf, FIFO_MSGLEN_UINT8, &rx_bytes);
         // validate FT_Read()
         if ((status == FT_OK) && (rx_bytes == FIFO_MSGLEN_UINT8) && (m_end_thread != TRUE)) {
            //
            // PIPE MESSAGE
            //
            if (m_rxbuf[0] == CM_ID_PIPE) {
               memcpy(m_nxt_pipe, m_rxbuf, FIFO_MSGLEN_UINT8);
               pipe = (pcm_pipe_daq_t)m_nxt_pipe;
               // packet arrival
               pipe->stamp_us = (uint32_t)tmr_get_elapsed_microseconds();
               // read rest of pipe message
               bytes_left = FIFO_PIPELEN_UINT8 - FIFO_MSGLEN_UINT8;
               m_nxt_pipe += FIFO_MSGLEN_UINT8;
               FT_Read(m_fifo, m_nxt_pipe, bytes_left, &rx_bytes);
               bytes_left -= rx_bytes;
               m_nxt_pipe += rx_bytes;
               // ensure entire block is read
               while (bytes_left != 0) {
                  Sleep(1);
                  FT_Read(m_fifo, m_nxt_pipe, bytes_left, &rx_bytes);
                  bytes_left -= rx_bytes;
                  m_nxt_pipe += rx_bytes;
                  if (rx_bytes == 0) break;
               }
               // last packet in block?
               if (++m_blkcnt == FIFO_PIPE_BLKS) {
                  m_blkcnt = 0;
                  // next slot in circular buffer
                  if (++m_head == FIFO_PIPE_SLOTS) m_head = 0;
                  m_nxt_pipe = m_pool + (m_head * FIFO_BLOCK_LEN);
                  // report partial pipe content
                  if (gc.trace & WIN_TRACE_PIPE) {
                     printf("fifo_thread() pipelen = %d\n", FIFO_BLOCK_LEN);
                     dump(m_blk_pipe, 32, LIB_ASCII, 0);
                  }
                  // send pipe message
                  cm_pipe_send((pcm_pipe_t)m_blk_pipe, FIFO_BLOCK_LEN);
                  // record next start of block
                  m_blk_pipe = m_nxt_pipe;
               }
            }
            //
            // CONTROL MESSAGE
            //
            else {
               msglen = ((m_rxbuf[7] & 0x0F) << 8) | m_rxbuf[6];
               if (msglen <= FIFO_MSGLEN_UINT8 && msglen >= 12) {
                  slot = cm_alloc();
                  if (slot != NULL) {
                     msg = (pcm_msg_t)slot->buf;
                     // preserve slotid
                     slotid = msg->h.slot;
                     // uint32_t boundary, copy multiple of 32-bits
                     // always read CM header + parms in order
                     // to determine message length
                     buf = (uint32_t *)m_rxbuf;
                     for (j=0;j<sizeof(cm_msg_t) >> 2;j++) {
                        slot->buf[j] = buf[j];
                     }
                     slot->msglen = msg->h.msglen;
                     // read rest of CM message body, uint32_t per cycle
                     if (slot->msglen > sizeof(cm_msg_t) && (slot->msglen <= FIFO_MSGLEN_UINT8)) {
                        k = (slot->msglen + 3 - sizeof(cm_msg_t)) >> 2;
                        for (j=0;j<k;j++) {
                           slot->buf[j + (sizeof(cm_msg_t) >> 2)] =
                                 buf[j + (sizeof(cm_msg_t) >> 2)];
                        }
                     }
                     // restore slotid
                     msg->h.slot = slotid;
                     // report message content
                     if ((gc.trace & WIN_TRACE_UART) && (slot != NULL)) {
                        printf("fifo_thread() msglen = %d\n", msg->h.msglen);
                        dump((uint8_t *)slot->buf, slot->msglen, LIB_ASCII, 0);
                     }
                     // queue the message
                     cm_qmsg((pcm_msg_t)slot->buf);
                  }
               }
            }
         }
         // Check for RX Characters
         FT_GetQueueStatus(m_fifo, &rx_bytes);
      }
   }

   return 0;

} // end fifo_thread()


// ===========================================================================

// 7.3

void fifo_tx(pcm_msg_t msg) {

/* 7.3.1   Functional Description

   This routine will transmit the message. The tx_mutex is used to prevent
   mulitple threads from interferring with a single message transfer.

   7.3.2   Parameters:

   msg     CM message to send.

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint8_t    *out = (uint8_t *)msg;
   DWORD       bytes_left, bytes_sent;
   uint8_t     retry = 0;

// 7.3.5   Code

   // Trace Entry
   if (gc.trace & WIN_TRACE_UART) {
      printf("fifo_tx() srvid:msgid:msglen = %02X:%02X:%04X\n",
               msg->p.srvid, msg->p.msgid, msg->h.msglen);
      dump((uint8_t *)msg, msg->h.msglen, LIB_ASCII, 0);
   }

   // Enter Critical Section
   EnterCriticalSection(&m_tx_mutex);

   // copy message to txbuf
   memset(m_txbuf, 0, sizeof(m_txbuf));
   memcpy(m_txbuf, msg, msg->h.msglen);
   bytes_left = FIFO_MSGLEN_UINT8;
   FT_Write(m_fifo, m_txbuf, bytes_left, &bytes_sent);
   bytes_left -= bytes_sent;
   //retry
   while (bytes_left != 0 && retry < FIFO_RETRIES) {
      Sleep(10);
      FT_Write(m_fifo, &m_txbuf[FIFO_MSGLEN_UINT8 - bytes_left], bytes_left, &bytes_sent);
      bytes_left -= bytes_sent;
      retry++;
   }

   // release message
   cm_free(msg);

   // Leave Critical Section
   LeaveCriticalSection(&m_tx_mutex);

} // end fifo_tx()


// ===========================================================================

// 7.4

void fifo_cmio(uint8_t op_code, pcm_msg_t msg) {

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
      printf("fifo_cmio() op_code = %02X\n", op_code);
   }

   // transmit message
   fifo_tx(msg);

} // end fifo_cmio()


// ===========================================================================

// 7.5

void fifo_head(void) {

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
   m_blkcnt    = 0;

} // end fifo_head()


// ===========================================================================

// 7.6

void fifo_final(void) {

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

   // Cancel Thread
   CloseHandle(m_thread);
   WaitForSingleObject(m_thread, 150);

   // Close FIFO
   FT_Close(m_fifo);

   // Release Memory
   free(m_pool);

} // end fifo_final()




