
#pragma once

#include "LogEdit.h"

#define  OUTPUT_WND_LOG       0
#define  OUTPUT_WND_TRAFFIC   1
#define  OUTPUT_WND_DEBUG     2
#define  OUTPUT_WND_FEC       3
#define  OUTPUT_WND_ALL       10


class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd();

	void UpdateFonts();
	void LogStr(CString str, COLORREF color = RGB(0, 0, 0));
	void TrafficStr(CString str, COLORREF color = RGB(0, 0, 0));
	void DebugStr(CString str, COLORREF color = RGB(0, 0, 0));

   afx_msg LRESULT OnChangeActiveTab(WPARAM wp, LPARAM lp);

// Attributes
protected:

	CMFCTabCtrl	   m_wndTabs;
   CFont          m_font;
	CLogEdit       m_wndLog;
	CLogEdit       m_wndTraffic;
	CLogEdit       m_wndDebug;

   int            m_nActiveTab;
   CTime          m_LogStamp;
   CTime          m_TrafficStamp;
   CTime          m_DebugStamp;
   time_t         m_szClock;

protected:

// Implementation
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
   
   int GetLines(CLogEdit* pCtrl);

   CRITICAL_SECTION   m_hLogMutex;
   CRITICAL_SECTION   m_hTrafficMutex;
   CRITICAL_SECTION   m_hDebugMutex;

	DECLARE_MESSAGE_MAP()

public:
   CMFCTabCtrl&   GetTab()      { return m_wndTabs; }
   CLogEdit&      GetLog()      { return m_wndLog; }
   CLogEdit&      GetTraffic()  { return m_wndTraffic; }
   CLogEdit&      GetDebug()    { return m_wndDebug; }
};

