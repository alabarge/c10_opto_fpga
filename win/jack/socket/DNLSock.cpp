// DNLSock.cpp: implementation of the CDNLServer and CDNLClient classes.

#include "stdafx.h"
#include "DNLSock.h"

//
// CDNLServer
//

CDNLServer::~CDNLServer()
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

BOOL CDNLServer::Open(UINT nPort)
{
	return Create(nPort);
}


// Send to Socket

void CDNLServer::SendEx(UINT id, UCHAR *lpBuf, UINT len)
{
	// locate the socket for this id
	CSocket *pSocket = m_mapSockets[id];
	if (pSocket) {
		m_SendData.pSocket = pSocket;
		m_SendData.lpBuf = lpBuf;
		m_SendData.len = len;

		// start the thread
		AfxBeginThread(DNL_SendThread, &m_SendData);
	}
}

// Listen to Socket

void CDNLServer::ListenEx(UINT hdrSz, UINT bodyPos, CMsgQueue *pQueue, CWnd *pWnd, UINT id)
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
void CDNLServer::OnAccept ( int nErrorCode )
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
		AfxBeginThread(DNL_RecvThread, &m_RecvData);

      // we've started
	   m_RecvData.pWnd->PostMessage(WM_SOCK_CONNECT, (WPARAM)&m_RecvData, (LPARAM)0);
	}
}

// Close Sockets

void CDNLServer::CloseEx()
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
// CDNLClient
//


// Open Socket

BOOL CDNLClient::Open(LPCTSTR lpszHostAddress, UINT nHostPort, CWnd *pWnd)
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
	      AfxBeginThread(DNL_OpenThread, &m_OpenData);
      	return TRUE;
      }
      return FALSE;
   }
}

// Send to Socket

void CDNLClient::SendEx(UCHAR *lpBuf, UINT len)
{
	// initialize the structure we will pass to thread
	m_SendData.pSocket = this;
	m_SendData.lpBuf   = lpBuf;
	m_SendData.len     = len;

	// start the thread
	AfxBeginThread(DNL_SendThread, &m_SendData);
}


// Listen to Socket

void CDNLClient::ListenEx(UINT hdrSz, UINT bodyPos, CMsgQueue *pQueue, CWnd *pWnd, UINT id)
{
	// initialize receive data
	m_RecvData.pSocket = this;
	m_RecvData.hdrSz   = hdrSz;
	m_RecvData.bodyPos = bodyPos;
	m_RecvData.pQueue  = pQueue;
	m_RecvData.pWnd    = pWnd;
	m_RecvData.id      = id;

	// start the thread
	AfxBeginThread(DNL_RecvThread, &m_RecvData);
}

//
// Threads
//

UINT DNL_SendThread(LPVOID pParam)
{
	// get data from thread creator
	DNL_SENDDATA *pSend = (DNL_SENDDATA *)pParam;

	// do the write
 	pSend->pSocket->Send(pSend->lpBuf, pSend->len);

	return 0;
}

UINT DNL_OpenThread(LPVOID pParam)
{
   DWORD result;

	// get data from thread creator
	DNL_OPENDATA *pOpen = (DNL_OPENDATA *)pParam;

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


UINT DNL_RecvThread(LPVOID pParam)
{
	// get data from thread creator
	DNL_RECVDATA *pRecv = (DNL_RECVDATA *)pParam;

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
         UINT bodyLen = pHdr[i] | (pHdr[i+1] << 8) | (pHdr[i+2] << 16) + (pHdr[i+3] << 24);
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

		// put message in queue
		pRecv->pQueue->Add(new CMsg(pRecv->id, pHdr, pBody, len, error));

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
