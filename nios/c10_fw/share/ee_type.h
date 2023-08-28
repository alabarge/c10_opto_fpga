#pragma once

//
// EEPROM DATA STRUCTURE, SHARED WITH WIN32
//
#ifdef WIN32
typedef struct _ee_type {
   uint8_t     rev;              // EE Revision
   uint8_t     magic[3];         // Magic Number
   uint32_t    debug;            // Software Debug
   uint32_t    trace;            // Software Trace
   uint32_t    run_time;         // Run-Time Options
   uint8_t     hw_part[15];      // HW Part Number String
   uint8_t     hw_rev;           // HW Revision
   uint8_t     pad[220];         // EEPROM Padding
   uint32_t    checksum;         // EE Checksum
} ee, *pee;
#else
typedef struct _ee_type {
   uint8_t     rev;              // EE Revision
   uint8_t     magic[3];         // Magic Number
   uint32_t    debug;            // Software Debug
   uint32_t    trace;            // Software Trace
   uint32_t    run_time;         // Run-Time Options
   uint8_t     hw_part[15];      // HW Part Number String
   uint8_t     hw_rev;           // HW Revision
   uint8_t     pad[220];         // EEPROM Padding
   uint32_t    checksum;         // EE Checksum
} ee, *pee;
#endif
