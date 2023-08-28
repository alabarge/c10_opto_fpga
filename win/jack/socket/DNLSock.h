// DNLSock.h: interface for the CMsgServer and CMsgClient class.

#pragma once

#include <afxsock.h>
#include <afxtempl.h>
#include "MsgQue.h"

#define  WM_SOCK_NEW_MSG            (WM_USER +  500)
#define  WM_SOCK_DONE               (WM_USER +  501)
#define  WM_SOCK_OPEN               (WM_USER +  502)
#define  WM_SOCK_CONNECT            (WM_USER +  503)

//
// Thread data
//
typedef struct t_DNL_SENDDATA
{
	CSocket *pSocket;
	UCHAR   *lpBuf;
	UINT     len;
} DNL_SENDDATA;

typedef struct t_DNL_OPENDATA
{
   LPCTSTR address;
   UINT    port;
	CWnd   *pWnd;
	CSocket *pSocket;
} DNL_OPENDATA;

typedef CMap<UINT,UINT,CSocket *,CSocket *> DNL_SOCKMAP;

typedef struct t_DNL_RECVDATA
{
	CSocket  *pSocket;
	UINT     hdrSz;
	UINT     bodyPos;
	CMsgQueue *pQueue;
	CWnd     *pWnd;
	UINT     id;
} DNL_RECVDATA;

//
// CMsgServer
//

class CDNLServer : public CSocket
{
public:
	CDNLServer(){};
	virtual ~CDNLServer();

	BOOL Open(UINT nPort);
	void CloseEx();
	void SendEx(UINT id, UCHAR *lpBuf, UINT len);
	void ListenEx(UINT hdrSz, UINT bodyPos, CMsgQueue *pQueue, CWnd *pWnd, UINT id);

// Overrides
	virtual void OnAccept (int nErrorCode);

private:
	UINT     m_id;
	DNL_SENDDATA m_SendData;
	DNL_RECVDATA m_RecvData;
	DNL_SOCKMAP  m_mapSockets;
};

//
// CDNLClient
//

class CDNLClient : public CSocket
{
public:
	CDNLClient() {};
	virtual ~CDNLClient() {};

	BOOL Open(LPCTSTR lpszHostAddress, UINT nHostPort, CWnd *pWnd = NULL);
	void SendEx(UCHAR *lpBuf, UINT len);
	void ListenEx(UINT hdrSz, UINT bodyPos, CMsgQueue *pQueue, CWnd *pWnd, UINT id);

// Overrides

private:
	DNL_SENDDATA m_SendData;
	DNL_RECVDATA m_RecvData;
	DNL_OPENDATA m_OpenData;
};

//
// Threads
//
UINT DNL_SendThread( LPVOID pParam );
UINT DNL_RecvThread( LPVOID pParam );
UINT DNL_OpenThread( LPVOID pParam );
