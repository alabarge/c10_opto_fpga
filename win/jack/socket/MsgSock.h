// MsgSock.h: interface for the CMsgServer and CMsgClient class.

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
typedef struct t_SENDDATA
{
	CSocket *pSocket;
	UCHAR   *lpBuf;
	UINT     len;
} SENDDATA;

typedef struct t_OPENDATA
{
   LPCTSTR address;
   UINT    port;
	CWnd   *pWnd;
	CSocket *pSocket;
} OPENDATA;

typedef CMap<UINT,UINT,CSocket *,CSocket *> SOCKMAP;

typedef struct t_RECVDATA
{
	CSocket  *pSocket;
	UINT     hdrSz;
	UINT     bodyPos;
	CMsgQueue *pQueue;
	CWnd     *pWnd;
	UINT     id;
} RECVDATA;

//
// CMsgServer
//

class CMsgServer : public CSocket
{
public:
	CMsgServer(){};
	virtual ~CMsgServer();

	BOOL Open(UINT nPort);
	void CloseEx();
	void SendEx(UINT id, UCHAR *lpBuf, UINT len);
	void ListenEx(UINT hdrSz, UINT bodyPos, CMsgQueue *pQueue, CWnd *pWnd, UINT id);

// Overrides
	virtual void OnAccept (int nErrorCode);

private:
	UINT     m_id;
	SENDDATA m_SendData;
	RECVDATA m_RecvData;
	SOCKMAP  m_mapSockets;
};

//
// CMsgClient
//

class CMsgClient : public CSocket
{
public:
	CMsgClient() {};
	virtual ~CMsgClient() {};

	BOOL Open(LPCTSTR lpszHostAddress, UINT nHostPort, CWnd *pWnd = NULL);
	void SendEx(UCHAR *lpBuf, UINT len);
	void ListenEx(UINT hdrSz, UINT bodyPos, CMsgQueue *pQueue, CWnd *pWnd, UINT id);

// Overrides

private:
	SENDDATA m_SendData;
	RECVDATA m_RecvData;
	OPENDATA m_OpenData;
};

//
// Threads
//
UINT SendThread( LPVOID pParam );
UINT RecvThread( LPVOID pParam );
UINT OpenThread( LPVOID pParam );
