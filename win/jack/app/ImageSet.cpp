// ImageSet.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrm.h"
#include "ImageSet.h"

// CImageSet dialog

IMPLEMENT_DYNAMIC(CImageSet, CDialog)

CImageSet::CImageSet(CWnd* pParent)
	: CDialog(CImageSet::IDD, pParent)
{
   m_pParent = (CImageWnd *)pParent;
}

CImageSet::~CImageSet()
{
}

void CImageSet::DoDataExchange(CDataExchange* pDX)
{
   CImageWnd* pP = (CImageWnd *)m_pParent;
	CDialog::DoDataExchange(pDX);
   // Transfer Dialog to Parent
   if (pDX->m_bSaveAndValidate == TRUE) {
   }
}

BOOL CImageSet::OnInitDialog()
{
   CDialog::OnInitDialog();

   CImageWnd* pP = (CImageWnd *)m_pParent;

	//Create the ToolTip control
	if (!m_toolTip.Create(this)) {
	   TRACE0("Unable to create the CViewEx ToolTip!");
	}

   // Transfer Doc to Dialog
   UpdateData(FALSE);

   // Default Dialog Background
   m_bgBrush.CreateSolidBrush(APP_DEF_BACK);

   return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CImageSet, CDialog)
   ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

//
// CImageSet Message Handlers
//

HBRUSH CImageSet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CImageSet::PreTranslateMessage(MSG* pMsg)
{
   m_toolTip.RelayEvent(pMsg);
   // Enable UI Update for Dialog Controls
   UpdateDialogControls(this, FALSE);
   return CDialog::PreTranslateMessage(pMsg);
}
