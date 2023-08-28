#include "stdafx.h"
#include "MainFrm.h"
#include "Resource.h"
#include "About.h"
#include "build.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAbout dialog


CAbout::CAbout(CWnd* pParent)
	: CDialog(CAbout::IDD, pParent)
{
}

void CAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OMNIWARE_LINK, m_link);
}

BEGIN_MESSAGE_MAP(CAbout, CDialog)
   ON_WM_CTLCOLOR()
   ON_WM_LBUTTONDOWN()
   ON_WM_RBUTTONDOWN()
   ON_WM_KEYDOWN()
END_MESSAGE_MAP()

BOOL CAbout::OnInitDialog()
{
   CString str;
   CStatic *pS;
   class CapMain *m_pDlg;

   CDialog::OnInitDialog();

   m_link.SetLinkUrl(_T("http://omniware.us"));
   m_link.SetBackgroundColor(APP_DEF_BACK);

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

   m_link.SetFont(&lf);

	//m_fntPropList.CreateFontIndirect(&lf);


   m_pDlg = (CapMain*)AfxGetApp()->m_pMainWnd;

   str.LoadString(IDS_PROGRAM);
   pS = (CStatic*)GetDlgItem(IDC_PROGRAM);
   pS->SetWindowText(str);
   str.Format(_T("Version %s"),  _T(BUILD_LO));
   pS = (CStatic*)GetDlgItem(IDC_VERSION);
   pS->SetWindowText(str);
   str.LoadString(IDS_COMPANY);
   pS = (CStatic*)GetDlgItem(IDC_COMPANY);
   pS->SetWindowText(str);

   // Program Name Bold
   CWnd *pWnd = GetDlgItem(IDC_PROGRAM);
   CFont *pFont = pWnd->GetFont();
   //LOGFONT lf;
   pFont->GetLogFont(&lf);
   lf.lfWeight+=200;
   m_font.CreateFontIndirect(&lf);
   pWnd->SetFont(&m_font);

   // Background for Dialog
   m_bgBrush.CreateSolidBrush(APP_DEF_BACK);

	return TRUE;
}


HBRUSH CAbout::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
   //
   // Specific Control Case
   //
   if (pWnd->GetDlgCtrlID() == IDC_STATIC) {
      pDC->SetTextColor(APP_DEF_TEXT);
      pDC->SetBkMode(TRANSPARENT);
      hbr = m_bgBrush;
   }
   // General Case
   else if (nCtlColor == CTLCOLOR_STATIC) {  
      pDC->SetTextColor(APP_DEF_TEXT);
      pDC->SetBkMode(TRANSPARENT);
      hbr = m_bgBrush;
   }
   hbr = m_bgBrush;
   return hbr;
}

void CAbout::OnLButtonDown(UINT nFlags, CPoint point)
{
   CDialog::OnLButtonDown(nFlags, point);
   EndDialog(0);
}

void CAbout::OnRButtonDown(UINT nFlags, CPoint point)
{
   CDialog::OnRButtonDown(nFlags, point);
   EndDialog(0);
}
