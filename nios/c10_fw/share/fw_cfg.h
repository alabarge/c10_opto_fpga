#pragma once

/*-----------------------------------------------------------------------------

   SOFTWARE COMPILER & VERSION

   gcc version 5.3.0 (Altera 16.1 Build 196)

   Typical Compiler Invocation with parameters:

      nios2-elf-gcc -xc -MP -MMD -c -I../chb_bsp//HAL/inc -I../de0_bsp/
         -I../de0_bsp//drivers/inc -Icore -Idriver -Ishare -Icp_srv -Idaq_srv
         -I.  -pipe -D__hal__ -DALT_NO_INSTRUCTION_EMULATION
         -DALT_SINGLE_THREADED    -Os  -Wall   -mhw-div -mhw-mul
         -mno-hw-mulx -mgpopt=global  -o obj/default/driver/ftdi.o driver/ftdi.c

   NOTE: The Nios II CPU is instantiated as LITTLE ENDIAN because
         of the Avalon bus interface usage.

-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------

   DESIGN DOCUMENTS & REFERENCES

-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------

   CONFIGURATION CONSTANTS

-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------

   HARDWARE CONFIGURATION CONSTANTS

-----------------------------------------------------------------------------*/
// PACKET UART BAUD RATE
//    This value is used to configure the Packet UART baud rate.

#define  CFG_BAUD_RATE           12000000

#define  CFG_TICKS_PER_SECOND    100
#define  CFG_TICKS_MS            (1000 / CFG_TICKS_PER_SECOND)


/*-----------------------------------------------------------------------------

   SOFTWARE DEBUG & TRACING CONFIGURATION CONSTANTS

-----------------------------------------------------------------------------*/
// TIMER CYCLE
//    This value is used to set the interval for the timer() callback,
//    in units of alt_nticks.
#define  CFG_TIMER_CYCLE         1

// ACTIVITY INDICATOR CYCLING
//    This value is used to determine the activity indicator
//    rate of flashing, based on the CFG_TIMER_CYCLE.
#define  CFG_LED_CYCLE           7

// PING RATE
//    This value is used to determine the rate at which
//    a CM Ping message will be sent to the host.
//    connection.
#define  CFG_PING_CYCLE          50

// PING TIMEOUT
//    This value is used to determine when a ping timeout has
//    occurred and negate CFG_STATUS_CONNECTED.
#define  CFG_PING_TIMEOUT        30

// CHANNEL BOARD MASTER ID
//    This value is used to designate the channel board
//    master identification.
#define  CFG_CHB_MASTER          0

#define  CFG_AUTO_COUNT          200

#define  CFG_CI_FILENAME         "/mnt/rozipfs/c10_fw_ci.txt"

#define  CFG_PROGNAME_LEN        64

/*-----------------------------------------------------------------------------

   CONFIGURABLE ITEMS DATABASE CONSTANTS

-----------------------------------------------------------------------------*/
// MAXIMUM CI PARAMETER LIST
//    Maximum number of parameters per single configurable item.

#define  CFG_MAX_CI_PARMS        16

// MAXIMUM CI STRING LENGTH
//    Maximum number of characters per CI line item, this is also the limit
//    imposed for the MAAC.INI file line length

#define  CFG_MAX_CI_STR          256

// MAXIMUM CI FORMAT LENGTH
//    Maximum number of characters to identify the sscanf & sprintf formatting
//    string used for getting and setting the CI.

#define  CFG_MAX_CI_FORMAT       16

// MAXIMUM CI RANGE LENGTH
//    Maximum number of characters in each of the CI range limits.

#define  CFG_MAX_CI_BOUND        32

// MAXIMUM CI FILE LENGTH
//    Maximum number of characters permissible in the *.INI file.

#define  CFG_MAX_CI_FILE         32768

// MAGIC NUMBER
//    Magic number embedded in the CI file.

#define  CFG_CI_MAGIC            0x55AA1234

/*-----------------------------------------------------------------------------

   COMMUNICATIONS MANAGER CONFIGURATION CONSTANTS

-----------------------------------------------------------------------------*/
// MAXIMUM INVALID RECIEVED MESSAGES
//    This macro will limit the number of replies
//    to invalid messages recieved, in order to
//    prevent flooding of the communications channel.

#define  CFG_MAX_INVALID_MSGS    3

// MAXIMUM ERROR STRING LENGTH
//    Maximum size of the Service Provider error
//    string.

#define  CFG_MAX_ERR_STR_LEN     64

// MAXIMUM FILENAME AND FILEPATH SIZE
//    Maximum size for FileName and FilePath strings. This constant
//    must be evenly divisible by four!

#define  CFG_MAX_FILENAME_SIZE   64

// MAXIMUM LOG STRING LENGTH
//    Maximum size of the Service Provider log
//    string.

#define  CFG_MAX_LOG_STR_LEN     384

// HDLC-LIKE FRAMING FLAGS
//    These flags are used to implement an HDLC-Like framing protocol.

#define CFG_START_FRAME          0x7E
#define CFG_END_FRAME            0x7D
#define CFG_ESCAPE               0x7C
#define CFG_STUFFED_BIT          0x20
#define CFG_BROADCAST            0x0F

// DATA TYPES
//    These data types are used by various components.

#define CFG_INT8U                0x00
#define CFG_INT8S                0x01
#define CFG_INT16U               0x02
#define CFG_INT16S               0x03
#define CFG_INT32U               0x04
#define CFG_INT32S               0x05
#define CFG_FP32                 0x06
#define CFG_INT64U               0x07
#define CFG_INT64S               0x08
#define CFG_FP64                 0x09

// FEATURE FLAGS
//    Flags used to enable/disable features.

#define CFG_FEATURE_PING         0x00000001
#define CFG_FEATURE_USE_CI_FILE  0x00000002

// TRACE FLAGS
//    Flags used to enable/disable tracing.

#define CFG_TRACE_CM             0x00000001
#define CFG_TRACE_CI             0x00000002
#define CFG_TRACE_SERVER         0x00000004
#define CFG_TRACE_CLIENT         0x00000008
#define CFG_TRACE_HAL            0x00000010
#define CFG_TRACE_DRIVER         0x00000020
#define CFG_TRACE_RUN            0x00000040
#define CFG_TRACE_ID             0x00000080
#define CFG_TRACE_LAN            0x00000100
#define CFG_TRACE_POST           0x00000200
#define CFG_TRACE_IRQ            0x00000400
#define CFG_TRACE_ERROR          0x00000800
#define CFG_TRACE_UART           0x00001000
#define CFG_TRACE_ROUTE          0x00002000
#define CFG_TRACE_LOCAL          0x00004000
#define CFG_TRACE_CMQ            0x00008000
#define CFG_TRACE_PIPE           0x00010000
#define CFG_TRACE_CM_LOG         0x00020000
#define CFG_TRACE_ALL            0xFFFFFFFF
#define CFG_TRACE_NONE           0x00000000

// DEBUG FLAGS
//    Flags to enable/disable debug tracing.

#define CFG_DEBUG_OS             0x00000001
#define CFG_DEBUG_CM             0x00000002
#define CFG_DEBUG_CI             0x00000004
#define CFG_DEBUG_SERVER         0x00000008
#define CFG_DEBUG_CLIENT         0x00000010
#define CFG_DEBUG_ALL            0xFFFFFFFF
#define CFG_DEBUG_NONE           0x00000000
#define CFG_DEBUG_DEFAULT        CFG_DEBUG_NONE

// MACHINE STATUS FLAGS
//    Global Machine status flags.

#define CFG_STATUS_CLEAR         0x00000000
#define CFG_STATUS_OK            0x00000000
#define CFG_STATUS_INIT          0x00000001
#define CFG_STATUS_RUN           0x00000002
#define CFG_STATUS_CONNECTED     0x00000004
#define CFG_STATUS_DAQ_RUN       0x00000008
#define CFG_STATUS_XL345_RUN     0x00000010
#define CFG_STATUS_DEV_ID        0x00000F00
#define CFG_STATUS_FTDI_CLOCK    0x00001000

// MACHINE ERROR FLAGS
//    Global Machine Error flags.

#define CFG_ERROR_CLEAR          0x00000000
#define CFG_ERROR_OK             0x00000000
#define CFG_ERROR_CI_MAGIC       0x00000001
#define CFG_ERROR_CI_CHKSUM      0x00000002
#define CFG_ERROR_ID             0x00000004
#define CFG_ERROR_MALLOC         0x00000008
#define CFG_ERROR_POST           0x00000010
#define CFG_ERROR_BOOT           0x00000020
#define CFG_ERROR_FSM            0x00000040
#define CFG_ERROR_SD             0x00000080
#define CFG_ERROR_FILE           0x00000100
#define CFG_ERROR_LAN            0x00000200
#define CFG_ERROR_OPTO           0x00000400
#define CFG_ERROR_STAMP          0x00000800
#define CFG_ERROR_CM             0x00001000
#define CFG_ERROR_CP             0x00002000
#define CFG_ERROR_DAQ            0x00004000
#define CFG_ERROR_ADC            0x00008000
#define CFG_ERROR_FIFO           0x00010000
#define CFG_ERROR_COM            0x00020000
#define CFG_ERROR_UDP            0x00040000
#define CFG_ERROR_TCP            0x00080000
#define CFG_ERROR_TTY            0x00100000
#define CFG_ERROR_OPC            0x00200000
#define CFG_ERROR_CI             0x00400000

// BOOT FAILURE CODES
//    Boot codes issued if software startup is terminated.

#define CFG_ERR_POST                1
#define CFG_ERR_BOOT_CHECK          2
#define CFG_ERR_STATUS_NOT_OK       3
