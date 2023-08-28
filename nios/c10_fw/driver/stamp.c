/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      System ID and Build Time Stamp Driver

   1.2 Functional Description

      The Stamp I/O Interface routines are contained in this module.

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
         7.1   stamp_init()
         7.2   stamp_sysid()
         7.3   stamp_epoch()
         7.4   stamp_date()
         7.5   stamp_time()
         7.6   stamp_version()
         7.7   stamp_count()
         7.8   stamp_magic();
         7.9   stamp_inc();

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// 4.2   External Data Structures

// 4.3 External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

// 6.2  Local Data Structures

   static   volatile pstamp_regs_t   regs = (volatile pstamp_regs_t)STAMP_BASE;

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t stamp_init(void) {

/* 7.1.1   Functional Description

   The Stamp Interface is initialized in this routine.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    result = CFG_ERROR_OK;

// 7.1.5   Code

   // Report H/W Details
   if (gc.trace & CFG_TRACE_ID) {
      xlprint("%-13s base:rev:irq %08X:%08X:%d\n", STAMP_NAME, STAMP_BASE, regs->version, STAMP_IRQ);
   }

   return result;

}  // end stamp_init()


// ===========================================================================

// 7.2

uint32_t stamp_sysid(void) {

/* 7.2.1   Functional Description

   This routine will return the timestamp SYSID register value.

   7.2.2   Parameters:

   NONE

   7.2.3   Return Values:

   return   SYSID register

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

// 7.2.5   Code

   return regs->pid;

} // end stamp_sysid()


// ===========================================================================

// 7.3

uint32_t stamp_epoch(void) {

/* 7.3.1   Functional Description

   This routine will return the timestamp EPOCH register value.

   7.3.2   Parameters:

   NONE

   7.3.3   Return Values:

   return   EPOCH register

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

// 7.3.5   Code

   return regs->epoch;

} // end stamp_epoch()


// ===========================================================================

// 7.4

uint32_t stamp_date(void) {

/* 7.4.1   Functional Description

   This routine will return the timestamp DATE register value.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   return   DATE register

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

// 7.4.5   Code

   return regs->date;

} // end stamp_date()


// ===========================================================================

// 7.5

uint32_t stamp_time(void) {

/* 7.5.1   Functional Description

   This routine will return the timestamp TIME register value.

   7.5.2   Parameters:

   NONE

   7.5.3   Return Values:

   return   TIME register

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

// 7.5.5   Code

   return regs->time;

} // end stamp_time()


// ===========================================================================

// 7.6

uint32_t stamp_version(void) {

/* 7.6.1   Functional Description

   This routine will return the timestamp VERSION register value.

   7.6.2   Parameters:

   NONE

   7.6.3   Return Values:

   return   VERSION register

-----------------------------------------------------------------------------
*/

// 7.6.4   Data Structures

// 7.6.5   Code

   return regs->version;

} // end stamp_version()


// ===========================================================================

// 7.7

uint32_t stamp_count(void) {

/* 7.7.1   Functional Description

   This routine will return the count register value.

   7.7.2   Parameters:

   NONE

   7.7.3   Return Values:

   return   count register

-----------------------------------------------------------------------------
*/

// 7.7.4   Data Structures

// 7.7.5   Code

   return regs->count;

} // end stamp_count()


// ===========================================================================

// 7.8

uint32_t stamp_magic(void) {

/* 7.8.1   Functional Description

   This routine will return the magic number register value.

   7.8.2   Parameters:

   NONE

   7.8.3   Return Values:

   return   magic# register

-----------------------------------------------------------------------------
*/

// 7.8.4   Data Structures

// 7.8.5   Code

   return regs->magic;

} // end stamp_magic()


// ===========================================================================

// 7.9

uint32_t stamp_inc(void) {

/* 7.9.1   Functional Description

   This routine will return the build increment register value.

   7.9.2   Parameters:

   NONE

   7.9.3   Return Values:

   return   build increment register

-----------------------------------------------------------------------------
*/

// 7.9.4   Data Structures

// 7.9.5   Code

   return regs->inc;

} // end stamp_inc()



