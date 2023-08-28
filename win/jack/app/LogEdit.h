// LogEdit.h : interface of the CLogEdit class
//

#pragma once

// LogEdit.h : header file
//

// CLogEdit window

class CLogEdit : public CRichEditCtrl
{
// Construction
public:
	CLogEdit();

// Attributes
public:

// Operations
public:

// Overrides

// Implementation
public:
	void SetTextColor(COLORREF rgb);
	void SetBackColor(COLORREF rgb);
	virtual ~CLogEdit();

   BOOL     m_bEnable;
   BOOL     m_bStamp;
   BOOL     m_bLocal;
   BOOL     m_bPipe;
   UINT     m_nCount;

	// Generated message map functions
protected:
	COLORREF m_crText;
	COLORREF m_crBackGnd;
	CBrush   m_brBackGnd;
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

};
