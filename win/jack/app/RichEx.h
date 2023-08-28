#pragma once

// CRichEx

class CRichEx : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CRichEx)

public:
	CRichEx();
	virtual ~CRichEx();

protected:
	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL PreTranslateMessage(MSG* pMsg);
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
};


