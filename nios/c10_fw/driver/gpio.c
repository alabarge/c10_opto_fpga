/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      GPIO and DIP Switch Driver.

   1.2 Functional Description

      The GPIO Interface routines are contained in this module.

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
      7.1   gpio_init()
      7.2   gpio_isr()
      7.3   gpio_val_set()
      7.4   gpio_key()
      7.5   gpio_i2c_set()
      7.6   gpio_i2c_get()
      7.7   gpio_xl345()
      7.8   gpio_xl345_get()
      7.9   gpio_dip()

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

   static   volatile pgpio_regs_t   regs    = (volatile pgpio_regs_t)GPX_BASE;
   static   volatile pgpin_regs_t   regs_in = (volatile pgpin_regs_t)GPI_BASE;

// 7 MODULE CODE

// ===========================================================================

// 7.1

uint32_t gpio_init(void) {

/* 7.1.1   Functional Description

   This routine is responsible for initializing the driver hardware.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t   result = CFG_STATUS_OK;
   uint8_t    buf;

   gpio0_dat_reg_t   dat0 = {0};

// 7.1.5   Code

   // Set GPIO0 I/O direction
   // default is all OUTPUT
   regs->dir = 0xFFFFFFFF;

   // Set GPIO0 Default I/O
   // default is all OFF
   dat0.b.gforce_cs = 1;
   dat0.b.i2c_sclk  = 1;
   dat0.b.i2c_sdat  = 1;
   regs->dat = dat0.i;

   // Set GPX1 Default Interrupt Mask
   regs_in->mask = 0;

   // ADXL345 Device ID
   gpio_i2c_get(GPIO_XL345_DEVID, 1, &buf);
   xlprint("ADXL345 ID: %02X\n", buf);

   // ADXL345 Initialize
   // Rate = 25Hz, FIFO Mode,
   // 25 Sample watermark
   // +/- 8g range
   gpio_i2c_set(GPIO_XL345_FORMAT, 0x00);
   gpio_i2c_set(GPIO_XL345_RATE,   0x08);
   gpio_i2c_set(GPIO_XL345_FIFO,   0x40 | CP_XL345_MARK);
   gpio_i2c_set(GPIO_XL345_POWER,  0x00);

   // Empty the FIFO
   gpio_xl345_get(NULL);

   // Enable the ADXL345 Interrupt
   gpio_i2c_set(GPIO_XL345_INT_EN, 0x01);

   // Register the GPI interrupt ISRs
   alt_ic_isr_register(GPI_IRQ_INTERRUPT_CONTROLLER_ID,
                       GPI_IRQ, gpio_isr, NULL, NULL);

   return result;

}  // end gpio_init()


// ===========================================================================

// 7.2

void gpio_isr(void *arg) {

/* 7.2.1   Functional Description

   This routine will service the GPX Interrupt.

   7.2.2   Parameters:

   NONE

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

// 7.2.5   Code

   // Disable the ADXL345 Interrupt
   gpio_i2c_set(GPIO_XL345_INT_EN, 0x00);

   // Send Interrupt Indication for Transferring samples
   cm_local(CM_ID_CP_SRV, CP_INT_IND, CP_IND_XL345, CP_OK);

} // end gpio_isr()


// ===========================================================================

// 7.3

void gpio_set_val(uint8_t gpio, uint8_t state) {

/* 7.3.1   Functional Description

   This routine will set the GPIO0 data bits.

   7.3.2   Parameters:

   gpio     GPIO to change
   state    ON/OFF/TOGGLE state

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

   gpio0_dat_reg_t   dat;

// 7.3.5   Code

   // Read Current Port Value
   dat.i = regs->dat;

   switch(state) {
      case GPIO_LED_OFF :
         dat.b.led &= ~gpio;
         break;
      case GPIO_LED_ON :
         dat.b.led |=  gpio;
         break;
      case GPIO_LED_TOGGLE :
         dat.b.led ^=  gpio;
         break;
      case GPIO_LED_ALL_OFF :
         dat.b.led  =  0x0;
         break;
      case GPIO_LED_ALL_ON :
         dat.b.led  =  0xF;
         break;
   }

   regs->dat = dat.i;

}  // end gpio_val_set()


// ===========================================================================

// 7.4

uint8_t gpio_key(void) {

/* 7.4.1   Functional Description

   This routine will return the current state of the User KEY 0-3 Switches
   and the slide switches SW 0-9.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   result   KEY 0-13 Switch states

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

   gpio1_dat_reg_t   dat;

// 7.4.5   Code

   // Read Current Port Value
   dat.i = regs_in->dat;

   return dat.b.key;

}  // end gpio_key()


// ===========================================================================

// 7.5

void gpio_i2c_set(uint8_t addr, uint8_t value) {

/* 7.5.1   Functional Description

   This routine will write the I2C ADXL345 Interface, 8-Bits only.

   7.5.2   Parameters:

   addr     ADXL345 Register Address
   value    8-Bit value to write

   7.5.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

   uint32_t   i,j;
   uint32_t   frame;
   uint32_t   mask;

   gpio0_dat_reg_t   dat;
   gpio0_dir_reg_t   dir;

// 7.5.5   Code

   // Read Current Port
   dat.i = regs->dat;
   dir.i = regs->dir;

   // Start Condition, high-to-low transition
   // on DAT while CLK remains high.
   dat.b.i2c_sdat = 0;
   regs->dat = dat.i;
   utick(2);

   // Write the 7-Bit Device Address + R/W Bit (MSB First)
   // Then Write the 8-Bit Register Address (MSB First)
   // Then Write the 8-Bit Value (MSB First)
   frame = (GPIO_XL345_ADDR << 16) | (addr << 8) | value;
   mask = 0x800000;
   for (j=0;j<3;j++) {
      for (i=0;i<8;i++) {
         // CLK Falling-Edge
         dat.b.i2c_sclk = 0;
         regs->dat = dat.i;
         utick(1);
         // Output Data Bit, tri-state off
         dat.b.i2c_sdat = (frame & mask) ? 1 : 0;
         dir.b.i2c_sdat = 1;
         regs->dat = dat.i;
         regs->dir = dir.i;
         utick(1);
         // CLK Rising-Edge
         dat.b.i2c_sclk = 1;
         regs->dat = dat.i;
         utick(1);
         mask >>= 1;
      }
      // CLK Falling-Edge
      dat.b.i2c_sclk = 0;
      regs->dat = dat.i;
      utick(1);
      // ACK From ADXL345, tri-state on
      dir.b.i2c_sdat = 0;
      regs->dir = dir.i;
      utick(1);
      // CLK Rising-Edge
      dat.b.i2c_sclk = 1;
      regs->dat = dat.i;
      utick(1);
   }

   // CLK Falling-Edge
   dat.b.i2c_sclk = 0;
   regs->dat = dat.i;
   utick(1);

   // SDA=0, tri-state off
   dat.b.i2c_sdat = 0;
   dir.b.i2c_sdat = 1;
   regs->dat = dat.i;
   regs->dir = dir.i;
   utick(1);

   // CLK Rising-Edge
   dat.b.i2c_sclk = 1;
   regs->dat = dat.i;
   utick(1);

   // Stop Condition, low-to-high transition
   // on DAT while CLK remains high.
   dat.b.i2c_sdat = 1;
   regs->dat = dat.i;
   utick(2);

} // end gpxI2CSet()


// ===========================================================================

// 7.6

void gpio_i2c_get(uint8_t addr, uint8_t len, uint8_t* buf) {

/* 7.6.1   Functional Description

   This routine will read the I2C ADXL345 Interface, 8-Bits only.

   7.6.2   Parameters:

   addr     ADXL345 Register Address
   len      Bytes to read
   buf      Storage for bytes read

   7.6.3   Return Values:

   result   8-Bit Value read

-----------------------------------------------------------------------------
*/

// 7.6.4   Data Structures

   uint32_t   i,j,k;
   uint32_t   frame;
   uint32_t   mask;

   gpio0_dat_reg_t   dat;
   gpio0_dir_reg_t   dir;

// 7.6.5   Code

   // Read Current Port
   dat.i = regs->dat;
   dir.i = regs->dir;

   // Start Condition, high-to-low transition
   // on DAT while CLK remains high.
   dat.b.i2c_sdat = 0;
   regs->dat = dat.i;
   utick(2);

   // Write the 7-Bit Device Address + R/W Bit (MSB First)
   // Then Write the 8-Bit Register Address (MSB First)
   frame = (GPIO_XL345_ADDR << 8) | addr;
   mask = 0x8000;
   for (j=0;j<2;j++) {
      for (i=0;i<8;i++) {
         // CLK Falling-Edge
         dat.b.i2c_sclk = 0;
         regs->dat = dat.i;
         utick(1);
         // Output Data Bit, tri-state off
         dat.b.i2c_sdat = (frame & mask) ? 1 : 0;
         dir.b.i2c_sdat = 1;
         regs->dat = dat.i;
         regs->dir = dir.i;
         utick(1);
         // CLK Rising-Edge
         dat.b.i2c_sclk = 1;
         regs->dat = dat.i;
         utick(1);
         mask >>= 1;
      }
      // CLK Falling-Edge
      dat.b.i2c_sclk = 0;
      regs->dat = dat.i;
      utick(1);
      // ACK From ADXL345, tri-state on
      dir.b.i2c_sdat = 0;
      regs->dir = dir.i;
      utick(1);
      // CLK Rising-Edge
      dat.b.i2c_sclk = 1;
      regs->dat = dat.i;
      utick(1);
   }

   // CLK Falling-Edge
   dat.b.i2c_sclk = 0;
   regs->dat = dat.i;
   utick(1);

   // SDA=0, tri-state off
   dat.b.i2c_sdat = 0;
   dir.b.i2c_sdat = 1;
   regs->dat = dat.i;
   regs->dir = dir.i;
   utick(1);

   // CLK Rising-Edge
   dat.b.i2c_sclk = 1;
   regs->dat = dat.i;
   utick(1);

   // Stop Condition, low-to-high transition
   // on DAT while CLK remains high.
   dat.b.i2c_sdat = 1;
   regs->dat = dat.i;
   utick(2);

   // Start Condition, high-to-low transition
   // on DAT while CLK remains high.
   dat.b.i2c_sdat = 0;
   regs->dat = dat.i;
   utick(2);

   // Write the 7-Bit Device Address + R/W Bit (MSB First)
   frame = GPIO_XL345_ADDR | 0x01;
   mask = 0x80;
   for (i=0;i<8;i++) {
      // CLK Falling-Edge
      dat.b.i2c_sclk = 0;
      regs->dat = dat.i;
      utick(1);
      // Output Data Bit, tri-state off
      dat.b.i2c_sdat = (frame & mask) ? 1 : 0;
      dir.b.i2c_sdat = 1;
      regs->dat = dat.i;
      regs->dir = dir.i;
      utick(1);
      // CLK Rising-Edge
      dat.b.i2c_sclk = 1;
      regs->dat = dat.i;
      utick(1);
      mask >>= 1;
   }

   // CLK Falling-Edge
   dat.b.i2c_sclk = 0;
   regs->dat = dat.i;
   utick(1);
   // ACK From ADXL345, tri-state on
   dir.b.i2c_sdat = 0;
   regs->dir = dir.i;
   utick(1);
   // CLK Rising-Edge
   dat.b.i2c_sclk = 1;
   regs->dat = dat.i;
   utick(1);

   for (k=0;k<len;k++) {
      // Read 8-Bit Value (MSB First)
      mask = 0x80;
      for (i=0,buf[k]=0;i<8;i++) {
         // CLK Falling-Edge
         dat.b.i2c_sclk = 0;
         regs->dat = dat.i;
         utick(1);
         // Tri-State ON
         dir.b.i2c_sdat = 0;
         regs->dir = dir.i;
         utick(1);
         // CLK Rising-Edge
         dat.b.i2c_sclk = 1;
         regs->dat = dat.i;
         // Input Data Bit
         dat.i = regs->dat;
         if (dat.b.i2c_sdat == 1) buf[k] |= mask;
         utick(1);
         // CLK Falling-Edge
         dat.b.i2c_sclk = 0;
         regs->dat = dat.i;
         utick(1);
         mask >>= 1;
      }
      // CLK Falling-Edge
      dat.b.i2c_sclk = 0;
      regs->dat = dat.i;
      utick(1);
      // ACK/NACK From CPU, Tri-State Off
      dat.b.i2c_sdat = (k == len - 1) ? 1 : 0;
      dir.b.i2c_sdat = 1;
      regs->dat = dat.i;
      regs->dir = dir.i;
      utick(1);
      // CLK Rising-Edge
      dat.b.i2c_sclk = 1;
      regs->dat = dat.i;
      utick(1);
   }

   // CLK Falling-Edge
   dat.b.i2c_sclk = 0;
   regs->dat = dat.i;
   utick(1);
   // Tri-State Off
   dat.b.i2c_sdat = 0;
   dir.b.i2c_sdat = 1;
   regs->dat = dat.i;
   regs->dir = dir.i;
   utick(1);

   // Stop Condition, low-to-high transition
   // on DAT while CLK remains high.
   dat.b.i2c_sclk = 1;
   regs->dat = dat.i;
   utick(2);
   dat.b.i2c_sdat = 1;
   regs->dat = dat.i;
   utick(2);

} // end gpxI2CGet()


// ===========================================================================

// 7.7

void gpx_xl345(uint8_t flags, uint8_t sense, uint8_t rate) {

/* 7.7.1   Functional Description

   This routine will enable/disable the XL345 force measurements.

   7.7.2   Parameters:

   flags    Start/Stop Measure flags
   sense    Sensitivity
   rate     Acquisition Rate

   7.7.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.7.4   Data Structures

   gpio1_mask_reg_t  mask;

// 7.7.5   Code

   mask.i = regs_in->mask;

   if (flags & CP_XL345_RUN) {
      // Empty the FIFO
      gpio_xl345_get(NULL);
      // Set Update Rate
      gpio_i2c_set(GPIO_XL345_RATE, rate & CP_XL345_RATE);
      // Set Sensitivity Range
      gpio_i2c_set(GPIO_XL345_FORMAT, sense & CP_XL345_SENSE);
      // Enable ADXL345 Interrupt
      mask.b.gforce_int = 1;
      regs_in->mask = mask.i;
      gpio_i2c_set(GPIO_XL345_INT_EN, 0x01);
      // Start measurements
      gpio_i2c_set(GPIO_XL345_POWER, 0x08);
   }
   else {
      // Stop measurements
      gpio_i2c_set(GPIO_XL345_POWER, 0x00);
      // Disable ADXL345 Interrupt
      mask.b.gforce_int = 0;
      regs_in->mask = mask.i;
      gpio_i2c_set(GPIO_XL345_INT_EN, 0x00);
      // Empty the FIFO
      gpio_xl345_get(NULL);
   }

}  // end gpio_xl345()


// ===========================================================================

// 7.8

void gpio_xl345_get(uint8_t *buf) {

/* 7.8.1   Functional Description

   This routine will check for the ADXL345 watermark interrupt and read the
   devices FIFO.

   7.8.2   Parameters:

   buf   Storage for samples, if NULL then measurements are discarded.

   7.8.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.8.4   Data Structures

   uint32_t   i;
   uint8_t    dat[CP_XL345_LEN];

// 7.8.5   Code

   // Empty the FIFO
   if (buf == NULL) {
      // Read the FIFO
      for (i=0;i<CP_XL345_MARK;i++) {
         // Discard measurements
         gpio_i2c_get(GPIO_XL345_DATA_X0, CP_XL345_LEN, dat);
      }
   }
   else {
      // Read the FIFO
      for (i=0;i<CP_XL345_MARK;i++) {
         // Store measurements
         gpio_i2c_get(GPIO_XL345_DATA_X0, CP_XL345_LEN, &buf[i*CP_XL345_LEN]);
      }
      // Enable ADXL345 Interrupt
      gpio_i2c_set(GPIO_XL345_INT_EN, 0x01);
   }

}  // end gpio_xl345_get()


// ===========================================================================

// 7.9

uint8_t gpio_dip(void) {

/* 7.9.1   Functional Description

   This routine will return the current state of the dip switch.

   7.9.2   Parameters:

   NONE

   7.9.3   Return Values:

   result   DIP Switch state

-----------------------------------------------------------------------------
*/

// 7.9.4   Data Structures

   gpio1_dat_reg_t   dat;

// 7.9.5   Code

   // Read Current Port Value
   dat.i = regs_in->dat;

   return dat.b.dipsw;

}  // end gpio_dip()


