// recTypes.h : header file
//
#pragma once

#include "type.h"

// RECORD TYPES
#define  EEG_REC_NULL         0x00
#define  EEG_REC_DIR          0x01
#define  EEG_REC_BEG          0x02
#define  EEG_REC_END          0x03
#define  EEG_REC_REG          0x04
#define  EEG_REC_IQ           0x05

// SAMPLE SET RECORDS FIELD ENTRIES AND HEADERS
#define  EEG_IQ_HEADER        8
#define  EEG_IQ_LEN           252
#define  EEG_REC_LEN          CFG_SECTOR_SIZE

// SAMPLE SET FLAGS
#define  EEG_REC_DATA_VALID   0x80

// RECORD TYPE SELECTION FILTERS
#define  EEG_REC_SEL_IQ       0x00000001

// ALL RECORDS ARE FIXED LENGTH OF 512 BYTES. EVERY RECORD MAY BE IDENTIFIED
// USING THE FIRST BYTE FIELD IN THE RECORD, recID.

typedef struct _rec_null_t {
   uint8_t   recid;                // Record ID = 0x00
   uint8_t   flags;                // Record Flags, Always Zero
   uint8_t   unused[510];          // Always Zero
} rec_null_t, *prec_null_t;

//
// BEGIN-END RECORDS, FRAMING RECORDS TO DEMARCATE THE BEGINNING AND ENDING OF A
// PARTICULAR DATA ACQUISITION SESSION.
//
typedef struct _rec_beg_t {
   uint8_t   recid;                // Record ID = 0x02
   uint8_t   flags[3];             // Flags, Record Type Specific
   uint32_t  seqid;                // 32-bit Session Block Sequence
   uint32_t  status;               // 32-bit Machine Status
   uint32_t  error;                // 32-bit Error Status
   uint32_t  sys_time;             // 32-bit Beginning System Time
   uint8_t   esn[4];               // 8-Byte H/W Serial Number
   uint8_t   sw_ver[4];            // Embedded Software Version
   uint32_t  os_ver;               // Embedded OS Version
   uint32_t  boot_ver;             // Bootloader Version
   uint8_t   hw_ver;               // 8-bit H/W Version
   uint8_t   dq_ver;               // DQ Module Version
   uint8_t   sd_ver;               // SD Module Version
   uint8_t   uart_ver;             // UART Module Version
   uint8_t   run_cnt;              // Run Number
   uint8_t   pad[3];               // 4 Byte alignment padding
   uint8_t   unused[468];          // Always Zero
} rec_beg_t, *prec_beg_t;

typedef struct _rec_end_t {
   uint8_t   recid;                // Record ID = 0x03
   uint8_t   flags[3];             // Flags, Record Type Specific
   uint32_t  seqid;                // 32-bit Session Block Sequence
   uint32_t  status;               // 32-bit Machine Status
   uint32_t  error;                // 32-bit Error Status
   uint32_t  sys_time;             // 32-bit Beginning System Time
   uint8_t   unused[492];          // Always Zero
} rec_end_t, *prec_end_t;

typedef struct _rec_reg_t {
   uint8_t   recid;                // Record ID = 0x04
   uint8_t   flags[3];             // Flags, Record Type Specific
   uint32_t  seqid;                // 32-bit Session Block Sequence
   uint32_t  run_cmd;              // 32-bit Run Command Register
   uint32_t  dc_freq;              // 32-bit DDC Frequency Register
   uint32_t  dc_phase;             // 32-bit DDC Phase Register
   uint32_t  dc_count;             // 32-bit Sample Set Count Register
   uint32_t  rf_freq;              // 32-bit RF Frequency Register
   uint32_t  rf_phase;             // 32-bit RF Phase Register
   uint32_t  pdat_buf;             // 32-bit Data Store Pointer
   uint32_t  dat_nxt;              // 32-bit Next Data Record Index
   uint8_t   unused[480];          // Always Zero
} rec_reg_t, *prec_reg_t;

//
// SAMPLE SET RECORDS, EACH TYPE OF DATA SET HAS IT'S OWN RECORD FORMAT.
//
typedef struct _rec_iq_t {
   uint8_t   recid;                // Record ID = 0x05
   uint8_t   flags;                // ADC Overflow Flags
   uint8_t   ch_sel                // Channels Selected
   uint8_t   length;               // Samples within record
   uint32_t  seqid;                // 32-bit Session Block Sequence
   uint8_t   data[504];            // IQ Sample Set
} rec_iq_t, *prec_iq_t;
