#pragma once

#define  NUM_MEM_BLOCKS     16
#define  CFG_SECTOR_SIZE    512

#define  LIB_OK          0
#define  LIB_ERR_CRC     1
#define  LIB_CALC_CRC    1
#define  LIB_CHECK_CRC   2

#define  LIB_ASCII       0x80
#define  LIB_16BIT       0x01
#define  LIB_32BIT       0x02
#define  LIB_ADDR        0x04
#define  LIB_OFFSET      0x08
#define  LIB_SPACE       0x10

#define  CPU_CYCLES      1
#define  MICROSECONDS   (SYSTIMER_FREQ / 1000000)
#define  MILLISECONDS   (SYSTIMER_FREQ / 1000)
#define  SECONDS        (SYSTIMER_FREQ)

#define  CLK_START      1
#define  CLK_STOP       2
#define  CLK_PRINT      4

void     dump(uint8_t *pBuf, uint32_t len, uint8_t options, uint32_t offset);
uint32_t crc(pcm_msg_t msg, BOOLEAN crcChk);
void     srand_32(uint32_t _seed);
uint32_t rand_32(void);
void     clk_sleep(uint32_t delay, uint32_t units);
uint32_t clk_time(uint8_t op, uint32_t units, uint32_t *tNow);
void     utick(uint32_t microseconds);
void     stick(uint32_t seconds);
