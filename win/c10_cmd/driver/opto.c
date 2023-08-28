/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Fast Serial OPTO I/O Driver

   1.2 Functional Description

      The OPTO I/O Interface routines are contained in this module.

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
         7.1   opto_init()
         7.2   opto_thread()
         7.3   opto_tx()
         7.4   opto_cmio()
         7.5   opto_head()
         7.6   opto_final()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"
#include "ftd2xx.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define OPTO_RESP          64
   #define OPTO_RETRIES       4

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static   DWORD WINAPI      opto_thread(LPVOID data);

// 6.2  Local Data Structures

   static   uint8_t           m_cm_port  = CM_PORT_NONE;
   static   uint8_t           m_com_port = 0;

   static   CRITICAL_SECTION  m_tx_mutex;
   static   HANDLE            m_thread;
   static   DWORD             m_tid;
   static   HANDLE            m_event;
   static   uint8_t          *m_pool = NULL;
   static   uint8_t          *m_nxt_pipe = NULL;
   static   uint8_t          *m_blk_pipe = NULL;
   static   uint8_t           m_end_thread = FALSE;
   static   uint32_t          m_pktcnt = 0;
   static   uint32_t          m_head = 0;


   static   uint32_t          m_sysid, m_stamp, m_cmdat;
   static   uint8_t           m_devid, m_numobjs, m_numcons;
   static   DWORD             m_librev, m_sysrev;

   static   HANDLE            m_opto = NULL;

   static   UCHAR             m_query[] = {0x7E, 0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00, 0x7D};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t opto_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port) {

/* 7.1.1   Functional Description

   The OPTO Interface is initialized in this routine.

   7.1.2   Parameters:

   baudrate  Serial Baud Rate
   cm_port   CM Port
   com_port  FTDI COM Port from FT_GetDeviceInfoList()

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    result = OPTO_OK;
   FT_STATUS   status;
   DWORD       dev_cnt, sent, recv;
   UCHAR       resp[OPTO_RESP], msg[OPTO_RESP];
   uint8_t     retry = 0;
   UINT        i,j,k;

   FT_DEVICE_LIST_INFO_NODE dev_info[OPTO_MAX_DEVICES];

// 7.1.5   Code

   // close OPTO if opened
   if (m_opto != NULL) FT_Close(m_opto);
   m_opto = NULL;

   // Update FTDI COM Port
   m_com_port = com_port;

   //
   // Open the Available Selected FTDI device
   //
   if (FT_CreateDeviceInfoList(&dev_cnt) == FT_OK) {
      if (dev_cnt <= OPTO_MAX_DEVICES) {
         // fill-out device info
         if (FT_GetDeviceInfoList(dev_info, &dev_cnt) != FT_OK) {
            result = OPTO_ERR_INFO;
         }
      }
      else {
         result = OPTO_ERR_DEV_CNT;
      }
   }
   else {
      result = OPTO_ERR_DEV;
   }

   // Okay to Go
   if (result == OPTO_OK) {
      printf("\nopto_init() selected port = %d\n", m_com_port);
      // check for valid OPTO interface
      for (i=0;i<dev_cnt;i++) {
         if (dev_info[i].SerialNumber[0] == 'O' && dev_info[i].SerialNumber[1] == '1') {
            if (gc.trace & WIN_TRACE_UART) {
               printf("\nopto_init() OPTO.%d :\n", i);
               printf("  flags  : %08X\n", dev_info[i].Flags);
               printf("  type   : %08X\n", dev_info[i].Type);
               printf("  id     : %08X\n", dev_info[i].ID);
               printf("  locid  : %08X\n", dev_info[i].LocId);
               printf("  serial : %s\n", dev_info[i].SerialNumber);
               printf("  desc   : %s\n\n", dev_info[i].Description);
            }

            // Open Selected Available port
            if (m_com_port == i) {
               status = FT_OpenEx((PVOID)dev_info[i].SerialNumber, FT_OPEN_BY_SERIAL_NUMBER, &m_opto);
               // Configure Device characteristics
               if (status == FT_OK) {
                  status |= FT_ResetDevice(m_opto);
                  status |= FT_Purge(m_opto, FT_PURGE_RX | FT_PURGE_TX);
                  status |= FT_ResetDevice(m_opto);
                  status |= FT_SetUSBParameters(m_opto, 65536, 65535);
                  status |= FT_SetChars(m_opto, FALSE, 0, FALSE, 0);
                  status |= FT_SetLatencyTimer(m_opto, 2);
                  status |= FT_SetTimeouts(m_opto, OPTO_RX_TIMEOUT, OPTO_TX_TIMEOUT);
                  // This order seems backwards, but it works!
                  status |= FT_SetBitMode(m_opto, 0x00, 0x10);
                  status |= FT_SetBitMode(m_opto, 0x00, 0x00);
                  // Check return status from OPTO
                  if (status != FT_OK) result |= OPTO_ERR_OPEN;
               }
               else
                  result = OPTO_ERR_OPEN;
               break;
            }
         }
      }
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
         // report message content
         if ((gc.trace & WIN_TRACE_UART)) {
            printf("opto_init() tx msglen = %d\n", (int)sizeof(m_query));
            dump((uint8_t *)m_query, sizeof(m_query), 0, 0);
         }
         // If sent OK then check response
         if (status == FT_OK && sent == sizeof(m_query)) {
            // Allow time for Response
            Sleep(50);
            // Read the Port
            status = FT_Read(m_opto, resp, OPTO_RESP, &recv);
            // report message content
            if ((gc.trace & WIN_TRACE_UART)) {
               printf("opto_init() rx msglen = %d\n", recv);
               dump((uint8_t *)resp, recv, 0, 0);
            }
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
         m_event = CreateEvent(NULL, FALSE, FALSE, "");
      else
         ResetEvent(m_event);

      // Init the Mutex's
      InitializeCriticalSection (&m_tx_mutex);

      // Update CM Port
      m_cm_port = cm_port;

      // Register the I/O Interface callback for CM
      cm_ioreg(opto_cmio, m_cm_port, CM_MEDIA_OPTO);

      // Allocate Pipe Message Pool
      m_pool  = malloc(OPTO_PIPE_SLOTS * OPTO_PIPELEN_UINT8);

      if (m_pool == NULL) {
         result = OPTO_ERR_POOL;
      }

      // Start the H/W Receive Thread
      m_end_thread = FALSE;
      if ((m_thread = CreateThread(NULL, 0, opto_thread, NULL, 0, &m_tid)) == NULL) {
         result = CFG_ERROR_OPTO;
      }
      else {
         SetThreadPriority(&m_tid, THREAD_PRIORITY_NORMAL);
      }

       // Print Hardware Version to Serial Port
      if (gc.trace & WIN_TRACE_ID) {
         printf("Opened OPTO.%d (%s) for Messaging\n", m_com_port, dev_info[i].SerialNumber);
         printf("OPTO.%d : ftd2xx.lib:ftd2xx.sys = %08X:%08X\n", m_com_port, m_librev, m_sysrev);
         printf("OPTO.%d : sysid:stamp:cm = %d:%d:%08X\n\n", m_com_port, m_sysid, m_stamp, m_cmdat);
      }
   }

   if ((gc.trace & WIN_TRACE_ERROR) && result != OPTO_OK) {
      printf("opto_init() Error : %08X\n", result);
   }

   return (result == OPTO_OK) ? WIN_ERROR_OK : WIN_ERROR_OPTO;

}  // end opto_init()


// ===========================================================================

// 7.2

static DWORD WINAPI opto_thread(LPVOID data) {


/* 7.2.1   Functional Description

   This thread will service the incoming characters from the OPTO serial
   interface.

   7.2.2   Parameters:

   data     Thread parameters

   7.2.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

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

// 7.2.5   Code

   if (gc.trace & WIN_TRACE_ID) {
      printf("opto_thread() started, tid 0x%X\n", m_tid);
   }

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
                        if (msg->h.msglen >= sizeof(cm_msg_t) &&
                            msg->h.msglen <= CM_MAX_MSG_INT8U) {
                           // report message content
                           if ((gc.trace & WIN_TRACE_UART) && (slot != NULL)) {
                              printf("opto_thread() msglen = %d\n", msg->h.msglen);
                              dump((uint8_t *)slot->buf, slot->msglen, LIB_ASCII, 0);
                           }
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
                     pipe->stamp_us = (uint32_t)tmr_get_elapsed_microseconds();
                     // next slot in circular buffer
                     if (++m_head == OPTO_PIPE_SLOTS) m_head = 0;
                     m_nxt_pipe = m_pool + (m_head * OPTO_PIPELEN_UINT8);
                     // last packet in block?
                     if (++m_pktcnt == OPTO_PACKET_CNT) {
                        m_pktcnt = 0;
                        // report partial pipe content
                        if (gc.trace & WIN_TRACE_PIPE) {
                           printf("opto_thread() pipelen = %d\n", OPTO_BLOCK_LEN);
                           dump(m_blk_pipe, 32, LIB_ASCII, 0);
                        }
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

// 7.3

void opto_tx(pcm_msg_t msg) {

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
      printf("opto_tx() srvid:msgid:msglen = %02X:%02X:%04X\n",
               msg->p.srvid, msg->p.msgid, msg->h.msglen);
      dump((uint8_t *)msg, msg->h.msglen, LIB_ASCII, 0);
   }

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
      Sleep(10);
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

// 7.4

void opto_cmio(uint8_t op_code, pcm_msg_t msg) {

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
      printf("opto_cmio() op_code = %02X\n", op_code);
   }

   // transmit message
   opto_tx(msg);

} // end opto_cmio()


// ===========================================================================

// 7.5

void opto_head(void) {

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

} // end opto_head()


// ===========================================================================

// 7.6

void opto_final(void) {

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
   SetEvent(m_event);

   // Close OPTO
   FT_Close(m_opto);

   // Release Memory
   free(m_pool);

} // end opto_final()




