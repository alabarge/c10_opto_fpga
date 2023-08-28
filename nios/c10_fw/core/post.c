/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Power-On Self Test

   1.2 Functional Description

      This code implements the first phase of power-on self test functions
      prior to main application boot.

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
        7.1   post_all()
        7.2   post_ddr()
        7.3   post_sd()
        7.4   post_esn()
        7.5   post_int()

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

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t post_all(void) {

/* 7.1.1   Functional Description

   This routine is the main entry into the first phase of power-on self test.
   All tests will be called from this routine the the results will be returned
   in a single 32-Bit flag.

   7.1.2   Parameters:

   pSec     Sector Buffer

   7.1.3   Return Values:

   result   32-Bit Flag containing the individual results from each test.

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t   result = POST_OK;

// 7.1.5   Code

   //
   // 32MB SDRAM
   //
   result |= post_ddr();
   //
   // SDHC Card
   //
   result |= post_sd();
   //
   // DS2433 ESN
   //
   result |= post_esn();
   //
   // CPU INTERRUPT
   //
   result |= post_int();

   // Print POST Result to Serial Port
   if (gc.trace & CFG_TRACE_POST) {
      xlprint("post    :  %08X\n", result);
   }

   return result;

} // end post_all()


// ===========================================================================

// 7.2

uint32_t post_ddr(void) {

/* 7.2.1   Functional Description

   This routine will test a portion of each megabyte in the 2GB DDR2 SODIMM.
   The memory will first be written with a pseudo-random sequence and then
   subsequently read and verified. This routine is a destructive memory test.

   7.2.2   Parameters:

    NONE

   7.2.3   Return Values:

   result      POST_DDR: If the memory test fails
               POST_OK:  Otherwise

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t   result = POST_OK;
   uint32_t   i,j,k;
   uint32_t   *pMem = (uint32_t *)(SDRAM_BASE + 0x40000);

   // 7.2.5   Code

   xlprint("postDDR()  %08X : %02d MiB\n", pMem, ((SDRAM_SPAN / 1048576) - 2));

   // Seed Pseudo-Random Number Generator
   srand_32(100);

   // Write CFG_DDR_SIZE 1MB Regions
   for (i=0,k=0;i<((SDRAM_SPAN / 1048576) - 2);i++) {
      // Write 128 32-Bit words in each region
      for (j=0;j<128;j++) {
         pMem[k+j] = rand_32();
      }
      // Next Region (2^20 + 512) Bytes
      k += 0x40080;
   }

   // Flush Data Cache
   alt_dcache_flush((void *)SDRAM_BASE, SDRAM_SPAN);

   // Seed Pseudo-Random Number Generator
   srand_32(100);

   // Read CFG_DDR_SIZE 1MB Regions
   for (i=0,k=0;i<((SDRAM_SPAN / 1048576) - 2);i++) {
      // Read 128 32-Bit words in each region
      for (j=0;j<128;j++) {
         if (pMem[k+j] != rand_32()) {
            result = POST_DDR;
            xlprint("postDDR() Error : %08X = %08X\n", &pMem[k+j], pMem[k+j]);
            break;
         }
      }
      // Next Region (2^20 + 512) Bytes
      k += 0x40080;
      // Early Termination
      if (result != POST_OK) break;
   }

   return result;

} // end postDDR()


// ===========================================================================

// 7.3

uint32_t post_sd(void) {

/* 7.3.1   Functional Description

   This routine will test the ability to read the micro-SD card boot sector.
   Fixed values and locations will be verified.

   7.3.2   Parameters:

   pSec     Sector Buffer

   7.3.3   Return Values:

   result      POST_SD: If the SD card test fails
               POST_OK:  Otherwise

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   uint32_t   result = POST_OK;

// 7.3.5   Code

   return result;

} // end post_sd()


// ===========================================================================

// 7.4

uint32_t post_esn(void) {

/* 7.4.1   Functional Description

   This routine will test for the presence of the ESN serial device.

   7.4.2   Parameters:

    NONE

   7.4.3   Return Values:

   result      POST_ESN: If the ESN presence detect Fails
               POST_OK:  Otherwise

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   uint32_t   result = POST_OK;

// 7.4.5   Code

   return result;

} // end post_esn()


// ===========================================================================

// 7.5

uint32_t post_int(void) {

/* 7.5.1   Functional Description

   This routine will test the timer interrupt.

   7.5.2   Parameters:

    NONE

   7.5.3   Return Values:

   result      POST_INT: If the Interrupt Test Fails
               POST_OK:  Otherwise

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

   uint32_t   result = POST_OK;

// 7.5.5   Code

   return result;

} // end post_int()

