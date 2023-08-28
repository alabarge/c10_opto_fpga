/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      FTDI D2XX API Dynamic Link Library

   1.2 Functional Description

      This module provides an API to access the UART D2XX Driver.

      NOTE: IN ORDER TO USE THIS DLL THE FTDI USB DEVICE MUST HAVE A SERIAL
            NUMBER THAT BEGINS WITH "O5".

   1.3 Specification/Design Reference

      UARTAPI_DLL.docx

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
      7.1   uart_query()
      7.2   uart_init()
      7.3   uart_thread()
      7.4   uart_tx()
      7.5   uart_cmio()
      7.6   uart_head()
      7.7   uart_sysid()
      7.8   uart_rev()
      7.9   uart_final()

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
#include "uart.h"
#include "uartapi.h"
#include "ftd2xx.h"
#include "timer.h"
#include "trace.h"

#include "build.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define  UART_RESP      64
   #define  UART_RETRIES   4

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static   DWORD WINAPI      uart_thread(LPVOID data);

// 6.2  Local Data Structures

   static   uint8_t           m_cm_port  = CM_PORT_NONE;
   static   uint8_t           m_com_port = 0;

   static   FT_DEVICE_LIST_INFO_NODE m_devinfo[UART_MAX_DEVICES] = {0};

   static   CRITICAL_SECTION  m_tx_mutex;
   static   HANDLE            m_thread;
   static   DWORD             m_tid;
   static   HANDLE            m_event = NULL;

   static   uint8_t          *m_pool = NULL;
   static   uint8_t          *m_nxt_pipe = NULL;
   static   uint8_t          *m_blk_pipe = NULL;
   static   uint8_t           m_end_thread = FALSE;
   static   uint32_t          m_pktcnt = 0;
   static   uint32_t          m_cmcnt = 0;
   static   uint32_t          m_dropcnt = 0;
   static   uint32_t          m_head = 0;
   static   CHRTimer          m_timer;
   static   uint32_t          m_devcnt = 0;

   static   uint32_t          m_sysid, m_stamp, m_cmdat;
   static   uint8_t           m_devid, m_numobjs, m_numcons;
   static   DWORD             m_librev, m_sysrev;

   static   HANDLE            m_uart = NULL;

   static   UCHAR             m_query[] = {0x7E, 0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00, 0x7D};

// 7 MODULE CODE

// ===========================================================================

// 7.1

UART_API uint32_t uart_query(uart_dev_info_t **devinfo) {

/* 7.1.1   Functional Description

   This routine will query the FTD2XX driver to acquire all the UART devices
   attached to the system.

   7.1.2   Parameters:

   devcnt   Pointer to the attached device count
   devinfo  Pointer to the puart_dev_info_t strucure

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
   if (m_devcnt <= UART_MAX_DEVICES) {
      status = FT_GetDeviceInfoList((FT_DEVICE_LIST_INFO_NODE *)m_devinfo, &devices);
      if (status == FT_OK) m_devcnt = (uint32_t)devices;
   }

   *devinfo = (uart_dev_info_t *)m_devinfo;

   return m_devcnt;

}  // end uart_query()


// ===========================================================================

// 7.2

UART_API int32_t uart_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port) {

/* 7.2.1   Functional Description

   This routine will open access to the UART device (VID=0x0403,PID=0x6001).

   NOTE: Only a single device is currently supported.

   7.2.2   Parameters:

   baudrate Baudrate in bps
   cm_port  CM Port Identifier
   com_port FTDI Port

   7.2.3   Return Values:

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    result = UART_OK;
   FT_STATUS   status;
   DWORD       sent, recv;
   UCHAR       resp[UART_RESP], msg[UART_RESP];
   uint8_t     retry = 0;
   UINT        i,j,k;

// 7.2.5   Code

   // close UART if opened
   if (m_uart != NULL) FT_Close(m_uart);
   m_uart = NULL;

   // Update FTDI COM Port
   m_com_port = com_port;

   // Okay to Go
   if (m_devcnt != 0) {
      // check for valid UART interface
      for (i=0;i<m_devcnt;i++) {
         if (m_devinfo[i].SerialNumber[0] == 'O' && m_devinfo[i].SerialNumber[1] == '5') {
            // Open Selected Available port
            if (m_com_port == i) {
               status = FT_OpenEx((PVOID)m_devinfo[i].SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &m_uart);
               // Configure Device characteristics
               if (status == FT_OK) {
                  status |= FT_ResetDevice(m_uart);
                  status |= FT_Purge(m_uart, FT_PURGE_RX | FT_PURGE_TX);
                  status |= FT_ResetDevice(m_uart);
                  status |= FT_SetUSBParameters(m_uart, 32768, 32768);
                  status |= FT_SetChars(m_uart, FALSE, 0, FALSE, 0);
                  status |= FT_SetLatencyTimer(m_uart, UART_LATENCY);
                  status |= FT_SetTimeouts(m_uart, UART_RX_TIMEOUT, UART_TX_TIMEOUT);
                  status |= FT_SetBaudRate(m_uart, baudrate);
                  // Check return status from UART
                  if (status != FT_OK) result |= UART_ERR_OPEN;
               }
               break;
            }
         }
      }
      if (m_devcnt == i) result = UART_ERR_DEV_CNT;
   }

   // Device Opened
   if (result == UART_OK && m_uart != NULL) {

      // Empty the TX and RX Queues
      status |= FT_Purge(m_uart, FT_PURGE_RX | FT_PURGE_TX);
      status |= FT_Read(m_uart, resp, UART_RESP, &recv);
      status |= FT_Read(m_uart, resp, UART_RESP, &recv);
      status |= FT_Read(m_uart, resp, UART_RESP, &recv);

      // Issue CM_QUERY_REQ multiple tries
      while (retry < UART_RETRIES) {

         result |= UART_ERR_RESP;

         // Send CM_QUERY_REQ to validate connection
         cm_crc((pcm_msg_t)&m_query[1], CM_CALC_CRC);
         status |= FT_Write(m_uart, m_query, sizeof(m_query), &sent);

         // If sent OK then check response
         if (status == FT_OK && sent == sizeof(m_query)) {
            // Allow time for Response
            Sleep(20);
            // Read the Port
            status = FT_Read(m_uart, resp, UART_RESP, &recv);
            // Check Response
            if (status == FT_OK && recv != 0) {
               // Check START FRAME & END FRAME
               if (resp[0] == UART_START_FRAME && resp[recv-1] == UART_END_FRAME) {
                  // Remove HDLC Coding
                  for (j=1,k=0;j<(recv-1);j++) {
                     if (resp[j] == UART_ESCAPE) {
                        msg[k++] = resp[j+1] ^ UART_STUFFED_BIT;
                        j++;
                     }
                     else {
                        msg[k++] = resp[j];
                     }
                  }
                  // Verify Magic Number
                  if (msg[12] == 0x34 && msg[13] == 0x12 &&
                        msg[14] == 0xAA && msg[15] == 0x55) {
                     // Purge Queues
                     FT_Purge(m_uart, FT_PURGE_RX | FT_PURGE_TX);
                     // Record SysID
                     m_sysid = (msg[19] << 24) | (msg[18] << 16) | (msg[17] << 8) | msg[16];
                     // Record Timestamp
                     m_stamp = (msg[23] << 24) | (msg[22] << 16) | (msg[21] << 8) | msg[20];
                     // Record Device ID, etc.
                     m_devid    = msg[24];
                     m_numobjs  = msg[25];
                     m_numcons  = msg[26];
                     m_cmdat    = (m_devid << 24) | (m_numobjs << 16) | (m_numcons << 8);
                     result = UART_OK;
                     break;
                  }
               }
            }
         }
         retry++;
         Sleep(100);
      }
   }

   // OK to Continue
   if (result == UART_OK && m_uart != NULL) {

      FT_GetLibraryVersion(&m_librev);
      FT_GetDriverVersion(m_uart, &m_sysrev);

      // Create or Reset Event for RX Sync
      if (m_event == NULL)
         m_event = CreateEvent(NULL, false, false, L"");
      else
         ResetEvent(m_event);

      // Init the Mutex's
      InitializeCriticalSection (&m_tx_mutex);

      // Update CM Port
      m_cm_port = cm_port;

      // Register the I/O Interface callback for CM
      cm_ioreg(uart_cmio, m_cm_port, CM_MEDIA_UART);

      // Allocate Pipe Message Pool
      m_pool  = (uint8_t *)malloc(UART_POOL_SLOTS * UART_PIPELEN_UINT8);
      if (m_pool == NULL) result = UART_ERR_POOL;

      // Start the H/W Receive Thread
      m_end_thread = FALSE;
      if ((m_thread = CreateThread(NULL, 0, uart_thread, NULL, 0, &m_tid)) == NULL) {
         result = UART_ERR_THREAD;
      }
      else {
         SetThreadPriority(&m_tid, THREAD_PRIORITY_NORMAL);
      }

      // Reset Hi-Res Timer
      m_timer.Start();

   }
   else {
      FT_Close(m_uart);
   }

   return result;

}  // end uart_init()


// ===========================================================================

// 7.3

static DWORD WINAPI uart_thread(LPVOID data) {


/* 7.3.1   Functional Description

   This thread will service the incoming characters from the UART serial
   interface.

   7.3.2   Parameters:

   data     Thread parameters

   7.3.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   DWORD       rx_bytes, tx_bytes, event, i;
   uint8_t     raw[UART_PIPELEN_UINT8 * 2];
   uint8_t     *buf;
   FT_STATUS   status;
   uint32_t    head = 0;
   com_rxq_t   rxq = {0};

   pcm_pipe_daq_t  pipe;

// 7.3.5   Code

   FT_Purge(m_uart, FT_PURGE_RX | FT_PURGE_TX);
   FT_SetEventNotification(m_uart, FT_EVENT_RXCHAR, m_event);

   // beginning of PIPE message circular buffer
   m_nxt_pipe  = m_pool;
   m_blk_pipe  = m_pool;
   m_head      = 0;

   while (m_end_thread == FALSE) {
      FT_GetStatus(m_uart, &rx_bytes, &tx_bytes, &event);
      if (rx_bytes == 0) WaitForSingleObject(m_event, INFINITE);
      FT_GetStatus(m_uart, &rx_bytes, &tx_bytes, &event);
      //
      // HDLC DECODING
      //
      if (rx_bytes != 0 && m_end_thread == FALSE) {
         // prevent buffer overflow, account for encoding
         rx_bytes = (rx_bytes > (UART_PIPELEN_UINT8 * 2)) ? UART_PIPELEN_UINT8 * 2 : rx_bytes;
         memset(raw, 0, sizeof(raw));
         status = FT_Read(m_uart, raw, rx_bytes, &rx_bytes);
         if ((status == FT_OK) && (rx_bytes > 0) && (m_end_thread != TRUE)) {
            for (i=0;i<rx_bytes;i++) {
               // handle restart outside of switch
               if (raw[i] == UART_START_FRAME) {
                  rxq.state = UART_RX_IDLE;
                  rxq.count = 0;
                  if (rxq.slot != NULL) {
                     cm_free((pcm_msg_t)rxq.slot->buf);
                     rxq.slot = NULL;
                  }
               }
               switch (rxq.state) {
                  case UART_RX_IDLE :
                     // all messages begin with UART_START_FRAME
                     // drop all characters until start-of-frame
                     if (raw[i] == UART_START_FRAME) {
                        rxq.state = UART_RX_TYPE;
                     }
                     break;
                  case UART_RX_TYPE :
                     if (raw[i] != CM_ID_PIPE) {
                        // allocate slot from cmq
                        rxq.slot = cm_alloc();
                        if (rxq.slot != NULL) {
                           rxq.state  = UART_RX_MSG;
                           rxq.count  = 0;
                           rxq.msg = (pcm_msg_t)rxq.slot->buf;
                           // preserve q slot id
                           rxq.slotid = rxq.msg->h.slot;
                           buf = (uint8_t *)rxq.slot->buf;
                           buf[rxq.count++] = raw[i];
                        }
                        else {
                           // no room at the inn, drop the entire message
                           rxq.state = UART_RX_IDLE;
                           rxq.count = 0;
                           m_dropcnt++;
                        }
                     }
                     else {
                        rxq.state = UART_RX_PIPE;
                        m_nxt_pipe[rxq.count++] = raw[i];
                     }
                     break;
                  case UART_RX_MSG :
                     // unstuff next octet
                     if (raw[i] == UART_ESCAPE) {
                        rxq.state = UART_RX_MSG_ESC;
                     }
                     // end-of-frame
                     else if (raw[i] == UART_END_FRAME) {
                        // restore q slot id
                        rxq.msg->h.slot  = rxq.slotid;
                        rxq.slot->msglen = rxq.msg->h.msglen;
                        // validate message length
                        if (rxq.msg->h.msglen >= sizeof(cm_msg_t) &&
                            rxq.msg->h.msglen <= CM_MAX_MSG_INT8U) {
                           // queue the message
                           cm_qmsg((pcm_msg_t)rxq.slot->buf);
                           rxq.state = UART_RX_IDLE;
                           rxq.count = 0;
                           m_cmcnt++;
                           rxq.slot  = NULL;
                        }
                        // drop the mal-formed message
                        else {
                           cm_free((pcm_msg_t)rxq.slot->buf);
                           rxq.state = UART_RX_IDLE;
                           rxq.count = 0;
                           m_dropcnt++;
                           rxq.slot  = NULL;
                        }
                     }
                     else {
                        // store message octet
                        buf[rxq.count] = raw[i];
                        // buffer length exceeded, drop the entire message
                        if (++rxq.count > UART_PIPELEN_UINT8) {
                           rxq.state = UART_RX_IDLE;
                           rxq.count = 0;
                           m_dropcnt++;
                           if (rxq.slot != NULL) {
                              cm_free((pcm_msg_t)rxq.slot->buf);
                              rxq.slot = NULL;
                           }
                        }
                     }
                     break;
                  case UART_RX_MSG_ESC :
                     // unstuff octet
                     buf[rxq.count] = raw[i] ^ UART_STUFFED_BIT;
                     // buffer length exceeded, drop the entire message
                     if (++rxq.count > UART_PIPELEN_UINT8) {
                        rxq.state = UART_RX_IDLE;
                        rxq.count = 0;
                        m_dropcnt++;
                        if (rxq.slot != NULL) {
                           cm_free((pcm_msg_t)rxq.slot->buf);
                        }
                     }
                     else {
                        rxq.state = UART_RX_MSG;
                     }
                     break;
                  case UART_RX_PIPE :
                     // unstuff next octet
                     if (raw[i] == UART_ESCAPE) {
                        rxq.state = UART_RX_PIPE_ESC;
                     }
                     // end-of-frame
                     else if (raw[i] == UART_END_FRAME) {
                        pipe = (pcm_pipe_daq_t)m_nxt_pipe;
                        // packet arrival
                        pipe->stamp_us = (uint32_t)m_timer.GetElapsedAsMicroSeconds();
                        // next slot in circular buffer
                        if (++m_head == UART_POOL_SLOTS) m_head = 0;
                        m_nxt_pipe = m_pool + (m_head * UART_PIPELEN_UINT8);
                        // last packet in block?
                        if (++m_pktcnt == UART_PACKET_CNT) {
                           m_pktcnt = 0;
                           // send pipe message
                           cm_pipe_send((pcm_pipe_t)m_blk_pipe, UART_BLOCK_LEN);
                           // record next start of block
                           m_blk_pipe = m_nxt_pipe;
                        }
                        rxq.state = UART_RX_IDLE;
                        rxq.count = 0;
                     }
                     else {
                        // store pipe octet
                        m_nxt_pipe[rxq.count] = raw[i];
                        // buffer length exceeded, drop the entire message
                        if (++rxq.count > UART_PIPELEN_UINT8) {
                           rxq.state = UART_RX_IDLE;
                           rxq.count = 0;
                           m_dropcnt++;
                        }
                     }
                     break;
                  case UART_RX_PIPE_ESC :
                     // unstuff octet
                     m_nxt_pipe[rxq.count] = raw[i] ^ UART_STUFFED_BIT;
                     // buffer length exceeded, drop the entire message
                     if (++rxq.count > UART_PIPELEN_UINT8) {
                        rxq.state = UART_RX_IDLE;
                        rxq.count = 0;
                        m_dropcnt++;
                     }
                     else {
                        rxq.state = UART_RX_PIPE;
                     }
                     break;

               }
            }
         }
      }
   }

   return 0;

} // end uart_thread()


// ===========================================================================

// 7.4

UART_API void uart_tx(pcm_msg_t msg) {

/* 7.4.1   Functional Description

   This routine will transmit the message. The tx_mutex is used to prevent
   mulitple threads from interferring with a single message transfer. HDLC
   coding is used to form a message packet.

   7.4.2   Parameters:

   msg     CM message to send.

   7.4.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint8_t    *out = (uint8_t *)msg;
   uint8_t    *buf;
   uint16_t    i, j = 0;
   DWORD       bytes_left, bytes_sent;
   uint8_t     retry = 0;

// 7.4.5   Code

   // Enter Critical Section
   EnterCriticalSection(&m_tx_mutex);

   buf = (uint8_t *)calloc(UART_MSGLEN_UINT8 * 2, sizeof(uint8_t));

   //
   // HDLC ENCODING
   //
   buf[j++] = UART_START_FRAME;
   for (i=0;i<msg->h.msglen;i++) {
      if (out[i] == UART_START_FRAME || out[i] == UART_END_FRAME || out[i] == UART_ESCAPE) {
         buf[j++] = UART_ESCAPE;
         buf[j++] = out[i] ^ UART_STUFFED_BIT;
      }
      else {
         buf[j++] = out[i];
      }
   }
   buf[j++] = UART_END_FRAME;

   bytes_left = j;
   FT_Write(m_uart, buf, bytes_left, &bytes_sent);
   bytes_left -= bytes_sent;
   //retry
   while (bytes_left != 0 && retry < UART_RETRIES) {
      Sleep(1);
      FT_Write(m_uart, &buf[j - bytes_left], bytes_left, &bytes_sent);
      bytes_left -= bytes_sent;
      retry++;
   }

   // release message
   cm_free(msg);

   free(buf);

   // Leave Critical Section
   LeaveCriticalSection(&m_tx_mutex);

} // end uart_tx()


// ===========================================================================

// 7.5

UART_API void uart_cmio(uint8_t op_code, pcm_msg_t msg) {

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
   uart_tx(msg);

} // end uart_cmio()


// ===========================================================================

// 7.6

UART_API void uart_head(void) {

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
   m_pktcnt    = 0;

} // end uart_head()


// ===========================================================================

// 7.7

UART_API void uart_rev(uint32_t *librev, uint32_t *sysrev, uint32_t *apirev) {

/* 7.7.1   Functional Description

   This routine will return the build versions for uart.dll, ftd2xx.sys
   and ftd2xx.lib.

   7.7.2   Parameters:

   librev   UART ftd2xx.lib revision
   sysrev   UART ftd2xx.sys revision
   apirev   uart.dll revision

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

}  // end uart_rev()


// ===========================================================================

// 7.8

UART_API void uart_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat) {

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

}  // end uart_sysid()


// ===========================================================================

// 7.9

UART_API void uart_final(void) {

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

   // Close UART
   FT_Close(m_uart);

   // Release Memory
   if (m_pool != NULL) free(m_pool);

   m_pool = NULL;

} // end uart_final()

