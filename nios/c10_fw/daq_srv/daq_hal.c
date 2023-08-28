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
         7.1   daq_hal_init()
         7.2   daq_hal_isr()
         7.3   daq_hal_intack()
         7.4   daq_hal_run()

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

   static   daq_sv_t    sv  = {0};

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t daq_hal_init(void) {

/* 7.1.1   Functional Description

   This routine is responsible for initializing the hardware
   abstraction layer (HAL).

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   result   STATUS_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t   result = CFG_ERROR_OK;

// 7.1.5   Code

   return result;

}  // end daq_hal_init()


// ===========================================================================

// 7.2

void daq_hal_isr(void *arg) {

/* 7.2.1   Functional Description

   This routine will service the Interrupt.

   7.2.2   Parameters:

   NONE

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures


} // end daq_hal_isr()


// ===========================================================================

// 7.3

void daq_hal_intack(uint8_t int_type) {

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

} // end daqIntAck()


// ===========================================================================

// 7.4

void daq_hal_run(pdaq_sv_t psv) {

/* 7.4.1   Functional Description

   This routine will start/stop the DAQ Controller.

   7.4.2   Parameters:

   psv      DAQ State Vector

   7.4.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

// 7.4.5   Code

   // Report Command Parameters, only for DAQ_CMD_RUN
   if (gc.trace & CFG_TRACE_RUN && psv->opcode & DAQ_CMD_RUN) {
      xlprint("daq_hal_run()\n");
      xlprint("  opcode     :  %08X\n", psv->opcode);
      xlprint("  packets    :  %d\n",   psv->packets);
   }

   //
   // Start the Acquisition Controller
   //
   if (psv->opcode & DAQ_CMD_RUN) {
      // Clear Sequence ID
      sv.seqid    = 0;
      sv.samcnt   = 0;
      // Initialize State Vector
      sv.opcode   = psv->opcode;
      sv.packets  = psv->packets;
      // Issue Pipe Stream Start
      opto_pipe(OPTO_OP_START, ADC_FIFO_BASE,
                ADC_FIFO_BASE + ADC_FIFO_SPAN - 1, sv.packets);
      // Issue ADC Run Command
      adc_run(sv.opcode, sv.packets);
      // Update machine status
      gc.status |= CFG_STATUS_DAQ_RUN;
   }
   //
   // Stop the Acquisition Controller
   //
   else if (psv->opcode & DAQ_CMD_STOP) {
      // IDLE State
      sv.state    = DAQH_STATE_IDLE;
      // Update State Vector
      sv.opcode   = psv->opcode;
      sv.packets  = psv->packets;
      // Issue Pipe Stream Stop
      opto_pipe(OPTO_OP_STOP, 0, 0, 0);
      // Issue ADC Stop Command
      adc_run(sv.opcode, sv.packets);
      // Update machine status
      gc.status &= ~CFG_STATUS_DAQ_RUN;
   }

} // end daq_hal_run()

