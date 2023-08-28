#pragma once

#define  STAMP_DEV_NAME        "stamp"

#define  STAMP_OK              0x00000000
#define  STAMP_ERROR           0x80000001
#define  STAMP_ERR_OPEN        0x80000002
#define  STAMP_ERR_NODEV       0x80000004
#define  STAMP_ERR_MMAP        0x80000008

// All Registers
typedef struct _stamp_regs_t {
   uint32_t       pid;
   uint32_t       epoch;
   uint32_t       date;
   uint32_t       time;
   uint32_t       inc;
   uint32_t       test;
   uint32_t       tp;
   uint32_t       magic;
   uint32_t       count;
   uint32_t       version;
   uint32_t       map_date;
} stamp_regs_t, *pstamp_regs_t;

uint32_t  stamp_init(void);
uint32_t  stamp_sysid(void);
uint32_t  stamp_epoch(void);
uint32_t  stamp_date(void);
uint32_t  stamp_time(void);
uint32_t  stamp_version(void);
uint32_t  stamp_count(void);
uint32_t  stamp_magic(void);
uint32_t  stamp_inc(void);

