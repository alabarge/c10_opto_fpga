// GraphSet.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrm.h"
#include "GraphSet.h"
#include "GraphWnd.h"

// CGraphSet dialog

IMPLEMENT_DYNAMIC(CGraphSet, CDialog)

CGraphSet::CGraphSet(CWnd* pParent)
	: CDialog(CGraphSet::IDD, pParent)
{
   m_pParent = (CGraphWnd *)pParent;
   m_cpGphType   = 0;
   m_cpGphXmin   = _T("");
   m_cpGphXmax   = _T("");
   m_cpGphYmin   = _T("");
   m_cpGphYmax   = _T("");
   m_cpGphAuto   = 0;
   m_cpGphCursor = 0;
   m_cpGphPoints = 0;
}

CGraphSet::~CGraphSet()
{
}

void CGraphSet::DoDataExchange(CDataExchange* pDX)
{
   CGraphWnd* pP = (CGraphWnd *)m_pParent;
	CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GPH_TYPE, m_Type);
   DDX_CBIndex(pDX, IDC_GPH_TYPE, m_cpGphType);
   DDX_Text(pDX, IDC_GPH_X_MIN, m_cpGphXmin);
   DDX_Text(pDX, IDC_GPH_X_MAX, m_cpGphXmax);
   DDX_Text(pDX, IDC_GPH_Y_MIN, m_cpGphYmin);
   DDX_Text(pDX, IDC_GPH_Y_MAX, m_cpGphYmax);
   DDX_Check(pDX, IDC_GPH_AUTO, m_cpGphAuto);
   DDX_Check(pDX, IDC_GPH_CURSOR, m_cpGphCursor);
   DDX_Check(pDX, IDC_GPH_POINTS, m_cpGphPoints);
   // Transfer Dialog to Parent
   if (pDX->m_bSaveAndValidate == TRUE) {
      pP->m_cpGphType = m_cpGphType;
      swscanf_s(m_cpGphXmin, _T("%lf"), &pP->m_cpGphXmin);
      swscanf_s(m_cpGphXmax, _T("%lf"), &pP->m_cpGphXmax);
      swscanf_s(m_cpGphYmin, _T("%lf"), &pP->m_cpGphYmin);
      swscanf_s(m_cpGphYmax, _T("%lf"), &pP->m_cpGphYmax);
      pP->m_cpGphAuto = m_cpGphAuto;
      pP->m_cpGphCursor = m_cpGphCursor;
      pP->m_cpGphPoints = m_cpGphPoints;
   }
}

BOOL CGraphSet::OnInitDialog()
{
   CDialog::OnInitDialog();

   CGraphWnd* pP = (CGraphWnd *)m_pParent;

	//Create the ToolTip control
	if (!m_toolTip.Create(this)) {
	   TRACE0("Unable to create the CViewEx ToolTip!");
	}

   // Transfer Doc to Dialog
   m_cpGphType = pP->m_cpGphType;
   m_cpGphXmin.Format(_T("%.2f"), pP->m_cpGphXmin);
   m_cpGphXmax.Format(_T("%.2f"), pP->m_cpGphXmax);
   m_cpGphYmin.Format(_T("%.2f"), pP->m_cpGphYmin);
   m_cpGphYmax.Format(_T("%.2f"), pP->m_cpGphYmax);
   m_cpGphAuto = pP->m_cpGphAuto;
   m_cpGphCursor = pP->m_cpGphCursor;
   m_cpGphPoints = pP->m_cpGphPoints;
   UpdateData(FALSE);

   // Disable Limits for Auto
   if (m_cpGphAuto) {
      GetDlgItem(IDC_GPH_X_MIN)->EnableWindow(FALSE);
      GetDlgItem(IDC_GPH_X_MAX)->EnableWindow(FALSE);
      GetDlgItem(IDC_GPH_Y_MIN)->EnableWindow(FALSE);
      GetDlgItem(IDC_GPH_Y_MAX)->EnableWindow(FALSE);
   }
   else {
      GetDlgItem(IDC_GPH_X_MIN)->EnableWindow(TRUE);
      GetDlgItem(IDC_GPH_X_MAX)->EnableWindow(TRUE);
      GetDlgItem(IDC_GPH_Y_MIN)->EnableWindow(TRUE);
      GetDlgItem(IDC_GPH_Y_MAX)->EnableWindow(TRUE);
   }

   // Available Graphs
   m_Type.AddString(GRAPH_NAME_NONE);
   m_Type.AddString(GRAPH_NAME_CPU_MEM);
   m_Type.AddString(GRAPH_NAME_DAQ);

   // Set Current Selection
   m_Type.SetCurSel(m_cpGphType);

   // Default Dialog Background
   m_bgBrush.CreateSolidBrush(APP_DEF_BACK);

   return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CGraphSet, CDialog)
   ON_WM_CTLCOLOR()
	ON_UPDATE_COMMAND_UI(IDC_GPH_AUTO, &CGraphSet::OnUpdateGphAuto)
END_MESSAGE_MAP()

//
// CGraphSet Message Handlers
//

HBRUSH CGraphSet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
   // General Case
   if (nCtlColor == CTLCOLOR_STATIC) {  
      pDC->SetTextColor(APP_DEF_TEXT);
      pDC->SetBkMode(TRANSPARENT);
      hbr = m_bgBrush;
   }
   hbr = m_bgBrush;
   return hbr;
}

void CGraphSet::OnUpdateGphAuto(CCmdUI* pCmdUI)
{
   if (IsDlgButtonChecked(pCmdUI->m_nID)) {
      GetDlgItem(IDC_GPH_X_MIN)->EnableWindow(FALSE);
      GetDlgItem(IDC_GPH_X_MAX)->EnableWindow(FALSE);
      GetDlgItem(IDC_GPH_Y_MIN)->EnableWindow(FALSE);
      GetDlgItem(IDC_GPH_Y_MAX)->EnableWindow(FALSE);
   }
   else {
      GetDlgItem(IDC_GPH_X_MIN)->EnableWindow(TRUE);
      GetDlgItem(IDC_GPH_X_MAX)->EnableWindow(TRUE);
      GetDlgItem(IDC_GPH_Y_MIN)->EnableWindow(TRUE);
      GetDlgItem(IDC_GPH_Y_MAX)->EnableWindow(TRUE);
   }
}


BOOL CGraphSet::PreTranslateMessage(MSG* pMsg)
{
   m_toolTip.RelayEvent(pMsg);
   // Enable UI Update for Dialog Controls
   UpdateDialogControls(this, FALSE);
   return CDialog::PreTranslateMessage(pMsg);
}
