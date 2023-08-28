// MsgQue.cpp: implementation of the CMsgQueue class.

#include "stdafx.h"
#include "MsgQue.h"

// Construct Message

CMsg::CMsg(UINT id, UCHAR *pHdr, UCHAR *pBody, UINT len, INT error)
{
	m_nID   = id;
	m_pHdr  = pHdr;
	m_pBody = pBody;
	m_len   = len;
	m_error = error;
}


CMsg::~CMsg()
{
	delete []m_pHdr;
	delete []m_pBody;
}

// Add Message

void CMsgQueue::Add(CMsg *pMsg)
{
	CSingleLock slock(&m_mutex);
	if (slock.Lock(1000)) {
		AddTail(pMsg);
	}
}

// Remove Message

CMsg *CMsgQueue::Remove()
{
	CSingleLock slock(&m_mutex);
	if (slock.Lock(1000)) {
		if (!IsEmpty())
			return (CMsg*)RemoveHead();
	}
	return NULL;
}
