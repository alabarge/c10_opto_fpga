/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Application Library

   1.2 Functional Description

      This code implements the Application Library Routines.

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
        7.1   dump()
        7.2   crc()
        7.3   srand_32()
        7.4   rand_32()
        7.5   kbhit()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// 4.2   External Data Structures

// 4.3   External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

// 6.2  Local Data Structures

   static   uint32_t seed = 100;

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


// 7 MODULE CODE

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

// 7.2

uint32_t crc(pcm_msg_t msg, BOOLEAN crc_chk) {

/* 7.2.1   Functional Description

   This routine will compute the messages payload CRC-8 and place it at the
   appropriate location. If crcChk is TRUE, the CRC is validated against the
   message. The CRC-8 is computed using the polynomial x^8 + x^5 + x^4 + 1.
   For reference see Maxim's Application Note 27, "Understanding and Using
   Cyclic Redundancy Checks with Maxim iButton Products".

   7.2.2   Parameters:

   pMsg     Pointer to Received Message
   crc_chk  CM_CHECK_CRC: Compute the messages checksum and compare
            CM_CALC_CRC:  Compute the messages checksum and place at pMsg->h.crc

   7.2.3   Return Values:

   result   CM_OK on success
            CM_ERR_CRC when CRC is invalid

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t      result = LIB_OK;
   uint8_t       k = 0, data = 0;
   uint16_t      i;
   uint16_t      msglen = msg->h.msglen;
   uint8_t       *pin = (uint8_t *)msg;

// 7.2.5   Code

   // COMPUTE CRC-8
   if (crc_chk == CM_CALC_CRC) {
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
      if (msg->h.crc8 != k) result = LIB_ERR_CRC;
   }

   if ((gc.trace & WIN_TRACE_ERROR) && (result == LIB_ERR_CRC)) {
      printf("crc(), msg : %02X, calc : %02X, len : %d\n", msg->h.crc8, k, msglen);
      dump(pin, 16, 0, 0);
   }

   return result;

} // end crc()


// ===========================================================================

// 7.3

void srand_32(uint32_t _seed) {

/* 7.3.1   Functional Description

   This routine sets the pseudo-random number generator seed.

   7.3.2   Parameters:

    seed     Random Number seed

   7.3.3   Return Values:

    NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

// 7.3.5   Code

   seed = _seed;

}  // end srand_32()


// ===========================================================================

// 7.4

uint32_t rand_32(void) {

/* 7.4.1   Functional Description

   This routine implements the Park-Miller minimal standard 31 bit pseudo-random
   number generator, using David G. Carta's optimization, with 32-Bit math and
   without division.

   7.4.2   Parameters:

    NONE

   7.4.3   Return Values:

    result  Pseudo-Random Number

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint32_t   hi,lo;

// 7.4.5   Code

   lo = 16807 * (seed & 0xFFFF);
   hi = 16807 * (seed >> 16);

   lo += (hi & 0x7FFF) << 16;
   lo += hi >> 15;

   if (lo > 0x7FFFFFFF) lo -= 0x7FFFFFFF;

   return (seed = lo);

}  // end rand_32()
