// CMSock.cpp: implementation of the CCMServer and CCMClient classes.

#include "stdafx.h"
#include "CMSock.h"
#include "cm_const.h"
#include "cmapi.h"
#include "timer.h"
#include "daq_msg.h"

//
// CCMServer
//

   static CHRTimer hrTimer;
	static DOUBLE timer[10000];
	static UINT j = 0;



CCMServer::~CCMServer()
{
	// cleanup all created sockets
	UINT id;
	CSocket *pSocket;
	for (POSITION pos = m_mapSockets.GetStartPosition(); pos;) {
		m_mapSockets.GetNextAssoc(pos, id, pSocket);
		delete pSocket;
	}
}

// Open Socket

BOOL CCMServer::Open(UINT nPort)
{
	return Create(nPort);
}


// Send to Socket

void CCMServer::SendEx(UINT id, UCHAR *lpBuf, UINT len)
{
	// locate the socket for this id
	CSocket *pSocket = m_mapSockets[id];
	if (pSocket) {
		m_SendData.pSocket = pSocket;
		m_SendData.lpBuf = lpBuf;
		m_SendData.len = len;

		// start the thread
		AfxBeginThread(CM_SendThread, &m_SendData);
	}
}

// Listen to Socket

void CCMServer::ListenEx(UINT hdrSz, UINT bodyPos, CCMQueue *pQueue, CWnd *pWnd, UINT id)
{
	// initialize receive data
	m_RecvData.hdrSz = hdrSz;
	m_RecvData.bodyPos = bodyPos;
	m_RecvData.pQueue = pQueue;
	m_RecvData.pWnd = pWnd;
	m_id = id;

 	// start listening
 	Listen();
}

// Listen() calls OnAccept() when a new client is attempting to connect
void CCMServer::OnAccept ( int nErrorCode )
{
	if (nErrorCode == 0) {
		// create a new socket and add to map
 		CSocket *pSocket = new CSocket;
		m_mapSockets[m_id] = pSocket;

		// use this new socket to connect to client
 		Accept((CAsyncSocket&)*pSocket);

		// put socket into synchronous mode
		DWORD arg = 0;
		pSocket->AsyncSelect(0);
		pSocket->IOCtl(FIONBIO, &arg);

		// setup this socket to listen for client messages
		m_RecvData.pSocket = pSocket;
		m_RecvData.id = m_id++;

		// start the thread
		AfxBeginThread(CM_RecvThread, &m_RecvData);

      // we've started
	   m_RecvData.pWnd->PostMessage(WM_SOCK_CONNECT, (WPARAM)&m_RecvData, (LPARAM)0);
	}
}

// Close Sockets

void CCMServer::CloseEx()
{
	UINT id;
	CSocket *pSocket;
	for (POSITION pos = m_mapSockets.GetStartPosition(); pos;) {
		m_mapSockets.GetNextAssoc(pos, id, pSocket);
		pSocket->Close();
	}
	Close();
}

//
// CCMClient
//


// Open Socket

BOOL CCMClient::Open(LPCTSTR lpszHostAddress, UINT nHostPort, CWnd *pWnd)
{
   // Open direct
   if (pWnd == NULL) {
      // open
      if (Create() && Connect(lpszHostAddress, nHostPort)) {
		   // Put socket into synchronous mode
		   DWORD arg = 0;
		   AsyncSelect(0);
		   IOCtl(FIONBIO, &arg);
 		   return TRUE;
      }
      // Reset Hi-Res Timer
      hrTimer.Start();
      return FALSE;
   }
   // Open using Thread
   else {
      if (Create()) {
 	      // initialize the structure passed to thread
	      m_OpenData.address = lpszHostAddress;
         m_OpenData.port    = nHostPort;
         m_OpenData.pWnd    = pWnd;
         m_OpenData.pSocket = this;
       	m_RecvData.pWnd    = pWnd;
	      // start the thread
	      AfxBeginThread(CM_OpenThread, &m_OpenData);
      	return TRUE;
      }
      // Reset Hi-Res Timer
      hrTimer.Start();
      return FALSE;
   }
}

// Send to Socket

void CCMClient::SendEx(UCHAR *lpBuf, UINT len)
{

	// initialize the structure we will pass to thread
	m_SendData.pSocket = this;
	m_SendData.lpBuf   = lpBuf;
	m_SendData.len     = len;

	UCHAR *buf = new UCHAR[len + 4];

	buf[0] = (len & 0xFF);
	buf[1] = (len & 0xFF00) >> 8;
	buf[2] = (len & 0xFF0000) >> 16;
	buf[3] = (len & 0xFF000000) >> 24;

	memcpy_s(&buf[4], len+4, lpBuf, len);

	cm_free((pcm_msg_t)lpBuf);

	// initialize the structure we will pass to thread
	m_SendData.pSocket = this;
	m_SendData.lpBuf   = buf;
	m_SendData.len     = len;

	// start the thread
//	AfxBeginThread(CM_SendThread, &m_SendData);

 	m_SendData.pSocket->Send(m_SendData.lpBuf, m_SendData.len + 4);

   // Release
//   cmapi_Free((pcm_msg_t)pSend->lpBuf);

	delete m_SendData.lpBuf;

}


// Listen to Socket

void CCMClient::ListenEx(UINT hdrSz, UINT bodyPos, CCMQueue *pQueue, CWnd *pWnd, UINT id)
{
	// initialize receive data
	m_RecvData.pSocket = this;
	m_RecvData.hdrSz   = hdrSz;
	m_RecvData.bodyPos = bodyPos;
	m_RecvData.pQueue  = pQueue;
	m_RecvData.pWnd    = pWnd;
	m_RecvData.id      = id;

	// start the thread
	AfxBeginThread(CM_RecvThread, &m_RecvData);
}

//
// Threads
//

UINT CM_SendThread(LPVOID pParam)
{
	// get data from thread creator
	CM_SENDDATA *pSend = (CM_SENDDATA *)pParam;

	// Header
// 	pSend->pSocket->Send(&pSend->len, 4);

   // Body
 	pSend->pSocket->Send(pSend->lpBuf, pSend->len + 4);

   // Release
//   cmapi_Free((pcm_msg_t)pSend->lpBuf);

	delete pSend->lpBuf;

	return 0;
}

UINT CM_OpenThread(LPVOID pParam)
{
   DWORD result;

	// get data from thread creator
	CM_OPENDATA *pOpen = (CM_OPENDATA *)pParam;

   // connect
   if (pOpen->pSocket->Connect(pOpen->address, pOpen->port)) {
		// Put socket into synchronous mode
		DWORD arg = 0;
		pOpen->pSocket->AsyncSelect(0);
		pOpen->pSocket->IOCtl(FIONBIO, &arg);
   	pOpen->pWnd->PostMessage(WM_SOCK_OPEN, (WPARAM)TRUE, (LPARAM)0);
	}
   else {
      result = ::GetLastError();
      if (result != WSAEINTR)
      	pOpen->pWnd->PostMessage(WM_SOCK_OPEN, (WPARAM)FALSE, (LPARAM)result);
   }

	return 0;
}


UINT CM_RecvThread(LPVOID pParam)
{
	// get data from thread creator
	CM_RECVDATA *pRecv = (CM_RECVDATA *)pParam;

	UINT len = 1;
	int error = 0;
   UINT   i,j;
	UCHAR *pBody = NULL;
	UCHAR *pHdr = NULL;

	// while both sockets are open
	while (TRUE) {
		// read the header
		int res;
		pBody = NULL;
		pHdr = new UCHAR[pRecv->hdrSz];
		if ((res = pRecv->pSocket->CAsyncSocket::Receive(pHdr, pRecv->hdrSz)) == SOCKET_ERROR)
			error = ::GetLastError();
		else
			len = res;

		// if closing down, exit thread
		if (len == 0                 ||
          error == WSAECONNRESET   ||
          error == WSAECONNABORTED ||
          error == WSAEINTR)
          break;

		// read the body
		if (!error && len && pRecv->bodyPos != -1) {
         j = 0;
         i = pRecv->bodyPos;
         UINT bodyLen = pHdr[0] | (pHdr[1] << 8) | (pHdr[2] << 16) + (pHdr[3] << 24);
			pBody = new UCHAR[bodyLen];
         while (j < bodyLen) {
			   if ((res = pRecv->pSocket->CAsyncSocket::Receive(&pBody[j], (bodyLen-j))) == SOCKET_ERROR) {
				   error =:: GetLastError();
               break;
            }
			   else {
				   j += res;
            }
         }

			// if closing down, exit thread
			if (res == SOCKET_ERROR      ||
             error == WSAECONNRESET   ||
             error == WSAECONNABORTED ||
             error == WSAEINTR)
             break;
         else
            len += j;
		}

		// Add packet arrival time to pipe message
      pcm_msg_t pMsg = (pcm_msg_t)pBody;
      if (pMsg->h.dst_cmid == CM_ID_PIPE) {
			pcm_pipe_daq_t pipe = (pcm_pipe_daq_t)pBody;
			pipe->stamp_us = (UINT)hrTimer.GetElapsedAsMicroSeconds();
			pipe->flags    = CM_PIPE_FREE;
      }

		// put message in queue, remove header size from length
		pRecv->pQueue->Add(new CCMsg(pRecv->id, pHdr, pBody, len-pRecv->hdrSz, error));

		// post message to window to process this new message
		pRecv->pWnd->PostMessage(WM_SOCK_NEW_MSG);

	}

	// cleanup anything we started
	delete []pHdr;
	delete []pBody;

	// we've stopped
	pRecv->pWnd->PostMessage(WM_SOCK_DONE, (WPARAM)pRecv->id, (LPARAM)error);

	return 0;
}
