// MsgQue.h: interface for the CMsgQueue class.


#pragma once


#include <afxmt.h>

class CMsg : public CObject
{
public:
	CMsg(UINT id, UCHAR *pHdr, UCHAR *pBody, UINT len, int error);
	virtual ~CMsg();

	UINT   m_nID;
	UCHAR *m_pHdr;
	UCHAR *m_pBody;
	UINT   m_len;
	INT    m_error;
};

class CMsgQueue : public CObList
{
public:
	void Add(CMsg *pMsg);
	CMsg *Remove();

private:
	CMutex m_mutex;
};

