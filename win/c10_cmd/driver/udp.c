/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      UDP API for LAN8270 PHY using Windows Sockets.

   1.2 Functional Description

      This module provides an API to access the LAN8270 using standard
      protocol stacks.

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
         7.1   udp_init()
         7.2   udp_thread()
         7.3   udp_tx()
         7.4   udp_cmio()
         7.5   udp_head()
         7.6   udp_final()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"
#include "lanpkt.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define UDP_RETRIES       4

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static   DWORD WINAPI      udp_thread(LPVOID data);

// 6.2  Local Data Structures

   static   uint8_t           m_cm_port  = CM_PORT_NONE;
   static   uint8_t           m_com_port = 0;

   static   uint16_t          m_seqid;
   static   SOCKET            m_sock = -1;
   static   struct sockaddr_in m_dst = {0};
   static   int32_t           m_slen = sizeof(struct sockaddr_in);
   static   char              m_daddr[32];
   static   uint32_t          m_udp_port;

   static   uint8_t           m_txbuf[UDP_PIPE_MSG_LEN] = {0};
   static   uint8_t           m_rxbuf[UDP_PIPE_MSG_LEN] = {0};

   static   HANDLE            m_thread;
   static   DWORD             m_tid;
   static   HANDLE            m_event = NULL;
   static   uint8_t          *m_pool = NULL;
   static   uint8_t          *m_nxt_pipe = NULL;
   static   uint8_t          *m_blk_pipe = NULL;
   static   uint8_t           m_end_thread = FALSE;
   static   uint32_t          m_frmcnt = 0;
   static   uint32_t          m_head = 0;

   static   CRITICAL_SECTION  m_tx_mutex;

   static   uint32_t          m_sysid, m_stamp, m_cmdat;
   static   uint8_t           m_devid, m_numobjs, m_numcons;
   static   uint32_t          m_sysrev = 0;
   static   char              m_librev[256];

   static   uint8_t           m_query[] = {0x00, 0x00, 0xAA, 0x55, 0x00, 0x00,
                                           0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t udp_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port,
                  uint16_t cm_udp_port, uint8_t *macip) {

/* 7.1.1   Functional Description

   The UDP Interface is initialized in this routine.

   7.1.2   Parameters:

   baudrate  Serial Baud Rate
   cm_port   CM Port
   com_port  unused

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    result = UDP_OK;
   uint8_t     retry = 0;
   int32_t     rxbytes, txbytes;

   WSADATA     data;

// 7.1.5   Code

   // close the socket
   closesocket(m_sock);

   // startup winsock
   if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
      result |=UDP_ERR_WSA;
   }

   // destination IP address
   sprintf_s(m_daddr, sizeof(m_daddr), "%d.%d.%d.%d",
      macip[16], macip[17], macip[18], macip[19]);

   // remote UDP port
   m_udp_port = cm_udp_port;

   //setup remote socket address
   memset((char *)&m_dst, 0, sizeof(m_dst));
   m_dst.sin_family = AF_INET;
   m_dst.sin_addr.S_un.S_addr = inet_addr(m_daddr);
   m_dst.sin_port = htons(m_udp_port);

   // create socket, client side
   if ( (m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
      result |= UDP_ERR_SOCKET;
   }
   else {
      // set recvfrom() timout
      DWORD timeout = UDP_RECV_TIMEOUT;
      setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
   }

   // Clear the TX & RX buffers
   memset(m_txbuf, 0, UDP_PIPE_MSG_LEN);
   memset(m_rxbuf, 0, UDP_PIPE_MSG_LEN);

   // Check return status
   if (m_sock < 0) result |= UDP_ERR_OPEN;

   // OK TO GO
   if (result == UDP_OK) {

      // Issue CM_QUERY_REQ multiple tries
      while (retry < UDP_RETRIES) {

         result |= UDP_ERR_RESP;

         // Send CM_QUERY_REQ to validate connection
         memcpy(m_txbuf, m_query, sizeof(m_query));
         txbytes = sendto(m_sock, (char *)m_txbuf, UDP_CTL_MSG_LEN, 0, (struct sockaddr *)&m_dst, m_slen);

         // If sent OK then check response
         if (txbytes == UDP_CTL_MSG_LEN) {
            // Allow time for response
            Sleep(50);
            // Read the Port
            rxbytes = recvfrom(m_sock, (char *)m_rxbuf, sizeof(m_rxbuf), 0, (struct sockaddr *)&m_dst, &m_slen);
            // Check Response
            if (rxbytes == UDP_CTL_MSG_LEN) {
               // Verify Magic Number
               if (m_rxbuf[18] == 0x34 && m_rxbuf[19] == 0x12 &&
                   m_rxbuf[20] == 0xAA && m_rxbuf[21] == 0x55) {
                  // Record SysID
                  m_sysid = (m_rxbuf[25] << 24) | (m_rxbuf[24] << 16) |
                            (m_rxbuf[23] << 8)  | m_rxbuf[22];
                  // Record Timestamp
                  m_stamp = (m_rxbuf[29] << 24) | (m_rxbuf[28] << 16) |
                            (m_rxbuf[27] << 8)  | m_rxbuf[26];
                  // Record Device ID, etc.
                  m_devid    = m_rxbuf[30];
                  m_numobjs  = m_rxbuf[31];
                  m_numcons  = m_rxbuf[32];
                  m_cmdat    = (m_devid << 24) | (m_numobjs << 16) | (m_numcons << 8);
                  result = UDP_OK;
                  break;
               }
            }
         }
         retry++;
      }
   }

   // OK to Continue
   if (result == UDP_OK) {

      // Init the Mutex
      InitializeCriticalSection (&m_tx_mutex);

      // Update CM Port
      m_cm_port = cm_port;

      // Register the I/O Interface callback for CM
      cm_ioreg(udp_cmio, m_cm_port, CM_MEDIA_LAN);

      // Allocate Pipe Message Pool
      m_pool  = (uint8_t *)malloc(UDP_PIPE_POOL);
      if (m_pool == NULL) result = UDP_ERR_POOL;

      // Start the H/W Receive Thread
      m_end_thread = FALSE;
      if ((m_thread = CreateThread(NULL, 0, udp_thread, NULL, 0, &m_tid)) == NULL) {
         result = UDP_ERR_THREAD;
      }
      else {
         SetThreadPriority(&m_tid, THREAD_PRIORITY_NORMAL);
      }

       // Report Hardware Version
      if (gc.trace & WIN_TRACE_ID) {
         printf("Opened UDP.0 for Messaging, Connecting to %s @ %d\n", m_daddr, cm_udp_port);
         printf("UDP.%d : sysid:stamp:cm = %d:%d:%08X\n\n", m_com_port, m_sysid, m_stamp, m_cmdat);
      }
   }
   // close the port
   else {
      closesocket(m_sock);
      m_sock = -1;
   }

   if ((gc.trace & WIN_TRACE_ERROR) && result != UDP_OK) {
      printf("udp_init() Error : %08X\n", result);
   }

   return (result == UDP_OK) ? WIN_ERROR_OK : WIN_ERROR_UDP;

}  // end udp_init()


// ===========================================================================

// 7.2

static DWORD WINAPI udp_thread(LPVOID data) {


/* 7.2.1   Functional Description

   This thread will service the incoming characters from the UDP serial
   interface.

   7.2.2   Parameters:

   data     Thread parameters

   7.2.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint8_t     slotid;
   uint32_t   *buf;
   uint16_t    msglen;
   uint16_t    frm_cnt = 0;
   uint32_t    j;
   int32_t     rxbytes;
   pcmq_t      slot;

   pcm_msg_t       msg;
   pcm_pipe_daq_t  pipe;

// 7.2.5   Code

   if (gc.trace & WIN_TRACE_ID) {
      printf("udp_thread() started, tid 0x%X\n", m_tid);
   }

   // beginning of PIPE message circular buffer
   m_nxt_pipe  = m_pool;
   m_blk_pipe  = m_pool;
   m_head      = 0;
   m_frmcnt    = 0;

   while (m_end_thread == FALSE) {
      rxbytes = recvfrom(m_sock, (char *)m_rxbuf, sizeof(m_rxbuf), 0, (struct sockaddr *)&m_dst, &m_slen);
      // timeout ?
      if (rxbytes < 0) continue;
      //
      //  CONTROL MESSAGE
      //
      if (rxbytes == UDP_CTL_MSG_LEN) {
         msg = (pcm_msg_t)&m_rxbuf[6];
         msglen = msg->h.msglen;
         // validate CM message length
         if ((msglen <= UDP_MSGLEN_UINT8) && (msglen >= sizeof(cm_msg_t))) {
            slot = cm_alloc();
            if (slot != NULL) {
               msg = (pcm_msg_t)slot->buf;
               // preserve slotid
               slotid = msg->h.slot;
               // uint32_t boundary, copy multiple of 32-bits
               // always read CM header + parms in order
               // to determine message length
               buf = (uint32_t *)&m_rxbuf[6];
               for (j=0;j<sizeof(cm_msg_t) >> 2;j++) {
                  slot->buf[j] = buf[j];
               }
               slot->msglen = msg->h.msglen;
               // read rest of CM message body, uint32_t per cycle
               if (slot->msglen > sizeof(cm_msg_t) && (slot->msglen <= UDP_MSGLEN_UINT8)) {
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
      //
      //  PIPE MESSAGE, ADC HARDWARE SPECIFIC
      //
      else if (rxbytes == UDP_PIPE_MSG_LEN) {
         // store 1K pipe message from packet body
         memcpy(m_nxt_pipe, &m_rxbuf[6], UDP_PIPELEN_UINT8);
         pipe = (pcm_pipe_daq_t)m_nxt_pipe;
         m_nxt_pipe += UDP_PIPELEN_UINT8;
         // packet arrival
         pipe->stamp_us = (uint32_t)tmr_get_elapsed_microseconds();
         // collect LAN_FRAME_CNT 1K pipe messages
         if (++m_frmcnt == UDP_FRAME_CNT) {
            m_frmcnt = 0;
            // next slot in circular buffer
            if (++m_head == UDP_POOL_SLOTS) m_head = 0;
            m_nxt_pipe = m_pool + (m_head * UDP_BLOCK_LEN);
            // send pipe message
            cm_pipe_send((pcm_pipe_t)m_blk_pipe, UDP_BLOCK_LEN);
            // record next start of block
            m_blk_pipe = m_nxt_pipe;
         }
      }
   }

   return 0;

} // end udp_thread()


// ===========================================================================

// 7.3

void udp_tx(pcm_msg_t msg) {

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

// 7.3.5   Code

   // Trace Entry
   if (gc.trace & WIN_TRACE_UART) {
      printf("udp_tx() srvid:msgid:msglen = %02X:%02X:%04X\n",
               msg->p.srvid, msg->p.msgid, msg->h.msglen);
      dump((uint8_t *)msg, msg->h.msglen, LIB_ASCII, 0);
   }

   // Enter Critical Section
   EnterCriticalSection(&m_tx_mutex);

   // 6-byte padding
   m_txbuf[0] = 0x00; m_txbuf[1] = 0x00; m_txbuf[2] = 0xAA;
   m_txbuf[3] = 0x55; m_txbuf[4] = 0x00; m_txbuf[5] = 0x00;

   // copy CM message to UDP body
   memcpy(&m_txbuf[6], (uint8_t *)msg, msg->h.msglen);

   // zero padding
   memset(&m_txbuf[msg->h.msglen + 6], 0, sizeof(m_txbuf) - (msg->h.msglen + 6));

   // send the packet
   sendto(m_sock, (char *)m_txbuf, UDP_CTL_MSG_LEN, 0, (struct sockaddr *)&m_dst, m_slen);

   // release message
   cm_free(msg);

   // Leave Critical Section
   LeaveCriticalSection(&m_tx_mutex);

} // end udp_tx()


// ===========================================================================

// 7.4

void udp_cmio(uint8_t op_code, pcm_msg_t msg) {

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
      printf("udp_cmio() op_code = %02X\n", op_code);
   }

   // transmit message
   udp_tx(msg);

} // end udp_cmio()


// ===========================================================================

// 7.5

void udp_head(void) {

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
   m_frmcnt    = 0;

} // end udp_head()


// ===========================================================================

// 7.6

void udp_final(void) {

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

   // close socket
   closesocket(m_sock);
   m_sock = -1;

   // close WSA
   WSACleanup();

   // Release Memory
   if (m_pool != NULL) free(m_pool);

   m_pool = NULL;

} // end udp_final()




