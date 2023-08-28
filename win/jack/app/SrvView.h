
// SrvView.h : interface of the CSrvView class
//

#pragma once

#include "MainFrm.h"
#include "ViewEx.h"

class CSrvView : public CViewEx
{
protected: // create from serialization only
	CSrvView();
	DECLARE_DYNCREATE(CSrvView)

public:
	enum{ IDD = IDD_SRV };

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
	virtual ~CSrvView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   void OnUpdateConfig(BOOL bDir);
   void OnRangeUpdateED(UINT nID);
   void OnComboUpdateED(UINT nID);
   void OnCheckUpdateED(UINT nID);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
   virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   afx_msg LRESULT OnClosing(WPARAM wParam, LPARAM lParam);
   void OnTimer(UINT_PTR nIDEvent);
};

