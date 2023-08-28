// appType.h : Type header file for the applicaton
//

#pragma once

#include <math.h>
#include "appCfg.h"

typedef unsigned char      BOOLEAN;
typedef unsigned char      INT8U;
typedef signed   char      INT8S;
typedef unsigned short     INT16U;
typedef signed   short     INT16S;
typedef unsigned int       INT32U;
typedef signed   int       INT32S;
typedef unsigned long long INT64U;
typedef signed   long long INT64S;
typedef float              FP32;
typedef double             FP64;

// InLine Functions
inline double _PHASE(double x, double y) {
   if ((x == 0) && (y == 0))
      return 0.0;
   else
      return atan2(y,x); // atan(y/x)
}

inline double _MAG(double x, double y) {
   return sqrt(x*x + y*y);
}

inline double _DBV(double x) {
   if (x <= 0.0) return 0.0;
   return (20.0 * log10(x));
}

inline double _CLIP(double x) {
   if (x < 0.0) return 0.0;
   if (x > 255.0) return 255.0;
   return x;
}

// Receive Queue
typedef struct _rxq_t {
   uint32_t         *buf[APP_RX_QUE];
   CRITICAL_SECTION   mutex;
   CONDITION_VARIABLE cv;
   DWORD             tid;
   HANDLE            thread;
   uint8_t           head;
   uint8_t           tail;
   uint8_t           slots;
   uint8_t           halt;
} rxq_t, *prxq_t;

// PCAP file header
typedef struct _PCAP_HDR {
   UINT    magic_number;
   USHORT  version_major;
   USHORT  version_minor;
   INT     thiszone;
   UINT    sigfigs;
   UINT    snaplen;
   UINT    network;
} PCAP_HDR, *PPCAP_HDR;

typedef struct _LASTREQ {
   UCHAR          srvID;
   UCHAR          msgID;
   UCHAR          cliID;
   UCHAR          flags;
   UCHAR          status;
   UCHAR          body[768];
   USHORT         msgLen;
} LASTREQ, *PLASTREQ;

typedef struct _PAGEENTRY {
   WORD           id;
   CView**        pView;
   HTREEITEM*     hParent;
   HTREEITEM*     hItem;
   CRuntimeClass* pRTClass;
   WORD           flags;
} PAGEENTRY, *PPAGEENTRY;

typedef struct _INIENTRY {
   TCHAR    key[APP_STR_LEN];
   TCHAR    value[APP_STR_LEN];
   BYTE     type;
   VOID    *parm;
   BYTE     dim;
   UINT     tag;
   TCHAR    name[APP_STR_LEN];
   TCHAR    desc[APP_STR_LEN];
} INIENTRY, *PINIENTRY;

typedef struct _PLOT {
   UCHAR    plot;
   UCHAR    type;
   UCHAR    lanes[2];
   UCHAR    legend;
   UCHAR    autoScale[2];
   DOUBLE   freq;
   INT      window;
   INT      record;
   INT      length;
   INT      chan;
   DOUBLE   xMin[2];
   DOUBLE   xMax[2];
   DOUBLE   yMin[2];
   DOUBLE   yMax[2];
   FLOAT    *pDatX;
   FLOAT    *pDatY;
} PLOT, *PPLOT;

typedef struct _IMAGE {
   UCHAR    image;
   UCHAR    type;
   UINT     rows;
   UINT     cols;
   UINT     depth;
   UINT     len;
   VOID*    pDat;
} IMAGE, *PIMAGE;

// CM PIPE DAQ PLOT MESSAGE DATA STRUCTURE
// VARIABLE LENGTH PIPE MESSAGE, 1024-Byte Header
typedef struct _CM_PIPE_PLOT {
   unsigned char  dstCMID;       // Destination CM Address
   unsigned char  msgID;         // Pipe Message ID, CM_PIPE_DAQ_PLOT = 0x12
   unsigned char  port;          // Destination Port
   unsigned char  flags;         // Message Flags
   unsigned int   seqID;         // Sequence ID
   unsigned int   varLen;        // Variable Length in 32-Bit Words
   unsigned int   stamp;         // 32-Bit FPGA Clock Count
   unsigned int   plot;          // Plotting Flags
   unsigned int   traceMap;      // Trace Enable Map
   unsigned int   swpLen;        // Sweep Length
   float          xStart;        // X-Axis Start
   float          xStop;         // X-Axis Stop
   float          rate;          // Sampling Rate
   unsigned int   datIdx;        // Index to Plot Data
   unsigned int   rawIdx;        // Index to Raw Data
   unsigned int   xIdx;          // Index to X-Axis Data
   unsigned int   reserved[243]; // Reserved
} CM_PIPE_PLOT, *PCM_PIPE_PLOT;

// CM PIPE DAQ FFT MESSAGE DATA STRUCTURE
// VARIABLE LENGTH PIPE MESSAGE, 1024-Byte Header
typedef struct _CM_PIPE_FFT {
   unsigned char  dstCMID;       // Destination CM Address
   unsigned char  msgID;         // Pipe Message ID, CM_PIPE_DAQ_FFT = 0x13
   unsigned char  port;          // Destination Port
   unsigned char  flags;         // Message Flags
   unsigned int   seqID;         // Sequence ID
   unsigned int   varLen;        // Variable Length in 32-Bit Words
   unsigned int   stamp;         // 32-Bit FPGA Clock Count
   unsigned int   plot;          // Plotting Flags
   unsigned int   traceMap;      // Trace Enable Map
   unsigned int   swpLen;        // Sweep Length
   float          xStart;        // X-Axis Start
   float          xStop;         // X-Axis Stop
   float          rate;          // Sampling Rate
   unsigned int   datIdx;        // Index to Plot Data
   unsigned int   rawIdx;        // Index to Raw Data
   unsigned int   xIdx;          // Index to X-Axis Data
   unsigned int   reserved[243];  // Reserved
} CM_PIPE_FFT, *PCM_PIPE_FFT;

typedef struct _INITYPE {
   // MainFram
   UINT     apLogStamp;
   UINT     apLogEnable;
   UINT     apTrafficStamp;
   UINT     apTrafficEnable;
   UINT     apTrafficLocal;
   UINT     apTrafficPipe;
   UINT     apDebugStamp;
   UINT     apDebugEnable;
   UINT     apLastTree;
   INT      apAutoSave;
   INT      apSysIDWarn;
   INT      apPingWarn;
   INT      apOverwriteWarn;
   INT      apMessageDropWarn;
   INT      apLogCMTraffic;

   // Connection
   INT      conComCon;
   TCHAR    conIpAddr[APP_STR_LEN];
   TCHAR    conMacAddr[APP_STR_LEN];
   UINT     conAutoConnect;
   UINT     conAutoPort;
   INT      conIpPort;
   // CP
   UINT     cpAddr;
   DOUBLE   cpValue;
   INT      cpType;
   INT      cpWR;
   UINT     cpTrace;
   TCHAR    cpMemFile[APP_STR_LEN];
   UINT     cpMemStart;
   UINT     cpMemLength;
   INT      cpMemType;
   INT      cpMemBin;
   INT      cpMemHex;
   INT      cpMemAll;
   INT      cpMemCycle;
   INT      cpMemAWGN;
   INT      cpMemVerify;
   INT      cpMemShow;
   INT      cpMemToFile;
   INT      cpGphType;
   DOUBLE   cpGphXmin;
   DOUBLE   cpGphXmax;
   DOUBLE   cpGphYmin;
   DOUBLE   cpGphYmax;
   INT      cpGphAuto;
   INT      cpGphCursor;
   INT      cpGphPoints;
   INT      cpImgType;
   INT      cpImgRows;
   INT      cpImgCols;
   INT      cpImgDepth;
   INT      cpImgData;
   INT      cpImgAuto;
   // DAQ PARAMETERS
   TCHAR    daqFile[APP_STR_LEN];
   INT      daqHex;
   INT      daqShow;
   INT      daqToFile;
   INT      daqRamp;
   INT      daqFloat;
   INT      daqPlot;
   INT      daqRaw;
   INT      daqPackets;
   // CPU Plot Settings
   INT      cplotLegend;
   INT      cplotAutoA;
   DOUBLE   cplotYminA;
   DOUBLE   cplotYmaxA;
   INT      cplotAutoB;
   DOUBLE   cplotYminB;
   DOUBLE   cplotYmaxB;
   // DAQ Plot Settings
   INT      qplotLegend;
   INT      qplotAutoA;
   DOUBLE   qplotYminA;
   DOUBLE   qplotYmaxA;
   INT      qplotAutoB;
   DOUBLE   qplotYminB;
   DOUBLE   qplotYmaxB;
   INT      qplotLength;
} INITYPE, *PINITYPE;

