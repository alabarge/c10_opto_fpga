#pragma once

#define  GPIO_OK               0x00

#define  GPIO_LED_ON           0
#define  GPIO_LED_OFF          1
#define  GPIO_LED_TOGGLE       2
#define  GPIO_LED_ALL_OFF      3
#define  GPIO_LED_ALL_ON       4

#define  GPIO_LED_1            0x01
#define  GPIO_LED_2            0x02
#define  GPIO_LED_3            0x04
#define  GPIO_LED_4            0x08

#define  GPIO_LED_HB           GPIO_LED_1
#define  GPIO_LED_COM          GPIO_LED_2
#define  GPIO_LED_DAQ          GPIO_LED_3
#define  GPIO_LED_PIPE         GPIO_LED_4

#define  GPIO_LED_ERR          GPIO_LED_4

#define  GPIO_KEY_0            1
#define  GPIO_KEY_1            2
#define  GPIO_KEY_ALL_OFF      0x03

#define  GPIO_DIP_0            1
#define  GPIO_DIP_1            2
#define  GPIO_DIP_2            4
#define  GPIO_DIP_3            8
#define  GPIO_DIP_ALL_OFF      0x0F

#define  GPIO_XL345_ADDR       0x3A
#define  GPIO_XL345_DEVID      0x00
#define  GPIO_XL345_POWER      0x2D
#define  GPIO_XL345_FORMAT     0x31
#define  GPIO_XL345_FIFO       0x38
#define  GPIO_XL345_RATE       0x2C
#define  GPIO_XL345_INT_EN     0x2E
#define  GPIO_XL345_INT_MAP    0x2F
#define  GPIO_XL345_INT_SRC    0x30
#define  GPIO_XL345_DATA_X0    0x32
#define  GPIO_XL345_DATA_X1    0x33
#define  GPIO_XL345_DATA_Y0    0x34
#define  GPIO_XL345_DATA_Y1    0x35
#define  GPIO_XL345_DATA_Z0    0x36
#define  GPIO_XL345_DATA_Z1    0x37

#define  GPIO_XL345_LEN        0x06
#define  GPIO_XL345_SENSE      0x03

// GPIO0 Data Register
typedef union _gpio0_dat_reg_t {
   struct {
      uint32_t led              : 4;  // ioGPIO<3:0>
      uint32_t i2c_sclk         : 1;  // ioGPIO<4>
      uint32_t i2c_sdat         : 1;  // ioGPIO<5>
      uint32_t gforce_cs        : 1;  // ioGPIO<6>
      uint32_t unused           : 25; // ioGPIO<31:7>
   } b;
   uint32_t i;
} gpio0_dat_reg_t, *pgpio0_dat_reg_t;

// GPIO0 Direction Register
typedef union _gpio0_dir_reg_t {
   struct {
      uint32_t led              : 4;  // ioGPIO<3:0>
      uint32_t i2c_sclk         : 1;  // ioGPIO<4>
      uint32_t i2c_sdat         : 1;  // ioGPIO<5>
      uint32_t gforce_cs        : 1;  // ioGPIO<6>
      uint32_t unused           : 25; // ioGPIO<31:7>
   } b;
   uint32_t i;
} gpio0_dir_reg_t, *pgpio0_dir_reg_t;

// GPIO1 Data Register
typedef union _gpio1_dat_reg_t {
   struct {
      uint32_t key              : 2;  // iGPIO<1:0>
      uint32_t dipsw            : 4;  // iGPIO<5:2>
      uint32_t gforce_int       : 1;  // iGPIO<6>
      uint32_t unused           : 25; // iGPIO<31:7>
   } b;
   uint32_t i;
} gpio1_dat_reg_t, *pgpio1_dat_reg_t;

// GPIO1 Interrupt Mask Register
typedef union _gpio1_mask_reg_t {
   struct {
      uint32_t key              : 2;  // iGPIO<1:0>
      uint32_t dipsw            : 4;  // iGPIO<5:2>
      uint32_t gforce_int       : 1;  // iGPIO<6>
      uint32_t unused           : 25; // iGPIO<31:7>
   } b;
   uint32_t i;
} gpio1_mask_reg_t, *pgpio1_mask_reg_t;

// All Registers
typedef struct _gpio_regs_t {
   uint32_t       dat;
   uint32_t       dir;
   uint32_t       mask;
   uint32_t       edge;
   uint32_t       outset;
} gpio_regs_t, *pgpio_regs_t;

typedef struct _gpin_regs_t {
   uint32_t       dat;
   uint32_t       dir;
   uint32_t       mask;
   uint32_t       edge;
   uint32_t       outset;
} gpin_regs_t, *pgpin_regs_t;

uint32_t gpio_init(void);
void     gpio_isr(void *arg);
void     gpio_set_val(uint8_t led, uint8_t state);
uint8_t  gpio_key(void);
void     gpio_i2c_set(uint8_t addr, uint8_t value);
void     gpio_i2c_get(uint8_t addr, uint8_t len, uint8_t* buf);
void     gpio_xl345(uint8_t flags, uint8_t sense, uint8_t rate);
void     gpio_xl345_get(uint8_t *buf);
uint8_t  gpio_dip(void);
