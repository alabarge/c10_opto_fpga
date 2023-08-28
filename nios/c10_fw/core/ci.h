#pragma once

#define  CI_MAX_KEY     32

#define  CI_INT         0
#define  CI_UINT        1
#define  CI_HEX         6
#define  CI_DBL         7
#define  CI_STR         9

//
// CI TABLE
//
typedef struct _ci_entry_t {
   char        key[CI_MAX_KEY];
   char        value[CI_MAX_KEY];
   uint8_t     type;
   void       *parm;
   uint8_t     dim;
} ci_entry_t, *pci_entry_t;

//
// CONFIGURABLE ITEMS
//
typedef struct _ci_t {
   uint32_t    rev;                    // CI Revision
   uint32_t    check;                  // CI Boot Checksum
   uint32_t    checksum;               // CI Checksum
   uint32_t    magic;                  // Magic Number
   uint32_t    debug;                  // Software Debug
   uint32_t    trace;                  // Software Trace
   uint32_t    feature;                // Run-Time Features
   uint32_t    mac_addr_hi;            // MAC Address HI
   uint32_t    mac_addr_lo;            // MAC Address LO
   uint32_t    ip_addr;                // IP Address
   uint32_t    cm_udp_port;            // CM UDP Port
   uint32_t    pad[117];               // Sector Pad
} ci_t, *pci_t;

// CI Return Errors
#define  CI_OK          0x00
#define  CI_FAIL        0x01

uint32_t ci_init(void);
uint32_t ci_read(void);
uint32_t ci_parse(char *ci_file);

