// ViewEx.h : interface of the CViewEx class
//

#pragma once

#include "WndResizer.h"
#include "appDoc.h"
#include "RichEx.h"
#include "MFCEditEx.h"

class CViewEx : public CFormView
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CViewEx)
	CViewEx();
   CViewEx(UINT nIDTemplate);

public:
	enum{ IDD = 0 };

// Attributes
public:
	CappDoc* GetDoc() const;

// Operations
public:

// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CViewEx();

   BOOL         m_bStamp;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);

   CBrush       m_bgBrush;
   CWndResizer  m_sizer;
   CSize        m_szIDD;
   CFont        m_font;
   CTime        m_stamp;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnIdleUpdateCmdUI();
   BOOL IsEdit(CWnd* pWnd);
   void OnUpdateNeedSel(CCmdUI* pCmdUI);
   void OnUpdateNeedClip(CCmdUI* pCmdUI);
   void OnEditCopy();
   void OnEditCut();
   void OnEditPaste();
   void OnEditUndo();
   void OnEditSelAll();
   void ToEdit(CRichEx &edit, CString str, COLORREF color = APP_CEDIT_LOG);
   INT  GetLines(CRichEx &edit);

};

#ifndef _DEBUG  // debug version in eegView.cpp
inline CappDoc* CViewEx::GetDoc() const
   { return reinterpret_cast<CappDoc*>(m_pDocument); }
#endif

