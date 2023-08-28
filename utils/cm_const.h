#pragma once

//
// WARNING: THIS FILE IS SHARED BETWEEN MULTIPLE APPLICATIONS, ANY CHANGES
// SHOULD BE UPDATED ACROSS PLATFORMS IN ORDER TO MAINTAIN CM COMPATIBILITY!
//

//
// CM MAXIMUM MESSAGE AND PAYLOAD LENGTHS
//
#define CM_MAX_MSG_INT8U            512
#define CM_MAX_MSG_INT32U           (CM_MAX_MSG_INT8U / sizeof(int))
#define CM_MAX_MSG_PAYLOAD_INT8U    (CM_MAX_MSG_INT8U - sizeof(cm_msg_t) + sizeof(int))
#define CM_MAX_MSG_PAYLOAD_INT16U   (CM_MAX_MSG_PAYLOAD_INT8U / sizeof(uint16_t))
#define CM_MAX_MSG_PAYLOAD_INT32U   (CM_MAX_MSG_PAYLOAD_INT8U / sizeof(uint32_t))

#define CM_MAX_PIPE_PAYLOAD_INT32U  252      // In 32-Bit Words
#define CM_MAX_PIPE_PAYLOAD_INT64U  126      // In 64-Bit Words
#define CM_MAX_PIPE_MSG             512      // In Bytes

//
// CM DATA STRUCTURES, SHARED WITH WIN32
//
#ifdef WIN32

// CM Message Header
typedef struct _cm_hdr_t {
   UINT8    dst_cmid;        // Destination CM Address
   UINT8    src_cmid;        // Source CM Address
   UINT8    dst_devid :  4;  // Destination Device ID
   UINT8    src_devid :  4;  // Source Device ID
   UINT8    seqid     :  4;  // Message Sequence Number
   UINT8    endian    :  1;  // Processor Endianess
   UINT8    event     :  1;  // Event ID
   UINT8    keep      :  1;  // Don't Delete
   UINT8    proto     :  1;  // Protocol ID
   UINT8    crc8;            // CRC-8 using x^8 + x^5 + x^4 + 1
   UINT8    slot;            // Slot position in queue
   UINT16   msglen    : 12;  // Message Length
   UINT16   port      :  4;  // Port Connection
} cm_hdr_t, *pcm_hdr_t;

// parameters common to all messages
typedef struct _msg_parms_t {
   UINT8    srvid;
   UINT8    msgid;
   UINT8    flags;
   UINT8    status;
} msg_parms_t, *pmsg_parms_t;

// cm message structure
typedef struct _cm_msg_t {
   cm_hdr_t       h;
   msg_parms_t    p;
} cm_msg_t, *pcm_msg_t;

// CM PIPE HEADER DATA STRUCTURE
// USED FOR VARIABLE LENGTH PIPE MESSAGES
typedef struct _cm_pipe_t {
   uint8_t     dst_cmid;       // Destination CM ID
   uint8_t     msgid;          // Pipe Message ID, CM_PIPE_DAQ_DATA = 0x10
   uint8_t     port;           // Destination Port
   uint8_t     flags;          // Message Flags
   uint32_t    msglen;         // Message Length in 32-Bit words
   uint32_t    seqid;          // Sequence ID
   uint32_t    stamp;          // 32-Bit FPGA Clock Count
   uint32_t    stamp_us;       // 32-Bit Time Stamp in microseconds
   uint32_t    status;         // Current Machine Status
   uint32_t    rate;           // ADC rate
   uint32_t    magic;          // Magic Number
   uint16_t    samples[496];   // DAQ Samples
} cm_pipe_t, *pcm_pipe_t;

// CM PIPE MESSAGE DATA STRUCTURE
// USED FOR FIXED 1024-BYTE PIPE MESSAGES
typedef struct _cm_pipe_fixed_t {
   uint8_t     dst_cmid;       // Destination CM ID
   uint8_t     msgid;          // Pipe Message ID
   uint8_t     port;           // Destination Port
   uint8_t     flags;          // Message Flags
   uint32_t    msglen;         // Message Length in 32-Bit words
   uint32_t    seqid;          // Sequence ID
   uint32_t    stamp;          // 32-Bit FPGA Clock Count
   uint32_t    stamp_us;       // 32-Bit Time Stamp in microseconds
   uint32_t    status;         // Current Machine Status
   uint32_t    rate;           // ADC rate
   uint32_t    magic;          // Magic Number
   uint16_t    samples[496];   // DAQ Samples
} cm_pipe_fixed_t, *pcm_pipe_fixed_t;

#else

// CM Message Header
typedef struct _cm_hdr_t {
   uint16_t   dst_cmid  :  8;  // Destination CM Address
   uint16_t   src_cmid  :  8;  // Source CM Address
   uint16_t   dst_devid :  4;  // Destination Device ID
   uint16_t   src_devid :  4;  // Source Device ID
   uint16_t   seqid     :  4;  // Message Sequence Number
   uint16_t   endian    :  1;  // Processor Endianess
   uint16_t   event     :  1;  // Event ID
   uint16_t   keep      :  1;  // Don't Delete
   uint16_t   proto     :  1;  // Protocol ID
   uint16_t   crc8      :  8;  // CRC-8 using x^8 + x^5 + x^4 + 1
   uint16_t   slot      :  8;  // Slot position in queue
   uint16_t   msglen    : 12;  // Message Length
   uint16_t   port      :  4;  // Port Connection
} cm_hdr_t, *pcm_hdr_t;

// Parameters Common to all Messages
typedef struct _cm_msg_parms_t {
   uint8_t    srvid;
   uint8_t    msgid;
   uint8_t    flags;
   uint8_t    status;
} msg_parms_t, *pmsg_parms_t;

// CM Message Structure
typedef struct _cm_msg_t {
   cm_hdr_t       h;
   msg_parms_t    p;
} cm_msg_t, *pcm_msg_t;

// CM PIPE HEADER DATA STRUCTURE
// USED FOR VARIABLE LENGTH PIPE MESSAGES
typedef struct _cm_pipe_t {
   uint8_t     dst_cmid;       // Destination CM ID
   uint8_t     msgid;          // Pipe Message ID, CM_PIPE_DAQ_DATA = 0x10
   uint8_t     port;           // Destination Port
   uint8_t     flags;          // Message Flags
   uint32_t    msglen;         // Message Length in 32-Bit words
   uint32_t    seqid;          // Sequence ID
   uint32_t    stamp;          // 32-Bit FPGA Clock Count
   uint32_t    stamp_us;       // 32-Bit Time Stamp in microseconds
   uint32_t    status;         // Current Machine Status
   uint32_t    rate;           // ADC rate
   uint32_t    magic;          // Magic Number
   uint16_t    samples[496];   // DAQ Samples
} cm_pipe_t, *pcm_pipe_t;

// CM PIPE MESSAGE DATA STRUCTURE
// USED FOR FIXED 1024-BYTE PIPE MESSAGES
typedef struct _cm_pipe_fixed_t {
   uint8_t     dst_cmid;       // Destination CM ID
   uint8_t     msgid;          // Pipe Message ID
   uint8_t     port;           // Destination Port
   uint8_t     flags;          // Message Flags
   uint32_t    msglen;         // Message Length in 32-Bit words
   uint32_t    seqid;          // Sequence ID
   uint32_t    stamp;          // 32-Bit FPGA Clock Count
   uint32_t    stamp_us;       // 32-Bit Time Stamp in microseconds
   uint32_t    status;         // Current Machine Status
   uint32_t    rate;           // ADC rate
   uint32_t    magic;          // Magic Number
   uint16_t    samples[496];   // DAQ Samples
} cm_pipe_fixed_t, *pcm_pipe_fixed_t;

#endif

// Macro for associating the CM ID with the Message ID
#define MSG(cm_id, msg_id) ((((cm_id) & 0xff) << 8) | (msg_id & 0xff))

// ===========================================================================
//
// COMMUNICATIONS MANAGER (CM) PROTOCOL CONSTANTS

#define CM_MAX_OBJS           8
#define CM_MAX_SUBS           4
#define CM_MAX_TIMERS         4
#define CM_MAX_PORTS          8
#define CM_MAX_ROUTES         8
#define CM_MAX_PIPES          4
#define CM_MAX_MEDIA          8

#define CM_MAX_DEV_STR_LEN    64
#define CM_MAX_FILE_LEN       256
#define CM_MAX_MSG_STR_LEN    128

#define CM_MAX_DEVICES        16
#define CM_MAX_DEV_ID         16

// Protocol Identifiers
#define CM_PROTO_OS           0x01
#define CM_PROTO              0x00

// Event IDs
#define CM_EVENT_MSG          0x00
#define CM_EVENT_TIMER        0x01

// Transaction IDs
#define CM_NULL_TRANS_ID      0x00

// Endianess Identifiers
#define CM_BIG_ENDIAN         0x00
#define CM_LITTLE_ENDIAN      0x01

#ifdef WIN32
// Intel x86 is a Little Endian CPU (WIN32)
#define CM_ENDIAN             CM_LITTLE_ENDIAN
#else
// NIOS is set as a Little Endian CPU
#define CM_ENDIAN             CM_LITTLE_ENDIAN
#endif

// CM Server and Client Addresses
// Bit 6&7 Defines the CM ID Type, the
// lower order 6-Bits defines the Object ID
#define CM_SERVER             0x00
#define CM_CLIENT             0x40
#define CM_LAYER              0x80

// Servers
#define CM_ID_CP_SRV          (0x01 | CM_SERVER)
#define CM_ID_RPC_SRV         (0x02 | CM_SERVER)
#define CM_ID_DAQ_SRV         (0x03 | CM_SERVER)
#define CM_ID_OPC_SRV         (0x04 | CM_SERVER)

// Clients
#define CM_ID_CON_CLI         (0x01 | CM_CLIENT)
#define CM_ID_CP_CLI          (0x02 | CM_CLIENT)
#define CM_ID_CP_MEM_CLI      (0x03 | CM_CLIENT)
#define CM_ID_DAQ_CLI         (0x04 | CM_CLIENT)

// CM Layer Control IDs
#define CM_ID_BCAST           (0x01 | CM_LAYER)
#define CM_ID_STREAM          (0x02 | CM_LAYER)
#define CM_ID_INSTANCE        (0x03 | CM_LAYER)
#define CM_ID_PIPE            (0x04 | CM_LAYER)
#define CM_ID_LCAST           (0x05 | CM_LAYER)

// Special Use IDs
#define CM_ID_NULL            0x00
#define CM_OBJ_NULL           0x00
#define CM_MSG_NULL           0x00
#define CM_TIMER_NULL         0x00

// CM Port Connections
#define CM_PORT_NONE          0x0
#define CM_PORT_LOCAL         0x1
#define CM_PORT_COM0          0x2
#define CM_PORT_COM1          0x3
#define CM_PORT_COM2          0x4

// CM Media Types
#define CM_MEDIA_NONE         0x0
#define CM_MEDIA_LAN          0x1
#define CM_MEDIA_FIFO         0x2
#define CM_MEDIA_OPTO         0x3
#define CM_MEDIA_COM          0x4
#define CM_MEDIA_TTY          0x5
#define CM_MEDIA_SIM          0x7

// Device Identifiers
#define CM_DEV_DE0            0x0
#define CM_DEV_WIN            0x1
#define CM_DEV_BCAST          0x2
#define CM_DEV_NULL           0xF

// Pipe Message IDs
#define CM_PIPE_NULL          0x00
#define CM_PIPE_DAQ_DATA      0x15

// Pipe Message Flags
#define CM_PIPE_KEEP          0x00
#define CM_PIPE_FREE          0x01
#define CM_PIPE_SEQID         0x02
