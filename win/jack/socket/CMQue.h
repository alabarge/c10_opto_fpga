// CMQue.h: interface for the CCMQueue class.


#pragma once


#include <afxmt.h>

class CCMsg : public CObject
{
public:
	CCMsg(UINT id, UCHAR *pHdr, UCHAR *pBody, UINT len, INT error);
	virtual ~CCMsg();

	UINT   m_nID;
	UCHAR *m_pHdr;
	UCHAR *m_pBody;
	UINT   m_len;
	INT    m_error;
};

class CCMQueue : public CObList
{
public:
	void Add(CCMsg *pMsg);
	CCMsg *Remove();

private:
	CMutex m_mutex;
};

