#if !defined(AFX_CHKBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_)
#define AFX_CHKBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_

// chkBox.h : Header file
//
#include <afxwin.h>

class CchkBox : public CCheckListBox
{
protected:
   COLORREF m_clrText;
   COLORREF m_clrBack;
   CBrush m_brBkgnd;
public:
   CchkBox();
   void SetTextColor (COLORREF clrText);
   void SetBkColor (COLORREF clrBack);
protected:
   afx_msg HBRUSH CtlColor (CDC* pDC, UINT nCtlColor);
   DECLARE_MESSAGE_MAP ()
};

#endif
