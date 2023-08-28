
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "app.h"
#include "appDoc.h"
#include "cm_const.h"
#include "cmapi.h"
#include "cm_msg.h"
#include "cp_msg.h"
#include "daq_msg.h"
#include "msg_str.h"
#include "ViewEx.h"
#include "ProjectWnd.h"
#include "OutputWnd.h"
#include "GraphWnd.h"
#include "ImageWnd.h"
#include "ParmsWnd.h"
#include "ConView.h"
#include "CpView.h"
#include "CpMemView.h"
#include "DAQView.h"
#include "fpga_build.h"
#include "timer.h"
#include "trace.h"

#define  MF_TIMER_NONE     0
#define  MF_TIMER_SYS      1
#define  MF_TIMER_INT      2

// Status Bar Panes
#define  MF_SBAR_STATUS    0
#define  MF_SBAR_COUNT     1
#define  MF_SBAR_TIMER     2
#define  MF_SBAR_COM       3
#define  MF_SBAR_CONNECT   4

class CMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

   void     Log(CString str, COLORREF color = APP_MSG_NORMAL);
   void     Traffic(CString str, COLORREF color = APP_MSG_NORMAL);
   void     Debug(CString str, COLORREF color = APP_MSG_NORMAL);
   void     OnError(UINT &nError);
   afx_msg  LRESULT OnChangeView(WPARAM wp, LPARAM lp);
   void     OnSelectView(UINT itemID);
   void     OnCreateView(UINT itemID);
   void     OnMessageView(UINT itemID, UINT msgID, WPARAM wParam, LPARAM lParam);
   void     OnNewDoc(PINITYPE ini);
   void     ProgressBarEnable(INT nIndex, LONG nTotal);
   void     ProgressBarSet(INT nIndex, LONG nCurr);

   // Interval Timers
   UINT     m_nSysTime;
   UINT     m_nSysTimeMS;
   UINT     m_nIntTimeMS;
   UINT     m_nIntTime;
   UINT     m_nShowTime;

   CHRTimer m_timer;
   uint32_t m_last_us;

   LARGE_INTEGER      m_pcFreq;

   // Connection Bitmaps and State
   HBITMAP  m_hBmpBlu;
   HBITMAP  m_hBmpRed;
   HBITMAP  m_hBmpGrn;
   HBITMAP  m_hBmpAct;
   INT      m_nCom;
   INT      m_nPort;
   UINT     m_nPortId;
   CHAR     m_chSerial[16];
   HANDLE   m_hCom;
   UINT     m_nComErr;
   UINT     m_nComAct;
   PVOID    m_pMIFftdi;
   INT      m_nLmcSpi;
   BOOL     m_bLogTraffic;

   // Currently Selected View
   WORD     m_nView;

   // Debug Trace and Errors
   UINT     m_nTrace;
   UINT     m_nError;

   // Intel IPP Thread
   CWinThread  *m_ippThread;

   // Current working directory and EXE path
   CString  m_cwd;
   CString  m_exe;
   TCHAR    m_exePath[MAX_PATH];
   TCHAR    m_mexPath[MAX_PATH];

   // Persistent Data
   UINT     m_lastTree;

   // Misc.
   BOOL     m_bModifiedFlag;
   CString  m_str_status;

protected:  
   
   // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CProjectWnd       m_wndProjectWnd;
	COutputWnd        m_wndOutput;
	CGraphWnd         m_wndGraphWnd;
	CImageWnd         m_wndImageWnd;
	CParmsWnd         m_wndParmsWnd;
   CTabbedPane       m_pTabbedBar;
      
   // All the Views and Tree Handles
   CView             *m_pCurView;
   CView             *m_pConView;
   CView             *m_pCpView;
   CView             *m_pCpMemView;
   CView             *m_pDAQView;
   HTREEITEM          m_hConView;
   HTREEITEM          m_hCpView;
   HTREEITEM          m_hCpMemView;
   HTREEITEM          m_hDAQView;
   
   CRITICAL_SECTION   m_hTrafficMutex;
   BOOL               m_bOnce;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnViewProjectWnd();
	afx_msg void OnUpdateViewProjectWnd(CCmdUI* pCmdUI);
	afx_msg void OnViewGraphWnd();
	afx_msg void OnUpdateViewGraphWnd(CCmdUI* pCmdUI);
	afx_msg void OnViewImageWnd();
	afx_msg void OnViewOutputWnd();
	afx_msg void OnUpdateViewOutputWnd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCustomize(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
   VOID ReportStartup();

   CMapWordToPtr  m_pageMap;
   PPAGEENTRY     m_pPage;
   UINT           m_pageLen;

   HDEVNOTIFY     m_hDevNotify;

public:
   afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
   void WriteSBar(UINT pain, BOOL show = TRUE);

   CMFCStatusBar&  GetStatusBar()   { return m_wndStatusBar; }
   CMFCMenuBar&    GetMenuBar()     { return m_wndMenuBar; }
   CMFCToolBar&    GetToolBar()     { return m_wndToolBar; }
   CProjectWnd&    GetProjectWnd()  { return m_wndProjectWnd; }
   COutputWnd&     GetOutputWnd()   { return m_wndOutput; }
   CGraphWnd&      GetGraphWnd()    { return m_wndGraphWnd; }
   CImageWnd&      GetImageWnd()    { return m_wndImageWnd; }
   CParmsWnd&      GetParmsWnd()    { return m_wndParmsWnd; }

   afx_msg void OnTimer(UINT_PTR nIDEvent);
   afx_msg void OnUpdateIndCom(CCmdUI *pCmdUI);
   afx_msg void OnUpdateIndConnect(CCmdUI *pCmdUI);

protected:
   afx_msg LRESULT OnPlot(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnLogMsg(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnTrafficMsg(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnDebugMsg(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnLogClear(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnCmTraffic(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnDropMsg(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnCollapseAll(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnExpandAll(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnStatusBar(WPARAM wParam, LPARAM lParam);
public:
   afx_msg void OnClose();
   BOOL OnDeviceChange(UINT EventType, DWORD_PTR dwData);
   virtual void Serialize(CArchive& ar);
   afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
   afx_msg void OnUpdateFileNew(CCmdUI *pCmdUI);
   afx_msg void OnUpdateFileOpen(CCmdUI *pCmdUI);
   afx_msg void SetParam(VOID * param);
   afx_msg LRESULT OnCmLog(WPARAM wParam, LPARAM lParam);

};
