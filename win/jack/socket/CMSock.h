// CMSock.h: interface for the CCMServer and CCMClient class.

#pragma once

#include <afxsock.h>
#include <afxtempl.h>
#include "CMQue.h"

#define  WM_SOCK_NEW_MSG            (WM_USER +  500)
#define  WM_SOCK_DONE               (WM_USER +  501)
#define  WM_SOCK_OPEN               (WM_USER +  502)
#define  WM_SOCK_CONNECT            (WM_USER +  503)

//
// Thread data
//
typedef struct t_CM_SENDDATA
{
	CSocket *pSocket;
	UCHAR   *lpBuf;
	UINT     len;
} CM_SENDDATA;

typedef struct t_CM_OPENDATA
{
   LPCTSTR address;
   UINT    port;
	CWnd   *pWnd;
	CSocket *pSocket;
} CM_OPENDATA;

typedef CMap<UINT,UINT,CSocket *,CSocket *> CM_SOCKMAP;

typedef struct t_CM_RECVDATA
{
	CSocket  *pSocket;
	UINT     hdrSz;
	UINT     bodyPos;
	CCMQueue *pQueue;
	CWnd     *pWnd;
	UINT     id;
} CM_RECVDATA;

//
// CCMServer
//

class CCMServer : public CSocket
{
public:
	CCMServer(){};
	virtual ~CCMServer();

	BOOL Open(UINT nPort);
	void CloseEx();
	void SendEx(UINT id, UCHAR *lpBuf, UINT len);
	void ListenEx(UINT hdrSz, UINT bodyPos, CCMQueue *pQueue, CWnd *pWnd, UINT id);

// Overrides
	virtual void OnAccept (int nErrorCode);

private:
	UINT     m_id;
	CM_SENDDATA m_SendData;
	CM_RECVDATA m_RecvData;
	CM_SOCKMAP  m_mapSockets;
};

//
// CCMClient
//

class CCMClient : public CSocket
{
public:
	CCMClient() {};
	virtual ~CCMClient() {};

	BOOL Open(LPCTSTR lpszHostAddress, UINT nHostPort, CWnd *pWnd = NULL);
	void SendEx(UCHAR *lpBuf, UINT len);
	void ListenEx(UINT hdrSz, UINT bodyPos, CCMQueue *pQueue, CWnd *pWnd, UINT id);

// Overrides

private:
	CM_SENDDATA m_SendData;
	CM_RECVDATA m_RecvData;
	CM_OPENDATA m_OpenData;
};

//
// Threads
//
UINT CM_SendThread( LPVOID pParam );
UINT CM_RecvThread( LPVOID pParam );
UINT CM_OpenThread( LPVOID pParam );
