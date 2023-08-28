#pragma once

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>


#include <winsock2.h>
#include <iphlpapi.h>
#include <sys/types.h>

// from f/w share
#include "fw_cfg.h"
#include "cm_const.h"
#include "daq_msg.h"
#include "cp_msg.h"
#include "fpga_build.h"

#include "log.h"
#include "udp_log.h"
#include "win_cfg.h"
#include "io.h"
#include "ci.h"
#include "cm.h"
#include "lib.h"

#include "opc_msg.h"

#include "opc_srv.h"
#include "cp_cli.h"

#include "build.h"

// BOOLEAN DEFINES
#define  TRUE           1
#define  FALSE          0

#define  DE0_OK         0

#define  CC_MAX_KEY     32

#define  CC_INT         0
#define  CC_UINT        1
#define  CC_HEX         6
#define  CC_DBL         7
#define  CC_STR         9

#define  MSG_MAX_STR    32

// Byte & Word Manipulation
#define  DIM(x)    (sizeof(x)/sizeof(*(x)))
#define  ABS(a)    (((a) < 0) ? (-(a)) : (a))

// Macros to Handle Endianess
#define  swap16(x) ((uint16_t) (((x) <<  8) | (((x) >>  8) & 0xFF)))
#define  swap32(x) ((uint32_t) (((x) << 24) | (((x) <<  8) & 0xFF0000L) | \
                   (((x) >> 8) & 0xFF00L) | (((x) >> 24) & 0xFFL)))
#define  swap32s(x) ((x & 0x0000FFFF) << 16) | \
                    ((x & 0xFFFF0000) >> 16)

// command and control table
typedef struct _cc_entry_t {
   char        key[CC_MAX_KEY];
   char        value[CC_MAX_KEY];
   uint8_t     type;
   void       *parm;
   uint8_t     dim;
} cc_entry_t, *pcc_entry_t;

// message string table
typedef struct _msg_entry_t {
   uint8_t     cmid;
   uint8_t     msgid;
   char        cmid_str[MSG_MAX_STR];
   char        msg_str[MSG_MAX_STR];
} msg_entry_t, *pmsg_entry_t;

//
// COMMAND & CONTROL
//
typedef struct _cc_t {
   int32_t     opc_opcode;
   uint32_t    opc_timeout;
   uint32_t    opc_comport;
   uint32_t    opc_mac_addr_hi;
   uint32_t    opc_mac_addr_lo;
   uint32_t    opc_ip_addr;
   uint32_t    opc_cm_udp_port;
   char        daq_file[CM_MAX_FILE_LEN];
   uint32_t    daq_opcmd;
   uint32_t    daq_packets;
   uint32_t    daq_to_file;
   uint32_t    daq_file_type;
   uint32_t    daq_file_stamp;
   uint32_t    daq_ramp;
   uint32_t    daq_real;
} cc_t, *pcc_t;

//
// GLOBAL CONTROL DATA STRUCTURE 
//
typedef struct _gc_t {
   uint32_t    feature;
   uint32_t    trace;
   uint32_t    debug;
   uint32_t    status;
   uint32_t    error;
   uint8_t     devid;
   uint8_t     winid;
   uint8_t     com_port;
   time_t      timestamp;
   uint32_t    sys_time;
   uint32_t    sysid;
   uint32_t    fpga_ver;
   uint32_t    fpga_time;
   uint32_t    fpga_date;
   uint8_t     buf[CM_MAX_MSG_INT8U];
   HANDLE      q_timer;
   HANDLE      h_timer;
   time_t      ping_time;
   uint8_t     ping_cnt;
   uint8_t     halt;
   uint8_t     quiet;
   char       *cmd_file;
   char        dev_str[CM_MAX_DEV_STR_LEN];
   char      **month;
   pmsg_entry_t msg_table;
   uint16_t     msg_table_len;
} gc_t, *pgc_t;

// Global Access
extern   ci_t ci;
extern   gc_t gc;
extern   cc_t cc;

VOID CALLBACK timer(PVOID lpParam, BOOLEAN TimerOrWaitFired);

void     user_control_c(void);
void     usage(void);
uint32_t cc_parse(char *cmd_file);


