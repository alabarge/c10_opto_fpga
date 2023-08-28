// ProjectSet.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrm.h"
#include "ProjectSet.h"

// CProjectSet dialog

IMPLEMENT_DYNAMIC(CProjectSet, CDialog)

CProjectSet::CProjectSet(CWnd* pParent)
	: CDialog(CProjectSet::IDD, pParent)
{
   m_pParent = (CProjectWnd *)pParent;
   m_apAutoSave  = 0;
   m_apReloadLast = 0;
   m_apSysIDWarn = 1;
   m_apPingWarn = 0;
   m_apOverwriteWarn = 0;
   m_apMessageDropWarn = 0;
   m_apLogCMTraffic = 0;
}

CProjectSet::~CProjectSet()
{
}

void CProjectSet::DoDataExchange(CDataExchange* pDX)
{
   CProjectWnd* pP = (CProjectWnd *)m_pParent;
	CDialog::DoDataExchange(pDX);
   DDX_Check(pDX, IDC_PRJ_SET_AUTO, m_apAutoSave);
   DDX_Check(pDX, IDC_PRJ_SET_RELOAD_LAST, m_apReloadLast);
   DDX_Check(pDX, IDC_PRJ_SET_SYSID_WARN, m_apSysIDWarn);
   DDX_Check(pDX, IDC_PRJ_SET_PING_WARN, m_apPingWarn);
   DDX_Check(pDX, IDC_PRJ_SET_OVERWRITE_WARN, m_apOverwriteWarn);
   DDX_Check(pDX, IDC_PRJ_SET_MESSAGE_DROPPED_WARN, m_apMessageDropWarn);
   DDX_Check(pDX, IDC_PRJ_SET_LOG_CM_TRAFFIC, m_apLogCMTraffic);
   // Transfer Dialog to Parent
   if (pDX->m_bSaveAndValidate == TRUE) {
      pP->m_apAutoSave = m_apAutoSave;
      pP->m_apReloadLast = m_apReloadLast;
      pP->m_apSysIDWarn = m_apSysIDWarn;
      pP->m_apPingWarn = m_apPingWarn;
      pP->m_apOverwriteWarn = m_apOverwriteWarn;
      pP->m_apMessageDropWarn = m_apMessageDropWarn;
      pP->m_apLogCMTraffic = m_apLogCMTraffic;
      if (m_apLogCMTraffic) cm_log(NULL, CM_LOG_ENABLE);
      else cm_log(NULL, CM_LOG_DISABLE);
   }
}

BOOL CProjectSet::OnInitDialog()
{
   CDialog::OnInitDialog();

   CProjectWnd* pP = (CProjectWnd *)m_pParent;

	//Create the ToolTip control
	if (!m_toolTip.Create(this)) {
	   TRACE0("Unable to create the CViewEx ToolTip!");
	}

   // Transfer Doc to Dialog
   m_apAutoSave = pP->m_apAutoSave;
   m_apReloadLast = pP->m_apReloadLast;
   m_apSysIDWarn = pP->m_apSysIDWarn;
   m_apPingWarn = pP->m_apPingWarn;
   m_apOverwriteWarn = pP->m_apOverwriteWarn;
   m_apMessageDropWarn = pP->m_apMessageDropWarn;
   m_apLogCMTraffic = pP->m_apLogCMTraffic;
   UpdateData(FALSE);

   // Default Dialog Background
   m_bgBrush.CreateSolidBrush(APP_DEF_BACK);

   return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CProjectSet, CDialog)
   ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//
// CProjectSet Message Handlers
//

HBRUSH CProjectSet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CProjectSet::PreTranslateMessage(MSG* pMsg)
{
   m_toolTip.RelayEvent(pMsg);
   // Enable UI Update for Dialog Controls
   UpdateDialogControls(this, FALSE);
   return CDialog::PreTranslateMessage(pMsg);
}
