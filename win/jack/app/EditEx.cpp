// EditEx.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrm.h"
#include "EditEx.h"


// CEditEx

IMPLEMENT_DYNAMIC(CEditEx, CEdit)

CEditEx::CEditEx()
{
}

CEditEx::~CEditEx()
{
}

BEGIN_MESSAGE_MAP(CEditEx, CEdit)
   ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

// CEditEx message handlers

BOOL CEditEx::PreTranslateMessage(MSG* pMsg)
{
   // Intercept Ctrl + Z (Undo), Ctrl + X (Cut), Ctrl + C (Copy), Ctrl + V (Paste) and Ctrl + A (Select All)
   // before CEdit base class gets a hold of them.
   if (pMsg->message == WM_KEYDOWN && ::GetKeyState(VK_CONTROL) < 0) {
      switch (pMsg->wParam) {
      case 'Z':
         Undo();
         return TRUE;
      case 'X':
         Cut();
         return TRUE;
      case 'C':
         Copy();
         return TRUE;
      case 'V':
         Paste();
         return TRUE;
      case 'A':
         SetSel(0, -1);
         return TRUE;
      }
   }
   return CEdit::PreTranslateMessage(pMsg);
}

void CEditEx::OnContextMenu(CWnd* pWnd, CPoint point)
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

   // Load the base menu for the edit logs
   menu.LoadMenu(IDR_CONTEXT);

   pContextMenu = menu.GetSubMenu(4);
   nCmd = pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD,
               point.x, point.y, AfxGetMainWnd());
   switch (nCmd) {
   case ID_EDITEX_CLEAR :
      SetWindowText(_T(""));
      break;
   case ID_EDITEX_COPYALL :
      SetSel(0, -1);
      Copy();
      SetSel(-1, -1);
      break;
   case ID_EDITEX_SAVE :
      file = t.Format(_T("C:\\log_%y%m%d_%H%M%S.txt"));
      break;
   }

   // Save the Log Text
   if (!file.IsEmpty()) {
      CFileDialog dlg(FALSE, _T("txt"), file, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Log Files (*.txt)|*.txt|All Files (*.*)|*.*||"));
      if (dlg.DoModal() == IDOK) {
         if (pFile.Open(dlg.GetPathName(), CFile::typeText | CFile::modeCreate | CFile::modeWrite)) {
            for (i=0;i<GetLineCount()-1;i++) {
               j = LineLength(LineIndex(i));
               GetLine(i, strEdit.GetBuffer(j), j);
               strEdit.ReleaseBuffer(j);
               str.Format(_T("%s\n"), strEdit);
               pFile.WriteString(str);
            }
            pFile.Close();
         }
      }
   }
}

