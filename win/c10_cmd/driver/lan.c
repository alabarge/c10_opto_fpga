/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      LAN8270 NPCAP I/O Driver, A point-to-point Ethernet Interface

   1.2 Functional Description

      This module provides an API to access the NPCAP Driver.

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
         7.1   lan_init()
         7.2   lan_thread()
         7.3   lan_tx()
         7.4   lan_cmio()
         7.5   lan_head()
         7.6   lan_crc()
         7.7   lan_final()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"
#include "lanpkt.h"
#include "pcap.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define LAN_RETRIES       4

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

   static   DWORD WINAPI      lan_thread(LPVOID data);

// 6.2  Local Data Structures

   static   uint8_t           m_cm_port  = CM_PORT_NONE;
   static   uint8_t           m_com_port = 0;

   static   lan_dev_info_t    m_devinfo[LAN_MAX_DEVICES] = {0};
   static   pcap_t           *m_fp = NULL;
   static   char              m_ebuf[PCAP_ERRBUF_SIZE+1];
   static   cm_udp_ctl_t      m_out = {0};
   static   uint16_t          m_seqid;

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

   static   UCHAR             m_query[] = {0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t lan_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port,
                  uint16_t cm_udp_port, uint8_t *macip) {

/* 7.1.1   Functional Description

   The LAN Interface is initialized in this routine.

   7.1.2   Parameters:

   baudrate  Serial Baud Rate
   cm_port   CM Port
   com_port  Device ID from pcap_findalldevs()

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    result = LAN_OK;
   uint32_t    devcnt = 0;
   pcap_if_t   *devs, *d;
   uint8_t     retry = 0;
   int32_t     status;
   char       *pcap_rev;

   const       u_char *pkt_data;
   struct      pcap_pkthdr *header;
   struct      bpf_program fcode;

   char        filter[256];
   cm_udp_ctl_t *pkt;

// 7.1.5   Code

   // close LAN if opened
   if (m_fp != NULL) pcap_close(m_fp);

   m_fp = NULL;

   // Update LAN COM Port
   m_com_port = com_port;

   // Query for attached devices
   if (pcap_findalldevs(&devs, m_ebuf) == -1) {
      result = LAN_ERR_DEV_CNT;
   }
   else {
      // Enumerate devices
      for (d=devs;d;d=d->next) {
         // Check for Zero Protocols, All protocols must be "un-checked"
         // in Adapter Ethernet Properties, then d->addresses will be NULL.
         if (d->addresses == NULL) {
            m_devinfo[devcnt].flags  = 1;
            m_devinfo[devcnt].type   = LAN_CHAN_CTL;
            m_devinfo[devcnt].id     = 0;
            m_devinfo[devcnt].locid  = 0;
            m_devinfo[devcnt].handle = NULL;
            strcpy_s(m_devinfo[devcnt].desc, 256, d->description);
            strcpy_s(m_devinfo[devcnt].serial, 256, d->name);
            if (gc.trace & WIN_TRACE_UART) {
               printf("\nlan_init() LAN.%d :\n", devcnt);
               printf("  flags  : %08X\n", m_devinfo[devcnt].flags);
               printf("  type   : %08X\n", m_devinfo[devcnt].type);
               printf("  id     : %08X\n", m_devinfo[devcnt].id);
               printf("  locid  : %08X\n", m_devinfo[devcnt].locid);
               printf("  serial : %s\n", m_devinfo[devcnt].serial);
               printf("  desc   : %s\n\n", m_devinfo[devcnt].desc);
            }
            devcnt++;
         }
      }
   }

   // Okay to Go
   if (result == LAN_OK) {
      if (gc.trace & WIN_TRACE_UART)
         printf("\nlan_init() selected port = %d\n", m_com_port);

      // Open LAN Device by Serial
      m_fp = pcap_open_live(m_devinfo[m_com_port].serial, LAN_SNAPLEN, 1,
                  LAN_READ_TO, m_ebuf);

      // Flush the Read Queue
      pcap_next_ex(m_fp, &header, &pkt_data);

      // Minimum size for a Kernel Copy
      pcap_setmintocopy(m_fp, sizeof(cm_udp_ctl_t));

      // Check return status from LAN
      if (m_fp == NULL) result |= LAN_ERR_OPEN;

      // Compile Filter
      sprintf_s(filter, 256, "dst %d.%d.%d.%d", macip[6], macip[7], macip[8], macip[9]);
      status = pcap_compile(m_fp, &fcode, filter, 1, 0xffffff);

      // Check return status from LAN
      if (status < 0) result |= LAN_ERR_OPEN;

      // Use Filter
      status = pcap_setfilter(m_fp, &fcode);

      // Check return status from LAN
      if (status < 0) result |= LAN_ERR_OPEN;
   }

   // Device Opened
   if (result == LAN_OK && m_fp != NULL) {

      // Init the Ethernet Frame Header, Fixed Frame Size
      memset(&m_out, 0, sizeof(cm_udp_ctl_t));
      memcpy(m_out.eth_hdr.smac.mac, &macip[0], sizeof(mac_addr_t));
      memcpy(m_out.eth_hdr.dmac.mac, &macip[10], sizeof(mac_addr_t));
      m_out.eth_hdr.type  = 0x0008;
      m_out.ip_hdr.ver    = 0x45;
      m_out.ip_hdr.tos    = 0x00;
      m_out.ip_hdr.tlen   = 0x0000;
      m_out.ip_hdr.id     = 0x0000;
      m_out.ip_hdr.flags  = 0x0000;
      m_out.ip_hdr.ttl    = 0x80;
      m_out.ip_hdr.proto  = 0x11;
      m_out.ip_hdr.crc    = 0x0000;
      memcpy(m_out.ip_hdr.saddr.ip, &macip[6], sizeof(ip_addr_t));
      memcpy(m_out.ip_hdr.daddr.ip, &macip[16], sizeof(ip_addr_t));
      m_out.udp_hdr.sport = swap16(0x5580);
      m_out.udp_hdr.dport = swap16(cm_udp_port);
      m_out.udp_hdr.len   = 0x0000;
      m_out.udp_hdr.crc   = 0x0000;
      m_out.pad[0]        = 0x00;
      m_out.pad[1]        = 0x00;
      m_out.pad[2]        = 0xAA;
      m_out.pad[3]        = 0x55;
      m_out.pad[4]        = 0x00;
      m_out.pad[5]        = 0x00;

      // IP sequence ID
      m_out.ip_hdr.id = swap16(m_seqid);
      m_seqid++;
      // populate length fields
      m_out.ip_hdr.tlen = sizeof(cm_udp_ctl_t) - sizeof(eth_header_t);
      m_out.udp_hdr.len = m_out.ip_hdr.tlen - sizeof(ip_header_t);
      m_out.ip_hdr.tlen = swap16(m_out.ip_hdr.tlen);
      m_out.udp_hdr.len = swap16(m_out.udp_hdr.len);
      // compute IP header checksum
      m_out.ip_hdr.crc = 0x0000;
      m_out.ip_hdr.crc = lan_crc((uint16_t *)&m_out.ip_hdr, sizeof(m_out.ip_hdr) >> 1);

      // Flush the Read Queue
      pcap_next_ex(m_fp, &header, &pkt_data);

      // Issue CM_QUERY_REQ multiple tries
      while (retry < LAN_RETRIES) {

         result |= LAN_ERR_RESP;

         // Send CM_QUERY_REQ to validate connection
         memcpy(m_out.body, m_query, sizeof(m_query));
         status = pcap_sendpacket(m_fp, (u_char *)&m_out, sizeof(cm_udp_ctl_t));

         // report message content
         if ((gc.trace & WIN_TRACE_UART)) {
            printf("lan_init() tx msglen = %d\n", (int)sizeof(m_query));
            dump((uint8_t *)m_query, sizeof(m_query), 0, 0);
         }

         // If sent OK then check response
         if (status == 0) {
            // Allow time for response
            Sleep(50);
            // Read the Port
            status = pcap_next_ex(m_fp, &header, &pkt_data);
            // report message content
            if ((gc.trace & WIN_TRACE_UART)) {
               printf("lan_init() rx msglen = %d\n", header->len);
               dump((uint8_t *)pkt_data, 64, 0, 0);
            }
            // Check Response
            if (status == 1 && (header->len == sizeof(cm_udp_ctl_t))) {
               pkt = (cm_udp_ctl_t *)pkt_data;
               // Verify Magic Number
               if (pkt->body[12] != 0x34 || pkt->body[13] != 0x12 ||
                     pkt->body[14] != 0xAA || pkt->body[15] != 0x55) {
                  result |= LAN_ERR_RESP;
               }
               else {
                  // Record SysID
                  m_sysid = (pkt->body[19] << 24) | (pkt->body[18] << 16) |
                            (pkt->body[17] << 8)  |  pkt->body[16];
                  // Record Timestamp
                  m_stamp = (pkt->body[23] << 24) | (pkt->body[22] << 16) |
                            (pkt->body[21] << 8)  |  pkt->body[20];
                  // Record Device ID, etc.
                  m_devid    = pkt->body[24];
                  m_numobjs  = pkt->body[25];
                  m_numcons  = pkt->body[26];
                  m_cmdat    = (m_devid << 24) | (m_numobjs << 16) | (m_numcons << 8);
                  result     = LAN_OK;
                  break;
               }
            }
         }
         retry++;
      }
   }

   // OK to Continue
   if (result == LAN_OK && m_fp != NULL) {

      pcap_rev = (char *)pcap_lib_version();
      if (pcap_rev != NULL) strcpy_s(m_librev, 256, pcap_rev);

      // Init the Mutex
      InitializeCriticalSection (&m_tx_mutex);

      // Update CM Port
      m_cm_port = cm_port;

      // Register the I/O Interface callback for CM
      cm_ioreg(lan_cmio, m_cm_port, CM_MEDIA_LAN);

      // Allocate Pipe Message Pool
      m_pool  = (uint8_t *)malloc(LAN_PIPE_POOL);
      if (m_pool == NULL) result = LAN_ERR_POOL;

      // Start the H/W Receive Thread
      m_end_thread = FALSE;
      if ((m_thread = CreateThread(NULL, 0, lan_thread, NULL, 0, &m_tid)) == NULL) {
         result = LAN_ERR_THREAD;
      }
      else {
         SetThreadPriority(&m_tid, THREAD_PRIORITY_NORMAL);
      }

       // Print Hardware Version to Serial Port
      if (gc.trace & WIN_TRACE_ID) {
         printf("Opened LAN.%d (%s) for Messaging\n", m_com_port, m_devinfo[m_com_port].desc);
         printf("LAN.%d : wpcap64.lib, %s\n", m_com_port, m_librev);
         printf("LAN.%d : sysid:stamp:cm = %d:%d:%08X\n\n", m_com_port, m_sysid, m_stamp, m_cmdat);
      }
   }

   if ((gc.trace & WIN_TRACE_ERROR) && result != LAN_OK) {
      printf("lan_init() Error : %08X\n", result);
   }

   return (result == LAN_OK) ? WIN_ERROR_OK : WIN_ERROR_LAN;

}  // end lan_init()


// ===========================================================================

// 7.2

static DWORD WINAPI lan_thread(LPVOID data) {


/* 7.2.1   Functional Description

   This thread will service the incoming characters from the LAN serial
   interface.

   7.2.2   Parameters:

   data     Thread parameters

   7.2.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    status;
   uint8_t     slotid;
   uint32_t   *buf;
   uint16_t    msglen;
   uint16_t    frm_cnt = 0;
   uint32_t    j;
   pcmq_t      slot;
   pcm_msg_t   msg;
   char       *pcap_err;

   struct      pcap_pkthdr *header;
   const       u_char *pkt_data;

   cm_udp_ctl_t   *ctl;
   cm_udp_pipe_t  *pkt;

   pcm_pipe_daq_t  pipe;

// 7.2.5   Code

   if (gc.trace & WIN_TRACE_ID) {
      printf("lan_thread() started, tid 0x%X\n", m_tid);
   }

   // beginning of PIPE message circular buffer
   m_nxt_pipe  = m_pool;
   m_blk_pipe  = m_pool;
   m_head      = 0;
   m_frmcnt    = 0;

   while (m_end_thread == FALSE) {
      // next packet, 100 ms timeout if no response
      status = pcap_next_ex(m_fp, &header, &pkt_data);
      // timeout ?
      if (status == 0) continue;
      //
      //  CONTROL MESSAGE
      //
      if ((status == 1) && (header->len == sizeof(cm_udp_ctl_t))) {
         ctl    = (pcm_udp_ctl_t)pkt_data;
         msglen = ((pcm_msg_t)ctl->body)->h.msglen;
         // validate CM message length
         if ((msglen <= LAN_MSGLEN_UINT8) && (msglen >= sizeof(cm_msg_t))) {
            slot = cm_alloc();
            if (slot != NULL) {
               msg = (pcm_msg_t)slot->buf;
               // preserve slotid
               slotid = msg->h.slot;
               // uint32_t boundary, copy multiple of 32-bits
               // always read CM header + parms in order
               // to determine message length
               buf = (uint32_t *)ctl->body;
               for (j=0;j<sizeof(cm_msg_t) >> 2;j++) {
                  slot->buf[j] = buf[j];
               }
               slot->msglen = msg->h.msglen;
               // read rest of CM message body, uint32_t per cycle
               if (slot->msglen > sizeof(cm_msg_t) && (slot->msglen <= LAN_MSGLEN_UINT8)) {
                  for (j=0;j<(slot->msglen + 3 - sizeof(cm_msg_t)) >> 2;j++) {
                     slot->buf[j + (sizeof(cm_msg_t) >> 2)] =
                           buf[j + (sizeof(cm_msg_t) >> 2)];
                  }
               }
               // restore slotid
               msg->h.slot = slotid;
               // report message content
               if ((gc.trace & WIN_TRACE_UART) && (slot != NULL)) {
                  printf("lan_thread() msglen = %d\n", msg->h.msglen);
                  dump((uint8_t *)slot->buf, slot->msglen, LIB_ASCII, 0);
               }
               // queue the message
               cm_qmsg((pcm_msg_t)slot->buf);
            }
         }
      }
      //
      //  PIPE MESSAGE, ADC HARDWARE SPECIFIC
      //
      else if ((status == 1) && (header->len == sizeof(cm_udp_pipe_t))) {
         pkt = (pcm_udp_pipe_t)pkt_data;
         // store 1K pipe message from packet body
         memcpy(m_nxt_pipe, pkt->body, LAN_PIPELEN_UINT8);
         pipe = (pcm_pipe_daq_t)m_nxt_pipe;
         m_nxt_pipe += LAN_PIPELEN_UINT8;
         // packet arrival
         pipe->stamp_us = (uint32_t)tmr_get_elapsed_microseconds();
         // collect LAN_FRAME_CNT 1K pipe messages
         if (++m_frmcnt == LAN_FRAME_CNT) {
            m_frmcnt = 0;
            // next slot in circular buffer
            if (++m_head == LAN_POOL_SLOTS) m_head = 0;
            m_nxt_pipe = m_pool + (m_head * LAN_BLOCK_LEN);
            // report partial pipe content
            if (gc.trace & WIN_TRACE_PIPE) {
               printf("lan_thread() pipelen = %d\n", LAN_BLOCK_LEN);
               dump(m_blk_pipe, 32, LIB_ASCII, 0);
            }
            // send pipe message
            cm_pipe_send((pcm_pipe_t)m_blk_pipe, LAN_BLOCK_LEN);
            // record next start of block
            m_blk_pipe = m_nxt_pipe;
         }
      }
      //
      //  PCAP ERROR
      //
      else if (status == -1 && m_end_thread == FALSE) {
         // get api error
         pcap_err = (char *)pcap_geterr(m_fp);
         if (pcap_err != NULL) strcpy_s(m_ebuf, PCAP_ERRBUF_SIZE, pcap_err);
      }
   }

   return 0;

} // end lan_thread()


// ===========================================================================

// 7.3

void lan_tx(pcm_msg_t msg) {

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
   uint8_t     retry = 0;

// 7.3.5   Code

   // Trace Entry
   if (gc.trace & WIN_TRACE_UART) {
      printf("lan_tx() srvid:msgid:msglen = %02X:%02X:%04X\n",
               msg->p.srvid, msg->p.msgid, msg->h.msglen);
      dump((uint8_t *)msg, msg->h.msglen, LIB_ASCII, 0);
   }

   // Enter Critical Section
   EnterCriticalSection(&m_tx_mutex);

   // IP sequence ID
   m_out.ip_hdr.id = swap16(m_seqid);
   m_seqid++;

   // populate length fields
   m_out.ip_hdr.tlen = sizeof(cm_udp_ctl_t) - sizeof(eth_header_t);
   m_out.udp_hdr.len = m_out.ip_hdr.tlen - sizeof(ip_header_t);
   m_out.ip_hdr.tlen = swap16(m_out.ip_hdr.tlen);
   m_out.udp_hdr.len = swap16(m_out.udp_hdr.len);

   // compute IP header checksum
   m_out.ip_hdr.crc = 0x0000;
   m_out.ip_hdr.crc = lan_crc((uint16_t *)&m_out.ip_hdr, sizeof(m_out.ip_hdr) >> 1);

   // copy CM message to UDP body
   memcpy(m_out.body, (uint8_t *)msg, msg->h.msglen);

   // zero padding
   memset(&m_out.body[msg->h.msglen], 0, sizeof(m_out.body) - msg->h.msglen);

   // send the packet
   pcap_sendpacket(m_fp, (u_char *)&m_out, sizeof(cm_udp_ctl_t));

   // release message
   cm_free(msg);

   // Leave Critical Section
   LeaveCriticalSection(&m_tx_mutex);

} // end lan_tx()


// ===========================================================================

// 7.4

void lan_cmio(uint8_t op_code, pcm_msg_t msg) {

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
      printf("lan_cmio() op_code = %02X\n", op_code);
   }

   // transmit message
   lan_tx(msg);

} // end lan_cmio()


// ===========================================================================

// 7.5

void lan_head(void) {

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

} // end lan_head()


// ===========================================================================

 // 7.6

uint16_t lan_crc(uint16_t *msg, uint32_t len) {

/* 7.6.1   Functional Description

   This routine is responsible for computing the 16-Bit one's complement
   checksum as per RFC 1071.

   7.6.2   Parameters:

   msg     Pointer to Array of 16-Bit words
   len     Number of 16-Bit words to sum over

   7.6.3   Return Values:

   checkSum

-----------------------------------------------------------------------------
*/

// 7.6.4   Data Structures

   uint32_t    i, j, chksum = 0;

// 7.6.5   Code

   // Compute the messages CRC.
   // Cycle through all the message bytes
   for (i=0;i<len;i++) {
      j = msg[i];
      j = (~j) & 0x0000FFFF;
      chksum += j;
   }

   // Account for the overflow into the
   // upper 16 bits.
   chksum = (chksum >> 16) + (chksum & 0x0000FFFF);
   chksum = (chksum >> 16) + (chksum & 0x0000FFFF);

   return chksum;

} // end lan_crc()


// ===========================================================================

// 7.7

void lan_final(void) {

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

   // Wake-up Thread and Cancel
   m_end_thread = TRUE;

   // close LAN if opened
   if (m_fp != NULL) pcap_close(m_fp);

   m_fp = NULL;

   // Release Memory
   if (m_pool != NULL) free(m_pool);

   m_pool = NULL;

} // end lan_final()




