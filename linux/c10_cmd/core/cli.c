/*-----------------------------------------------------------------------------

   1  ABSTRACT

   1.1 Module Type

      Command Line Interpreter (CLI) Commands

   1.2 Functional Description

      This code implements the CLI Command Routines.

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
        7.1   cli_gen()

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

   static void user_uart_println(char *string);

   static void help_f(int argc, char **argv);
   static void ver_f(int argc, char **argv);
   static void md_f(int argc, char **argv);
   static void mw_f(int argc, char **argv);
   static void traffic_f(int argc, char **argv);
   static void uart_f(int argc, char **argv);
   static void trace_f(int argc, char **argv);
   static void reset_f(int argc, char **argv);
   static void recon_f(int argc, char **argv);
   static void mdio_f(int argc, char **argv);
   static void lan_f(int argc, char **argv);
   static void rpc_f(int argc, char **argv);
   static void irq_f(int argc, char **argv);

   static cmd_t cmd_tbl[] = {
      {.cmd = "help",      .func = help_f       },
      {.cmd = "ver",       .func = ver_f        },
      {.cmd = "md",        .func = md_f         },
      {.cmd = "mw",        .func = mw_f         },
      {.cmd = "traffic",   .func = traffic_f    },
      {.cmd = "uart",      .func = uart_f       },
      {.cmd = "trace",     .func = trace_f      },
      {.cmd = "reset",     .func = reset_f      },
      {.cmd = "recon",     .func = recon_f      },
      {.cmd = "mdio",      .func = mdio_f       },
      {.cmd = "lan",       .func = lan_f        },
      {.cmd = "rpc",       .func = rpc_f        },
      {.cmd = "irq",       .func = irq_f        },
   };

// 7 MODULE CODE

// ===========================================================================

// 7.1

void cli_gen(void) {

/* 7.1.1   Functional Description

   This routine will initialize the command line interpreter and populate the
   CLI commands, from this source file.

   7.1.2   Parameters:

   NONE

   7.1.3   Return Values:

   NONE

-----------------------------------------------------------------------------
*/

// 7.1.4   Data Structures

// 7.1.5   Code

    gc.cli.println = user_uart_println;
    gc.cli.cmd_tbl = cmd_tbl;
    gc.cli.cmd_cnt = sizeof(cmd_tbl)/sizeof(cmd_t);
    cli_init(&gc.cli);

} // end cli_gen()

void user_uart_println(char *string) {
    xlprint(string);
}


#define  LIB_ASCII       0x80
#define  LIB_16BIT       0x01
#define  LIB_32BIT       0x02


void help_f(int argc, char **argv) {
   xlprint("\n");
   xlprint("%s\n", gc.dev_str);
   xlprint("hw stamp.date: %08X\n", stamp_date());
   xlprint("hw stamp.time: %08X\n", stamp_time());
   xlprint("available commands :\n\n");
   xlprint("   help,       report commands and parameters\n");
   xlprint("   ver,        firmware versions\n");
   xlprint("   md,         memory display : md address length options\n");
   xlprint("               md 0x00001000 0x100 0x00\n");
   xlprint("               options: ASCII 0x80, 16BIT 0x01, 32BIT 0x02\n");
   xlprint("   mw,         memory write : mw address value options\n");
   xlprint("               mw 0x00001000 0x100\n");
   xlprint("               options: 8BIT 0x00, 16BIT 0x01, 32BIT 0x02\n");
   xlprint("   traffic,    toggle CM traffic\n");
   xlprint("   uart,       toggle UART traffic\n");
   xlprint("   trace,      set/get trace flags\n");
   xlprint("               trace [0x00001000]\n");
   xlprint("   reset,      reset using watchdog\n");
   xlprint("   recon,      reconfigure FPGA\n");
   xlprint("   mdio,       read/write the LAN SMI : mdio register value\n");
   xlprint("               mdio 0x10 0x1234, an absent value will only read\n");
   xlprint("   lan,        toggle LAN traffic\n");
   xlprint("   rpc,        remote procedure call, project specific for debug\n");
   xlprint("   irq,        toggle IRQ trace\n");
   xlprint("\n");
   xlprint("status : \n\n");
   if (gc.trace & CFG_TRACE_CM_LOG)
      xlprint("   cm traffic = on\n");
   if (gc.trace & CFG_TRACE_UART)
      xlprint("   uart traffic = on\n");
   xlprint("   trace   : %08X = %08X\n", (uint32_t)&gc.trace, gc.trace);
   xlprint("   feature : %08X = %08X\n", (uint32_t)&gc.feature, gc.feature);
   xlprint("   debug   : %08X = %08X\n\n", (uint32_t)&gc.debug, gc.debug);
}

void ver_f(int argc, char **argv) {
   xlprint("\n");
   xlprint("hw stamp.date: %08X\n", stamp_date());
   xlprint("hw stamp.time: %08X\n", stamp_time());
   xlprint("%-13s base:irq %08X:%d\n", STAMP_NAME, STAMP_BASE, STAMP_IRQ);
   xlprint("%-13s base:irq %08X:%d\n", ADC_NAME, ADC_BASE, ADC_IRQ);
}

void md_f(int argc, char **argv) {
   uint32_t addr;
   uint32_t len;
   uint32_t options;
   if(argc != 4) {
      xlprint("   md,    memory display : md address length options\n");
      xlprint("          md 0x00001000 0x100 0x00\n");
      xlprint("          options: ASCII 0x80, 16BIT 0x01, 32BIT 0x02\n");
   }
   else {
      sscanf(argv[1], "%lx", &addr);
      sscanf(argv[2], "%lx", &len);
      sscanf(argv[3], "%lx", &options);
      dump((uint8_t *)addr, len, options, addr);
   }
}

void mw_f(int argc, char **argv) {
   uint32_t addr;
   uint32_t val;
   uint32_t options;
   if(argc != 4) {
      xlprint("   mw,    memory write : mw address value options\n");
      xlprint("          mw 0x00001000 0x100 0x00\n");
      xlprint("          options: ASCII 0x80, 16BIT 0x01, 32BIT 0x02\n");
   }
   else {
      sscanf(argv[1], "%lx", &addr);
      sscanf(argv[2], "%lx", &val);
      sscanf(argv[3], "%lx", &options);
      if (options & LIB_32BIT)
         *(uint32_t *)addr = val;
      else if (options & LIB_16BIT)
         *(uint16_t *)addr = (uint16_t)val;
      else
         *(uint8_t *)addr = (uint8_t)val;
   }
}

void traffic_f(int argc, char **argv) {
   if (gc.trace & CFG_TRACE_CM_LOG) {
      gc.trace &= ~CFG_TRACE_CM_LOG;
      xlprint("cm traffic log = off\n");
   }
   else {
      gc.trace |=  CFG_TRACE_CM_LOG;
      xlprint("cm traffic log = on\n");
   }
}

void uart_f(int argc, char **argv) {
   if (gc.trace & CFG_TRACE_UART) {
      gc.trace &= ~CFG_TRACE_UART;
      xlprint("uart traffic log = off\n");
   }
   else {
      gc.trace |=  CFG_TRACE_UART;
      xlprint("uart traffic log = on\n");
   }
}

void trace_f(int argc, char **argv) {
   uint32_t trace;
   if (argc == 2) {
      sscanf(argv[1], "%lx", &trace);
      gc.trace = trace;
   }
   xlprint("trace = 0x%08X\n", gc.trace);
}

void reset_f(int argc, char **argv) {
   xlprint("\n*** RESET USING WATCHDOG ***\n\n");
   while(1);
}

void recon_f(int argc, char **argv) {
   xlprint("\n*** RECONFIGURE FPGA ***\n\n");
   clk_sleep(500, MILLISECONDS);
   out32(UPDATE_BASE + 0x74, 1);
}

void mdio_f(int argc, char **argv) {
}

void lan_f(int argc, char **argv) {
   if (gc.trace & CFG_TRACE_LAN) {
      gc.trace &= ~CFG_TRACE_LAN;
      xlprint("lan traffic log = off\n");
   }
   else {
      gc.trace |=  CFG_TRACE_LAN;
      xlprint("lan traffic log = on\n");
   }
}

void rpc_f(int argc, char **argv) {
}

void irq_f(int argc, char **argv) {
   if (gc.trace & CFG_TRACE_IRQ) {
      gc.trace &= ~CFG_TRACE_IRQ;
      xlprint("IRQ trace = off\n");
   }
   else {
      gc.trace |=  CFG_TRACE_IRQ;
      xlprint("IRQ trace = on\n");
   }
}

