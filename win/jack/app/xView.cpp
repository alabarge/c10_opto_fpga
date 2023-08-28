
// xView.cpp : implementation of the CxView class
//

#include "stdafx.h"
#include "xView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CM Registrations
static const CM_SUB conSubs[] = {
   {CM_ID_NULL, NULL, NULL}
};

// CxView

IMPLEMENT_DYNCREATE(CxView, CViewEx)

BEGIN_MESSAGE_MAP(CxView, CViewEx)
   ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_CP_ADDR, IDC_CP_TRACE, OnRangeUpdateED)
   ON_CONTROL_RANGE(BN_CLICKED, IDC_CP_WR, IDC_CP_ERR, OnCheckUpdateED)
   ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_CP_DATA_TYPE, IDC_CP_DATA_TYPE, OnComboUpdateED)
   ON_MESSAGE(WM_CM_MESSAGE, &CxView::OnCmMsg)
   ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CxView construction/destruction

CxView::CxView()
	: CViewEx(CxView::IDD)
{
}

CxView::~CxView()
{
}

void CxView::DoDataExchange(CDataExchange* pDX)
{
	CViewEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_CP_EDIT, m_Edit);
}

BOOL CxView::OnInitDialog()
{
   CViewEx::OnInitDialog();

   // Register this Client with CM
   cmapi_Register(CM_ID_CP_CLI, m_hWnd, m_hCM, (PCM_SUB)&conSubs);

   AddAnchor(IDC_CP_EDIT, TOP_LEFT, BOTTOM_RIGHT);

   m_font.CreatePointFont(80, _T("Consolas"));
   m_Edit.SetFont(&m_font);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CxView::OnInitialUpdate()
{
   // Prevent Focus Rectangle on Connect Button
   GetDlgItem(IDC_CP_MEM_STATIC)->SetFocus();
}

void CxView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   // Transfer data to dialog items
   OnUpdateConfig(FALSE);
}

void CxView::OnRangeUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CxView::OnComboUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CxView::OnCheckUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   UpdateData(TRUE);
}

void CxView::OnUpdateConfig(BOOL bDir)
{
   ClmxDoc *pDoc = GetDoc();

   if (bDir == TRUE) {
      // Retrieve data from Form
      UpdateData(TRUE);
   }
   else {
      // Update Form
      UpdateData(FALSE);
   }
}

// CxView diagnostics

#ifdef _DEBUG
void CxView::AssertValid() const
{
	CViewEx::AssertValid();
}

void CxView::Dump(CDumpContext& dc) const
{
	CViewEx::Dump(dc);
}

#endif //_DEBUG

//
// CxView Message Handlers
//

HBRUSH CxView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   HBRUSH hbr = CViewEx::OnCtlColor(pDC, pWnd, nCtlColor);
   // Blue Text for CEdit
   if (pWnd->GetDlgCtrlID() == IDC_CP_EDIT) {
      pDC->SetTextColor(RGB(0, 0, 255));
      pDC->SetBkMode(TRANSPARENT);
      hbr = m_bgBrush;
   }
   return hbr;
}

afx_msg LRESULT CxView::OnCmMsg(WPARAM wParam, LPARAM lParam)
{
   PCM_MSG pMsg = (PCM_MSG)wParam;
   UCHAR   type = (UCHAR)lParam;
   USHORT  cmMsg = MSG(pMsg->p.srvID, pMsg->p.msgID);
   CString str;

   PCP_TRACE_BODY pTrace;
   PCP_MEM_BODY   pMem;

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

   ClmxDoc *pDoc = GetDoc();

   // Update Connection Status Indicator
   pMainFrm->m_nComErr = pMsg->p.status;

   //
   // Check for non-zero Status, indicating a CM Error
   //
   if (pMsg->p.status != CM_OK) {
      str.Format(_T("Error : CM Message status %02X, srvID %02X, msgID %02X\n"), 
         pMsg->p.status, pMsg->p.srvID, pMsg->p.msgID);
      GetDoc()->Log(str, LMX_MSG_ERROR);
   }

   // Release the Message
   cmapi_Free(pMsg);

   return 0;
}
