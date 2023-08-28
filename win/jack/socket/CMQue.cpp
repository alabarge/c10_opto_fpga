// CMQue.cpp: implementation of the CCMQueue class.

#include "stdafx.h"
#include "CMQue.h"

// Construct Message

CCMsg::CCMsg(UINT id, UCHAR *pHdr, UCHAR *pBody, UINT len, INT error)
{
	m_nID   = id;
	m_pHdr  = pHdr;
	m_pBody = pBody;
	m_len   = len;
	m_error = error;
}


CCMsg::~CCMsg()
{
	delete []m_pHdr;
}

// Add Message

void CCMQueue::Add(CCMsg *pMsg)
{
	CSingleLock slock(&m_mutex);
	if (slock.Lock(1000)) {
		AddTail(pMsg);
	}
}

// Remove Message

CCMsg *CCMQueue::Remove()
{
	CSingleLock slock(&m_mutex);
	if (slock.Lock(1000)) {
		if (!IsEmpty())
			return (CCMsg*)RemoveHead();
	}
	return NULL;
}
