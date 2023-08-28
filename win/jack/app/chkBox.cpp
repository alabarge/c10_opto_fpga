// chkBox.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrm.h"
#include "chkBox.h"

BEGIN_MESSAGE_MAP (CchkBox, CCheckListBox)
   ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP ()

CchkBox::CchkBox()
{
   m_clrText = APP_DEF_TEXT;
   m_clrBack = ::GetSysColor(COLOR_3DFACE);
   m_brBkgnd.CreateSolidBrush(m_clrBack);
}

void CchkBox::SetTextColor (COLORREF clrText)
{
   m_clrText = clrText;
   Invalidate();
}

void CchkBox::SetBkColor (COLORREF clrBack)
{
   m_clrBack = clrBack;
   m_brBkgnd.DeleteObject();
   m_brBkgnd.CreateSolidBrush(clrBack);
   Invalidate();
}

HBRUSH CchkBox::CtlColor(CDC* pDC, UINT nCtlColor)
{
   pDC->SetTextColor (m_clrText);
   pDC->SetBkColor (m_clrBack);
   return (HBRUSH) m_brBkgnd;
}