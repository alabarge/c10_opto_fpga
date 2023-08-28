#pragma once

/*-----------------------------------------------------------------------------

   HARDWARE CONFIGURATION CONSTANTS

-----------------------------------------------------------------------------*/
// PACKET UART BAUD RATE
//    This value is used to configure the Packet UART baud rate.

#define  LIN_BAUD_RATE           12000000

/*-----------------------------------------------------------------------------

   SOFTWARE DEBUG & TRACING CONFIGURATION CONSTANTS

-----------------------------------------------------------------------------*/

#define  LIN_CI_FILENAME         "de0_win_ci.bin"

#define  LIN_PROGNAME_LEN        64

#define  LIN_CI_MAGIC            0x55AA1234

#define  LIN_TICKS_PER_SECOND    100

#define  LIN_TICKS_MS            (1000 / LIN_TICKS_PER_SECOND)

/*-----------------------------------------------------------------------------

   CONFIGURABLE ITEMS DATABASE CONSTANTS

-----------------------------------------------------------------------------*/

// FEATURE FLAGS
//    Flags used to enable/disable features.

#define LIN_FEATURE_PING         0x00000001
#define LIN_FEATURE_USE_CI_FILE  0x00000002
#define LIN_FEATURE_LOG_TRAFFIC  0x00000004
#define LIN_FEATURE_IGNORE_HALT  0x00000008
#define LIN_FEATURE_DEFAULT      LIN_FEATURE_PING  | LIN_FEATURE_LOG_TRAFFIC

// TRACE FLAGS
//    Flags used to enable/disable tracing.

#define LIN_TRACE_CM             0x00000001
#define LIN_TRACE_CI             0x00000002
#define LIN_TRACE_SERVER         0x00000004
#define LIN_TRACE_CLIENT         0x00000008
#define LIN_TRACE_HAL            0x00000010
#define LIN_TRACE_DRIVER         0x00000020
#define LIN_TRACE_RUN            0x00000040
#define LIN_TRACE_ID             0x00000080
#define LIN_TRACE_LAN            0x00000100
#define LIN_TRACE_POST           0x00000200
#define LIN_TRACE_IRQ            0x00000400
#define LIN_TRACE_ERROR          0x00000800
#define LIN_TRACE_UART           0x00001000
#define LIN_TRACE_ROUTE          0x00002000
#define LIN_TRACE_LOCAL          0x00004000
#define LIN_TRACE_CMQ            0x00008000
#define LIN_TRACE_PIPE           0x00010000
#define LIN_TRACE_TIMER          0x00020000
#define LIN_TRACE_CM_LOG         0x00040000
#define LIN_TRACE_ALL            0xFFFFFFFF
#define LIN_TRACE_NONE           0x00000000
#define LIN_TRACE_DEFAULT        0x00000000

// DEBUG FLAGS
//    Flags to enable/disable debug tracing.

#define LIN_DEBUG_OS             0x00000001
#define LIN_DEBUG_CM             0x00000002
#define LIN_DEBUG_CI             0x00000004
#define LIN_DEBUG_SERVER         0x00000008
#define LIN_DEBUG_CLIENT         0x00000010
#define LIN_DEBUG_ALL            0xFFFFFFFF
#define LIN_DEBUG_NONE           0x00000000
#define LIN_DEBUG_DEFAULT        LIN_DEBUG_NONE

// MACHINE STATUS FLAGS
//    Global Machine status flags.

#define LIN_STATUS_CLEAR         0x00000000
#define LIN_STATUS_OK            0x00000000
#define LIN_STATUS_INIT          0x00000001
#define LIN_STATUS_RUN           0x00000002
#define LIN_STATUS_CONNECTED     0x00000004
#define LIN_STATUS_DAQ_RUN       0x00000008
#define LIN_STATUS_XL345_RUN     0x00000010
#define LIN_STATUS_DEV_ID        0x00000F00
#define LIN_STATUS_FTDI_CLOCK    0x00001000

// MACHINE ERROR FLAGS
//    Global Machine Error flags.

#define LIN_ERROR_CLEAR          0x00000000
#define LIN_ERROR_OK             0x00000000
#define LIN_ERROR_CI_MAGIC       0x00000001
#define LIN_ERROR_CI_CHKSUM      0x00000002
#define LIN_ERROR_ID             0x00000004
#define LIN_ERROR_MALLOC         0x00000008
#define LIN_ERROR_POST           0x00000010
#define LIN_ERROR_BOOT           0x00000020
#define LIN_ERROR_FSM            0x00000040
#define LIN_ERROR_SD             0x00000080
#define LIN_ERROR_FILE           0x00000100
#define LIN_ERROR_LAN            0x00000200
#define LIN_ERROR_OPTO           0x00000400
#define LIN_ERROR_STAMP          0x00000800
#define LIN_ERROR_CM             0x00001000
#define LIN_ERROR_CP             0x00002000
#define LIN_ERROR_DAQ            0x00004000
#define LIN_ERROR_ADC            0x00008000
#define LIN_ERROR_FIFO           0x00010000
#define LIN_ERROR_COM            0x00020000
#define LIN_ERROR_UDP            0x00040000
#define LIN_ERROR_TCP            0x00080000
#define LIN_ERROR_TTY            0x00100000
#define LIN_ERROR_OPC            0x00200000
#define LIN_ERROR_CC             0x00400000
#define LIN_ERROR_APP_TIMEOUT    0x00800000
#define LIN_ERROR_PING_TIMEOUT   0x01000000
#define LIN_ERROR_OP_CODE        0x02000000
#define LIN_ERROR_HALT           0x04000000
#define LIN_ERROR_CI             0x08000000
