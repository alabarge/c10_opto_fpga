
// CliView.cpp : implementation of the CCliView class
//

#include "stdafx.h"
#include "CliView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCliView

IMPLEMENT_DYNCREATE(CCliView, CViewEx)

BEGIN_MESSAGE_MAP(CCliView, CViewEx)
   ON_MESSAGE(WM_CLOSING, &CCliView::OnClosing)
   ON_WM_TIMER()
END_MESSAGE_MAP()

// CCliView construction/destruction

CCliView::CCliView()
	: CViewEx(CCliView::IDD)
{
}

CCliView::~CCliView()
{
}

void CCliView::DoDataExchange(CDataExchange* pDX)
{
	CViewEx::DoDataExchange(pDX);
}

void CCliView::OnTimer(UINT_PTR nIDEvent)
{
   CViewEx::OnTimer(nIDEvent);
}

void CCliView::OnInitialUpdate()
{
   CViewEx::OnInitialUpdate();

}

void CCliView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   // Transfer data to dialog items
   OnUpdateConfig(FALSE);
}

void CCliView::OnRangeUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CCliView::OnComboUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CCliView::OnCheckUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CCliView::OnUpdateConfig(BOOL bDir)
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

// CCliView diagnostics

#ifdef _DEBUG
void CCliView::AssertValid() const
{
	CViewEx::AssertValid();
}

void CCliView::Dump(CDumpContext& dc) const
{
	CViewEx::Dump(dc);
}

#endif //_DEBUG

//
// CCliView Message Handlers
//

afx_msg LRESULT CCliView::OnClosing(WPARAM wParam, LPARAM lParam)
{
   return 0;
}
