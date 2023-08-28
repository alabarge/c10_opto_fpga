#include "fw_cfg.h"

// Application Specific Windows Messages
#define  WM_CHANGE_VIEW             (WM_USER +   1)
#define  WM_LOG_MSG                 (WM_USER +   2)
#define  WM_TRAFFIC_MSG             (WM_USER +   3)
#define  WM_DEBUG_MSG               (WM_USER +   4)
#define  WM_CLOSING                 (WM_USER +   5)
#define  WM_DEV_ARRIVE              (WM_USER +   6)
#define  WM_DEV_REMOVE              (WM_USER +   7)
#define  WM_DEV_ERROR               (WM_USER +   8)
#define  WM_LOG_CLEAR               (WM_USER +   9)
#define  WM_CTL_MESSAGE             (WM_USER +  10)
#define  WM_CM_TRAFFIC              (WM_USER +  11)
#define  WM_PLOT_MSG                (WM_USER +  12)
#define  WM_EXPAND_ALL              (WM_USER +  13)
#define  WM_COLLAPSE_ALL            (WM_USER +  14)
#define  WM_PIPE_MESSAGE            (WM_USER +  15)
#define  WM_WRITE_LOG               (WM_USER +  16)
#define  WM_UDP_MESSAGE             (WM_USER +  17)
#define  WM_PING_MESSAGE            (WM_USER +  18)
#define  WM_TCP_MESSAGE             (WM_USER +  19)
#define  WM_DROP_MESSAGE            (WM_USER +  20)
#define  WM_DISCONNECT              (WM_USER +  21)
#define  WM_REG_MESSAGE             (WM_USER +  22)
#define  WM_UPDATE_CONFIG           (WM_USER +  23)
#define  WM_STATUS_BAR              (WM_USER +  24)
#define  WM_CM_LOG                  (WM_USER +  25)

#define  WM_IPP_END_THREAD          (WM_USER +  50)
#define  WM_IPP_INIT_THREAD         (WM_USER +  51)
#define  WM_IPP_GET_CPU_FREQ        (WM_USER +  52)
#define  WM_IPP_POST_MY_MESSAGE_NOW (WM_USER +  53)
#define	WM_IPP_COMPUTEMYDATA	      (WM_USER +  54)

// Log Message Colors
#define  APP_MSG_NORMAL             RGB(0, 0, 0)
#define  APP_MSG_WARNING            RGB(111, 52, 196)
#define  APP_MSG_HIGHLITE           RGB(0, 0, 255)
#define  APP_MSG_LOWLITE            RGB(0, 128, 0)
#define  APP_MSG_ERROR              RGB(255, 0, 0)
#define  APP_MSG_STAMP              RGB(43, 145, 188)

// Traffic Message Colors
#define  APP_MSG_INBOUND            RGB(0, 0, 255)
#define  APP_MSG_OUTBOUND           RGB(0, 0, 0)
#define  APP_MSG_LOCAL              RGB(192, 0, 192)
#define  APP_MSG_IN_PIPE            RGB(0, 0, 255)
#define  APP_MSG_OUT_PIPE           RGB(0, 0, 192)
#define  APP_MSG_TIMER              RGB(192, 0, 0)

// Traffic Message Types
#define  APP_TRAFFIC_INBOUND        0
#define  APP_TRAFFIC_OUTBOUND       1
#define  APP_TRAFFIC_LOCAL          2
#define  APP_TRAFFIC_IN_PIPE        3
#define  APP_TRAFFIC_OUT_PIPE       4
#define  APP_TRAFFIC_ERROR          5

// Other Default Colors
#define  APP_DEF_BACK               RGB(255, 255, 255)
#define  APP_DEF_TEXT               RGB(0, 0, 0)
#define  APP_DEF_TRANS              RGB(192, 192, 192)
#define  APP_DEF_LINK               RGB(0, 0, 255)
#define  APP_DEF_IMAGE_BACK         RGB(159, 181, 210)
#define  APP_DEF_CONNECT            RGB(0, 0, 255)
#define  APP_DEF_PAGE_SEL           RGB(0, 0, 255)

// CEdit Colors
#define  APP_READ_ONLY_TEXT         RGB(128, 128, 128)
#define  APP_CEDIT_LOG              RGB(0, 0, 0)
#define  APP_CEDIT_THREAD           RGB(92, 92, 255)
#define  APP_CEDIT_STATUS           RGB(0, 0, 200)
#define  APP_CEDIT_ERROR            RGB(255, 0, 0)
#define  APP_CEDIT_SUCCESS          RGB(0, 200, 0)
#define  APP_CEDIT_LIMIT            1048576
#define  APP_CEDIT_FONT_SIZE        80
#define  APP_CEDIT_FONT             _T("Consolas")
#define  APP_CEDIT_MARGINS          50

// View's CM RX Queue Depth
#define  APP_RX_QUE                 8

// 2D Graph Lanes
#define  APP_GRAPH_LANES            6

// Byte & Word Manipulation
#define DIM(x)    (sizeof(x)/sizeof(*(x)))

// Byte Swapping Macros
#define swap16(x) ((unsigned short) (((x) <<  8) | (((x) >>  8) & 0xFF)))
#define swap32(x) ((unsigned int) (((x) << 24) | (((x) <<  8) & 0xFF0000L) | \
                  (((x) >> 8) & 0xFF00L) | (((x) >> 24) & 0xFFL)))
#define swap64(x) ((unsigned long long) \
                     (((x) << 56) & 0xFF00000000000000) | \
                     (((x) << 40) & 0x00FF000000000000) | \
                     (((x) << 24) & 0x0000FF0000000000) | \
                     (((x) <<  8) & 0x000000FF00000000) | \
                     (((x) >> 56) & 0x00000000000000FF) | \
                     (((x) >> 40) & 0x000000000000FF00) | \
                     (((x) >> 24) & 0x0000000000FF0000) | \
                     (((x) >>  8) & 0x00000000FF000000) )

// COM Connections
#define  APP_CON_NONE           -1
#define  APP_CON_COM             0
#define  APP_CON_UART            1
#define  APP_CON_OPTO            2
#define  APP_CON_FIFO            3
#define  APP_CON_LAN             4
#define  APP_CON_UDP             5
#define  APP_CON_TCP             6

#define  APP_DEVID               CM_DEV_C10

#define  APP_PORT_NONE          -1
#define  APP_SPEED_NONE         -1
#define  APP_DEV_NONE           -1
#define  APP_MSG_PORT            2782
#define  APP_TCP_PORT            2785

#define  APP_MAX_DEVICES         8
#define  APP_MAX_CP_MEM_SIZE     65536

// Trace Flags
#define APP_TRACE_OS             0x00000001
#define APP_TRACE_CM             0x00000002
#define APP_TRACE_CI             0x00000004
#define APP_TRACE_SERVER         0x00000008
#define APP_TRACE_CLIENT         0x00000010
#define APP_TRACE_HAL            0x00000020
#define APP_TRACE_DRIVER         0x00000040
#define APP_TRACE_TASK           0x00000080
#define APP_TRACE_RUN            0x00000100
#define APP_TRACE_ID             0x00000200
#define APP_TRACE_MALLOC         0x00000400
#define APP_TRACE_SDHC           0x00000800
#define APP_TRACE_POST           0x00001000
#define APP_TRACE_IND            0x00002000
#define APP_TRACE_REGS           0x00004000
#define APP_TRACE_IRQ            0x00008000
#define APP_TRACE_STATS          0x01000000
#define APP_TRACE_ERROR          0x02000000
#define APP_TRACE_TIME           0x04000000
#define APP_TRACE_ALL            0xFFFFFFFF
#define APP_TRACE_NONE           0x00000000
#define APP_TRACE_DEFAULT        APP_TRACE_ERROR | APP_TRACE_CI | APP_TRACE_SDHC | \
                                 APP_TRACE_ID | APP_TRACE_MALLOC | APP_TRACE_POST

// Preferred Maximum X & Y, not used
#define APP_MAX_SIZE_X           2048
#define APP_MAX_SIZE_Y           2048

// Preferred Minimum X & Y.
#define APP_MIN_SIZE_X           700
#define APP_MIN_SIZE_Y           550
#define APP_MIN_SIZE_PANE_X      100
#define APP_MIN_SIZE_PANE_Y      150

// Max DOC_STR Type Length
#define  APP_STR_LEN             256
#define  APP_PIPE_STR_LEN        32

// Error Flags
#define APP_ERR_KEY              0x00000001
#define APP_ERR_KEY_EMPTY        0x00000002
#define APP_ERR_KEY_TYPE         0x00000004
#define APP_ERR_KEY_COUNT        0x00000008

