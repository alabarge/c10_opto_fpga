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
   static void loop_f(int argc, char **argv);
   static void clear_f(int argc, char **argv);
   static void traffic_f(int argc, char **argv);
   static void uart_f(int argc, char **argv);
   static void trace_f(int argc, char **argv);
   static void reset_f(int argc, char **argv);
   static void recon_f(int argc, char **argv);
   static void mdio_f(int argc, char **argv);
   static void lan_f(int argc, char **argv);
   static void rpc_f(int argc, char **argv);
   static void irq_f(int argc, char **argv);
   static void md_loop_f(int argc, char **argv);
   static void mw_loop_f(int argc, char **argv);
   static void reg_test_f(int argc, char **argv);
   static void mem_test_f(int argc, char **argv);

   static cmd_t cmd_tbl[] = {
      {.cmd = "help",      .func = help_f       },
      {.cmd = "ver",       .func = ver_f        },
      {.cmd = "md",        .func = md_f         },
      {.cmd = "mw",        .func = mw_f         },
      {.cmd = "loop",      .func = loop_f       },
      {.cmd = "clear",     .func = clear_f      },
      {.cmd = "traffic",   .func = traffic_f    },
      {.cmd = "uart",      .func = uart_f       },
      {.cmd = "trace",     .func = trace_f      },
      {.cmd = "reset",     .func = reset_f      },
      {.cmd = "recon",     .func = recon_f      },
      {.cmd = "mdio",      .func = mdio_f       },
      {.cmd = "lan",       .func = lan_f        },
      {.cmd = "rpc",       .func = rpc_f        },
      {.cmd = "irq",       .func = irq_f        },
      {.cmd = "md_loop",   .func = md_loop_f    },
      {.cmd = "mw_loop",   .func = mw_loop_f    },
      {.cmd = "reg_test",  .func = reg_test_f   },
      {.cmd = "mem_test",  .func = mem_test_f   },
   };

   static uint32_t   loop_ms = 10;

// 7 MODULE CODE

// ===========================================================================

// 7.1

void cli_init(void) {

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
    cli_lib_init(&gc.cli);

} // end cli_init()

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
   xlprint("               md 0x10030014 0x100 0x00\n");
   xlprint("               options: ASCII 0x80, 16BIT 0x01, 32BIT 0x02\n");
   xlprint("   mw,         memory write : mw address value options\n");
   xlprint("               mw 0x10030014 0x100\n");
   xlprint("   loop,       set loop count in milliseconds\n");
   xlprint("               loop ms\n");
   xlprint("               loop 1000\n");
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
   xlprint("   md_loop     periodically read the selected address\n");
   xlprint("               md_loop [address] [loop_ms]\n");
   xlprint("               md_loop 0x10030014 1\n");
   xlprint("   mw_loop     periodically write the selected address\n");
   xlprint("               mw_loop [address] [data] [loop_ms]\n");
   xlprint("               mw_loop 0x10030014 0x01234567 1\n");
   xlprint("   reg_test    periodically write and read verify the selected register address\n");
   xlprint("               reg_test [address] [data] [loop_ms]\n");
   xlprint("               reg_test 0x10030014 0x01234567 1\n");
   xlprint("   mem_test    periodically write and read verify the selected memory address\n");
   xlprint("               mem_test [address] [count] [loop_ms]\n");
   xlprint("               mem_test 0x00800000 0x400 1\n");
   xlprint("\n");
   xlprint("default loop time : %d ms\n\n", loop_ms);
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
   version();
}

void md_f(int argc, char **argv) {
   if(argc == 1) {
      xlprint("   md,    memory display : md address length options\n");
      xlprint("          md 0x10003014 0x100 0x00\n");
      xlprint("          options: ASCII 0x80, 32BIT 0x02\n");
   }
   else {
      if (argv[1] != NULL)
         sscanf(argv[1], "%lx", &gc.cli.loop_addr);
      if (argv[2] != NULL)
         gc.cli.loop_len = (uint32_t)strtol(argv[2], NULL, 16);
      if (argv[3] != NULL)
         gc.cli.loop_flags = (uint32_t)strtol(argv[3], NULL, 16);
      gc.cli.loop_flags &= 0xF0;
      gc.cli.loop_flags |= 0x02; // only 32-bit access allowed
     dump((uint8_t *)gc.cli.loop_addr, gc.cli.loop_len,
           gc.cli.loop_flags, gc.cli.loop_addr);
   }
}

void mw_f(int argc, char **argv) {
   if(argc == 1) {
      xlprint("   mw,    memory write : mw address value\n");
      xlprint("          mw 0x10003014 0x100\n");
   }
   else {
      if (argv[1] != NULL)
         sscanf(argv[1], "%lx", &gc.cli.loop_addr);
      if (argv[2] != NULL)
         sscanf(argv[2], "%lx", &gc.cli.loop_val);
      *(volatile uint32_t *)gc.cli.loop_addr = gc.cli.loop_val;
   }
}

void loop_f(int argc, char **argv) {
   uint32_t loop_time = loop_ms;

   if (argv[1] != NULL)
      loop_time = (uint32_t)strtol(argv[1], NULL, 10);
   loop_ms = loop_time;

   xlprint("\nloop interval : %d ms\n\n", loop_ms);
}

void clear_f(int argc, char **argv) {
   char     clr_scrn[]  = {0x1B, '[', '2', 'J', 0x00};
   char     cur_home[]  = {0x1B, '[', 'H', 0x00};
   xlprint(clr_scrn);
   xlprint(cur_home);
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

void md_loop_f(int argc, char **argv) {
   // loop init
   if (gc.cli.looping == 0) {
      gc.cli.looping = 1;
      gc.cli.loop_end = 0;
      gc.cli.loop_fn = md_loop_f;
      gc.cli.loop_ms = MILLISECONDS * loop_ms;
      gc.cli.loop_snap = alt_timestamp();
      gc.cli.loop_cnt = 0;
      gc.cli.loop_addr = 0x10030014;
      gc.cli.loop_val = 0x0;
      gc.cli.loop_flags = 0x0;
      if (argv[1] != NULL) sscanf(argv[1], "%lx", &gc.cli.loop_addr);
      if (argv[2] != NULL) {
         loop_ms = (uint8_t)strtol(argv[2], NULL, 10);
         gc.cli.loop_ms = MILLISECONDS * loop_ms;
      }
      xlprint("*** Hit Enter to Exit Looping, Reading Address %08X ***\n",
         gc.cli.loop_addr);
   }
   // loop run
   else {
      gc.cli.loop_val = *(volatile uint32_t *)gc.cli.loop_addr;
   }
}

void mw_loop_f(int argc, char **argv) {
   // loop init
   if (gc.cli.looping == 0) {
      gc.cli.looping = 1;
      gc.cli.loop_end = 0;
      gc.cli.loop_fn = mw_loop_f;
      gc.cli.loop_ms = MILLISECONDS * loop_ms;
      gc.cli.loop_snap = alt_timestamp();
      gc.cli.loop_cnt = 0;
      gc.cli.loop_addr = 0x10030014;
      gc.cli.loop_val = 0x0;
      gc.cli.loop_flags = 0x0;
      if (argv[1] != NULL) sscanf(argv[1], "%lx", &gc.cli.loop_addr);
      if (argv[2] != NULL) sscanf(argv[2], "%lx", &gc.cli.loop_val);
      if (argv[3] != NULL) {
         loop_ms = (uint8_t)strtol(argv[3], NULL, 10);
         gc.cli.loop_ms = MILLISECONDS * loop_ms;
      }
      xlprint("*** Hit Enter to Exit Looping, Writing Address %08X, Value %08X ***\n",
         gc.cli.loop_addr, gc.cli.loop_val);
   }
   // loop run
   else {
      *(volatile uint32_t *)gc.cli.loop_addr = gc.cli.loop_val;
   }
}

void reg_test_f(int argc, char **argv) {
   volatile uint32_t readback;
   volatile uint32_t *paddr;

   // loop init
   if (gc.cli.looping == 0) {
      gc.cli.looping = 1;
      gc.cli.loop_end = 0;
      gc.cli.loop_fn = reg_test_f;
      gc.cli.loop_ms = MILLISECONDS * loop_ms;
      gc.cli.loop_snap = alt_timestamp();
      gc.cli.loop_cnt = 0;
      gc.cli.loop_addr = 0x10030014;
      gc.cli.loop_val = 0x0;
      gc.cli.loop_flags = 0x0;
      if (argv[1] != NULL) sscanf(argv[1], "%lx", &gc.cli.loop_addr);
      if (argv[2] != NULL) sscanf(argv[2], "%lx", &gc.cli.loop_val);
      if (argv[3] != NULL) {
         loop_ms = (uint8_t)strtol(argv[3], NULL, 10);
         gc.cli.loop_ms = MILLISECONDS * loop_ms;
      }
      srand(100);
      if (gc.cli.loop_val == 0x0) {
         gc.cli.loop_flags = 0x1;
         xlprint("*** Hit Enter to Exit Looping, Writing Address %08X, Value %08X ***\n",
            gc.cli.loop_addr, gc.cli.loop_val);
      }
      else {
         xlprint("*** Hit Enter to Exit Looping, Writing Address %08X, Value rand() ***\n",
            gc.cli.loop_addr);
      }
   }
   // loop run
   else {
      paddr = (volatile uint32_t *)gc.cli.loop_addr;
      if (gc.cli.loop_flags & 0x1) gc.cli.loop_val = rand() ^ (rand() << 1);
      paddr[0] = gc.cli.loop_val;
      readback = paddr[0];
      if (readback != gc.cli.loop_val) {
         xlprint("reg_test() failed: %08X %08X\n", readback, gc.cli.loop_val);
         gc.cli.loop_end = 1;
      }
      else if ((gc.cli.loop_cnt % 5000) == 0) {
         xlprint("reg_test() passed : %d\n", gc.cli.loop_cnt);
         xlprint("reg_test() %08X %08X\n", readback, gc.cli.loop_val);
      }
   }
}

void mem_test_f(int argc, char **argv) {
   volatile uint32_t *paddr;
   uint32_t readback;
   uint32_t val;
   uint32_t i;

   // loop init
   if (gc.cli.looping == 0) {
      gc.cli.looping = 1;
      gc.cli.loop_end = 0;
      gc.cli.loop_fn = mem_test_f;
      gc.cli.loop_ms = MILLISECONDS * loop_ms;
      gc.cli.loop_snap = alt_timestamp();
      gc.cli.loop_cnt = 0;
      gc.cli.loop_addr = 0x00800000;
      gc.cli.loop_val = 0x400;
      gc.cli.loop_flags = 0x0;
      if (argv[1] != NULL) sscanf(argv[1], "%lx", &gc.cli.loop_addr);
      if (argv[2] != NULL) sscanf(argv[2], "%lx", &gc.cli.loop_val);
      if (argv[3] != NULL) {
         loop_ms = (uint8_t)strtol(argv[3], NULL, 10);
         gc.cli.loop_ms = MILLISECONDS * loop_ms;
      }
      xlprint("*** Hit Enter to Exit Looping, Write/Read Address %08X, Count %d ***\n",
         gc.cli.loop_addr, gc.cli.loop_val);
   }
   // loop run
   else {
      paddr = (volatile uint32_t *)gc.cli.loop_addr;
      srand(100);
      // fill memory block with 32-Bit random numbers
      for (i=0;i<gc.cli.loop_val;i++) {
         paddr[i] = rand() ^ (rand() << 1);
      }
      // verify fill
      srand(100);
      for (i=0;i<gc.cli.loop_val;i++) {
         readback = paddr[i];
         val = rand() ^ (rand() << 1);
         if (readback != val) {
            xlprint("mem_test() failed @: %08X %08X %08X\n", &paddr[i], readback, val);
            gc.cli.loop_end = 1;
            break;
         }
      }
      if (i == gc.cli.loop_val) {
         xlprint("mem_test() passed : %d\n", gc.cli.loop_cnt);
      }
   }
}
