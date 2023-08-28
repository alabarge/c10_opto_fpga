#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

   // CM Message Header
   typedef struct _cm_hdr_t {
      uint8_t  dst_cmid;        // Destination CM Address
      uint8_t  src_cmid;        // Source CM Address
      uint8_t  dst_devid :  4;  // Destination Device ID
      uint8_t  src_devid :  4;  // Source Device ID
      uint8_t  seqid     :  4;  // Message Sequence Number
      uint8_t  endian    :  1;  // Processor Endianess
      uint8_t  event     :  1;  // Event ID
      uint8_t  keep      :  1;  // Don't Delete
      uint8_t  proto     :  1;  // Protocol ID
      uint8_t  crc8;            // CRC-8 using x^8 + x^5 + x^4 + 1
      uint8_t  slot;            // Slot position in queue
      uint16_t msglen    : 12;  // Message Length
      uint16_t port      :  4;  // Port Connection
   } cm_hdr_t, *pcm_hdr_t;

   // parameters common to all messages
   typedef struct _msg_parms_t {
      uint8_t srvid;
      uint8_t msgid;
      uint8_t flags;
      uint8_t status;
   } msg_parms_t, *pmsg_parms_t;

   // cm message structure
   typedef struct _cm_msg_t {
      cm_hdr_t       h;
      msg_parms_t    p;
   } cm_msg_t, *pcm_msg_t;

   // CM PIPE HEADER DATA STRUCTURE
   // USED FOR VARIABLE LENGTH PIPE MESSAGES
   typedef struct _cm_pipe_t {
      uint8_t     dst_cmid;       // Destination CM ID
      uint8_t     msgid;          // Pipe Message ID, CM_PIPE_DAQ_DATA = 0x10
      uint8_t     port;           // Destination Port
      uint8_t     flags;          // Message Flags
      uint32_t    msglen;         // Message Length in 32-Bit words
      uint32_t    seqid;          // Sequence ID
      uint32_t    stamp;          // 32-Bit FPGA Clock Count
      uint32_t    stamp_us;       // 32-Bit Time Stamp in microseconds
      uint32_t    status;         // Current Machine Status
      uint32_t    rate;           // ADC rate
      uint32_t    magic;          // Magic Number
      uint16_t    samples[496];   // DAQ Samples
   } cm_pipe_t, *pcm_pipe_t;

   // CM PIPE MESSAGE DATA STRUCTURE
   // USED FOR FIXED 1024-BYTE PIPE MESSAGES
   typedef struct _cm_pipe_fixed_t {
      uint8_t     dst_cmid;       // Destination CM ID
      uint8_t     msgid;          // Pipe Message ID
      uint8_t     port;           // Destination Port
      uint8_t     flags;          // Message Flags
      uint32_t    msglen;         // Message Length in 32-Bit words
      uint32_t    seqid;          // Sequence ID
      uint32_t    stamp;          // 32-Bit FPGA Clock Count
      uint32_t    stamp_us;       // 32-Bit Time Stamp in microseconds
      uint32_t    status;         // Current Machine Status
      uint32_t    rate;           // ADC rate
      uint32_t    magic;          // Magic Number
      uint16_t    samples[496];   // DAQ Samples
   } cm_pipe_fixed_t, *pcm_pipe_fixed_t;

   static uint8_t crc_array[] = {
      0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
      0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
      0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
      0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
      0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
      0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
      0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
      0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
      0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
      0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
      0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
      0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
      0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
      0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
      0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
      0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
      0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
      0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
      0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
      0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
      0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
      0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
      0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
      0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
      0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
      0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
      0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
      0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
      0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
      0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
      0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
      0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
   };

// CM Return Errors
enum cm_err {
      CM_OK,
      CM_ERR_BAD_MSG_ID,
      CM_ERR_NULL_PTR,
      CM_ERR_LEN_NULL,
      CM_ERR_PARMS_NULL,
      CM_ERR_LEN_MAX,
      CM_ERR_MSG_NULL,
      CM_ERR_CRC,
      CM_ERR_DEV_ID,
      CM_ERR_OBJ_ID,
      CM_ERR_MSGQ_EMPTY,
      CM_ERR_IF_MAX,
      CM_ERR_ROUTE,
      CM_ERR_TIMER_ID,
      CM_ERR_TASK_ID,
      CM_ERR_EVENT_ID,
      CM_ERR_MEDIA_NONE,
};

#define CM_CALC_CRC           0
#define CM_CHECK_CRC          1

#define  LIB_ASCII       0x80
#define  LIB_16BIT       0x01
#define  LIB_32BIT       0x02
#define  LIB_ADDR        0x04
#define  LIB_OFFSET      0x08
#define  LIB_SPACE       0x10

// ===========================================================================

// 7.1

void dump(uint8_t *pBuf, uint32_t len, uint8_t options, uint32_t offset) {

/* 7.1.1   Functional Description

   This routine will display, on the debug serial port, the buffer contents.

   7.1.2   Parameters:

   pBuf        Buffer pointer
   len         Buffer length
   options     Output options
   offset      Reported Offset

   7.1.3   Return Values:

   result      NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t   i,j,k = 0;
   uint8_t   *p8  = (uint8_t *)pBuf;
   uint16_t  *p16 = (uint16_t *)pBuf;
   uint32_t  *p32 = (uint32_t *)pBuf;

   char    *fmt1 = "%03X ";
   char    *fmt2 = "%08X ";
   char    *fmt = fmt1;

// 7.1.5   Code

   if (options & LIB_OFFSET) {
      k = offset;
      fmt = fmt2;
   }

   // 32-Bit
   if (options & LIB_32BIT) {
      for (i=0;i<len;i+=4) {
         printf(fmt,i+k);
         for (j=0;j<4;j++) {
            if ((i+j) == len) printf("         ");
            else printf(" %08X", p32[i+j]);
         }
         printf("\n");
      }
      printf("\n");
   }
   // 16-Bit
   else if (options & LIB_16BIT) {
      for (i=0;i<len;i+=8) {
         printf(fmt,i+k);
         for (j=0;j<8;j++) {
            if ((i+j) == len) printf("     ");
            else printf(" %04X", p16[i+j]);
         }
         printf("\n");
      }
      printf("\n");
   }
   // 8-Bit with space
   else if (options & LIB_SPACE) {
      for (i=0;i<len;i+=16) {
         printf(fmt,i+k);
         for (j=0;j<16;j++) {
            if ((i+j) >= len) printf("  ");
            else printf("%02X ", p8[i+j]);
            if (j == 7) printf("  ");
         }
         // Show ASCII characters
         if (options & LIB_ASCII) {
            printf(" ; ");
            for (j=0;j<16;j++) {
               if ((i+j) >= len) {
                  printf(" ");
               }
               else {
                  if (p8[i+j] >= 0x20 && p8[i+j] <= 0x7F)
                     printf("%c", p8[i+j]);
                  else
                     printf(".");
                  if (j == 7) printf(" ");
               }
            }
         }
         printf("\n");
      }
      printf("\n");
   }
   // 8-Bit, Default
   else {
      for (i=0;i<len;i+=16) {
         printf(fmt,i+k);
         for (j=0;j<16;j++) {
            if ((i+j) >= len) printf("  ");
            else printf("%02X", p8[i+j]);
            if (j == 7) printf(" ");
         }
         // Show ASCII characters
         if (options & LIB_ASCII) {
            printf(" ; ");
            for (j=0;j<16;j++) {
               if ((i+j) >= len) {
                  printf(" ");
               }
               else {
                  if (p8[i+j] >= 0x20 && p8[i+j] <= 0x7F)
                     printf("%c", p8[i+j]);
                  else
                     printf(".");
                  if (j == 7) printf(" ");
               }
            }
         }
         printf("\n");
      }
      printf("\n");
   }

} // end dump()


// ===========================================================================

// 7.7

uint32_t cm_crc(pcm_msg_t msg, uint8_t crc_chk) {

/* 7.7.1   Functional Description

   This routine will compute the messages PAYLOAD CRC-8 and place it at the
   appropriate location. If crcChk is TRUE, the CRC is validated against the
   message. The CRC-8 is computed using the polynomial x^8 + x^5 + x^4 + 1.
   For reference see Maxim's Application Note 27, "Understanding and Using
   Cyclic Redundancy Checks with Maxim iButton Products".

   NOTE: THE CM HEADER IS EXCLUDED FROM THE CRC-8 COMPUTATION.

   7.7.2   Parameters:

   msg      Pointer to Received Message
   crc_chk  CM_CHECK_CRC: Compute the messages checksum and compare
            CM_CALC_CRC:  Compute the messages checksum and place at msg->h.crc

   7.7.3   Return Values:

   result   CM_OK on success
            CM_ERR_CRC when CRC is invalid

-----------------------------------------------------------------------------
*/

// 7.7.4   Data Structures

   uint32_t    result = CM_OK;
   uint8_t     k = 0, data = 0;
   uint16_t    i;
   uint16_t    msglen = msg->h.msglen - sizeof(cm_hdr_t);
   uint8_t    *pin = (uint8_t *)msg;

// 7.7.5   Code

   // COMPUTE CRC-8
   if (crc_chk == CM_CALC_CRC) {
      // exclude CM header
      for (i=sizeof(cm_hdr_t);i<msglen;i++) {
         data = pin[i] ^ k;
         k = crc_array[data];
      }
      msg->h.crc8 = k;
   }
   // VERIFY CRC-8
   else {
      for (i=sizeof(cm_hdr_t);i<msglen;i++) {
         data = pin[i] ^ k;
         k = crc_array[data];
      }
      if (msg->h.crc8 != k) result = CM_ERR_CRC;
   }

   printf("\ncm_crc(), msg : %02X, calc : %02X, len : %d\n\n", msg->h.crc8, k, msglen);

   dump(pin, msg->h.msglen, 0, 0);

   return result;

} // end cm_crc()


int main(int argc, char *argv[]) {

   FILE     *fid;
   char      line[512];
   char      cwd[128];
   uint8_t   bin[1024] = {0};
   char     *token;

   uint32_t  i=0,j;

   printf("\nCRC Checksum Utility 1.0 [AEL]\n");

   // command Line
   printf("cmd : ");
   for (j=0;j<argc;j++) {
      printf("%s ", argv[j]);
   }
   printf("\n");

   if (argc < 2) {
      printf("Error: Filename not specified.\n");
      return -1;
   }

   // cycle over CRC file
   if ((fid = fopen(argv[1], "rt")) != NULL) {
      j = 0;
      // read next line
      while (fgets(line, sizeof(line), fid) != NULL) {
         // ignore comments
         if (line[0] == '#') continue;
         // ignore blank lines
         if (line[0] == '\n') continue;
         // ignore blank lines
         if (line[0] == '\r') continue;
         // ignore lines that begin with a space
         if (line[0] == ' ') continue;
         token = strtok(line, " \n\r");
         for (i=0;i<16;i++) {
            // store byte
            sscanf(token, "%hhx", &bin[(j*16)+i]);
            token = strtok(NULL, " \n\r");
         }
         j++;
      }
      fclose(fid);
      // Calculate CRC Checksum
      cm_crc((pcm_msg_t)bin, CM_CALC_CRC);
   }
   else {
      printf("Fatal Error : CRC File %s did not Open for Read\n", argv[1]);
      return -1;
   }

   return 0;
}
