// MFCEditEx.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrm.h"
#include "MFCEditEx.h"


// CMFCEditEx

IMPLEMENT_DYNAMIC(CMFCEditEx, CMFCEditBrowseCtrl)

CMFCEditEx::CMFCEditEx()
{
}

CMFCEditEx::~CMFCEditEx()
{
}

BEGIN_MESSAGE_MAP(CMFCEditEx, CMFCEditBrowseCtrl)
   ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

// CMFCEditEx message handlers

BOOL CMFCEditEx::PreTranslateMessage(MSG* pMsg)
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
