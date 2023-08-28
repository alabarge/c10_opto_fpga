// ImageWnd.cpp : implementation of the CGraphWnd class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "ImageWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CImageWnd

CImageWnd::CImageWnd()
{
}

CImageWnd::~CImageWnd()
{
}

BEGIN_MESSAGE_MAP(CImageWnd, CDockablePane)
	ON_COMMAND(ID_IMAGE_1, OnSettings)
	ON_WM_CREATE()
	ON_WM_SIZE()
   ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers

int CImageWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetToolBar().Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_IMAGE);
	GetToolBar().LoadToolBar(IDR_IMAGE, 0, 0, TRUE);
	GetToolBar().SetPaneStyle(GetToolBar().GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	GetToolBar().SetPaneStyle(GetToolBar().GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	GetToolBar().SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	GetToolBar().SetRouteCommandsViaFrame(FALSE);
	AdjustLayout();

   // OnSize handles re-sizing
   CRect rect;
   rect.bottom = rect.left = rect.right = rect.top = 0;
   m_2DImage.SetBackColor(APP_DEF_IMAGE_BACK);
   m_2DImage.Create(this, rect, 11001, WS_VISIBLE | WS_CHILD);
   m_2DImage.DrawGreyScale();

   return 0;
}

void CImageWnd::OnSize(UINT nType, int cx, int cy)
{
   CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();

   // Set Image Position
   CRect rect;
   GetToolBar().GetWindowRect(rect);
   ::MoveWindow(m_2DImage, 0, rect.Height(), cx, cy, FALSE);
   m_2DImage.DrawGreyScale();
}

void CImageWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL) return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = GetToolBar().CalcFixedLayout(FALSE, TRUE).cy;
	GetToolBar().SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CImageWnd::OnSettings()
{
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();
   CImageSet dlg(this);
   if (dlg.DoModal() == IDOK) {
      pDoc->SetModifiedFlag(TRUE);
   }
}

BOOL CImageWnd::OnEraseBkgnd(CDC* pDC)
{
   return CDockablePane::OnEraseBkgnd(pDC);
}

void CImageWnd::Serialize(CArchive& ar)
{
   CappDoc* pDoc = (CappDoc *)ar.m_pDocument;

   if (ar.IsStoring()) {
   }
   //
   // De-serialize the MainFrame data from storage
   //
   else {
   }
}

void CImageWnd::OnNewDoc(PINITYPE ini)
{
   if (ini != NULL) {
   }
}

