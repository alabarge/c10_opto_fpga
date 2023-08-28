
// About.h : interface of the CAbout class
//

#pragma once

#include "stdafx.h"
#include "HyperLink.h"

class CAbout : public CDialog
{
public:
	CAbout(CWnd* pParent = NULL);

	enum { IDD = IDD_ABOUT };

	CHyperLink m_link;
   CFont m_font;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

   CBrush m_bgBrush;

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
   afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};
