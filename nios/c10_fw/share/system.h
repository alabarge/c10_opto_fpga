/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'cpu' in SOPC Builder design 'c10_fpga'
 * SOPC Builder design path: D:/c10_opto_fpga/c10_top/PR_R2/c10_fpga.sopcinfo
 *
 * Generated: Wed Feb 04 08:18:57 PST 2026
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x10010020
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 100000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "fast"
#define ALT_CPU_DATA_ADDR_WIDTH 0x1d
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_EXCEPTION_ADDR 0x00000020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 100000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 1
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 1
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_DIVISION_ERROR_EXCEPTION
#define ALT_CPU_HAS_EXTRA_EXCEPTION_INFO
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 32
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_ICACHE_SIZE 4096
#define ALT_CPU_INST_ADDR_WIDTH 0x1d
#define ALT_CPU_NAME "cpu"
#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x102be6e0


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x10010020
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 100000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "fast"
#define NIOS2_DATA_ADDR_WIDTH 0x1d
#define NIOS2_DCACHE_LINE_SIZE 0
#define NIOS2_DCACHE_LINE_SIZE_LOG2 0
#define NIOS2_DCACHE_SIZE 0
#define NIOS2_EXCEPTION_ADDR 0x00000020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 1
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 1
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_DIVISION_ERROR_EXCEPTION
#define NIOS2_HAS_EXTRA_EXCEPTION_INFO
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 32
#define NIOS2_ICACHE_LINE_SIZE_LOG2 5
#define NIOS2_ICACHE_SIZE 4096
#define NIOS2_INST_ADDR_WIDTH 0x1d
#define NIOS2_NUM_OF_SHADOW_REG_SETS 0
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x102be6e0


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ADC
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_TIMER
#define __ALTERA_AVALON_UART
#define __ALTERA_EPCQ_CONTROLLER2
#define __ALTERA_NIOS2_GEN2
#define __ALTERA_REMOTE_UPDATE
#define __ALTPLL
#define __OPTO
#define __SDRAM
#define __STAMP


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "Cyclone 10 LP"
#define ALT_ENHANCED_INTERRUPT_API_PRESENT
#define ALT_IRQ_BASE NULL
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/stdout"
#define ALT_STDERR_BASE 0x10090000
#define ALT_STDERR_DEV stdout
#define ALT_STDERR_IS_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_uart"
#define ALT_STDIN "/dev/stdout"
#define ALT_STDIN_BASE 0x10090000
#define ALT_STDIN_DEV stdout
#define ALT_STDIN_IS_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_uart"
#define ALT_STDOUT "/dev/stdout"
#define ALT_STDOUT_BASE 0x10090000
#define ALT_STDOUT_DEV stdout
#define ALT_STDOUT_IS_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_uart"
#define ALT_SYSTEM_NAME "c10_fpga"
#define ALT_SYS_CLK_TICKS_PER_SEC SYSCLK_TICKS_PER_SEC
#define ALT_TIMESTAMP_CLK_TIMER_DEVICE_TYPE SYSTIMER_TIMER_DEVICE_TYPE


/*
 * adc configuration
 *
 */

#define ADC_BASE 0x100b0000
#define ADC_IRQ 6
#define ADC_IRQ_INTERRUPT_CONTROLLER_ID 0
#define ADC_NAME "/dev/adc"
#define ADC_SPAN 8192
#define ADC_TYPE "adc"
#define ALT_MODULE_CLASS_adc adc


/*
 * altera_ro_zipfs configuration
 *
 */

#define ALTERA_RO_ZIPFS_BASE 0x10200000
#define ALTERA_RO_ZIPFS_NAME "/mnt/rozipfs"
#define ALTERA_RO_ZIPFS_OFFSET 0xe0000


/*
 * epcq_avl_csr configuration
 *
 */

#define ALT_MODULE_CLASS_epcq_avl_csr altera_epcq_controller2
#define EPCQ_AVL_CSR_BASE 0x100d0000
#define EPCQ_AVL_CSR_FLASH_TYPE "EPCQ16A"
#define EPCQ_AVL_CSR_IRQ 0
#define EPCQ_AVL_CSR_IRQ_INTERRUPT_CONTROLLER_ID 0
#define EPCQ_AVL_CSR_IS_EPCS 0
#define EPCQ_AVL_CSR_NAME "/dev/epcq_avl_csr"
#define EPCQ_AVL_CSR_NUMBER_OF_SECTORS 32
#define EPCQ_AVL_CSR_PAGE_SIZE 256
#define EPCQ_AVL_CSR_SECTOR_SIZE 65536
#define EPCQ_AVL_CSR_SPAN 64
#define EPCQ_AVL_CSR_SUBSECTOR_SIZE 4096
#define EPCQ_AVL_CSR_TYPE "altera_epcq_controller2"


/*
 * epcq_avl_mem configuration
 *
 */

#define ALT_MODULE_CLASS_epcq_avl_mem altera_epcq_controller2
#define EPCQ_AVL_MEM_BASE 0x10200000
#define EPCQ_AVL_MEM_FLASH_TYPE "EPCQ16A"
#define EPCQ_AVL_MEM_IRQ -1
#define EPCQ_AVL_MEM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define EPCQ_AVL_MEM_IS_EPCS 0
#define EPCQ_AVL_MEM_NAME "/dev/epcq_avl_mem"
#define EPCQ_AVL_MEM_NUMBER_OF_SECTORS 32
#define EPCQ_AVL_MEM_PAGE_SIZE 256
#define EPCQ_AVL_MEM_SECTOR_SIZE 65536
#define EPCQ_AVL_MEM_SPAN 2097152
#define EPCQ_AVL_MEM_SUBSECTOR_SIZE 4096
#define EPCQ_AVL_MEM_TYPE "altera_epcq_controller2"


/*
 * gpi configuration
 *
 */

#define ALT_MODULE_CLASS_gpi altera_avalon_pio
#define GPI_BASE 0x10080000
#define GPI_BIT_CLEARING_EDGE_REGISTER 1
#define GPI_BIT_MODIFYING_OUTPUT_REGISTER 0
#define GPI_CAPTURE 0
#define GPI_DATA_WIDTH 9
#define GPI_DO_TEST_BENCH_WIRING 1
#define GPI_DRIVEN_SIM_VALUE 0
#define GPI_EDGE_TYPE "NONE"
#define GPI_FREQ 100000000
#define GPI_HAS_IN 1
#define GPI_HAS_OUT 0
#define GPI_HAS_TRI 0
#define GPI_IRQ 4
#define GPI_IRQ_INTERRUPT_CONTROLLER_ID 0
#define GPI_IRQ_TYPE "LEVEL"
#define GPI_NAME "/dev/gpi"
#define GPI_RESET_VALUE 0
#define GPI_SPAN 16
#define GPI_TYPE "altera_avalon_pio"


/*
 * gpx configuration
 *
 */

#define ALT_MODULE_CLASS_gpx altera_avalon_pio
#define GPX_BASE 0x10070000
#define GPX_BIT_CLEARING_EDGE_REGISTER 0
#define GPX_BIT_MODIFYING_OUTPUT_REGISTER 0
#define GPX_CAPTURE 0
#define GPX_DATA_WIDTH 7
#define GPX_DO_TEST_BENCH_WIRING 1
#define GPX_DRIVEN_SIM_VALUE 0
#define GPX_EDGE_TYPE "NONE"
#define GPX_FREQ 100000000
#define GPX_HAS_IN 0
#define GPX_HAS_OUT 0
#define GPX_HAS_TRI 1
#define GPX_IRQ -1
#define GPX_IRQ_INTERRUPT_CONTROLLER_ID -1
#define GPX_IRQ_TYPE "NONE"
#define GPX_NAME "/dev/gpx"
#define GPX_RESET_VALUE 0
#define GPX_SPAN 16
#define GPX_TYPE "altera_avalon_pio"


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 32
#define ALT_SYS_CLK SYSCLK
#define ALT_TIMESTAMP_CLK SYSTIMER


/*
 * opto configuration
 *
 */

#define ALT_MODULE_CLASS_opto opto
#define OPTO_BASE 0x100c0000
#define OPTO_IRQ 7
#define OPTO_IRQ_INTERRUPT_CONTROLLER_ID 0
#define OPTO_NAME "/dev/opto"
#define OPTO_SPAN 16384
#define OPTO_TYPE "opto"


/*
 * pll configuration
 *
 */

#define ALT_MODULE_CLASS_pll altpll
#define PLL_BASE 0x10000000
#define PLL_IRQ -1
#define PLL_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PLL_NAME "/dev/pll"
#define PLL_SPAN 16
#define PLL_TYPE "altpll"


/*
 * sdram configuration
 *
 */

#define ALT_MODULE_CLASS_sdram sdram
#define SDRAM_BASE 0x0
#define SDRAM_IRQ -1
#define SDRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SDRAM_NAME "/dev/sdram"
#define SDRAM_SPAN 8388608
#define SDRAM_TYPE "sdram"


/*
 * stamp configuration
 *
 */

#define ALT_MODULE_CLASS_stamp stamp
#define STAMP_BASE 0x10030000
#define STAMP_IRQ -1
#define STAMP_IRQ_INTERRUPT_CONTROLLER_ID -1
#define STAMP_NAME "/dev/stamp"
#define STAMP_SPAN 4096
#define STAMP_TYPE "stamp"


/*
 * stdout configuration
 *
 */

#define ALT_MODULE_CLASS_stdout altera_avalon_uart
#define STDOUT_BASE 0x10090000
#define STDOUT_BAUD 115200
#define STDOUT_DATA_BITS 8
#define STDOUT_FIXED_BAUD 1
#define STDOUT_FREQ 100000000
#define STDOUT_IRQ 5
#define STDOUT_IRQ_INTERRUPT_CONTROLLER_ID 0
#define STDOUT_NAME "/dev/stdout"
#define STDOUT_PARITY 'N'
#define STDOUT_SIM_CHAR_STREAM ""
#define STDOUT_SIM_TRUE_BAUD 0
#define STDOUT_SPAN 32
#define STDOUT_STOP_BITS 1
#define STDOUT_SYNC_REG_DEPTH 2
#define STDOUT_TYPE "altera_avalon_uart"
#define STDOUT_USE_CTS_RTS 0
#define STDOUT_USE_EOP_REGISTER 0


/*
 * sysclk configuration
 *
 */

#define ALT_MODULE_CLASS_sysclk altera_avalon_timer
#define SYSCLK_ALWAYS_RUN 1
#define SYSCLK_BASE 0x10040000
#define SYSCLK_COUNTER_SIZE 32
#define SYSCLK_FIXED_PERIOD 1
#define SYSCLK_FREQ 100000000
#define SYSCLK_IRQ 1
#define SYSCLK_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SYSCLK_LOAD_VALUE 999999
#define SYSCLK_MULT 0.001
#define SYSCLK_NAME "/dev/sysclk"
#define SYSCLK_PERIOD 10
#define SYSCLK_PERIOD_UNITS "ms"
#define SYSCLK_RESET_OUTPUT 0
#define SYSCLK_SNAPSHOT 1
#define SYSCLK_SPAN 32
#define SYSCLK_TICKS_PER_SEC 100
#define SYSCLK_TIMEOUT_PULSE_OUTPUT 0
#define SYSCLK_TIMER_DEVICE_TYPE 1
#define SYSCLK_TYPE "altera_avalon_timer"


/*
 * systimer configuration
 *
 */

#define ALT_MODULE_CLASS_systimer altera_avalon_timer
#define SYSTIMER_ALWAYS_RUN 1
#define SYSTIMER_BASE 0x10050000
#define SYSTIMER_COUNTER_SIZE 32
#define SYSTIMER_FIXED_PERIOD 1
#define SYSTIMER_FREQ 100000000
#define SYSTIMER_IRQ 2
#define SYSTIMER_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SYSTIMER_LOAD_VALUE -2
#define SYSTIMER_MULT 1.0E-8
#define SYSTIMER_NAME "/dev/systimer"
#define SYSTIMER_PERIOD -1
#define SYSTIMER_PERIOD_UNITS "clocks"
#define SYSTIMER_RESET_OUTPUT 0
#define SYSTIMER_SNAPSHOT 1
#define SYSTIMER_SPAN 32
#define SYSTIMER_TICKS_PER_SEC 0
#define SYSTIMER_TIMEOUT_PULSE_OUTPUT 0
#define SYSTIMER_TIMER_DEVICE_TYPE 1
#define SYSTIMER_TYPE "altera_avalon_timer"


/*
 * update configuration
 *
 */

#define ALT_MODULE_CLASS_update altera_remote_update
#define UPDATE_BASE 0x100a0000
#define UPDATE_IRQ -1
#define UPDATE_IRQ_INTERRUPT_CONTROLLER_ID -1
#define UPDATE_NAME "/dev/update"
#define UPDATE_SPAN 128
#define UPDATE_TYPE "altera_remote_update"


/*
 * watchdog configuration
 *
 */

#define ALT_MODULE_CLASS_watchdog altera_avalon_timer
#define WATCHDOG_ALWAYS_RUN 1
#define WATCHDOG_BASE 0x10060000
#define WATCHDOG_COUNTER_SIZE 32
#define WATCHDOG_FIXED_PERIOD 1
#define WATCHDOG_FREQ 100000000
#define WATCHDOG_IRQ 3
#define WATCHDOG_IRQ_INTERRUPT_CONTROLLER_ID 0
#define WATCHDOG_LOAD_VALUE 199999999
#define WATCHDOG_MULT 1.0
#define WATCHDOG_NAME "/dev/watchdog"
#define WATCHDOG_PERIOD 2
#define WATCHDOG_PERIOD_UNITS "s"
#define WATCHDOG_RESET_OUTPUT 1
#define WATCHDOG_SNAPSHOT 0
#define WATCHDOG_SPAN 32
#define WATCHDOG_TICKS_PER_SEC 0
#define WATCHDOG_TIMEOUT_PULSE_OUTPUT 0
#define WATCHDOG_TIMER_DEVICE_TYPE 1
#define WATCHDOG_TYPE "altera_avalon_timer"

#endif /* __SYSTEM_H_ */
