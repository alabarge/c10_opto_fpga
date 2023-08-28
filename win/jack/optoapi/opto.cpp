/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      FTDI OPTO D2XX API Dynamic Link Library

   1.2 Functional Description

      This module provides an API to access the OPTO D2XX Driver.

            NOTE: IN ORDER TO USE THIS DLL THE FTDI USB DEVICE MUST HAVE A SERIAL
            NUMBER THAT BEGINS WITH "O1".

   1.3 Specification/Design Reference

      OPTOAPI_DLL.docx

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
      7.1   opto_query()
      7.2   opto_init()
      7.3   opto_thread()
      7.4   opto_tx()
      7.5   opto_cmio()
      7.6   opto_head()
      7.7   opto_sysid()
      7.8   opto_rev()
      7.9   opto_final()

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
#include "opto.h"
#include "optoapi.h"
#include "ftd2xx.h"
#include "timer.h"
#include "trace.h"

#include "build.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define  OPTO_RESP      64
   #define  OPTO_RETRIES   4

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static   DWORD WINAPI      opto_thread(LPVOID data);

// 6.2  Local Data Structures

   static   uint8_t           m_cm_port  = CM_PORT_NONE;
   static   uint8_t           m_com_port = 0;

   static   FT_DEVICE_LIST_INFO_NODE m_devinfo[OPTO_MAX_DEVICES] = {0};

   static   CRITICAL_SECTION  m_tx_mutex;
   static   HANDLE            m_thread;
   static   DWORD             m_tid;
   static   HANDLE            m_event = NULL;

   static   uint8_t          *m_pool = NULL;
   static   uint8_t          *m_nxt_pipe = NULL;
   static   uint8_t          *m_blk_pipe = NULL;
   static   uint8_t           m_end_thread = FALSE;
   static   uint32_t          m_pktcnt = 0;
   static   uint32_t          m_head = 0;
   static   CHRTimer          m_timer;
   static   uint32_t          m_devcnt = 0;

   static   uint32_t          m_sysid, m_stamp, m_cmdat;
   static   uint8_t           m_devid, m_numobjs, m_numcons;
   static   DWORD             m_librev, m_sysrev;

   static   HANDLE            m_opto = NULL;

   static   UCHAR             m_query[] = {0x7E, 0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00, 0x7D};

// 7 MODULE CODE

// ===========================================================================

// 7.1

OPTO_API uint32_t opto_query(opto_dev_info_t **devinfo) {

/* 7.1.1   Functional Description

   This routine will query the FTD2XX driver to acquire all the OPTO devices
   attached to the system.

   7.1.2   Parameters:

   devcnt   Pointer to the attached device count
   devinfo  Pointer to the popto_dev_info_t strucure

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
   if (m_devcnt <= OPTO_MAX_DEVICES) {
      status = FT_GetDeviceInfoList((FT_DEVICE_LIST_INFO_NODE *)m_devinfo, &devices);
      if (status == FT_OK) m_devcnt = (uint32_t)devices;
   }

   *devinfo = (opto_dev_info_t *)m_devinfo;

   return m_devcnt;

}  // end opto_query()


// ===========================================================================

// 7.2

OPTO_API int32_t opto_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port) {

/* 7.2.1   Functional Description

   This routine will open access to the OPTO device (VID=0x0403,PID=0x6001).

   NOTE: Only a single device is currently supported.

   7.2.2   Parameters:

   baudrate Baudrate in bps
   cm_port  CM Port Identifier
   com_port FTDI Port

   7.2.3   Return Values:

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    result = OPTO_OK;
   FT_STATUS   status;
   DWORD       sent, recv;
   UCHAR       resp[OPTO_RESP], msg[OPTO_RESP];
   uint8_t     retry = 0;
   UINT        i,j,k;

// 7.2.5   Code

   // close OPTO if opened
   if (m_opto != NULL) FT_Close(m_opto);
   m_opto = NULL;

   // Update FTDI COM Port
   m_com_port = com_port;

   // Okay to Go
   if (m_devcnt != 0) {
      // check for valid OPTO interface
      for (i=0;i<m_devcnt;i++) {
         if (m_devinfo[i].SerialNumber[0] == 'O' && m_devinfo[i].SerialNumber[1] == '1') {
            // Open Selected Available port
            if (m_com_port == i) {
               status = FT_OpenEx((PVOID)m_devinfo[i].SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &m_opto);
               // Configure Device characteristics
               if (status == FT_OK) {
                  status |= FT_ResetDevice(m_opto);
                  status |= FT_Purge(m_opto, FT_PURGE_RX | FT_PURGE_TX);
                  status |= FT_ResetDevice(m_opto);
                  status |= FT_SetUSBParameters(m_opto, 32768, 32768);
                  status |= FT_SetChars(m_opto, FALSE, 0, FALSE, 0);
                  status |= FT_SetLatencyTimer(m_opto, OPTO_LATENCY);
                  status |= FT_SetTimeouts(m_opto, OPTO_RX_TIMEOUT, OPTO_TX_TIMEOUT);
                  // This order seems backwards, but it works!
                  status |= FT_SetBitMode(m_opto, 0x00, 0x10);
                  status |= FT_SetBitMode(m_opto, 0x00, 0x00);
                  // Check return status from OPTO
                  if (status != FT_OK) result |= OPTO_ERR_OPEN;
               }
               break;
            }
         }
      }
      if (m_devcnt == i) result = OPTO_ERR_DEV_CNT;
   }

   // Device Opened
   if (result == OPTO_OK && m_opto != NULL) {

      // Empty the TX and RX Queues
      status |= FT_Purge(m_opto, FT_PURGE_RX | FT_PURGE_TX);
      status |= FT_Read(m_opto, resp, OPTO_RESP, &recv);
      status |= FT_Read(m_opto, resp, OPTO_RESP, &recv);
      status |= FT_Read(m_opto, resp, OPTO_RESP, &recv);

      // Issue CM_QUERY_REQ multiple tries
      while (retry < OPTO_RETRIES) {

         result |= OPTO_ERR_RESP;

         // Send CM_QUERY_REQ to validate connection
         cm_crc((pcm_msg_t)&m_query[1], CM_CALC_CRC);
         status |= FT_Write(m_opto, m_query, sizeof(m_query), &sent);

         // If sent OK then check response
         if (status == FT_OK && sent == sizeof(m_query)) {
            // Allow time for Response
            Sleep(20);
            // Read the Port
            status = FT_Read(m_opto, resp, OPTO_RESP, &recv);
            // Check Response
            if (status == FT_OK && recv != 0) {
               // Check SOF & EOF
               if (resp[0] == OPTO_SOF && resp[recv-1] == OPTO_EOF) {
                  // Remove HDLC Coding
                  for (j=1,k=0;j<(recv-1);j++) {
                     if (resp[j] == OPTO_ESC) {
                        msg[k++] = resp[j+1] ^ OPTO_BIT;
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
                     FT_Purge(m_opto, FT_PURGE_RX | FT_PURGE_TX);
                     // Record SysID
                     m_sysid = (msg[19] << 24) | (msg[18] << 16) | (msg[17] << 8) | msg[16];
                     // Record Timestamp
                     m_stamp = (msg[23] << 24) | (msg[22] << 16) | (msg[21] << 8) | msg[20];
                     // Record Device ID, etc.
                     m_devid    = msg[24];
                     m_numobjs  = msg[25];
                     m_numcons  = msg[26];
                     m_cmdat    = (m_devid << 24) | (m_numobjs << 16) | (m_numcons << 8);
                     result = OPTO_OK;
                     break;
                  }
               }
            }
         }
         retry++;
      }
   }

   // OK to Continue
   if (result == OPTO_OK && m_opto != NULL) {

      FT_GetLibraryVersion(&m_librev);
      FT_GetDriverVersion(m_opto, &m_sysrev);

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
      cm_ioreg(opto_cmio, m_cm_port, CM_MEDIA_OPTO);

      // Allocate Pipe Message Pool
      m_pool  = (uint8_t *)malloc(OPTO_POOL_SLOTS * OPTO_PIPELEN_UINT8);
      if (m_pool == NULL) result = OPTO_ERR_POOL;

      // Start the H/W Receive Thread
      m_end_thread = FALSE;
      if ((m_thread = CreateThread(NULL, 0, opto_thread, NULL, 0, &m_tid)) == NULL) {
         result = OPTO_ERR_THREAD;
      }
      else {
         SetThreadPriority(&m_tid, THREAD_PRIORITY_NORMAL);
      }

      // Reset Hi-Res Timer
      m_timer.Start();

   }
   else {
      FT_Close(m_opto);
   }

   return result;

}  // end opto_init()


// ===========================================================================

// 7.3

static DWORD WINAPI opto_thread(LPVOID data) {


/* 7.3.1   Functional Description

   This thread will service the incoming characters from the OPTO serial
   interface.

   7.3.2   Parameters:

   data     Thread parameters

   7.3.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   DWORD       rx_bytes, tx_bytes, event, i, j, k;
   uint8_t     raw[OPTO_PIPELEN_UINT8 * 2];
   uint32_t    buf[OPTO_PIPELEN_UINT8 >> 2];
   uint8_t     *buf_u8 = (uint8_t *)buf;
   uint8_t     esc = 0, newmsg = 0;
   FT_STATUS   status;
   uint8_t     type = OPTO_EPID_NONE;
   uint8_t     slotid;
   uint32_t    head = 0;
   pcmq_t      slot;
   pcm_msg_t   msg;

   pcm_pipe_daq_t  pipe;

// 7.3.5   Code

   FT_Purge(m_opto, FT_PURGE_RX | FT_PURGE_TX);
   FT_SetEventNotification(m_opto, FT_EVENT_RXCHAR, m_event);

   // beginning of PIPE message circular buffer
   m_nxt_pipe  = m_pool;
   m_blk_pipe  = m_pool;
   m_head      = 0;

   // clear staging buffer
   memset(buf, 0, sizeof(buf));

   while (m_end_thread == FALSE) {
      FT_GetStatus(m_opto, &rx_bytes, &tx_bytes, &event);
      if (rx_bytes == 0) WaitForSingleObject(m_event, INFINITE);
      FT_GetStatus(m_opto, &rx_bytes, &tx_bytes, &event);
      if (rx_bytes != 0 && m_end_thread == FALSE) {
         // prevent buffer overflow, account for encoding
         rx_bytes = (rx_bytes > (OPTO_PIPELEN_UINT8 * 2)) ? OPTO_PIPELEN_UINT8 * 2 : rx_bytes;
         memset(raw, 0, sizeof(raw));
         status = FT_Read(m_opto, raw, rx_bytes, &rx_bytes);
         if ((status == FT_OK) && (rx_bytes > 0) && (m_end_thread != TRUE)) {
            for (i=0;i<rx_bytes;i++) {
               //
               // HDLC END-OF-FILE
               //
               if (raw[i] == OPTO_EOF && newmsg == 1) {
                  //
                  // CONTROL MESSAGE
                  //
                  if (type == OPTO_EPID_CTL) {
                     slot = cm_alloc();
                     if (slot != NULL) {
                        msg = (pcm_msg_t)slot->buf;
                        // preserve slotid
                        slotid = msg->h.slot;
                        // uint32_t boundary, copy multiple of 32-bits
                        // always read CM header + parms in order
                        // to determine message length
                        for (j=0;j<sizeof(cm_msg_t) >> 2;j++) {
                           slot->buf[j] = buf[j];
                        }
                        slot->msglen = msg->h.msglen;
                        // read rest of CM message body, uint32_t per cycle
                        if (slot->msglen > sizeof(cm_msg_t) && (slot->msglen <= OPTO_MSGLEN_UINT8)) {
                           for (j=0;j<(slot->msglen + 3 - sizeof(cm_msg_t)) >> 2;j++) {
                              slot->buf[j + (sizeof(cm_msg_t) >> 2)] =
                                    buf[j + (sizeof(cm_msg_t) >> 2)];
                           }
                        }
                        // restore slotid
                        msg->h.slot = slotid;
                        // clear staging buffer
                        memset(buf, 0, sizeof(buf));
                        // validate message length
                        if (msg->h.msglen >= sizeof(cm_msg_t) &&
                            msg->h.msglen <= CM_MAX_MSG_INT8U) {
                           // queue the message
                           cm_qmsg((pcm_msg_t)slot->buf);
                        }
                     }
                  }
                  //
                  // PIPE MESSAGE
                  //
                  else if (type == OPTO_EPID_PIPE) {
                     pipe = (pcm_pipe_daq_t)m_nxt_pipe;
                     // packet arrival
                     pipe->stamp_us = (uint32_t)m_timer.GetElapsedAsMicroSeconds();
                     // next slot in circular buffer
                     if (++m_head == OPTO_POOL_SLOTS) m_head = 0;
                     m_nxt_pipe = m_pool + (m_head * OPTO_PIPELEN_UINT8);
                     // last packet in block?
                     if (++m_pktcnt == OPTO_PACKET_CNT) {
                        m_pktcnt = 0;
                        // send pipe message
                        cm_pipe_send((pcm_pipe_t)m_blk_pipe, OPTO_BLOCK_LEN);
                        // record next start of block
                        m_blk_pipe = m_nxt_pipe;
                     }
                  }
                  newmsg = 0;
               }
               //
               // HDLC SOF
               //
               else if (raw[i] == OPTO_SOF) {
                  k      = 0;
                  esc    = 0;
                  newmsg = 1;
                  type   = OPTO_EPID_NEXT;
               }
               //
               // CTL OR PIPE
               //
               else if (type == OPTO_EPID_NEXT) {
                  type = (raw[i] == CM_ID_PIPE) ? OPTO_EPID_PIPE : OPTO_EPID_CTL;
                  // store cmid
                  if (type == OPTO_EPID_PIPE)
                     m_nxt_pipe[k++] = raw[i];
                  else
                     buf_u8[k++] = raw[i];
               }
               //
               // HDLC ESC
               //
               else if (raw[i] == OPTO_ESC) {
                  // unstuff next byte
                  esc = 1;
               }
               //
               // HDLC UNSTUFF OCTET
               //
               else if (esc == 1) {
                  esc = 0;
                  // unstuff byte
                  if (type == OPTO_EPID_PIPE)
                     m_nxt_pipe[k++] = raw[i] ^ OPTO_BIT;
                  else
                     buf_u8[k++] = raw[i] ^ OPTO_BIT;
               }
               //
               // NORMAL INCOMING OCTET
               //
               else {
                  if (type == OPTO_EPID_PIPE)
                     m_nxt_pipe[k++] = raw[i];
                  else
                     buf_u8[k++] = raw[i];
               }
            }
         }
      }
   }

   return 0;

} // end opto_thread()


// ===========================================================================

// 7.4

OPTO_API void opto_tx(pcm_msg_t msg) {

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

   buf = (uint8_t *)calloc(OPTO_MSGLEN_UINT8 * 2, sizeof(uint8_t));

   buf[j++] = OPTO_SOF;
   for (i=0;i<msg->h.msglen;i++) {
      if (out[i] == OPTO_SOF || out[i] == OPTO_EOF || out[i] == OPTO_ESC) {
         buf[j++] = OPTO_ESC;
         buf[j++] = out[i] ^ OPTO_BIT;
      }
      else {
         buf[j++] = out[i];
      }
   }
   buf[j++] = OPTO_EOF;
   bytes_left = j;
   FT_Write(m_opto, buf, bytes_left, &bytes_sent);
   bytes_left -= bytes_sent;
   //retry
   while (bytes_left != 0 && retry < OPTO_RETRIES) {
      Sleep(1);
      FT_Write(m_opto, &buf[j - bytes_left], bytes_left, &bytes_sent);
      bytes_left -= bytes_sent;
      retry++;
   }

   // release message
   cm_free(msg);

   free(buf);

   // Leave Critical Section
   LeaveCriticalSection(&m_tx_mutex);

} // end opto_tx()


// ===========================================================================

// 7.5

OPTO_API void opto_cmio(uint8_t op_code, pcm_msg_t msg) {

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
   opto_tx(msg);

} // end opto_cmio()


// ===========================================================================

// 7.6

OPTO_API void opto_head(void) {

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

} // end opto_head()


// ===========================================================================

// 7.7

OPTO_API void opto_rev(uint32_t *librev, uint32_t *sysrev, uint32_t *apirev) {

/* 7.7.1   Functional Description

   This routine will return the build versions for opto.dll, ftd2xx.sys
   and ftd2xx.lib.

   7.7.2   Parameters:

   librev   OPTO ftd2xx.lib revision
   sysrev   OPTO ftd2xx.sys revision
   apirev   opto.dll revision

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

}  // end opto_rev()


// ===========================================================================

// 7.8

OPTO_API void opto_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat) {

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

}  // end opto_sysid()


// ===========================================================================

// 7.9

OPTO_API void opto_final(void) {

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

   // Close OPTO
   FT_Close(m_opto);

   // Release Memory
   if (m_pool != NULL) free(m_pool);

   m_pool = NULL;

} // end opto_final()

