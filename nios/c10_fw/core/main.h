#pragma once

#include "sys/alt_stdio.h"
#include "sys/alt_alarm.h"
#include "sys/alt_timestamp.h"
#include "altera_epcq_controller2.h"
#include "sys/alt_flash.h"
#include "sys/alt_irq.h"
#include "sys/alt_cache.h"
#include "nios2.h"
#include "alt_types.h"

#include <math.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fw_cfg.h"
#include "type.h"
#include "cm_const.h"
#include "io.h"
#include "ci.h"
#include "cm.h"
#include "build.h"
#include "cli_defs.h"
#include "cli_lib.h"
#include "cli.h"

#include "cp_msg.h"
#include "cp_hal.h"
#include "cp_srv.h"

#include "daq_msg.h"
#include "daq_hal.h"
#include "daq_srv.h"

// BOOLEAN DEFINES
#define TRUE      1
#define FALSE     0

#define DE0_OK    0

#define  MSG_MAX_STR    32

// Byte & Word Manipulation
#define DIM(x)    (sizeof(x)/sizeof(*(x)))
#define HIBYTE(x) (((x) >> 8) & 0xFF)
#define LOBYTE(x) ((x) & 0xFF)
#define HIWORD(x) (((x) >> 16) & 0xFFFFL)
#define LOWORD(x) ((x) & 0xFFFFL)
#define ABS(a)    (((a) < 0) ? (-(a)) : (a))

// Macros to Handle Endianess
#define swap16(x) ((uint16_t) (((x) <<  8) | (((x) >>  8) & 0xFF)))
#define swap32(x) ((uint32_t) (((x) << 24) | (((x) <<  8) & 0xFF0000L) | \
                  (((x) >> 8) & 0xFF00L) | (((x) >> 24) & 0xFFL)))
#define swap32s(x) ((x & 0x0000FFFF) << 16) | \
                   ((x & 0xFFFF0000) >> 16)

// Input/Output Macros - Cache is bypassed
#define in32(BASE) \
  __builtin_ldwio ((void *)((uint8_t*)(BASE)))
#define out32(BASE, DATA) \
  __builtin_stwio ((void *)((uint8_t*)(BASE)), DATA)
#define in16(BASE) \
  __builtin_ldhuio ((void *)((uint8_t*)(BASE)))
#define out16(BASE, DATA) \
  __builtin_sthio ((void *)((uint8_t*)(BASE)), DATA)
#define in8(BASE) \
  __builtin_ldbuio ((void *)((uint8_t*)(BASE)))
#define out8(BASE, DATA) \
  __builtin_stbio ((void *)((uint8_t*)(BASE)), DATA)

// message string table
typedef struct _msg_entry_t {
   uint8_t     cmid;
   uint8_t     msgid;
   char        cmid_str[MSG_MAX_STR];
   char        msg_str[MSG_MAX_STR];
} msg_entry_t, *pmsg_entry_t;

//
// GLOBAL CONTROL DATA STRUCTURE
//
typedef struct _gc_t {
   uint32_t       feature;
   uint32_t       trace;
   uint32_t       debug;
   uint32_t       status;
   uint32_t       error;
   uint8_t        devid;
   uint8_t        winid;
   uint8_t        com_port;
   uint8_t        int_flag;
   time_t         timestamp;
   uint32_t       sys_time;
   uint32_t       sysid;
   uint32_t       fpga_ver;
   uint32_t       fpga_time;
   uint32_t       fpga_date;
   uint8_t        dip_sw;
   uint32_t       key;
   uint8_t        sw_reset;
   uint8_t        buf[CM_MAX_MSG_INT8U];
   alt_alarm      alarm;
   alt_flash_fd   *fd;
   struct stat    st;
   wav_t          wav;
   uint32_t       ping_time;
   uint8_t        ping_cnt;
   uint8_t        led_cycle;
   FILE          *fp;
   alt_fd*        ro_fd;
   char           dev_str[CM_MAX_DEV_STR_LEN];
   char         **month;
   pmsg_entry_t   msg_table;
   uint16_t       msg_table_len;
   cli_t          cli;
}

gc_t, *pgc_t;

// Global Access
extern   ci_t ci;
extern   gc_t gc;

extern   uint8_t __bss_start;
extern   uint8_t __bss_end;
extern   uint8_t __alt_stack_limit;
extern   uint8_t __alt_stack_pointer;
extern   uint8_t __alt_heap_start;
extern   uint8_t __alt_heap_limit;

alt_u32  timer(void *context);
void     version(void);
uint32_t com_hwver(void);
