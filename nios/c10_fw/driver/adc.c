/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      MAX11300 Driver

   1.2 Functional Description

      The ADC Interface routines are contained in this module.

   1.3 Specification/Design Reference

      See fw_cfg.h under the BOOT/SHARE directory.

   1.4 Module Test Specification Reference

      None

   1.5 Compilation Information

      See fw_cfg.h under the BOOT/SHARE directory.

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
      7.1   adc_init()
      7.2   adc_isr()
      7.3   adc_intack()
      7.4   adc_run()
      7.5   adc_version()

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

   static   volatile padc_regs_t   regs = (volatile padc_regs_t)(ADC_BASE);

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t adc_init(void) {

/* 7.1.1   Functional Description

   This routine is responsible for initializing the driver hardware.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t    result = CFG_STATUS_OK;

   adc_ctl_reg_t ctl;

// 7.1.5   Code

   // Reset Control
   ctl.i = 0;
   regs->ctl = ctl.i;
   utick(10);

   // Init Control register
   ctl.b.head        = 0;
   ctl.b.scan        = 0;
   ctl.b.ramp        = 0;
   ctl.b.run         = 0;
   ctl.b.pkt_int     = 0;
   ctl.b.done_int    = 0;
   regs->ctl         = ctl.i;

   // Init Circular Address
   regs->addr_beg    = ADC_FIFO_BASE;
   regs->addr_end    = ADC_FIFO_BASE + ADC_FIFO_SPAN - 1;

   // Init Registers
   regs->pkt_cnt   = 0;
   regs->pool_cnt  = ADC_POOL_CNT;
   regs->adc_rate  = ADC_SAM_RATE;
   regs->xfer_size = ADC_XFER_SIZE;

   // default to ADC sweep mode, 200 KSPS
   regs->dev_cfg     = 0x0043;

   // 0 => +10V ADC range
   regs->port_cfg    = 0x7100;

   // Clear any Pending Interrupts
   adc_intack(ADC_INT_ALL);

   // Register the ADC interrupt ISRs
   alt_ic_isr_register(ADC_IRQ_INTERRUPT_CONTROLLER_ID,
                       ADC_IRQ, adc_isr, NULL, NULL);

   // Report H/W Details
   if (gc.trace & CFG_TRACE_ID) {
      xlprint("%-13s base:rev:irq %08X:%d:%d\n", ADC_NAME, ADC_BASE, regs->version, ADC_IRQ);
   }

   return result;

}  // end adc_init()


// ===========================================================================

// 7.2

void adc_isr(void *arg) {

/* 7.2.1   Functional Description

   This routine will service the hardware interrupt.

   7.2.2   Parameters:

   arg     IRQ arguments

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   adc_int_reg_t irq;

// 7.2.5   Code

   // local irq copy
   irq.i = regs->irq;

   // report interrupt request
   if (gc.trace & CFG_TRACE_IRQ) {
      xlprint("adc_isr() irq = %02X\n", irq.i);
   }

   //
   // PACKET READY INTERRUPT
   //
   if (irq.b.pkt == 1) {
      // clear the interrupt
      adc_intack(ADC_INT_PKT);
      // send interrupt indication
      cm_local(CM_ID_DAQ_SRV, DAQ_INT_IND, DAQ_INT_FLAG_PKT, DAQ_OK);
   }

   //
   // DONE INTERRUPT
   //
   if (irq.b.done == 1) {
      // clear the interrupt
      adc_intack(ADC_INT_DONE);
      // send interrupt indication
      cm_local(CM_ID_DAQ_SRV, DAQ_INT_IND, DAQ_INT_FLAG_DONE, DAQ_OK);
   }

} // end adc_isr()


// ===========================================================================

// 7.3

void adc_intack(uint8_t int_type) {

/* 7.3.1   Functional Description

   This routine will Acknowledge specific ADC Interrupts, or ALL.

   7.3.2   Parameters:

   intType  ADC_INT_RDY:   ADC Block Ready Interrupt
            ADC_INT_DONE:  ADC Done Interrupt
            ADC_INT_ALL:   All ADC Interrupts

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   adc_int_reg_t  irq = {0};

// 7.3.5   Code

   if (int_type & ADC_INT_DONE) {
      irq.b.done = 1;
   }

   if (int_type & ADC_INT_PKT) {
      irq.b.pkt = 1;
   }

   if (int_type & ADC_INT_ALL) {
      irq.b.done = 1;
      irq.b.pkt = 1;
   }

   regs->irq = irq.i;

} // end adc_intack()


// ===========================================================================

// 7.4

void adc_run(uint32_t flags, uint32_t packets) {

/* 7.4.1   Functional Description

   This routine will start or stop the ADC acquisition.

   7.4.2   Parameters:

   flags    Start/Stop flags
   blocks   Number of 1024-Byte packets to acquire

   7.4.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   adc_ctl_reg_t  ctl;

// 7.4.5   Code

   ctl.i = regs->ctl;

   //
   // Start ADC Acquisition
   //
   if (flags & DAQ_CMD_RUN) {
      ctl.b.enable   = 1;
      regs->ctl      = ctl.i;
      utick(10);
      // write begin & end addresses
      regs->addr_beg = ADC_FIFO_BASE;
      regs->addr_end = ADC_FIFO_BASE + ADC_FIFO_SPAN - 1;
      // packet count
      regs->pkt_cnt  = packets;
      // pool count, port type dependent (see adc_init() above)
      regs->pool_cnt = ADC_POOL_CNT;
      // set run parameters
      regs->adc_rate  = ADC_SAM_RATE;
      regs->xfer_size = ADC_XFER_SIZE;
      ctl.b.run       = 1;
      regs->ctl       = ctl.i;
      // prevent packet interrupts when using head/tail in hardware,
      // packet interrupts are used when explicitly sending packets
      // from the interface, interrupt will occur every ADC_POOL_CNT
      ctl.b.pkt_int  = 0;
      // this interrupt is not used for FIFO, FTDI or COM
      ctl.b.done_int = 0;
      ctl.b.ramp     = (flags & DAQ_CMD_RAMP) ? 1 : 0;
      // When scan mode change device config, 200 KSPS
      regs->dev_cfg  = (flags & DAQ_CMD_SCAN) ? 0x0043 : 0x0042;
      // Set Run Parameters
      ctl.b.head     = (flags & DAQ_CMD_HEAD) ? 1 : 0;
      ctl.b.scan     = (flags & DAQ_CMD_SCAN) ? 1 : 0;
      regs->ctl      = ctl.i;
   }
   //
   // Stop ADC Acquisition
   //
   else {
      ctl.b.enable   = 1;
      ctl.b.run      = 0;
      ctl.b.pkt_int  = 0;
      ctl.b.done_int = 0;
      regs->ctl      = ctl.i;
   }

} // end adc_run()


// ===========================================================================

// 7.5

uint32_t adc_version(void) {

/* 7.5.1   Functional Description

   This routine will return the VERSION register value.

   7.5.2   Parameters:

   NONE

   7.5.3   Return Values:

   return   VERSION register

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

// 7.5.5   Code

   return regs->version;

} // end adc_version()

