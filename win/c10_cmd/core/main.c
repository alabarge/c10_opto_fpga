/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      DE10-I DATA ACQUISITION APPLICATION

   1.2 Functional Description

      This module is responsible for implementing the main embedded
      application for the TERASIC DE0-Nano board.

      Requires --override=nios2-flash-override.txt when running the
      nios2-flash-programmer command.

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
        7.1  main()
        7.2  timer()
        7.3  user_control_c()
        7.4  usage()
        7.5  cc_parse()

-----------------------------------------------------------------------------*/

// 3 VOCABULARY

// 4 EXTERNAL RESOURCES

// 4.1  Include Files

#include "main.h"

// command & control table
#include "cc.h"

// message string table
#include "msg_str.h"

#pragma comment(lib,"ws2_32.lib")

// 4.2   External Data Structures

   // global control
   gc_t     gc;

   // configurable items
   ci_t     ci;

   // command & control
   cc_t     cc;

   // month table for date-time strings
   char  *month_table[] = {
            "JAN", "FEB", "MAR", "APR",
            "MAY", "JUN", "JUL", "AUG",
            "SEP", "OCT", "NOV", "DEC"
          };

// 4.3   External Function Prototypes

// 5 LOCAL CONSTANTS AND MACROS

// 6 MODULE DATA STRUCTURES

// 6.1  Local Function Prototypes

// 6.2  Local Data Structures

   // source MAC and IP (the device running this application)
   uint8_t  macip[20] = { 0x6C, 0x2B, 0x59, 0xD6, 0xF5, 0x04,
                          0xC0, 0xA8, 0x01, 0x3E };

// 7 MODULE CODE

// ===========================================================================

// 7.1

int main(int argc, char *argv[]) {

/* 7.1.1   Functional Description

   This is the main entry point for the embedded application.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

   int32_t     i;

// 7.1.5   Code

   // Start the Log
   log_reset_state();
   log_set_log_file("de0_win_log.txt");

   // UDP Log
   udp_log("\nDE0-I WIN, %s\n\n", BUILD_HI);

   // Report the Startup Banner
   printf("\nDE0-I WIN, %s\n\n", BUILD_HI);

   // Clear gc, the apps global data structure
   memset(&gc, 0, sizeof(gc_t));

   // Initialize gc
   gc.feature     = 0;
   gc.trace       = 0;
   gc.debug       = 0;
   gc.status      = WIN_STATUS_INIT;
   gc.error       = WIN_ERROR_CLEAR;
   gc.devid       = CM_DEV_WIN;
   gc.winid       = CM_DEV_WIN;
   gc.com_port    = CM_PORT_COM0;
   gc.sys_time    = 0;
   gc.ping_cnt    = 0;
   gc.halt        = FALSE;
   gc.cmd_file    = NULL;
   gc.quiet       = FALSE;
   gc.month       = month_table;
   gc.msg_table   = msg_table;
   gc.msg_table_len = DIM(msg_table);

   sprintf(gc.dev_str,"DE0-I WIN, %s", BUILD_STR);

   // parse the command line
   for (i=1;i<argc;i++) {
      if (strcmp(argv[i], "-h") == 0) {
         usage();
      }
      else if (strcmp(argv[i], "-f") == 0)
         gc.cmd_file = argv[i+1];
      else if (strcmp(argv[i], "-q") == 0)
         gc.quiet = TRUE;
   }

   printf("%s ", argv[0]);
   for (i=1;i<argc;i++) {
      printf("%s ", argv[i]);
   }
   printf("\n\n");

   // Initialize the Configurable Items DataBase
   // and read the stored CIs
   gc.error |= ci_init();
   gc.error |= ci_read();

   // Parse the Command & Control File, this
   // function can modify CI values
   gc.error |= cc_parse(gc.cmd_file);

   //
   // INIT THE HARDWARE & CM
   //

   // CM Init
   gc.error |= cm_init();

   // Load MAC Address
   macip[10] = (cc.opc_mac_addr_hi & 0xFF000000) >> 24;
   macip[11] = (cc.opc_mac_addr_hi & 0x00FF0000) >> 16;
   macip[12] = (cc.opc_mac_addr_hi & 0x0000FF00) >> 8;
   macip[13] = (cc.opc_mac_addr_hi & 0x000000FF) >> 0;
   macip[14] = (cc.opc_mac_addr_lo & 0xFF000000) >> 24;
   macip[15] = (cc.opc_mac_addr_lo & 0x00FF0000) >> 16;

   // Load IP Address
   macip[16] = (cc.opc_ip_addr & 0xFF000000) >> 24;
   macip[17] = (cc.opc_ip_addr & 0x00FF0000) >> 16;
   macip[18] = (cc.opc_ip_addr & 0x0000FF00) >> 8;
   macip[19] = (cc.opc_ip_addr & 0x000000FF) >> 0;

   // UDP Init
//   gc.error |= udp_init(CFG_BAUD_RATE, CM_PORT_COM0, cc.opc_comport, cc.opc_cm_udp_port, macip);

   // FIFO Init
   gc.error |= fifo_init(CFG_BAUD_RATE, CM_PORT_COM0, cc.opc_comport);

   // Check for fatal Errors
   if (gc.error != WIN_ERROR_OK) {
      printf("main() Fatal Error : Hardware Initialization, gc.error = %08X\n", gc.error);
      return -1;
   }

   // Start main()'s Periodic Timer, WIN_TICKS_PER_SECOND
   gc.q_timer = CreateTimerQueue();
   CreateTimerQueueTimer(&gc.h_timer, gc.q_timer, timer,
         NULL, WIN_TICKS_MS, WIN_TICKS_MS, 0);

   // Start high-performance timer
   tmr_start();

   // Report Status and Error Results
   if (gc.trace & WIN_TRACE_POST) {
      printf("trace   :  %08X\n", gc.trace);
      printf("feature :  %08X\n", gc.feature);
      printf("status  :  %08X\n", gc.status);
      printf("error   :  %08X\n\n", gc.error);
   }

   //
   // START THE CLIENTS & SERVICES
   //

   // Control Panel (CP) Client
   gc.error |= cp_init();

   // Operation Code (OPC) Service
   gc.error |= opc_init();

   // Send CM Registration Request
   cm_send_reg_req(CM_DEV_DE0, CM_PORT_COM0, CM_REG_OPEN, gc.dev_str);

   // Main Thread
   while (1) {
      Sleep(100);
      // keyboard ctl c signal
      if (_kbhit()) {
         getchar();
         break;
      }
      // halt the application
      if (gc.halt == TRUE && !(gc.feature & WIN_FEATURE_IGNORE_HALT)) {
         break;
      }
   }

   // Shutdown Timers and Threads
   user_control_c();

   return 0;

} // end main()


// ===========================================================================

// 7.2

VOID CALLBACK timer(PVOID lpParam, BOOLEAN TimerOrWaitFired) {

/* 7.2.1   Functional Description

   This is the main system timer callback function for handling background
   periodic events.

   NOTE: Timing intervals are not precise for this callback.

   7.2.2   Parameters:

   lpParam
   TimerOrWaitFired

   7.2.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.2.4   Data Structures

// 7.2.5   Code

   // System Time Tick
   gc.sys_time++;

   // Periodic Client & Service Ticks
   cp_tick();
   opc_tick();
   cm_tick();

} // end timer()


// ===========================================================================

// 7.3

void user_control_c(void) {

/* 7.3.1   Functional Description

   This routine will handle the user control-c signal and shutdown the timers
   and threads.

   7.3.2   Parameters:

   NONE

   7.3.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.3.4   Data Structures

// 7.2.5   Code

   printf("exit() ... ");

   // Delete Timer Queue
   DeleteTimerQueue(gc.q_timer);

   // Close WinSock
   WSACleanup();

   // Stop Threads
   cp_final();
   opc_final();
   opto_final();
   cm_final();

   printf("done\n");

   exit((gc.error == WIN_ERROR_OK) ? 0 : -1);

} // end user_control_c()


// ===========================================================================

// 7.4

void usage(void) {

/* 7.4.1   Functional Description

   This routine will report the command line parameters.

   7.4.2   Parameters:

   NONE

   7.4.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.4.4   Data Structures

// 7.4.5   Code

   printf("\n");
   printf("usage: de0_cmd [-h][-f filename][-q]\n\n");
   printf("  This utility will execute the operation code and parameters in the command file.\n");
   printf("  -h       ... usage\n");
   printf("  -f       ... specifies the command input filename\n");
   printf("  -q       ... disable stdio output\n");
   printf("\n");

   exit(0);

} // end usage()


// ===========================================================================

// 7.5

uint32_t cc_parse(char *cmd_file) {

/* 7.5.1   Functional Description

   This routine will parse the cc command file.

   7.5.2   Parameters:

   cmd_file    Command Filename and Path

   7.5.3   Return Values:

   result   WIN_ERROR_OK
            WIN_ERROR_CC if parsing error

-----------------------------------------------------------------------------
*/

// 7.5.4   Data Structures

   uint32_t    result = WIN_ERROR_OK;
   uint32_t    i;
   FILE       *cmd;
   char        line[512];
   char       *token;

// 7.5.5   Code

   // load default values from table
   for (i=0;i<DIM(cc_table);i++) {
      switch(cc_table[i].type) {
         case CC_INT :
            sscanf(cc_table[i].value, "%d", (int32_t *)cc_table[i].parm);
            break;
         case CC_UINT :
            sscanf(cc_table[i].value, "%d", (uint32_t *)cc_table[i].parm);
            break;
         case CC_HEX :
            sscanf(cc_table[i].value, "%x", (uint32_t *)cc_table[i].parm);
            break;
         case CC_DBL :
            sscanf(cc_table[i].value, "%lf", (double *)cc_table[i].parm);
            break;
         case CC_STR :
            sscanf(cc_table[i].value, "%s", (char *)cc_table[i].parm);
            break;
      }
   }

   // filename isn't null
   if (cmd_file != NULL) {
      // cycle over command file and parse parameters
      if ((cmd = fopen(cmd_file, "rt")) != NULL) {
         // read next line
         while (fgets(line, sizeof(line), cmd) != NULL) {
            // ignore comments
            if (line[0] == '#') continue;
            // ignore blank lines
            if (line[0] == '\n') continue;
            // ignore lines that begin with a space
            if (line[0] == ' ') continue;
            // end-of-file
            if (strcmp(line, "@EOF") == 0) break;
            token = strtok(line, " ;\t=");
            for (i=0;i<DIM(cc_table);i++) {
               if (strcmp(token, cc_table[i].key) == 0) {
                  token = strtok(NULL, " ;\t=");
                  switch(cc_table[i].type) {
                     case CC_INT :
                        sscanf(token, "%d", (int32_t *)cc_table[i].parm);
                        break;
                     case CC_UINT :
                        sscanf(token, "%d", (uint32_t *)cc_table[i].parm);
                        break;
                     case CC_HEX :
                        sscanf(token, "%x", (uint32_t *)cc_table[i].parm);
                        break;
                     case CC_DBL :
                        sscanf(token, "%lf", (double *)cc_table[i].parm);
                        break;
                     case CC_STR :
                        sscanf(token, "%s", (char *)cc_table[i].parm);
                        break;
                  }
                  break;
               }
            }
            if (i == DIM(cc_table))
               printf("cc_parse() Warning : Unknown Parameter %s\n", token);
         }
         fclose(cmd);
         // Report CC Parameters
         if (gc.trace & WIN_TRACE_CI) {
            for (i=0;i<DIM(cc_table);i++) {
               switch(cc_table[i].type) {
                  case CC_INT :
                     printf("%s = %d\n", cc_table[i].key, *(int32_t *)cc_table[i].parm);
                     break;
                  case CC_UINT :
                     printf("%s = %d\n", cc_table[i].key, *(uint32_t *)cc_table[i].parm);
                     break;
                  case CC_HEX :
                     printf("%s = 0x%x\n", cc_table[i].key, *(uint32_t *)cc_table[i].parm);
                     break;
                  case CC_DBL :
                     printf("%s = %lf\n", cc_table[i].key, *(double *)cc_table[i].parm);
                     break;
                  case CC_STR :
                     printf("%s = %s\n", cc_table[i].key, (char *)cc_table[i].parm);
                     break;
               }
            }
         }
      }
      else {
         result = WIN_ERROR_CC;
         printf("cc_parse() Fatal Error : CC File %s did not Open\n", cmd_file);
      }
   }

   return result;

} // end cc_parse()

