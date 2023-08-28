/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Configurable Items

   1.2 Functional Description

      This code implements the Configurable Items functionality.

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
        7.1    ci_init()
        7.2    ci_read()
        7.3    ci_parse()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// 4.2   External Data Structures

// 4.3   External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

// 6.2  Local Data Structures

   static ci_entry_t ci_table[] = {
      //
      // Variable Text Name      Default Value           Data Type      Address to Variable        Dim
      // ==================      =============           =========      ===================        =====
      //
      { "ci.rev",                "0x00000000",           CI_HEX,        &ci.rev,                   1 },
      { "ci.check",              "0x00000000",           CI_HEX,        &ci.check,                 1 },
      { "ci.checksum",           "0x00000000",           CI_HEX,        &ci.checksum,              1 },
      { "ci.magic",              "0x55AA1234",           CI_HEX,        &ci.magic,                 1 },
      { "ci.debug",              "0x00000000",           CI_HEX,        &ci.debug,                 1 },
      { "ci.trace",              "0x00000A82",           CI_HEX,        &ci.trace,                 1 },
      { "ci.feature",            "0x00000000",           CI_HEX,        &ci.feature,               1 },
      { "ci.mac_addr_hi",        "0x0002C94E",           CI_HEX,        &ci.mac_addr_hi,           1 },
      { "ci.mac_addr_lo",        "0x7FC80000",           CI_HEX,        &ci.mac_addr_lo,           1 },
      { "ci.ip_addr",            "0xC0A80146",           CI_HEX,        &ci.ip_addr,               1 },
      { "ci.cm_udp_port",        "0x00000ADD",           CI_HEX,        &ci.cm_udp_port,           1 },
   };

// 7 MODULE CODE

// ===========================================================================

// 7.1

   uint32_t   ci_init(void) {

/* 7.1.1   Functional Description

   Initialize the Configurable Items software module.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   uint32_t   result = CFG_ERROR_OK;
   uint32_t   *pci   = (uint32_t*)&ci;
   uint32_t   i;

// 7.1.5   Code

   // Clear the CI DataBase
   memset(&ci, 0, sizeof(ci_t));

   // Initialize the CI with Default
   // Safe Values
   for (i=0;i<DIM(ci_table);i++) {
      switch(ci_table[i].type) {
         case CI_INT :
            sscanf(ci_table[i].value, "%ld", (int32_t *)ci_table[i].parm);
            break;
         case CI_UINT :
            sscanf(ci_table[i].value, "%ld", (uint32_t *)ci_table[i].parm);
            break;
         case CI_HEX :
            sscanf(ci_table[i].value, "%lx", (uint32_t *)ci_table[i].parm);
            break;
         case CI_DBL :
            sscanf(ci_table[i].value, "%lf", (double *)ci_table[i].parm);
            break;
         case CI_STR :
            sscanf(ci_table[i].value, "%s", (char *)ci_table[i].parm);
            break;
      }
   }

   // Compute Checksum
   for (i=0;i<sizeof(ci_t)>>2;i++) {
      ci.checksum += pci[i];
   }

   // Update global control
   gc.feature     = ci.feature;
   gc.trace       = ci.trace;
   gc.debug       = ci.debug;

   return result;

}  // end ci_init()

// ===========================================================================

// 7.2

uint32_t   ci_read(void) {

/* 7.2.1   Functional Description

   Initialize the Configurable Items software module.

   7.2.2   Parameters:

   NONE

   7.2.3   Return Values:

   result   CFG_ERROR_OK

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

   uint32_t   result = CFG_ERROR_OK;
   uint32_t   i;
   uint32_t   *pci = (uint32_t*)&ci;
   uint32_t   checksum, check_back;

// 7.2.5   Code

   if (gc.feature & CFG_FEATURE_USE_CI_FILE) {
      // Open and Parse CI File
      if (ci_parse(CFG_CI_FILENAME) != CFG_ERROR_OK) {
         if (gc.trace & CFG_TRACE_ERROR) {
           xlprint("ci_read() Error : CI file %s parse\n", CFG_CI_FILENAME);
         }
         // Restore Defaults
         ci_init();
      }
      // Validate Checksum
      else {
         // Validate CI Fields
         if (ci.magic != CFG_CI_MAGIC) {
            // Restore Defaults
            ci_init();
            // Flag the Error
            result = CFG_ERROR_CI_MAGIC;
            if (gc.trace & CFG_TRACE_ERROR) {
               xlprint("ci_read() Error : CI Magic Number Invalid, %08X\n", ci.magic);
            }
         }
         // Validate Checksum
         check_back = ci.checksum;
         ci.checksum = 0;
         checksum = 0;
         for (i=0;i<sizeof(ci_t)>>2;i++) {
            checksum += pci[i];
         }
         if (checksum != check_back) {
            // Restore Defaults
            ci_init();
            // Flag the Error
            result = CFG_ERROR_CI_CHKSUM;
            if (gc.trace & CFG_TRACE_ERROR) {
               xlprint("ci_read() Error : CI Checksum Invalid, %08X\n", check_back);
            }
         }
         // Restore Original Checksum
         else {
            ci.checksum = check_back;
         }
         // Update global control
         if (result == CFG_ERROR_OK) {
            gc.feature = ci.feature;
            gc.trace   = ci.trace;
            gc.debug   = ci.debug;
         }
      }
   }
   else {
      // Report CI Parameters
      if (gc.trace & CFG_TRACE_CI) {
         xlprint("ci database\n");
         xlprint("  ci.rev:          %08X\n", ci.rev);
         xlprint("  ci.check:        %08X\n", ci.check);
         xlprint("  ci.checksum:     %08X\n", ci.checksum);
         xlprint("  ci.magic:        %08X\n", ci.magic);
         xlprint("  ci.debug:        %08X\n", ci.debug);
         xlprint("  ci.trace:        %08X\n", ci.trace);
         xlprint("  ci.feature:      %08X\n", ci.feature);
         xlprint("  ci.mac_addr_hi:  %08X\n", ci.mac_addr_hi);
         xlprint("  ci.mac_addr_lo:  %08X\n", ci.mac_addr_lo);
         xlprint("  ci.ip_addr:      %08X\n", ci.ip_addr);
         xlprint("  ci.cm_udp_port:  %08X\n", ci.cm_udp_port);
         xlprint("\n");
      }
   }

   return result;

}  // end ci_read()


// ===========================================================================

// 7.3

uint32_t ci_parse(char *ci_file) {

/* 7.3.1   Functional Description

   This routine will parse the CI parameter file.

   7.3.2   Parameters:

   ci_file    CI Filename and Path

   7.3.3   Return Values:

   result   CFG_ERROR_OK
            CFG_ERROR_CI if parsing error

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

   uint32_t    result = CFG_ERROR_OK;
   uint32_t    i;
   FILE       *fid;
   char        line[512];
   char       *token;

// 7.5.5   Code

   // Validate filename
   if (ci_file != NULL) {

      // cycle over CI file and parse parameters
      if ((fid = fopen(ci_file, "rt")) != NULL) {
         // read next line
         while (fgets(line, sizeof(line), fid) != NULL) {
            // ignore comments
            if (line[0] == '#') continue;
            // ignore blank lines
            if (line[0] == '\n') continue;
            if (line[0] == '\r') continue;
            // ignore lines that begin with a space
            if (line[0] == ' ') continue;
            // extract key
            token = strtok(line, " ;\t=\n\r");
            // end-of-file
            if (strcmp(token, "@EOF") == 0) break;
            // cycle over table
            for (i=0;i<DIM(ci_table);i++) {
               if (strcmp(token, ci_table[i].key) == 0) {
                  token = strtok(NULL, " ;\t=\n\r");
                  switch(ci_table[i].type) {
                     case CI_INT :
                        sscanf(token, "%ld", (int32_t *)ci_table[i].parm);
                        break;
                     case CI_UINT :
                        sscanf(token, "%ld", (uint32_t *)ci_table[i].parm);
                        break;
                     case CI_HEX :
                        sscanf(token, "%lx", (uint32_t *)ci_table[i].parm);
                        break;
                     case CI_DBL :
                        sscanf(token, "%lf", (double *)ci_table[i].parm);
                        break;
                     case CI_STR :
                        sscanf(token, "%s", (char *)ci_table[i].parm);
                        break;
                  }
                  break;
               }
            }
            if (i == DIM(ci_table))
               xlprint("ci_parse() Warning : Unknown Parameter %s\n", token);
         }
         fclose(fid);

         // Report CI Parameters
         if (gc.trace & CFG_TRACE_CI) {
            xlprint("ci_parse() : Read from %s File\n", ci_file);
            for (i=0;i<DIM(ci_table);i++) {
               switch(ci_table[i].type) {
                  case CI_INT :
                     xlprint("    %s = %d\n", ci_table[i].key, *(int32_t *)ci_table[i].parm);
                     break;
                  case CI_UINT :
                     xlprint("    %s = %d\n", ci_table[i].key, *(uint32_t *)ci_table[i].parm);
                     break;
                  case CI_HEX :
                     xlprint("    %s = 0x%08X\n", ci_table[i].key, *(uint32_t *)ci_table[i].parm);
                     break;
                  case CI_DBL :
                     xlprint("    %s = %lf\n", ci_table[i].key, *(double *)ci_table[i].parm);
                     break;
                  case CI_STR :
                     xlprint("    %s = %s\n", ci_table[i].key, (char *)ci_table[i].parm);
                     break;
               }
            }
            xlprint("\n");
         }
      }
      else {
         result = CFG_ERROR_CI;
         xlprint("ci_parse() Fatal Error : CI File %s did not Open\n", ci_file);
      }

   }

   return result;

} // end ci_parse()

