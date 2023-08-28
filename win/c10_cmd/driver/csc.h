//
//  CSC.H Version 7.1
//
//  For Win32 compilers supporting the "declspec" keyword.
//

#ifdef STATIC_LIBRARY
  #define DLL_IMPORT_EXPORT
#else
  #ifdef DLL_SOURCE_CODE
     #define DLL_IMPORT_EXPORT __declspec(dllexport) __stdcall
  #else
     #define DLL_IMPORT_EXPORT __declspec(dllimport) __stdcall
  #endif
#endif

#ifdef __cplusplus
  #define NoMangle extern "C"
#else
  #define NoMangle
#endif

#define CSC_SET_CONNECT_WAIT       1
#define CSC_SET_SLEEP_TIME         2
#define CSC_SET_WRITE_BUFSIZE      4
#define CSC_SET_BLOCKING_MODE      5
#define CSC_SET_LOG_FILE           6
#define CSC_SET_FILE_PATH          7
#define CSC_SET_DEBUG_LEVEL        8
#define CSC_SET_BUFFER_SIZE        9
#define CSC_SET_TIMEOUT_VALUE     10
#define CSC_SET_TCP_NODELAY       11
#define CSC_SET_LINGER            12
#define CSC_SET_STOP_BYTE         13
#define CSC_SET_FILE_APPEND       14
#define CSC_SET_FILE_OVERWRITE    15
#define CSC_SET_DELAY_TIME        16
#define CSC_SET_MAX_PACKET_SIZE   17
#define CSC_SET_CLOSE_TIMEOUT     18
#define CSC_SET_SOCK_REUSE        19

#define CSC_GET_ERROR_TEXT        20
#define CSC_GET_SOCK_ERROR        22
#define CSC_GET_SOCKET            23
#define CSC_GET_CONNECT_STATUS    24
#define CSC_GET_REGISTRATION      25
#define CSC_GET_LOCAL_IP          26
#define CSC_GET_REMOTE_IP         27
#define CSC_GET_VERSION           28
#define CSC_GET_BUILD             29
#define CSC_GET_FILE_NAME         30
#define CSC_GET_FILE_LENGTH       31
#define CSC_GET_REMOTE_SERVER_IP  32
#define CSC_GET_REMOTE_CLIENT_IP  33
#define CSC_GET_COMPUTER_NAME     34
#define CSC_GET_BUFFER_SIZE       35
#define CSC_GET_DAYS_LEFT         36
#define CSC_GET_MAX_PACKET_SIZE   37
#define CSC_GET_MAX_UDP_SIZE      38

#define CSC_WRITE_TO_LOG          50

#define CSC_SET_PAD_TX_INDEX      61
#define CSC_SET_PAD_RX_INDEX      62

#define CSC_NO_ERROR               1
#define CSC_FILES_MATCH            2

#define CSC_EOF                   -1
#define CSC_ABORTED               -2
#define CSC_ACCEPT_ERROR          -3
#define CSC_ALREADY_ATTACHED      -4
#define CSC_CANNOT_COMPLY         -5
#define CSC_NO_SUCH_SOCKET        -6
#define CSC_CONNECT_ERROR         -7
#define CSC_LISTEN_ERROR          -8
#define CSC_NO_SUCH_HOST          -9
#define CSC_NOT_ATTACHED         -10
#define CSC_NULL_ARGUMENT        -11
#define CSC_NULL_POINTER         -12
#define CSC_CANNOT_ALLOCATE      -13
#define CSC_BUFFER_SIZE_ERROR    -14
#define CSC_CRC_ERROR            -15
#define CSC_TOO_MANY_SOCKETS     -16
#define CSC_NO_FREE_SOCKETS      -17
#define CSC_NO_SUCH_FILE         -18
#define CSC_FILE_FORMAT_ERROR    -19
#define CSC_FILE_NAME_ONLY       -20
#define CSC_PACKET_TIMEOUT       -21
#define CSC_PACKET_ERROR         -22
#define CSC_XFER_CANCELLED       -23
#define CSC_FILE_TOO_LARGE       -24
#define CSC_NO_LISTEN_SOCK       -25
#define CSC_ARGUMENT_RANGE       -26
#define CSC_DATA_SIZE            -27
#define CSC_CONNECT_TIMEOUT      -28
#define CSC_PACKET_SIZE          -29
#define CSC_CANNOT_RESOLVE       -30
#define CSC_BAD_KEY_CODE         -74
#define CSC_BAD_OFFSET           -75

#define CSC_EXPIRED   (-104)
#define CSC_KEYCODE   (-108)

#define CSC_DEBUG_OFF            0
#define CSC_DEBUG_LOW            1
#define CSC_DEBUG_HIGH           2

NoMangle int   DLL_IMPORT_EXPORT cscAcceptConnect(int);
NoMangle int   DLL_IMPORT_EXPORT cscAttach(int,int,int);
NoMangle int   DLL_IMPORT_EXPORT cscAwaitConnect(int,int);
NoMangle int   DLL_IMPORT_EXPORT cscAwaitData(int,int);
NoMangle int   DLL_IMPORT_EXPORT cscByteToShort(char *);
NoMangle int   DLL_IMPORT_EXPORT cscChallenge(char *);
NoMangle int   DLL_IMPORT_EXPORT cscClient(char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscClientExt(char *,int,char *);
NoMangle int   DLL_IMPORT_EXPORT cscClose(int);
NoMangle int   DLL_IMPORT_EXPORT cscConnectMessage(unsigned int,int,int);
NoMangle int   DLL_IMPORT_EXPORT cscCreateUDP(int,char *);
NoMangle int   DLL_IMPORT_EXPORT cscCryptoGetData(int,char *,int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscCryptoGetFile(int,char *,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscCryptoGetFileExt(int,int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscCryptoGetPacket(int,char *,int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscCryptoPutData(int,char *,int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscCryptoPutFile(int,char *,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscCryptoPutFileExt(int,char *,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscCryptoPutPacket(int,char *,int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscDataMessage(unsigned int,int,int);
NoMangle int   DLL_IMPORT_EXPORT cscErrorText(int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscFileLength(char *);
NoMangle int   DLL_IMPORT_EXPORT cscFillRandom(char *,int,unsigned int);
NoMangle int   DLL_IMPORT_EXPORT cscGetData(int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscGetFile(int,char *);
NoMangle int   DLL_IMPORT_EXPORT cscGetFileExt(int,int);
NoMangle int   DLL_IMPORT_EXPORT cscGetInteger(int,int);
NoMangle int   DLL_IMPORT_EXPORT cscGetPacket(int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscGetString(int,int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscGetUDP(int,char *,int,char *);
NoMangle int   DLL_IMPORT_EXPORT cscIsConnected(int);
NoMangle int   DLL_IMPORT_EXPORT cscLaunch(char *, char *);
NoMangle int   DLL_IMPORT_EXPORT cscMakeDotted(unsigned int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscMakeDotted4(int,int,int,int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscMulticast(int,unsigned int);
NoMangle int   DLL_IMPORT_EXPORT cscNetToHost16(int Integer);
NoMangle int   DLL_IMPORT_EXPORT cscNetToHost32(int Integer);
NoMangle int   DLL_IMPORT_EXPORT cscPutData(int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscPutFile(int,char *);
NoMangle int   DLL_IMPORT_EXPORT cscPutFileExt(int,char *);
NoMangle int   DLL_IMPORT_EXPORT cscPutPacket(int,char *,int);
NoMangle int   DLL_IMPORT_EXPORT cscPutUDP(int,char *,int,unsigned int,int);
NoMangle int   DLL_IMPORT_EXPORT cscReadSize(int);
NoMangle int   DLL_IMPORT_EXPORT cscRelease(void);
NoMangle int   DLL_IMPORT_EXPORT cscResponse(char *,unsigned int,unsigned int,int,char *);
NoMangle int   DLL_IMPORT_EXPORT cscServer(char *,int,int);
NoMangle int   DLL_IMPORT_EXPORT cscSetInteger(int,int,unsigned int);
NoMangle int   DLL_IMPORT_EXPORT cscSetString(int,int,char *);
NoMangle int   DLL_IMPORT_EXPORT cscShortToByte(char *);
NoMangle int   DLL_IMPORT_EXPORT cscSleep(int);
NoMangle int   DLL_IMPORT_EXPORT cscTestDotted(char *);
NoMangle unsigned int DLL_IMPORT_EXPORT cscDataCRC(unsigned int, char *, int);
NoMangle unsigned int DLL_IMPORT_EXPORT cscFileCRC(char *);
NoMangle unsigned int DLL_IMPORT_EXPORT cscResolve(char *,int);
NoMangle unsigned int DLL_IMPORT_EXPORT cscSystemTics(void);
