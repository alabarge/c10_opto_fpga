// GraphWnd.cpp : implementation of the CGraphWnd class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "ProjectWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CProjectWnd

CProjectWnd::CProjectWnd()
{
   m_apAutoSave = 0;
   m_apReloadLast = 0;
   m_apSysIDWarn = 1;
   m_apPingWarn = 0;
   m_apOverwriteWarn = 0;
   m_apMessageDropWarn = 0;
   m_apLogCMTraffic = 0;
}

CProjectWnd::~CProjectWnd()
{
}

BEGIN_MESSAGE_MAP(CProjectWnd, CDockablePane)
	ON_COMMAND(ID_PROJECT_1, OnSettings)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//ON_WM_PAINT()
	//ON_WM_SETFOCUS()
   ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


// CWorkspaceBar message handlers

int CProjectWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	rect.SetRectEmpty();

	// Create view:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	if (!GetTree().Create(dwViewStyle, rect, this, 4))
	{
		TRACE0("Failed to create file view\n");
		return -1;      // fail to create
	}

	GetToolBar().Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROJECT);
	GetToolBar().LoadToolBar(IDR_PROJECT, 0, 0, TRUE /* Is locked */);
	GetToolBar().SetPaneStyle(GetToolBar().GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	GetToolBar().SetPaneStyle(GetToolBar().GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	GetToolBar().SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	GetToolBar().SetRouteCommandsViaFrame(FALSE);

   // In case we don't serialize
   m_apReloadLast = theApp.GetProfileInt(_T("Settings"), _T("ReloadLast"), 1);


	AdjustLayout();

	return 0;
}

void CProjectWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CProjectWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL) return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = GetToolBar().CalcFixedLayout(FALSE, TRUE).cy;

	GetToolBar().SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	GetTree().SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CProjectWnd::OnSettings()
{
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();
   CProjectSet dlg(this);
   if (dlg.DoModal() == IDOK) {
      pDoc->SetModifiedFlag(TRUE);
      // Update document if items are used elsewhere
      pDoc->m_ini->apAutoSave = m_apAutoSave;
      pDoc->m_ini->apSysIDWarn = m_apSysIDWarn;
      pDoc->m_ini->apPingWarn = m_apPingWarn;
      pDoc->m_ini->apOverwriteWarn = m_apOverwriteWarn;
      pDoc->m_ini->apMessageDropWarn = m_apMessageDropWarn;
      pDoc->m_ini->apLogCMTraffic = m_apLogCMTraffic;
   }
}

void CProjectWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	GetTree().GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CProjectWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	GetTree().SetFocus();
}

void CProjectWnd::Serialize(CArchive& ar)
{
   CappDoc* pDoc = (CappDoc *)ar.m_pDocument;

   if (ar.IsStoring()) {
      pDoc->m_ini->apAutoSave   = m_apAutoSave;
      theApp.WriteProfileInt(_T("Settings"), _T("ReloadLast"), m_apReloadLast);
      pDoc->m_ini->apSysIDWarn   = m_apSysIDWarn;
      pDoc->m_ini->apPingWarn   = m_apPingWarn;
      pDoc->m_ini->apOverwriteWarn   = m_apOverwriteWarn;
      pDoc->m_ini->apMessageDropWarn   = m_apMessageDropWarn;
      pDoc->m_ini->apLogCMTraffic   = m_apLogCMTraffic;
   }
   //
   // De-serialize the MainFrame data from storage
   //
   else {
      m_apAutoSave   = pDoc->m_ini->apAutoSave;
      m_apReloadLast = theApp.GetProfileInt(_T("Settings"), _T("ReloadLast"), 1);
      m_apSysIDWarn   = pDoc->m_ini->apSysIDWarn;
      m_apPingWarn   = pDoc->m_ini->apPingWarn;
      m_apOverwriteWarn   = pDoc->m_ini->apOverwriteWarn;
      m_apMessageDropWarn   = pDoc->m_ini->apMessageDropWarn;
      m_apLogCMTraffic   = pDoc->m_ini->apLogCMTraffic;
   }
}

void CProjectWnd::OnNewDoc(PINITYPE ini)
{
   if (ini != NULL) {
      m_apAutoSave   = ini->apAutoSave;
      m_apSysIDWarn   = ini->apSysIDWarn;
      m_apPingWarn   = ini->apPingWarn;
      m_apOverwriteWarn   = ini->apOverwriteWarn;
      m_apMessageDropWarn   = ini->apMessageDropWarn;
      m_apLogCMTraffic   = ini->apLogCMTraffic;
   }
}

UINT CProjectWnd::AddPage(PAGEENTRY &pPage)
{
   CString  str;
   // Add Root
   if (pPage.hParent == NULL) {
      str.LoadString(pPage.id);
      *pPage.hItem = GetTree().InsertItem(str, 0, 0);
   	GetTree().SetItemState(*pPage.hItem, TVIS_BOLD, TVIS_BOLD);
      GetTree().SetItemData(*pPage.hItem, pPage.id);
   }
   // Add Child
   else {
      str.LoadString(pPage.id);
      *pPage.hItem = GetTree().InsertItem(str, 0, 0, *pPage.hParent);
      GetTree().SetItemData(*pPage.hItem, pPage.id);
   }

   // Special Case for the Connection FormView
   if (pPage.id == IDD_CONNECT) {
   	GetTree().SetItemColor(*pPage.hItem, APP_DEF_CONNECT);
   }

   return 0;
}

BOOL CProjectWnd::SetItem(TVITEM* pItem)
{
   return GetTree().SetItem(pItem);
}

BOOL CProjectWnd::GetItem(TVITEM* pItem)
{
  return GetTree().GetItem(pItem);
}

UINT CProjectWnd::SelectPage(PAGEENTRY &pPage)
{
   GetTree().SelectItem(*pPage.hItem);
   return 0;
}

BOOL CProjectWnd::Expand(HTREEITEM hItem, UINT nCode)
{
  return GetTree().Expand(hItem, nCode);
}

void CProjectWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
   CMenu    menu;
   CMenu    *pContextMenu;
   int      nID, nCmd;
   CString  file, str, strEdit;
   CTime    t = CTime::GetCurrentTime();
   CFileDialog dlg(TRUE);

   CStdioFile  pFile;

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc *pDoc = (CappDoc *)pMainFrm->GetActiveDocument();

   // Get specific Log ID
   nID = pWnd->GetDlgCtrlID();

   // Load the base menu
   menu.LoadMenu(IDR_CONTEXT);

   pContextMenu = menu.GetSubMenu(5);
   nCmd = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
               point.x, point.y, AfxGetMainWnd());
   switch (nCmd) {
   case ID_EXPAND_ALL :
      pMainFrm->SendMessage(WM_EXPAND_ALL, 0, 0);
      break;
   case ID_COLLAPSE_ALL :
      pMainFrm->SendMessage(WM_COLLAPSE_ALL, 0, 0);
      break;
   }
}