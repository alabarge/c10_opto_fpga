/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Hardware Abstraction Layer Driver

   1.2 Functional Description

      The HAL Interface routines are contained in this module.

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
      7.1   cp_hal_init()
      7.2   cp_hal_isr()
      7.3   cp_hal_intack()

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

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t cp_hal_init(void) {

/* 7.1.1   Functional Description

   This routine is responsible for initializing the hardware
   abstraction layer (HAL).

   7.1.2   Parameters:

   result      CFG_ERROR_OK

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t   result = CFG_ERROR_OK;

// 7.1.5   Code

   return result;

}  // end cp_hal_init()


// ===========================================================================

// 7.2

void cp_hal_isr(void) {

/* 7.2.1   Functional Description

   This routine will service the Interrupt.

   7.2.2   Parameters:

   NONE

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

// 7.2.5   Code

} // end cp_hal_isr()


// ===========================================================================

// 7.3

void cp_hal_intack(uint8_t int_type) {

/* 7.3.1   Functional Description

   This routine will Acknowledge the Interrupts.

   7.3.2   Parameters:

   intType

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

// 7.3.5   Code

} // end cp_hal_intack()

