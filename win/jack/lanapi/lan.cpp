/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      LAN8270 NPCAP I/O Driver, A point-to-point Ethernet Interface

   1.2 Functional Description

      This module provides an API to access the NPCAP Driver.

   1.3 Specification/Design Reference

      LANAPI_DLL.docx

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
      7.1   lan_query()
      7.2   lan_init()
      7.3   lan_thread()
      7.4   lan_tx()
      7.5   lan_cmio()
      7.6   lan_head()
      7.7   lan_sysid()
      7.8   lan_rev()
      7.9   lan_final()

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
#include "lan.h"
#include "lanapi.h"
#include "lanpkt.h"
#include "pcap.h"
#include "timer.h"
#include "trace.h"
#include <tchar.h>

#include "build.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

   #define  swap16(x) ((unsigned short) (((x) <<  8) | (((x) >>  8) & 0xFF)))

   #define  LAN_RETRIES   4

   #define  LAN_CAPTURE

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
   static   pcap_hdr_t        m_pcap = {0};
   static   FILE             *m_fd = NULL;

   static   HANDLE            m_thread;
   static   DWORD             m_tid;
   static   HANDLE            m_event = NULL;
   static   uint8_t          *m_pool = NULL;
   static   uint8_t          *m_nxt_pipe = NULL;
   static   uint8_t          *m_blk_pipe = NULL;
   static   uint8_t           m_end_thread = FALSE;
   static   uint32_t          m_frmcnt = 0;
   static   uint32_t          m_head = 0;
   static   CHRTimer          m_timer;

   static   CRITICAL_SECTION  m_tx_mutex;

   static   uint32_t          m_sysid, m_stamp, m_cmdat;
   static   uint8_t           m_devid, m_numobjs, m_numcons;
   static   uint32_t          m_sysrev = 0;
   static   char              m_librev[256];

   static   u_char            m_query[] = {0x83, 0x83, 0x10, 0x10, 0x00, 0x00,
                                           0x0C, 0x20, 0x83, 0x09, 0x00, 0x00};

// 7 MODULE CODE

// ===========================================================================

// 7.1

LAN_API uint32_t lan_query(lan_dev_info_t **devinfo) {

/* 7.1.1   Functional Description

   This routine will query the NPAC driver to acquire all the LAN devices
   attached to the system.

   7.1.2   Parameters:

   devcnt   Pointer to the attached device count
   devinfo  Pointer to the plan_dev_info_t strucure

   7.1.3   Return Values:

   devcnt  FTDI Device Count

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    devcnt = 0, len;
   pcap_if_t   *devs;
   pcap_if_t   *d;

   _TCHAR npcap_dir[512];

// 7.1.5   Code

   // Load Npcap DLL, not Winpcap
   len = GetSystemDirectory(npcap_dir, 480);
   if (len != 0) {
      _tcscat_s(npcap_dir, 512, _T("\\Npcap"));
      SetDllDirectory(npcap_dir);
   }

   // Query for attached devices
   if (pcap_findalldevs(&devs, m_ebuf) == -1) {
      *devinfo = NULL;
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
            devcnt++;
         }
      }
      *devinfo = (lan_dev_info_t *)m_devinfo;
   }

   return devcnt;

}  // end lan_query()


// ===========================================================================

// 7.2

LAN_API int32_t lan_init(uint32_t baudrate, uint8_t cm_port, uint8_t com_port,
                         uint16_t cm_udp_port, uint8_t *macip) {

/* 7.2.1   Functional Description

   This routine will open access to the LAN device.

   NOTE: Only a single device is currently supported.

   7.2.2   Parameters:

   baudrate    Baudrate in bps
   cm_port     CM Port Identifier
   com_port    LAN Port
   macip       uint8_t array holding MAC and IP addresses

   7.2.3   Return Values:

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t    result = LAN_OK;
   uint8_t     retry = 0;
   int32_t     status;
   char       *pcap_rev;
   errno_t     err;

   const       u_char *pkt_data;
   struct      pcap_pkthdr *header;
   struct      bpf_program fcode;

   char        filter[256];
   cm_udp_ctl_t *pkt;

// 7.2.5   Code

   // close LAN if opened
   if (m_fp != NULL) pcap_close(m_fp);

   m_fp = NULL;

   // Update LAN COM Port
   m_com_port = com_port;

   // Open LAN Device by Serial
   m_fp = pcap_open_live(m_devinfo[com_port].serial, LAN_SNAPLEN, 1,
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

   // OK TO GO
   if (result == LAN_OK) {

      // Init the Ethernet Frame Header, Fixed Frame Size
      ::ZeroMemory(&m_out, sizeof(cm_udp_ctl_t));
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

         // If sent OK then check response
         if (status == 0) {
            // Allow time for response
            Sleep(50);
            // Read the Port
            status = pcap_next_ex(m_fp, &header, &pkt_data);
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

      // Reset Hi-Res Timer
      m_timer.Start();

#ifdef LAN_CAPTURE
      // Init PCAP header
      m_pcap.magic_number = 0xa1b2c3d4;
      m_pcap.version_major = 2;
      m_pcap.version_minor = 4;
      m_pcap.thiszone = 0;
      m_pcap.sigfigs = 0;
      m_pcap.snaplen = 262144;
      m_pcap.network = 1;
      err = fopen_s(&m_fd, "lan_cap.pcap", "wb");
      if (err != 0) m_fd = NULL;
      if (m_fd != NULL) fwrite(&m_pcap, sizeof(m_pcap), 1, m_fd);
#endif

   }
   // close the port
   else {
      if (m_fp != NULL) pcap_close(m_fp);
      m_fp = NULL;
   }

   return result;

}  // end lan_init()


// ===========================================================================

// 7.3

static DWORD WINAPI lan_thread(LPVOID data) {


/* 7.3.1   Functional Description

   This thread will service the incoming characters from the LAN serial
   interface.

   7.3.2   Parameters:

   data     Thread parameters

   7.3.3   Return Values:

   return   Thread exit status

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

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


// 7.3.5   Code

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
      // write pcap header and packet to capture file
      if (m_fd != NULL) fwrite(header, sizeof(pcap_pkthdr), 1, m_fd);
      if (m_fd != NULL) fwrite(pkt_data, header->len, 1, m_fd);
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
         pipe->stamp_us = (uint32_t)m_timer.GetElapsedAsMicroSeconds();
         // collect LAN_FRAME_CNT 1K pipe messages
         if (++m_frmcnt == LAN_FRAME_CNT) {
            m_frmcnt = 0;
            // next slot in circular buffer
            if (++m_head == LAN_POOL_SLOTS) m_head = 0;
            m_nxt_pipe = m_pool + (m_head * LAN_BLOCK_LEN);
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

// 7.4

LAN_API void lan_tx(pcm_msg_t msg) {

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

// 7.4.5   Code

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

// 7.5

LAN_API void lan_cmio(uint8_t op_code, pcm_msg_t msg) {

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
   lan_tx(msg);

} // end lan_cmio()


// ===========================================================================

// 7.6

LAN_API void lan_head(void) {

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
   m_frmcnt    = 0;

} // end lan_head()


// ===========================================================================

// 7.7

LAN_API void lan_rev(char **librev, uint32_t *sysrev, uint32_t *apirev) {

/* 7.7.1   Functional Description

   This routine will return the build versions for lan.dll, ftd2xx.sys
   and ftd2xx.lib.

   7.7.2   Parameters:

   librev   LAN ftd2xx.lib revision
   sysrev   LAN ftd2xx.sys revision
   apirev   lan.dll revision

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

}  // end lan_rev()


// ===========================================================================

// 7.8

LAN_API void lan_sysid(uint32_t *sysid, uint32_t *stamp, uint32_t *cmdat) {

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

}  // end lan_sysid()


// ===========================================================================

 // 7.9

uint16_t lan_crc(uint16_t *msg, uint32_t len) {

/* 7.9.1   Functional Description

   This routine is responsible for computing the 16-Bit one's complement
   checksum as per RFC 1071.

   7.9.2   Parameters:

   msg     Pointer to Array of 16-Bit words
   len     Number of 16-Bit words to sum over

   7.9.3   Return Values:

   checkSum

-----------------------------------------------------------------------------
*/

// 7.9.4   Data Structures

   uint32_t    i, j, chksum = 0;

// 7.9.5   Code

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

// 7.10

LAN_API void lan_final(void) {

/* 7.10.1   Functional Description

   This routine will clean-up any allocated resources.

   7.10.2   Parameters:

   NONE

   7.10.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.10.4   Data Structures

// 7.10.5   Code

   // Wake-up Thread and Cancel
   m_end_thread = TRUE;

   // close LAN if opened
   if (m_fp != NULL) pcap_close(m_fp);

   m_fp = NULL;

   // Release Memory
   if (m_pool != NULL) free(m_pool);

   // Close capture file
   if (m_fd != NULL) fclose(m_fd);

   m_fd = NULL;

   m_pool = NULL;

} // end lan_final()

