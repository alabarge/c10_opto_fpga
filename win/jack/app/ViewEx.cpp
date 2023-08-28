// ViewEx.cpp : implementation of the CViewEx class
//

#include "stdafx.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CViewEx

IMPLEMENT_DYNCREATE(CViewEx, CFormView)

BEGIN_MESSAGE_MAP(CViewEx, CFormView)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CViewEx::OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CViewEx::OnUpdateNeedSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CViewEx::OnUpdateNeedClip)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
   ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
   ON_COMMAND(ID_EDIT_COPY, &CViewEx::OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, &CViewEx::OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, &CViewEx::OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, &CViewEx::OnEditUndo)
	ON_COMMAND(ID_EDIT_SELECT_ALL, &CViewEx::OnEditSelAll)
   ON_WM_CTLCOLOR()
   ON_WM_CREATE()
END_MESSAGE_MAP()

// CViewEx construction/destruction

CViewEx::CViewEx()
	: CFormView(CViewEx::IDD)
{
   m_font.CreatePointFont(APP_CEDIT_FONT_SIZE, APP_CEDIT_FONT);
   m_sizer.m_pHookedWnd = NULL;
}
CViewEx::CViewEx(UINT nIDTemplate)
	: CFormView(nIDTemplate)
{
   m_font.CreatePointFont(APP_CEDIT_FONT_SIZE, APP_CEDIT_FONT);
   m_sizer.m_pHookedWnd = NULL;
}

CViewEx::~CViewEx()
{
}

void CViewEx::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

void CViewEx::OnInitialUpdate()
{
   CFormView::OnInitialUpdate();
   // Hook into the WndProc
   if (m_sizer.m_pHookedWnd == NULL) m_sizer.Hook(this, m_szIDD);
   // Create background brush
   if (m_bgBrush.m_hObject == NULL) m_bgBrush.CreateSolidBrush(APP_DEF_BACK);
   // Pull in application data to Form
   UpdateData(FALSE);
   // Enable Form Tool Tips
   EnableToolTips(TRUE);
   // Disable Time Stamp
   m_bStamp = FALSE;
}

void CViewEx::ToEdit(CRichEx &edit, CString str, COLORREF color)
{
	CHARFORMAT cf;
	INT nVisible = 0;

   cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;

   //
   // Time Stamp
   //
   if (m_bStamp) {
      m_stamp = CTime::GetCurrentTime();
      edit.SetSel(edit.GetWindowTextLength(), -1);
   	cf.crTextColor = APP_MSG_STAMP;
      edit.SetSelectionCharFormat(cf);
      edit.ReplaceSel(m_stamp.Format(_T("%X  ")));
   }

   //
   // Log Text
   //
	edit.SetSel(edit.GetWindowTextLength(), -1);
   cf.crTextColor = color;
   edit.SetSelectionCharFormat(cf);
   edit.ReplaceSel(str);
   nVisible = GetLines(edit);
	if (&edit != edit.GetFocus()) {
		edit.LineScroll(INT_MAX);
		edit.LineScroll(1 - nVisible);
	}
	edit.SetSel(-1, 0);
   SendMessage(WM_KILLFOCUS, 0, 0);
}

INT CViewEx::GetLines(CRichEx &edit)
{
	CRect rect;
	long nFirstChar, nLastChar;
	long nFirstLine, nLastLine;

	// Get client rect of rich edit control
	edit.GetClientRect(rect);

	// Get character index close to upper left corner
	nFirstChar = edit.CharFromPos(CPoint(0, 0));

	// Get character index close to lower right corner
	nLastChar = edit.CharFromPos(CPoint(rect.right, rect.bottom));
	if (nLastChar < 0)
	{
		nLastChar = edit.GetTextLength();
	}

	// Convert to lines
	nFirstLine = edit.LineFromChar(nFirstChar);
	nLastLine  = edit.LineFromChar(nLastChar);

	return (nLastLine - nFirstLine);
}

// CViewEx diagnostics

#ifdef _DEBUG
void CViewEx::AssertValid() const
{
	CFormView::AssertValid();
}

void CViewEx::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CappDoc* CViewEx::GetDoc() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CappDoc)));
	return (CappDoc*)m_pDocument;
}
#endif //_DEBUG

// CViewEx message handlers

HBRUSH CViewEx::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
   switch (nCtlColor) {
   case CTLCOLOR_BTN :
   case CTLCOLOR_STATIC :
      pDC->SetBkMode(TRANSPARENT);
   case CTLCOLOR_DLG :
      hbr = m_bgBrush;
   }
   return hbr;
}

//Notification handler - add entry to class definition
BOOL CViewEx::OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult)
{
   TOOLTIPTEXT *pText = (TOOLTIPTEXT *)pNMH;
   int control_id =  ::GetDlgCtrlID((HWND)pNMH->idFrom);
   if(control_id) {
      pText->lpszText = MAKEINTRESOURCE(control_id);
      pText->hinst = AfxGetInstanceHandle();
      return TRUE;
   }
   return FALSE;
}

void CViewEx::OnIdleUpdateCmdUI()
{
	UpdateDialogControls(this, FALSE);
}

// IsEdit: a helper function to determine if a given CWnd pointer
// points to a CEDit control.
// Use the SDK ::GetClassName() function because MFC IsKindOf
// fails if no CEdit variable has been created for the control you're
// trying to test.
BOOL CViewEx::IsEdit(CWnd* pWnd)
{
   ASSERT( pWnd != NULL );
   HWND hWnd = pWnd->GetSafeHwnd();
   if (hWnd == NULL)
      return FALSE;

   TCHAR szClassName[6];
   return ::GetClassName(hWnd, szClassName, 6) &&
            _tcsicmp(szClassName, _T("Edit")) == 0;
}

// UPDATE_COMMAND_UI handler for Edit Copy and Edit Cut which both
// require that the current focus is on an edit control that has
// text selected.
void CViewEx::OnUpdateNeedSel(CCmdUI* pCmdUI)
{
   // get the current focus & determine if its on a CEdit control
   CWnd* pWnd = GetFocus();
   if (NULL == pWnd || !IsEdit( pWnd ))
   {
      pCmdUI->Enable( FALSE );
   }
   else
   {
      CEdit* pEdit = (CEdit*)pWnd;
      int nBeg, nEnd;

      pEdit->GetSel( nBeg, nEnd );
      pCmdUI->Enable( nBeg != nEnd );
   }
}

// UPDATE_COMMAND_UI handlers for Edit Paste requires that focus be
// on an edit control and that the clipboard contains text to be
// pasted into the control.
void CViewEx::OnUpdateNeedClip(CCmdUI* pCmdUI)
{
   // get the current focus & determine if its on a CEdit control
   // also check to see if the control is read-only.
   CWnd* pWnd = GetFocus();
   if (  NULL == pWnd ||
         !IsEdit( pWnd ) ||
         (pWnd->GetStyle() & ES_READONLY) != 0 )
   {
      pCmdUI->Enable( FALSE );
   }
   else
      pCmdUI->Enable(::IsClipboardFormatAvailable(CF_TEXT));
}

void CViewEx::OnEditCopy()
{
   CEdit* pEdit = (CEdit*)GetFocus();
   ASSERT( IsEdit( pEdit) );
   pEdit->Copy();
}

void CViewEx::OnEditCut()
{
   CEdit* pEdit = (CEdit*)GetFocus();
   ASSERT( IsEdit( pEdit) );
   pEdit->Cut();
}

void CViewEx::OnEditPaste()
{
   CEdit* pEdit = (CEdit*)GetFocus();
   ASSERT( IsEdit( pEdit) );
   ASSERT(::IsClipboardFormatAvailable(CF_TEXT));
   pEdit->Paste();
}

void CViewEx::OnEditUndo()
{
   CEdit* pEdit = (CEdit*)GetFocus();
   ASSERT( IsEdit( pEdit) );
   ASSERT(::IsClipboardFormatAvailable(CF_TEXT));
   pEdit->Undo();
}

void CViewEx::OnEditSelAll()
{
   CEdit* pEdit = (CEdit*)GetFocus();
   ASSERT( IsEdit( pEdit) );
   ASSERT(::IsClipboardFormatAvailable(CF_TEXT));
   pEdit->SetSel(0, 1);
}

int CViewEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if (CFormView::OnCreate(lpCreateStruct) == -1)
      return -1;
   m_szIDD.SetSize(lpCreateStruct->cx, lpCreateStruct->cy);
   return 0;
}
