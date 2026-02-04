/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'cpu' in SOPC Builder design 'c10_fpga'
 * SOPC Builder design path: D:/c10_opto_fpga/c10_top/PR_R2/c10_fpga.sopcinfo
 *
 * Generated: Wed Feb 04 10:55:19 PST 2026
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

#define ALT_CPU_ARCHITECTURE "intel_niosv_m"
#define ALT_CPU_CPU_FREQ 100000000u
#define ALT_CPU_DATA_ADDR_WIDTH 0x20
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_FREQ 100000000
#define ALT_CPU_HAS_CSR_SUPPORT 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_ICACHE_LINE_SIZE 0
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_ICACHE_SIZE 0
#define ALT_CPU_INST_ADDR_WIDTH 0x20
#define ALT_CPU_INT_MODE 0
#define ALT_CPU_MTIME_OFFSET 0x100d0000
#define ALT_CPU_NAME "cpu"
#define ALT_CPU_NIOSV_CORE_VARIANT 1
#define ALT_CPU_NUM_GPR 32
#define ALT_CPU_RESET_ADDR 0x102be6e0
#define ALT_CPU_TICKS_PER_SEC NIOSV_INTERNAL_TIMER_TICKS_PER_SECOND
#define ALT_CPU_TIMER_DEVICE_TYPE 2


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define ABBOTTSLAKE_CPU_FREQ 100000000u
#define ABBOTTSLAKE_DATA_ADDR_WIDTH 0x20
#define ABBOTTSLAKE_DCACHE_LINE_SIZE 0
#define ABBOTTSLAKE_DCACHE_LINE_SIZE_LOG2 0
#define ABBOTTSLAKE_DCACHE_SIZE 0
#define ABBOTTSLAKE_HAS_CSR_SUPPORT 1
#define ABBOTTSLAKE_HAS_DEBUG_STUB
#define ABBOTTSLAKE_ICACHE_LINE_SIZE 0
#define ABBOTTSLAKE_ICACHE_LINE_SIZE_LOG2 0
#define ABBOTTSLAKE_ICACHE_SIZE 0
#define ABBOTTSLAKE_INST_ADDR_WIDTH 0x20
#define ABBOTTSLAKE_INT_MODE 0
#define ABBOTTSLAKE_MTIME_OFFSET 0x100d0000
#define ABBOTTSLAKE_NIOSV_CORE_VARIANT 1
#define ABBOTTSLAKE_NUM_GPR 32
#define ABBOTTSLAKE_RESET_ADDR 0x102be6e0
#define ABBOTTSLAKE_TICKS_PER_SEC NIOSV_INTERNAL_TIMER_TICKS_PER_SECOND
#define ABBOTTSLAKE_TIMER_DEVICE_TYPE 2


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ADC
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_UART
#define __ALTERA_EPCQ_CONTROLLER2
#define __ALTERA_REMOTE_UPDATE
#define __ALTPLL
#define __INTEL_NIOSV_M
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
#define ALT_STDERR_BASE 0x10050000
#define ALT_STDERR_DEV stdout
#define ALT_STDERR_IS_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_uart"
#define ALT_STDIN "/dev/stdout"
#define ALT_STDIN_BASE 0x10050000
#define ALT_STDIN_DEV stdout
#define ALT_STDIN_IS_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_uart"
#define ALT_STDOUT "/dev/stdout"
#define ALT_STDOUT_BASE 0x10050000
#define ALT_STDOUT_DEV stdout
#define ALT_STDOUT_IS_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_uart"
#define ALT_SYSTEM_NAME "c10_fpga"
#define ALT_SYS_CLK_TICKS_PER_SEC ALT_CPU_TICKS_PER_SEC
#define ALT_TIMESTAMP_CLK_TIMER_DEVICE_TYPE ALT_CPU_TIMER_DEVICE_TYPE


/*
 * adc configuration
 *
 */

#define ADC_BASE 0x10060000
#define ADC_IRQ 3
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
#define ALTERA_RO_ZIPFS_OFFSET 0x3c0000


/*
 * cpu_dm_agent configuration
 *
 */

#define ALT_MODULE_CLASS_cpu_dm_agent intel_niosv_m
#define CPU_DM_AGENT_BASE 0x100e0000
#define CPU_DM_AGENT_CPU_FREQ 100000000u
#define CPU_DM_AGENT_DATA_ADDR_WIDTH 0x20
#define CPU_DM_AGENT_DCACHE_LINE_SIZE 0
#define CPU_DM_AGENT_DCACHE_LINE_SIZE_LOG2 0
#define CPU_DM_AGENT_DCACHE_SIZE 0
#define CPU_DM_AGENT_HAS_CSR_SUPPORT 1
#define CPU_DM_AGENT_HAS_DEBUG_STUB
#define CPU_DM_AGENT_ICACHE_LINE_SIZE 0
#define CPU_DM_AGENT_ICACHE_LINE_SIZE_LOG2 0
#define CPU_DM_AGENT_ICACHE_SIZE 0
#define CPU_DM_AGENT_INST_ADDR_WIDTH 0x20
#define CPU_DM_AGENT_INTERRUPT_CONTROLLER_ID 0
#define CPU_DM_AGENT_INT_MODE 0
#define CPU_DM_AGENT_IRQ -1
#define CPU_DM_AGENT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CPU_DM_AGENT_MTIME_OFFSET 0x100d0000
#define CPU_DM_AGENT_NAME "/dev/cpu_dm_agent"
#define CPU_DM_AGENT_NIOSV_CORE_VARIANT 1
#define CPU_DM_AGENT_NUM_GPR 32
#define CPU_DM_AGENT_RESET_ADDR 0x102be6e0
#define CPU_DM_AGENT_SPAN 65536
#define CPU_DM_AGENT_TICKS_PER_SEC NIOSV_INTERNAL_TIMER_TICKS_PER_SECOND
#define CPU_DM_AGENT_TIMER_DEVICE_TYPE 2
#define CPU_DM_AGENT_TYPE "intel_niosv_m"


/*
 * cpu_timer_sw_agent configuration
 *
 */

#define ALT_MODULE_CLASS_cpu_timer_sw_agent intel_niosv_m
#define CPU_TIMER_SW_AGENT_BASE 0x100d0000
#define CPU_TIMER_SW_AGENT_CPU_FREQ 100000000u
#define CPU_TIMER_SW_AGENT_DATA_ADDR_WIDTH 0x20
#define CPU_TIMER_SW_AGENT_DCACHE_LINE_SIZE 0
#define CPU_TIMER_SW_AGENT_DCACHE_LINE_SIZE_LOG2 0
#define CPU_TIMER_SW_AGENT_DCACHE_SIZE 0
#define CPU_TIMER_SW_AGENT_HAS_CSR_SUPPORT 1
#define CPU_TIMER_SW_AGENT_HAS_DEBUG_STUB
#define CPU_TIMER_SW_AGENT_ICACHE_LINE_SIZE 0
#define CPU_TIMER_SW_AGENT_ICACHE_LINE_SIZE_LOG2 0
#define CPU_TIMER_SW_AGENT_ICACHE_SIZE 0
#define CPU_TIMER_SW_AGENT_INST_ADDR_WIDTH 0x20
#define CPU_TIMER_SW_AGENT_INTERRUPT_CONTROLLER_ID 0
#define CPU_TIMER_SW_AGENT_INT_MODE 0
#define CPU_TIMER_SW_AGENT_IRQ -1
#define CPU_TIMER_SW_AGENT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CPU_TIMER_SW_AGENT_MTIME_OFFSET 0x100d0000
#define CPU_TIMER_SW_AGENT_NAME "/dev/cpu_timer_sw_agent"
#define CPU_TIMER_SW_AGENT_NIOSV_CORE_VARIANT 1
#define CPU_TIMER_SW_AGENT_NUM_GPR 32
#define CPU_TIMER_SW_AGENT_RESET_ADDR 0x102be6e0
#define CPU_TIMER_SW_AGENT_SPAN 64
#define CPU_TIMER_SW_AGENT_TICKS_PER_SEC NIOSV_INTERNAL_TIMER_TICKS_PER_SECOND
#define CPU_TIMER_SW_AGENT_TIMER_DEVICE_TYPE 2
#define CPU_TIMER_SW_AGENT_TYPE "intel_niosv_m"


/*
 * epcq_avl_csr configuration
 *
 */

#define ALT_MODULE_CLASS_epcq_avl_csr altera_epcq_controller2
#define EPCQ_AVL_CSR_BASE 0x10020000
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
#define GPI_BASE 0x10040000
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
#define GPI_IRQ 1
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
#define GPX_BASE 0x10030000
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
 * hal2 configuration
 *
 */

#define ALT_MAX_FD 32
#define ALT_SYS_CLK CPU
#define ALT_TIMESTAMP_CLK CPU
#define INTEL_FPGA_DFL_START_ADDRESS 0xffffffffffffffff
#define INTEL_FPGA_USE_DFL_WALKER 0


/*
 * intel_niosv_m_hal_driver configuration
 *
 */

#define NIOSV_INTERNAL_TIMER_TICKS_PER_SECOND 100


/*
 * opto configuration
 *
 */

#define ALT_MODULE_CLASS_opto opto
#define OPTO_BASE 0x10070000
#define OPTO_IRQ 4
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
#define STAMP_BASE 0x10010000
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
#define STDOUT_BASE 0x10050000
#define STDOUT_BAUD 115200
#define STDOUT_DATA_BITS 8
#define STDOUT_FIXED_BAUD 1
#define STDOUT_FREQ 100000000
#define STDOUT_IRQ 2
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
 * update configuration
 *
 */

#define ALT_MODULE_CLASS_update altera_remote_update
#define UPDATE_BASE 0x10080000
#define UPDATE_IRQ -1
#define UPDATE_IRQ_INTERRUPT_CONTROLLER_ID -1
#define UPDATE_NAME "/dev/update"
#define UPDATE_SPAN 128
#define UPDATE_TYPE "altera_remote_update"

#endif /* __SYSTEM_H_ */
