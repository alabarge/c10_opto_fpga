
// xView.h : interface of the CxView class
//

#pragma once

#include "MainFrm.h"
#include "resource.h"
#include "ViewEx.h"
#include "EditEx.h"

class CxView : public CViewEx
{
protected: // create from serialization only
	CxView();
	DECLARE_DYNCREATE(CxView)

public:
	enum{ IDD = IDD_MIF };

// Attributes
public:

// Operations
public:

// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CxView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   void OnUpdateConfig(BOOL bDir);
   void OnRangeUpdateED(UINT nID);
   void OnComboUpdateED(UINT nID);
   void OnCheckUpdateED(UINT nID);

   CHAR           m_hCM;
   CFont          m_font;
   CEditEx        m_Edit;

// Generated message map functions
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   afx_msg LRESULT OnCmMsg(WPARAM wParam, LPARAM lParam);
};

