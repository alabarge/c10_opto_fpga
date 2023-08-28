#pragma once


// CMFCEditEx

class CMFCEditEx : public CMFCEditBrowseCtrl
{
	DECLARE_DYNAMIC(CMFCEditEx)

public:
	CMFCEditEx();
	virtual ~CMFCEditEx();

protected:
	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL PreTranslateMessage(MSG* pMsg);
};


