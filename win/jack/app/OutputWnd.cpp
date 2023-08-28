
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
   ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, OnChangeActiveTab)
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CString strTabName;
	BOOL bNameValid;

	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	rect.SetRectEmpty();

	// Create tabs window:
	if (!GetTab().Create(CMFCTabCtrl::STYLE_3D, rect, this, 1))
	{
		TRACE0("Failed to create output tab window\n");
		return -1;      // fail to create
	}

	// Create output panes:
	const DWORD dwStyle = WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_HSCROLL|WS_VSCROLL;

	if (!GetLog().Create(dwStyle, rect, &GetTab(), IDC_EDIT_LOG) ||
		!GetTraffic().Create(dwStyle, rect, &GetTab(), IDC_EDIT_TRAFFIC) ||
		!GetDebug().Create(dwStyle, rect, &GetTab(), IDC_EDIT_DEBUG))
	{
		TRACE0("Failed to create output windows\n");
		return -1;      // fail to create
	}

   GetLog().SetReadOnly(TRUE);
   GetTraffic().SetReadOnly(TRUE);
   GetDebug().SetReadOnly(TRUE);

   PARAFORMAT pf;
   pf.cbSize = sizeof(PARAFORMAT);
   pf.dwMask = PFM_TABSTOPS;
   pf.cTabCount = 1;
   pf.rgxTabs[0] = 350;
   GetTraffic().SetParaFormat(pf);

   // RichEdit Margins
   pf.dwMask = PFM_STARTINDENT | PFM_RIGHTINDENT;
   pf.dxStartIndent = APP_CEDIT_MARGINS;
   pf.dxRightIndent = APP_CEDIT_MARGINS;
   GetTraffic().SetParaFormat(pf);
   GetLog().SetParaFormat(pf);
   GetDebug().SetParaFormat(pf);

	UpdateFonts();

	// Attach edit windows to tab:
	bNameValid = strTabName.LoadString(IDS_LOG_TAB);
	ASSERT(bNameValid);
	GetTab().AddTab(&GetLog(), strTabName, OUTPUT_WND_LOG);
	bNameValid = strTabName.LoadString(IDS_TRAFFIC_TAB);
	ASSERT(bNameValid);
	GetTab().AddTab(&GetTraffic(), strTabName, OUTPUT_WND_TRAFFIC);
	bNameValid = strTabName.LoadString(IDS_DEBUG_TAB);
	ASSERT(bNameValid);
	GetTab().AddTab(&GetDebug(), strTabName, OUTPUT_WND_DEBUG);

   GetTab().SetActiveTabBoldFont();
   GetTab().SetActiveTab(0);
   GetTab().EnableTabSwap(FALSE);

   // Enable Auto Colored Tabs
   GetTab().EnableAutoColor(TRUE);

   // Tab Colors, Auto doesn't quite work
   GetTab().SetTabBkColor(0, 0x00f2d4c5);
   GetTab().SetTabBkColor(1, 0x0078dcff);
   GetTab().SetTabBkColor(2, 0x00a1cebe);
   GetTab().SetTabBkColor(3, 0x00a1a0f0);
   GetTab().SetTabBkColor(4, 0x00e1a8bc);

   // Create Mutex for thread safety
   InitializeCriticalSection(&m_hLogMutex);
   InitializeCriticalSection(&m_hTrafficMutex);
   InitializeCriticalSection(&m_hDebugMutex);

   m_nActiveTab = OUTPUT_WND_LOG;

   // Clear time stamps
   ::ZeroMemory(&m_LogStamp, sizeof(m_LogStamp));
   ::ZeroMemory(&m_TrafficStamp, sizeof(m_TrafficStamp));
   ::ZeroMemory(&m_DebugStamp, sizeof(m_DebugStamp));

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	GetTab().SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::LogStr(CString str, COLORREF color)
{
   CString strTab, strCnt;
	long nVisible = 0;
	CHARFORMAT cf;

   // Enter Critical Section
   EnterCriticalSection(&m_hLogMutex);

   // Initialize character format structure
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;

   //
   // Time Stamp
   //
   m_LogStamp = CTime::GetCurrentTime();
   if (GetLog().m_bStamp) {
	   GetLog().SetSel(GetLog().GetWindowTextLength(), -1);
   	cf.crTextColor = APP_MSG_STAMP;
   	GetLog().SetSelectionCharFormat(cf);
      GetLog().ReplaceSel(m_LogStamp.Format(_T("%X   ")));
   }
   //
   // Log Text
   //
	GetLog().SetSel(GetLog().GetWindowTextLength(), -1);
   cf.crTextColor = color;
   GetLog().SetSelectionCharFormat(cf);
   GetLog().ReplaceSel(str);
   nVisible = GetLines(&GetLog());
	if (&GetLog() != GetLog().GetFocus()) {
		GetLog().LineScroll(INT_MAX);
		GetLog().LineScroll(1 - nVisible);
	}
   //
   // Log Title Bar
   //
   GetLog().m_nCount++;
   if (m_nActiveTab == OUTPUT_WND_LOG) {
      strTab.LoadString(IDS_LOG_TAB);
      strCnt.Format(_T("%s  [ %d ] %s"), strTab, GetLog().m_nCount, m_LogStamp.Format(_T("%X")));
      SetWindowText(strCnt);
   }

   // Leave Critical Section
   LeaveCriticalSection(&m_hLogMutex);

}

void COutputWnd::TrafficStr(CString str, COLORREF color)
{
   CString strTab, strCnt;
	long nVisible = 0;
	CHARFORMAT cf;

   // Enter Critical Section
   EnterCriticalSection(&m_hTrafficMutex);

   // Initialize character format structure
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;

   //
   // Time Stamp
   //
   //m_TrafficStamp = CTime::GetCurrentTime();
   //if (GetTraffic().m_bStamp) {
	  // GetTraffic().SetSel(GetTraffic().GetWindowTextLength(), -1);
   //	cf.crTextColor = APP_MSG_STAMP;
   //	GetTraffic().SetSelectionCharFormat(cf);
   //   GetTraffic().ReplaceSel(m_TrafficStamp.Format(_T("%X   ")));
   //}
   LARGE_INTEGER timeMicro;
   LARGE_INTEGER timeFreq;
   ::QueryPerformanceCounter(&timeMicro);
   ::QueryPerformanceFrequency(&timeFreq);
   timeMicro.QuadPart *= 1000000;
   timeMicro.QuadPart /= timeFreq.QuadPart;
   if (GetTraffic().m_bStamp) {
      GetTraffic().SetSel(GetTraffic().GetWindowTextLength(), -1);
      cf.crTextColor = APP_MSG_STAMP;
      GetTraffic().SetSelectionCharFormat(cf);
      CString str;
      str.Format(_T("%lld:%02lld.%03lld.%03lld"), (timeMicro.QuadPart / 1000000) / 60, 
         (timeMicro.QuadPart / 1000000) % 60, (timeMicro.QuadPart % 1000000) / 1000, (timeMicro.QuadPart % 1000));
      GetTraffic().ReplaceSel(str);
   }
   //
   // Log Text
   //
	GetTraffic().SetSel(GetTraffic().GetWindowTextLength(), -1);
   cf.crTextColor = color;
   GetTraffic().SetSelectionCharFormat(cf);
   GetTraffic().ReplaceSel(str);
   nVisible = GetLines(&GetTraffic());
	if (&GetTraffic() != GetTraffic().GetFocus()) {
		GetTraffic().LineScroll(INT_MAX);
		GetTraffic().LineScroll(1 - nVisible);
	}
   //
   // Log Title Bar
   //
   GetTraffic().m_nCount++;
   if (m_nActiveTab == OUTPUT_WND_TRAFFIC) {
      strTab.LoadString(IDS_TRAFFIC_TAB);
      strCnt.Format(_T("%s  [ %d ] %s"), strTab, GetTraffic().m_nCount, m_TrafficStamp.Format(_T("%X")));
      SetWindowText(strCnt);
   }

   // Leave Critical Section
   LeaveCriticalSection(&m_hTrafficMutex);

}

void COutputWnd::DebugStr(CString str, COLORREF color)
{
   CString strTab, strCnt;
	long nVisible = 0;
	CHARFORMAT cf;

   // Enter Critical Section
   EnterCriticalSection(&m_hDebugMutex);

   // Initialize character format structure
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;

   //
   // Time Stamp
   //
   m_DebugStamp = CTime::GetCurrentTime();
   if (GetDebug().m_bStamp) {
	   GetDebug().SetSel(GetDebug().GetWindowTextLength(), -1);
   	cf.crTextColor = APP_MSG_STAMP;
   	GetDebug().SetSelectionCharFormat(cf);
      GetDebug().ReplaceSel(m_DebugStamp.Format(_T("%X   ")));
   }
   //
   // Log Text
   //
	GetDebug().SetSel(GetDebug().GetWindowTextLength(), -1);
   cf.crTextColor = color;
   GetDebug().SetSelectionCharFormat(cf);
   GetDebug().ReplaceSel(str);
   nVisible = GetLines(&GetDebug());
	if (&GetDebug() != GetDebug().GetFocus()) {
		GetDebug().LineScroll(INT_MAX);
		GetDebug().LineScroll(1 - nVisible);
	}
   //
   // Log Title Bar
   //
   GetDebug().m_nCount++;
   if (m_nActiveTab == OUTPUT_WND_DEBUG) {
      strTab.LoadString(IDS_DEBUG_TAB);
      strCnt.Format(_T("%s  [ %d ] %s"), strTab, GetDebug().m_nCount, m_DebugStamp.Format(_T("%X")));
      SetWindowText(strCnt);
   }

   // Leave Critical Section
   LeaveCriticalSection(&m_hDebugMutex);

}

void COutputWnd::UpdateFonts()
{
   AFX_GLOBAL_DATA *afxGlobalData = GetGlobalData();

   LOGFONT lf;
   ::ZeroMemory(&lf, sizeof(lf));
   CClientDC dc(this);

   GetLog().SetFont(&afxGlobalData->fontRegular);
   GetLog().SetBackColor(APP_DEF_BACK);
   GetLog().SetTextColor(APP_DEF_TEXT);

   // Traffic Font
   lf.lfHeight         = MulDiv(10, dc.GetDeviceCaps(LOGPIXELSX), 72);
   lf.lfWidth          = 0;
   lf.lfEscapement     = 0;
   lf.lfOrientation    = 0;
   lf.lfWeight         = FW_NORMAL;
   lf.lfItalic         = FALSE;
   lf.lfUnderline      = FALSE;
   lf.lfStrikeOut      = FALSE;
   lf.lfCharSet        = DEFAULT_CHARSET;
   lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
   lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
   lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
   wcscpy_s(lf.lfFaceName, _T("Consolas"));
   m_font.CreateFontIndirect(&lf);

   GetTraffic().SetFont(&m_font);
   GetTraffic().SetBackColor(APP_DEF_BACK);
   GetTraffic().SetTextColor(APP_DEF_TEXT);

	GetDebug().SetFont(&afxGlobalData->fontRegular);
   GetDebug().SetBackColor(APP_DEF_BACK);
   GetDebug().SetTextColor(APP_DEF_TEXT);

}

LRESULT COutputWnd::OnChangeActiveTab(WPARAM wp, LPARAM lp)
{
   CString str, strCnt, strTim;
   BOOL    bValid;

   switch(wp) {
      case OUTPUT_WND_LOG :
   	   bValid = str.LoadString(IDS_LOG_TAB);
         if (GetLog().m_bStamp && GetLog().m_nCount) 
            strTim = m_LogStamp.Format(_T("%X"));
         strCnt.Format(_T("%s  [ %d ] %s"), str, GetLog().m_nCount, strTim);
         m_nActiveTab = OUTPUT_WND_LOG;
         break;
      case OUTPUT_WND_TRAFFIC :
   	   bValid = str.LoadString(IDS_TRAFFIC_TAB);
         if (GetTraffic().m_bStamp && GetTraffic().m_nCount) 
            strTim = m_TrafficStamp.Format(_T("%X"));
         strCnt.Format(_T("%s  [ %d ] %s"), str, GetTraffic().m_nCount, strTim);
         m_nActiveTab = OUTPUT_WND_TRAFFIC;
         break;
      case OUTPUT_WND_DEBUG :
   	   bValid = str.LoadString(IDS_DEBUG_TAB);
         if (GetDebug().m_bStamp && GetDebug().m_nCount) 
            strTim = m_DebugStamp.Format(_T("%X"));
         strCnt.Format(_T("%s  [ %d ] %s"), str, GetDebug().m_nCount, strTim);
         m_nActiveTab = OUTPUT_WND_DEBUG;
         break;
      case OUTPUT_WND_ALL :
         GetLog().m_nCount = 0;
         GetLog().SetWindowText(_T(""));
         GetTraffic().m_nCount = 0;
         GetTraffic().SetWindowText(_T(""));
         GetDebug().m_nCount = 0;
         GetDebug().SetWindowText(_T(""));
         bValid = str.LoadString(IDS_LOG_TAB);
         if (GetLog().m_bStamp && GetLog().m_nCount) 
            strTim = m_LogStamp.Format(_T("%X"));
         strCnt.Format(_T("%s  [ %d ] %s"), str, GetLog().m_nCount, strTim);
         m_nActiveTab = OUTPUT_WND_LOG;
         break;
   }
	ASSERT(bValid);
   SetWindowText(strCnt);

   return 0;
}

int COutputWnd::GetLines(CLogEdit* pCtrl)
{
	CRect rect;
	long nFirstChar, nLastChar;
	long nFirstLine, nLastLine;

	// Get client rect of rich edit control
	pCtrl->GetClientRect(rect);

	// Get character index close to upper left corner
	nFirstChar = pCtrl->CharFromPos(CPoint(0, 0));

	// Get character index close to lower right corner
	nLastChar = pCtrl->CharFromPos(CPoint(rect.right, rect.bottom));
	if (nLastChar < 0)
	{
		nLastChar = pCtrl->GetTextLength();
	}

	// Convert to lines
	nFirstLine = pCtrl->LineFromChar(nFirstChar);
	nLastLine  = pCtrl->LineFromChar(nLastChar);

	return (nLastLine - nFirstLine);
}
