/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      FTDI FT245 Synchronous FIFO I/O Driver

   1.2 Functional Description

      This module provides an API to access the FIFO D2XX Driver.

            NOTE: IN ORDER TO USE THIS DLL THE FTDI USB DEVICE MUST HAVE A SERIAL
            NUMBER THAT BEGINS WITH "O2".

   1.3 Specification/Design Reference

      FIFOAPI_DLL.docx

   1.4 Module Test Specification Reference

      None

   1.5 Compilation Information

      Microsoft Visual Studio 2012 C/C++ using multithreaded CLIB.

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
      7.1   fifo_query()
      7.2   fifo_init()
      7.3   fifo_thread()
      7.4   fifo_tx()
      7.5   fifo_cmio()
      7.6   fifo_head()
      7.7   fifo_sysid()
      7.8   fifo_rev()
      7.9   fifo_final()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "atlbase.h"
#include "atlstr.h"

#include "cm_const.h"
#include "cmapi.h"
#include "daq_msg.h"
#include "fifo.h"
#include "fifoapi.h"
#include "ftd2xx.h"
#include "timer.h"
#include "trace.h"

#include "build.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define  FIFO_RETRIES   4

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static   DWORD WINAPI      fifo_thread(LPVOID data);

// 6.2  Local Data Structures

   static   uint8_t           m_cm_port  = CM_PORT_NONE;
   static   uint8_t           m_com_port = 0;

   static   FT_DEVICE_LIST_INFO_NODE m_devinfo[FIFO_MAX_DEVICES] = {0};

   static   CRITICAL_SECTION  m_tx_mutex;
   static   HANDLE            m_thread;
   static   DWORD             m_tid;
   static   HANDLE            m_event = NULL;
   static   uint8_t          *m_pool = NULL;
   static   uint8_t          *m_nxt_pipe = NULL;
   static   uint8_t          *m_blk_pipe = NULL;
   static   uint8_t           m_end_thread = FALSE;
   static   uint32_t          m_blkcnt = 0;
   static   uint32_t          m_head = 0;
   static   CHRTimer          m_timer;
   static   uint32_t          m_devcnt = 0;

   static   uint8_t           m_txbuf[FIFO_MSGLEN_UINT8] = {0};
   static   uint8_t           m_rxbuf[FIFO_MSGLEN_UINT8] = {0};

   static   uint32_t          m_sysid, m_stamp, m_cmdat;
   static   uint8_t           m_devid, m_numobjs, m_numcons;
   static   DWORD             m_librev, m_sysrev;

   static   HANDLE            m_fifo = NULL;

   static   UCHAR             m_query[] = {0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00};

// 7 MODULE CODE

// ===========================================================================

// 7.1

FIFO_API uint32_t fifo_query(fifo_dev_info_t **devinfo) {

/* 7.1.1   Functional Description

   This routine will query the D2XX driver to acquire all the FIFO devices
   attached to the system.

   7.1.2   Parameters:

   devcnt   Pointer to the attached device count
   devinfo  Pointer to the pfifo_dev_info_t strucure

   7.1.3   Return Values:

   devcnt  FTDI Device Count

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   FT_STATUS   status;
   DWORD       devices;

// 7.1.5   Code

   // Query for attached devices
   status = FT_CreateDeviceInfoList(&devices);
   if (status == FT_OK) m_devcnt = (uint32_t)devices;

   // Query for Device Info
   if (m_devcnt <= FIFO_MAX_DEVICES) {
      status = FT_GetDeviceInfoList((FT_DEVICE_LIST_INFO_NODE *)m_devinfo, &devices);
      if (status == FT_OK) m_devcnt = (uint32_t)devices;
   }

   *devinfo = (fifo_dev_info_t *)m_devinfo;

   return m_devcnt;

}  // end fifo_query()


// ===========================================================================

// 7.2

FIFO_API int32_t fifo_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port) {

/* 7.2.1   Functional Description

   This routine will open access to the FIFO device (VID=0x0403,PID=0x6001).

   NOTE: Only a single device is currently supported.

   7.2.2   Parameters:

   baudrate Baudrate in bps
   cm_port  CM Port Identifier
   com_port FTDI Port

   7.2.3   Return Values:

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    result = FIFO_OK;
   FT_STATUS   status;
   DWORD       sent, recv;
   uint8_t     retry = 0;
   UINT        i;

// 7.2.5   Code

   // close FIFO if opened
   if (m_fifo != NULL) FT_Close(m_fifo);
   m_fifo = NULL;

   // Update FTDI COM Port
   m_com_port = com_port;

   // Okay to Go
   if (m_devcnt != 0) {
      // check for valid FIFO interface
      for (i=0;i<m_devcnt;i++) {
         if (m_devinfo[i].SerialNumber[0] == 'O' && m_devinfo[i].SerialNumber[1] == '2') {
            // Open Selected Available port
            if (m_com_port == i) {
               status = FT_OpenEx((PVOID)m_devinfo[i].SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &m_fifo);
               // Configure Device characteristics
               if (status == FT_OK) {
                  status |= FT_ResetDevice(m_fifo);
                  status |= FT_Purge(m_fifo, FT_PURGE_RX | FT_PURGE_TX);
                  status |= FT_ResetDevice(m_fifo);
                  status |= FT_SetUSBParameters(m_fifo, 32768, 32768);
                  status |= FT_SetChars(m_fifo, FALSE, 0, FALSE, 0);
                  status |= FT_SetLatencyTimer(m_fifo, FIFO_LATENCY);
                  status |= FT_SetTimeouts(m_fifo, FIFO_RX_TIMEOUT, FIFO_TX_TIMEOUT);
                  // Set Sync 245 FIFO Mode
                  status |= FT_SetBitMode(m_fifo, 0x00, 0x40);
                  status |= FT_Purge(m_fifo, FT_PURGE_RX | FT_PURGE_TX);
                  // Check return status from FIFO
                  if (status != FT_OK) result |= FIFO_ERR_OPEN;
               }
               break;
            }
         }
      }
      if (m_devcnt == i) result = FIFO_ERR_DEV_CNT;
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

         // If sent OK then check response
         if (status == FT_OK && sent == FIFO_MSGLEN_UINT8) {
            // Allow time for response
            Sleep(50);
            // Read the Port
            status = FT_Read(m_fifo, m_rxbuf, FIFO_MSGLEN_UINT8, &recv);
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
         m_event = CreateEvent(NULL, false, false, L"");
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

      // Reset Hi-Res Timer
      m_timer.Start();

   }
   // Close the FIFO
   else {
      FT_Close(m_fifo);
   }

   return result;

}  // end fifo_init()


// ===========================================================================

// 7.3

static DWORD WINAPI fifo_thread(LPVOID data) {


/* 7.3.1   Functional Description

   This thread will service the incoming characters from the FIFO serial
   interface.

   7.3.2   Parameters:

   data     Thread parameters

   7.3.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   DWORD       rx_bytes, j;
   DWORD       bytes_left;
   FT_STATUS   status;
   uint8_t     slotid;
   uint32_t   *buf;
   uint16_t    msglen;
   pcmq_t      slot;
   pcm_msg_t   msg;

   pcm_pipe_daq_t  pipe;

// 7.3.5   Code

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
               pipe->stamp_us = (uint32_t)m_timer.GetElapsedAsMicroSeconds();
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
                  if (++m_head == FIFO_POOL_SLOTS) m_head = 0;
                  m_nxt_pipe = m_pool + (m_head * FIFO_BLOCK_LEN);
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
               if (m_rxbuf[0] == m_rxbuf[1] && m_rxbuf[1] == m_rxbuf[2] &&
                   m_rxbuf[2] == m_rxbuf[3] && m_rxbuf[3] == m_rxbuf[4]) {
               }
               else {
                  msglen = ((m_rxbuf[7] & 0x0F) << 8) | m_rxbuf[6];
                  if (msglen <= FIFO_MSGLEN_UINT8 && msglen >= 12 && m_rxbuf != 0) {
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
                           for (j=0;j<(slot->msglen + 3 - sizeof(cm_msg_t)) >> 2;j++) {
                              slot->buf[j + (sizeof(cm_msg_t) >> 2)] =
                                    buf[j + (sizeof(cm_msg_t) >> 2)];
                           }
                        }
                        // restore slotid
                        msg->h.slot = slotid;
                        // queue the message
                        cm_qmsg((pcm_msg_t)slot->buf);
                     }
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

// 7.4

FIFO_API void fifo_tx(pcm_msg_t msg) {

/* 7.4.1   Functional Description

   This routine will transmit the message. The tx_mutex is used to prevent
   mulitple threads from interferring with a single message transfer.

   7.4.2   Parameters:

   msg     CM message to send.

   7.4.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint8_t    *out = (uint8_t *)msg;
   DWORD       bytes_left, bytes_sent;
   uint8_t     retry = 0;

// 7.4.5   Code

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
      Sleep(1);
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

// 7.5

FIFO_API void fifo_cmio(uint8_t op_code, pcm_msg_t msg) {

/* 7.5.1   Functional Description

   OPCODES

   CM_IO_TX : Transmit the message.

   7.5.2   Parameters:

   msg     Message Pointer
   opCode  CM_IO_TX

   7.5.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

// 7.5.5   Code

   // transmit message
   fifo_tx(msg);

} // end fifo_cmio()


// ===========================================================================

// 7.6

FIFO_API void fifo_head(void) {

/* 7.6.1   Functional Description

   This routine will reset the pipe circular buffer pointers and
   associated counters.

   7.6.2   Parameters:

   NONE

   7.6.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.6.4   Data Structures

// 7.6.5   Code

   // reset pipe circular buffer
   m_nxt_pipe  = m_pool;
   m_blk_pipe  = m_pool;
   m_head      = 0;
   m_blkcnt    = 0;

} // end fifo_head()


// ===========================================================================

// 7.7

FIFO_API void fifo_rev(uint32_t *librev, uint32_t *sysrev, uint32_t *apirev) {

/* 7.7.1   Functional Description

   This routine will return the build versions for fifo.dll, ftd2xx.sys
   and ftd2xx.lib.

   7.7.2   Parameters:

   librev   FIFO ftd2xx.lib revision
   sysrev   FIFO ftd2xx.sys revision
   apirev   fifo.dll revision

   7.7.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.7.4   Data Structures

// 7.7.5   Code

   *librev = m_librev;
   *sysrev = m_sysrev;
   *apirev = (BUILD_MAJOR << 24) | (BUILD_MINOR << 16) |
             (BUILD_NUM << 8)    | BUILD_INC;

}  // end fifo_rev()


// ===========================================================================

// 7.8

FIFO_API void fifo_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat) {

/* 7.8.1   Functional Description

   This routine will report connected device IDs.

   7.8.2   Parameters:

   sysid    System ID returned
   stamp    Timestamp returned
   cmdat    CM Data returned

   7.8.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.8.4   Data Structures

// 7.8.5   Code

   *sysid = m_sysid;
   *stamp = m_stamp;
   *cmdat = m_cmdat;

}  // end fifo_sysid()


// ===========================================================================

// 7.9

FIFO_API void fifo_final(void) {

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

   // Wake-up Thread and Cancel
   m_end_thread = TRUE;
   SetEvent(m_event);

   // Close FIFO
   FT_Close(m_fifo);

   // Release Memory
   if (m_pool != NULL) free(m_pool);

   m_pool = NULL;

} // end fifo_final()

