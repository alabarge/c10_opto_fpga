
// SrvView.cpp : implementation of the CSrvView class
//

#include "stdafx.h"
#include "SrvView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSrvView

IMPLEMENT_DYNCREATE(CSrvView, CViewEx)

BEGIN_MESSAGE_MAP(CSrvView, CViewEx)
   ON_MESSAGE(WM_CLOSING, &CSrvView::OnClosing)
   ON_WM_TIMER()
END_MESSAGE_MAP()

// CSrvView construction/destruction

CSrvView::CSrvView()
	: CViewEx(CSrvView::IDD)
{
}

CSrvView::~CSrvView()
{
}

void CSrvView::DoDataExchange(CDataExchange* pDX)
{
	CViewEx::DoDataExchange(pDX);
}

void CSrvView::OnTimer(UINT_PTR nIDEvent)
{
   CViewEx::OnTimer(nIDEvent);
}

void CSrvView::OnInitialUpdate()
{
   CViewEx::OnInitialUpdate();
   
}

void CSrvView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   // Transfer data to dialog items
   OnUpdateConfig(FALSE);
}

void CSrvView::OnRangeUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CSrvView::OnComboUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CSrvView::OnCheckUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CSrvView::OnUpdateConfig(BOOL bDir)
{
   CappDoc *pDoc = GetDoc();

   if (bDir == TRUE) {
      // Retrieve data from Form
      UpdateData(TRUE);
   }
   else {
      // Update Form
      UpdateData(FALSE);
   }
}

// CSrvView diagnostics

#ifdef _DEBUG
void CSrvView::AssertValid() const
{
	CViewEx::AssertValid();
}

void CSrvView::Dump(CDumpContext& dc) const
{
	CViewEx::Dump(dc);
}

#endif //_DEBUG

//
// CSrvView Message Handlers
//

afx_msg LRESULT CSrvView::OnClosing(WPARAM wParam, LPARAM lParam)
{
   return 0;
}
