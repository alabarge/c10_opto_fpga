#include "stdafx.h"
#include "LogEdit.h"
#include "MainFrm.h"

// CLogEdit

CLogEdit::CLogEdit()
{
	//default text color
	m_crText = APP_DEF_TEXT;
   m_bEnable = TRUE;
   m_nCount = 0;
   m_bStamp = TRUE;
   m_bLocal = FALSE;
   m_bPipe  = FALSE;
}

CLogEdit::~CLogEdit()
{
	//delete brush
	if (m_brBackGnd.GetSafeHandle())
       m_brBackGnd.DeleteObject();
}

BEGIN_MESSAGE_MAP(CLogEdit, CEdit)
	ON_WM_CTLCOLOR_REFLECT()
   ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


// CLogEdit message handlers



HBRUSH CLogEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
	//set text color
	pDC->SetTextColor(m_crText);
	//set the text's background color
	pDC->SetBkColor(m_crBackGnd);

	//return the brush used for background this sets control background
	return m_brBackGnd;
}


void CLogEdit::SetBackColor(COLORREF rgb)
{
	//set background color ref (used for text's background)
	m_crBackGnd = rgb;

	//free brush
	if (m_brBackGnd.GetSafeHandle())
       m_brBackGnd.DeleteObject();
	//set brush to new color
	m_brBackGnd.CreateSolidBrush(rgb);

	//redraw
	Invalidate(TRUE);
}


void CLogEdit::SetTextColor(COLORREF rgb)
{
	//set text color ref
	m_crText = rgb;

	//redraw
	Invalidate(TRUE);
}

void CLogEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
   CMenu    menu;
   CMenu    *pContextMenu;
   int      nID, nCmd, i,j;
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

   // Choose the appropriate Log
   switch (nID) {
   case IDC_EDIT_LOG :
      pContextMenu = menu.GetSubMenu(0);
      if (m_bStamp == TRUE)
            menu.CheckMenuItem(ID_LOG_STAMP, MF_CHECKED | MF_BYCOMMAND);
         else
            menu.CheckMenuItem(ID_LOG_STAMP, MF_UNCHECKED | MF_BYCOMMAND);
      nCmd = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                 point.x, point.y, AfxGetMainWnd());
      switch (nCmd) {
      case ID_LOG_STAMP :
         pDoc->SetModifiedFlag(TRUE);
         m_bStamp = (m_bStamp == TRUE) ? FALSE : TRUE;
         break;
      case ID_LOG_CLEAR :
         SetWindowText(_T(""));
         m_nCount = 0;
         pMainFrm->PostMessage(WM_LOG_CLEAR, OUTPUT_WND_LOG, 0);
         break;
      case ID_LOG_COPY_ALL :
         SetSel(0, -1);
         Copy();
         SetSel(-1, -1);
         break;
      case ID_LOG_SAVE :
         file = t.Format(_T("C:\\log_%y%m%d_%H%M%S.txt"));
         break;
      }
      break;
   case IDC_EDIT_TRAFFIC :
      pContextMenu = menu.GetSubMenu(1);
      if (m_bEnable == TRUE)
            menu.CheckMenuItem(ID_TRAFFIC_ENABLE, MF_CHECKED | MF_BYCOMMAND);
         else
            menu.CheckMenuItem(ID_TRAFFIC_ENABLE, MF_UNCHECKED | MF_BYCOMMAND);
      if (m_bStamp == TRUE)
            menu.CheckMenuItem(ID_TRAFFIC_STAMP, MF_CHECKED | MF_BYCOMMAND);
         else
            menu.CheckMenuItem(ID_TRAFFIC_STAMP, MF_UNCHECKED | MF_BYCOMMAND);
      if (m_bLocal == TRUE)
            menu.CheckMenuItem(ID_TRAFFIC_LOCAL, MF_CHECKED | MF_BYCOMMAND);
         else
            menu.CheckMenuItem(ID_TRAFFIC_LOCAL, MF_UNCHECKED | MF_BYCOMMAND);
      if (m_bPipe == TRUE)
            menu.CheckMenuItem(ID_TRAFFIC_PIPE, MF_CHECKED | MF_BYCOMMAND);
         else
            menu.CheckMenuItem(ID_TRAFFIC_PIPE, MF_UNCHECKED | MF_BYCOMMAND);
      nCmd = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                 point.x, point.y, AfxGetMainWnd());
      switch (nCmd) {
      case ID_TRAFFIC_STAMP :
         pDoc->SetModifiedFlag(TRUE);
         m_bStamp = (m_bStamp == TRUE) ? FALSE : TRUE;
         break;
      case ID_TRAFFIC_LOCAL :
         pDoc->SetModifiedFlag(TRUE);
         m_bLocal = (m_bLocal == TRUE) ? FALSE : TRUE;
         break;
      case ID_TRAFFIC_PIPE :
         pDoc->SetModifiedFlag(TRUE);
         m_bPipe = (m_bPipe == TRUE) ? FALSE : TRUE;
         break;
      case ID_TRAFFIC_ENABLE :
         pDoc->SetModifiedFlag(TRUE);
         m_bEnable = (m_bEnable == TRUE) ? FALSE : TRUE;
         pMainFrm->m_bLogTraffic = m_bEnable;
         break;
      case ID_TRAFFIC_CLEAR :
         SetWindowText(_T(""));
         m_nCount = 0;
         pMainFrm->PostMessage(WM_LOG_CLEAR, OUTPUT_WND_TRAFFIC, 0);
         break;
      case ID_TRAFFIC_COPY_ALL :
         SetSel(0, -1);
         Copy();
         SetSel(-1, -1);
         break;
      case ID_TRAFFIC_SAVE :
         file = t.Format(_T("C:\\traffic_%y%m%d_%H%M%S.txt"));
         break;
      }
      break;
   }

   // Save the Log Text
   if (!file.IsEmpty()) {
      CFileDialog dlg(FALSE, _T("txt"), file, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Log Files (*.txt)|*.txt|All Files (*.*)|*.*||"));
      if (dlg.DoModal() == IDOK) {
         if (pFile.Open(dlg.GetPathName(), CFile::typeText | CFile::modeCreate | CFile::modeWrite)) {
            for (i=0;i<GetLineCount()-1;i++) {
               j = LineLength(LineIndex(i));
               if (j!=0) {
                  GetLine(i, strEdit.GetBuffer(j), j);
                  strEdit.ReleaseBuffer(j);
                  str.Format(_T("%s\n"), strEdit);
                  pFile.WriteString(str);
               }
               else {
                  str.Format(_T("\n"), strEdit);
                  pFile.WriteString(str);
               }
            }
            pFile.Close();
         }
      }
   }
}
