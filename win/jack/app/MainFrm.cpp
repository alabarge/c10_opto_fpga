
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "Dbt.h"
#include "cm_const.h"
#include "comapi.h"
#include "optoapi.h"
#include "fifoapi.h"
#include "cmapi.h"
#include "build.h"
#include "Path.h"
#include "Mmsystem.h"

using namespace nsPath;

// Device Class GUIDs used for Device Change Notifications
static const GUID devIFGuid[] = {
   // FTDI_VCP_& FTDI_D2XX_Device Class GUIDs
   { 0xa5dcbf10, 0x6530, 0x11d2,  { 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed } },
   { 0x53f56307, 0xb6bf, 0x11d0,  { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
   { 0x4d1e55b2, 0xf16f, 0x11Cf,  { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
   { 0xad498944, 0x762f, 0x11d0,  { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } },
   { 0x219d0508, 0x57a8, 0x4ff5,  { 0x97, 0xa1, 0xbd, 0x86, 0x58, 0x7c, 0x6c, 0x7e } },
   { 0x86e0d1e0L, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },
   // EZ-USB FX2 Device Class GUID
   { 0x7494c071, 0xfabb, 0x408e, { 0x8a, 0xaa, 0xf2, 0x59, 0x8e, 0x28, 0x0a, 0xbd } },
   // EZ-USB FX3 Device Class GUID
   { 0xae18aa60, 0x7f6a, 0x11d4, { 0x97, 0xdd, 0x00, 0x01, 0x02, 0x29, 0xb9, 0x59 } },
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CM Traffic Log Callback
static   void  cm_traffic_log(uint8_t op_code, pcm_msg_t msg);

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
   ON_WM_CREATE()
   ON_WM_GETMINMAXINFO()
   ON_WM_TIMER()
   ON_WM_CLOSE()
   ON_WM_DEVICECHANGE()
   ON_COMMAND(ID_PANE_PROJECT, &CMainFrame::OnViewProjectWnd)
   ON_UPDATE_COMMAND_UI(ID_PANE_PROJECT, &CMainFrame::OnUpdateViewProjectWnd)
   ON_COMMAND(ID_PANE_GRAPH, &CMainFrame::OnViewGraphWnd)
   ON_UPDATE_COMMAND_UI(ID_PANE_GRAPH, &CMainFrame::OnUpdateViewGraphWnd)
   ON_COMMAND(ID_PANE_IMAGE, &CMainFrame::OnViewImageWnd)
   ON_COMMAND(ID_PANE_OUTPUT, &CMainFrame::OnViewOutputWnd)
   ON_UPDATE_COMMAND_UI(ID_PANE_OUTPUT, &CMainFrame::OnUpdateViewOutputWnd)
   ON_UPDATE_COMMAND_UI(ID_VIEW_CUSTOMIZE, &CMainFrame::OnUpdateCustomize)
   ON_UPDATE_COMMAND_UI(ID_IND_COM, &CMainFrame::OnUpdateIndCom)
   ON_UPDATE_COMMAND_UI(ID_IND_CONNECT, &CMainFrame::OnUpdateIndConnect)
   ON_MESSAGE(WM_CHANGE_VIEW, &CMainFrame::OnChangeView)
   ON_MESSAGE(WM_PLOT_MSG, &CMainFrame::OnPlot)
   ON_MESSAGE(WM_LOG_MSG, &CMainFrame::OnLogMsg)
   ON_MESSAGE(WM_TRAFFIC_MSG, &CMainFrame::OnTrafficMsg)
   ON_MESSAGE(WM_DEBUG_MSG, &CMainFrame::OnDebugMsg)
   ON_MESSAGE(WM_LOG_CLEAR, &CMainFrame::OnLogClear)
   ON_MESSAGE(WM_CM_TRAFFIC, &CMainFrame::OnCmTraffic)
   ON_MESSAGE(WM_CM_LOG, &CMainFrame::OnCmLog)
   ON_MESSAGE(WM_DROP_MESSAGE, &CMainFrame::OnDropMsg)
   ON_MESSAGE(WM_EXPAND_ALL, &CMainFrame::OnExpandAll)
   ON_MESSAGE(WM_COLLAPSE_ALL, &CMainFrame::OnCollapseAll)
   ON_MESSAGE(WM_STATUS_BAR, &CMainFrame::OnStatusBar)
   ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CMainFrame::OnUpdateFileSave)
   ON_UPDATE_COMMAND_UI(ID_FILE_NEW, &CMainFrame::OnUpdateFileNew)
   ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, &CMainFrame::OnUpdateFileOpen)
END_MESSAGE_MAP()

// Status Line Partitions
static UINT indicators[] = {
   ID_SEPARATOR,
   ID_IND_COUNT,
   ID_IND_TIMER,
   ID_IND_COM,
   ID_IND_CONNECT
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
   //
   // List of Views to Dynamically Create
   //
   //    Steps for Adding a FormView :
   //
   //    1. Create new Class derived from CViewEx.
   //    2. Create Dialog Resource, placing controls.
   //    3. Add OnInitDialog override to the new class and define AddAnchor()
   //       for selected controls, including Statics.
   //    4. Add persistent data to the INITYPE data structure in eegType.h.
   //    5. Add the appropriate entries in iniConst[] in appDoc.cpp.
   //    6. Create a string resource with the same ID as the Dialog for the Tree Item label.
   //    7. Create another string resource with ID+1 for the Status Bar message for this dialog.
   //    8. Initialize the View's Pointer to NULL.
   //
   //    NOTE: Insertion order must be Parent/Child to ensure the parent exists prior to the child.
   //          All Views are dynamically created in the order they are selected by the user,
   //          if a view must exist at startup then add a call to OnCreateView(ID) in the one time
   //          initialization section of OnTimer().
   //    
   //          For MainFrame or Docking Pane persistent data, follow the pattern in MainFrame's
   //          OnNewDoc() and Serialize() functions; the new persistent data must also be defined
   //          in the INITYPE data structure in eegType.h. Also, when adding dockable panes, be sure to 
   //          place a call to the new pane from the OnNewDoc() and Serialize() functions.
   //
   PAGEENTRY pageConst[] = {
      //
      //
      // Dialog ID      View Pointer      Parent         Tree Item         Runtime Class                 Flags
      // =========      ============      ======         =========         =============                 ======
      //
      {IDD_CONNECT,     &m_pConView,      NULL,          &m_hConView,      RUNTIME_CLASS(CConView),      0x0000},
      {IDD_CP,          &m_pCpView,       NULL,          &m_hCpView,       RUNTIME_CLASS(CCpView),       0x0001},
      {IDD_CP_MEM,      &m_pCpMemView,    &m_hCpView,    &m_hCpMemView,    RUNTIME_CLASS(CCpMemView),    0x0001},
      {IDD_DAQ,         &m_pDAQView,      NULL,          &m_hDAQView,      RUNTIME_CLASS(CDAQView),      0x0001},
   };

   // Allocate space for View Table
   m_pPage = (PPAGEENTRY) new BYTE [sizeof(pageConst)];

   // Number of Views
   m_pageLen = DIM(pageConst);

   // Initialize the View Table
   for (UINT i=0;i<m_pageLen;i++) {
      m_pPage[i] = pageConst[i];
   }

   // Create I/O Mutex for callback thread
   InitializeCriticalSection(&m_hTrafficMutex);

   m_bOnce = FALSE;
   m_bLogTraffic = FALSE;
   m_bModifiedFlag = FALSE;
}

CMainFrame::~CMainFrame()
{
   // Delete the View Table
   if (m_pPage != NULL) delete m_pPage;

   // Delete Tabbed Pane
   if (m_pTabbedBar) delete m_pTabbedBar;

}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   CString  str;
   UINT     i;
   BOOL     bNameValid;

   // Initialize CM
   cm_init(cm_traffic_log);

   // Reset Hi-Res Timer
   m_last_us = 0;
   m_timer.Start();

   if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
      return -1;

   if (!GetMenuBar().Create(this))
   {
      TRACE0("Failed to create menubar\n");
      return -1;      // fail to create
   }

   GetMenuBar().SetPaneStyle(GetMenuBar().GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

   // prevent the menu bar from taking the focus on activation
   CMFCPopupMenu::SetForceMenuFocus(FALSE);

   if (!GetToolBar().CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
      !GetToolBar().LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
   {
      TRACE0("Failed to create toolbar\n");
      return -1;      // fail to create
   }

   CString strToolBarName;
   bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
   ASSERT(bNameValid);
   GetToolBar().SetWindowText(strToolBarName);

   CString strCustomize;
   bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
   ASSERT(bNameValid);
   GetToolBar().EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

   if (!GetStatusBar().Create(this))
   {
      TRACE0("Failed to create status bar\n");
      return -1;      // fail to create
   }

   bNameValid = str.LoadString(IDS_IND_COUNT);
   ASSERT(bNameValid);
   GetStatusBar().SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
   GetStatusBar().SetPaneWidth(1, 130);
   GetStatusBar().SetPaneStyle(1, SBT_NOBORDERS);
   GetStatusBar().SetTipText(1, str);

   bNameValid = str.LoadString(IDS_IND_TIMER);
   ASSERT(bNameValid);
   GetStatusBar().SetPaneWidth(2, 50);
   GetStatusBar().SetPaneStyle(2, SBT_NOBORDERS);
   GetStatusBar().SetTipText(2, str);

   bNameValid = str.LoadString(IDS_IND_COM);
   ASSERT(bNameValid);
   GetStatusBar().SetPaneWidth(3, 50);
   GetStatusBar().SetPaneStyle(3, SBT_NOBORDERS);
   GetStatusBar().SetTipText(3, str);

   bNameValid = str.LoadString(IDS_IND_CONNECT);
   ASSERT(bNameValid);
   GetStatusBar().SetPaneWidth(4, 0);
   GetStatusBar().SetPaneStyle(4, SBT_NOBORDERS);
   m_hBmpBlu = (HBITMAP)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BIGBLU),
                   IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
   m_hBmpRed = (HBITMAP)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BIGRED),
                   IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
   m_hBmpGrn = (HBITMAP)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BIGGRN),
                   IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
   m_hBmpAct = (HBITMAP)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BIGACT),
                   IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
   GetStatusBar().SetPaneIcon(4, m_hBmpBlu, APP_DEF_TRANS);
   GetStatusBar().SetTipText(4, str);

   // Allow Status pane to shrink
   UINT nID, nStyle;
   int  cxWidth;
   GetStatusBar().GetPaneInfo(0, nID, nStyle, cxWidth);
   GetStatusBar().SetPaneInfo(0, nID, nStyle, 1);

   // Clear all panes of Text
   m_str_status.Format(_T(""));
   WriteSBar(1);
   WriteSBar(2);
   WriteSBar(3);
   WriteSBar(4);
   
   // Enable Docking
   GetMenuBar().EnableDocking(CBRS_ALIGN_ANY);
   GetToolBar().EnableDocking(CBRS_ALIGN_ANY);
   EnableDocking(CBRS_ALIGN_ANY);
   DockPane(&GetMenuBar());
   DockPane(&GetToolBar());
   GetToolBar().ShowPane(FALSE, FALSE, FALSE);

   // enable Visual Studio 2005 style docking window behavior
   CDockingManager::SetDockingMode(DT_SMART);
   // enable Visual Studio 2005 style docking window auto-hide behavior
   EnableAutoHidePanes(CBRS_ALIGN_ANY);

   // Load menu item image (not placed on any standard toolbars):
   CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

   // Init View Pointers
   m_nView        = IDD_CONNECT;
   m_pConView     = NULL;
   m_pCpView      = NULL;
   m_pCpMemView   = NULL;
   m_pDAQView     = NULL;

   // create docking windows
   if (!CreateDockingWindows())
   {
      TRACE0("Failed to create docking windows\n");
      return -1;
   }

   // Insert ProjectWnd Page Items and Create map entry
   m_pageMap.RemoveAll();
   m_pageMap.InitHashTable(m_pageLen + (m_pageLen / 4));

   for (i=0;i<m_pageLen;i++) {
      GetProjectWnd().AddPage(m_pPage[i]);
      m_pageMap[m_pPage[i].id] = &m_pPage[i];
   }

   // Expand to accomodate item labels
   GetProjectWnd().AdjustLayout();

   // set the visual manager used to draw all user interface elements
   CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));

   // Enable toolbar and docking window menu replacement
   EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

   // Prevent MFC from disabling menu/toolbar items
   m_bAutoMenuEnable = FALSE;

   // Interval Timer Pane
   m_nShowTime = MF_TIMER_SYS;
   m_nSysTime  = 0;
   m_nIntTime  = 0;
   m_nSysTimeMS = 0;
   m_nIntTimeMS = 0;
   SetTimer(ID_TIMER_ONE_SEC, 250, NULL);

   // Init App Data
   m_nTrace   = APP_TRACE_DEFAULT;
   m_nError   = 0;
   m_nCom     = APP_CON_NONE;
   m_nPort    = APP_PORT_NONE;
   m_nPortId  = 0;
   m_hCom     = NULL;
   m_nComErr  = 0;
   m_nComAct  = 0;
   m_pMIFftdi = NULL;
   m_nLmcSpi  = -1;
   memset(m_chSerial, 0, 16);

   // Register for Device Change Notifications
   DEV_BROADCAST_DEVICEINTERFACE dbch = { 0 };
   dbch.dbcc_size = sizeof(dbch);
   dbch.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
   for (i=0;i<sizeof(devIFGuid);i++) {
      dbch.dbcc_classguid = devIFGuid[i];
      dbch.dbcc_name[0] = '\0';
      m_hDevNotify = RegisterDeviceNotification(GetSafeHwnd(), &dbch, DEVICE_NOTIFY_WINDOW_HANDLE);
   }

   return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
   cs.style &= ~(LONG)FWS_ADDTOTITLE;
   return CFrameWndEx::PreCreateWindow(cs);
}

BOOL CMainFrame::CreateDockingWindows()
{
   BOOL bNameValid;
   CRect rect;

   // Create project view
   CString strProjectWnd;
   bNameValid = strProjectWnd.LoadString(IDS_PANE_PROJECT);
   ASSERT(bNameValid);
   if (!GetProjectWnd().Create(strProjectWnd, this, CRect(0, 0, 175, 250), TRUE, ID_PANE_PROJECT, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI))
   {
      TRACE0("Failed to create File View window\n");
      return FALSE; // failed to create
   }
   GetProjectWnd().SetMinSize(CSize(175,250));

   // Create output window
   CString strOutputWnd;
   bNameValid = strOutputWnd.LoadString(IDS_PANE_OUTPUT);
   ASSERT(bNameValid);
   if (!GetOutputWnd().Create(strOutputWnd, this, CRect(0, 0, 120, 150), TRUE, ID_PANE_OUTPUT, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
   {
      TRACE0("Failed to create Output window\n");
      return FALSE; // failed to create
   }
   GetOutputWnd().SetMinSize(CSize(120,150));

   // Create graphing window
   CString strGraphWnd;
   bNameValid = strGraphWnd.LoadString(IDS_PANE_GRAPH);
   ASSERT(bNameValid);
   if (!GetGraphWnd().Create(strGraphWnd, this, CRect(0, 0, 200, 200), TRUE, ID_PANE_GRAPH, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
   {
      TRACE0("Failed to create Graph window\n");
      return FALSE; // failed to create
   }
   GetGraphWnd().SetMinSize(CSize(APP_MIN_SIZE_PANE_X,APP_MIN_SIZE_PANE_Y));

   // Create imaging window
   CString strImageWnd;
   bNameValid = strImageWnd.LoadString(IDS_PANE_IMAGE);
   ASSERT(bNameValid);
   if (!GetImageWnd().Create(strImageWnd, this, CRect(0, 0, 200, 200), TRUE, ID_PANE_IMAGE, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
   {
      TRACE0("Failed to create Image window\n");
      return FALSE; // failed to create
   }
   GetImageWnd().SetMinSize(CSize(APP_MIN_SIZE_PANE_X,APP_MIN_SIZE_PANE_Y));

   // Create Parameters window
   CString strParmsWnd;
   bNameValid = strParmsWnd.LoadString(IDS_PANE_PARMS);
   ASSERT(bNameValid);
   if (!GetParmsWnd().Create(strParmsWnd, this, CRect(0, 0, 175, 250), TRUE, IDS_PANE_PARMS, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
   {
      TRACE0("Failed to create Parameter window\n");
      return FALSE; // failed to create
   }
   GetParmsWnd().SetMinSize(CSize(175,250));

   if (!m_pTabbedBar.Create (_T(""), this, CRect (0, 0, 200, 200), TRUE, (UINT) -1, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
   {
       TRACE0("Failed to create Tabbed Pane Bar\n");
       return FALSE; // fail to create
   }
   m_pTabbedBar.SetMinSize(CSize(APP_MIN_SIZE_PANE_X, APP_MIN_SIZE_PANE_Y));

   GetProjectWnd().EnableDocking(CBRS_ALIGN_ANY);
   GetParmsWnd().EnableDocking(CBRS_ALIGN_ANY);
   GetOutputWnd().EnableDocking(CBRS_ALIGN_ANY);
   GetGraphWnd().EnableDocking(CBRS_ALIGN_ANY);
   GetImageWnd().EnableDocking(CBRS_ALIGN_ANY);

   // Dock All Panes
   //DockPane(&GetOutputWnd());

   DockPane(&GetProjectWnd());
	CDockablePane* pTab0 = NULL;
	GetParmsWnd().AttachToTabWnd(&GetProjectWnd(), DM_SHOW, FALSE, &pTab0);
   pTab0->SetMinSize(CSize(175, 250));
   pTab0->AdjustDockingLayout();

   DockPane(&GetOutputWnd());

   // Hide Project and Output Panes
   //GetProjectWnd().ShowPane(FALSE, FALSE, FALSE);
   //GetOutputWnd().ShowPane(FALSE, FALSE, FALSE);

   GetWindowRect(&rect);
   rect.top = rect.bottom + 2;
   rect.bottom = rect.top + (APP_MIN_SIZE_Y / 2);
   GetGraphWnd().FloatPane(rect, DM_RECT, false);
   GetImageWnd().FloatPane(rect, DM_RECT, false);
   m_pTabbedBar.EnableDocking(CBRS_ALIGN_ANY);
   CDockablePane* pTab1 = DYNAMIC_DOWNCAST(CDockablePane, &m_pTabbedBar);
   pTab1->FloatPane(rect, DM_RECT, false);
   m_pTabbedBar.EnableTabAutoColor(TRUE);

   // Attach Panes as Tabs
   GetGraphWnd().AttachToTabWnd(&m_pTabbedBar, DM_SHOW, TRUE);
   GetImageWnd().AttachToTabWnd(&m_pTabbedBar, DM_SHOW, TRUE);

   m_pTabbedBar.ShowPane(FALSE, TRUE, FALSE);

   return TRUE;
}

VOID CMainFrame::ReportStartup()
{
   CString     str, str1;
   CTime       t = CTime::GetCurrentTime();
   SYSTEM_INFO siSysInfo;
   CStdioFile  pParm;

   // Report Application EXE and CWD
   str.Format(_T("%s, started %s\n"), m_exe, t.Format(_T("%#c")));
   Log(str);
   str.Format(_T("Version %s\n"), _T(BUILD_HI));
   Log(str);

   // Report Company
   str1.LoadString(IDS_COMPANY);
   str.Format(_T("Copyright(c) 2009-2016, %s, All rights reserved\n"), str1);
   Log(str);

   // Report Command Line
   str.Format(_T("%s"), theApp.m_lpCmdLine);
   if (!str.IsEmpty()) {
      str.Format(_T("Command Line : %s\n"), theApp.m_lpCmdLine);
      Log(str);
   }

   str.Format(_T("Git Head   \t: %s\n"), _T(BUILD_GIT_REV));
   Log(str);
   str.Format(_T("Git Author \t: %s\n"), _T(BUILD_GIT_AUTH));
   Log(str);
   str.Format(_T("Git Email  \t: %s\n"), _T(BUILD_GIT_EMAIL));
   Log(str);
   str.Format(_T("Git Date   \t: %s\n"), _T(BUILD_GIT_DATE));
   Log(str);

   // Report Hardware System ID and Timestamp
   str.Format(_T("Associated Firmware System ID : %d, Timestamp : %d\n"), FPGA_PID, FPGA_EPOCH);
   Log(str);
   str.Format(_T("System Name : %s, Device Family : %s\n"), _T(ALT_SYSTEM_NAME), _T(ALT_DEVICE_FAMILY));
   Log(str);
   str.Format(_T("Hardware Baudrate : %dbps\n"), CFG_BAUD_RATE);
   Log(str);

   #ifdef _DEBUG
   // Report Debug Build Warning
   str.Format(_T("Warning : Running a DEBUG Build\n"));
   Log(str,APP_MSG_WARNING);
   #endif

   // Report Performance Counter Frequency
   ::QueryPerformanceFrequency(&m_pcFreq);
   str.Format(_T("Performance Counter Frequency : %lld Hz\n"), m_pcFreq.QuadPart);
   Log(str);

   // Report Computer Hardware Info, number of processors etc.
   GetSystemInfo(&siSysInfo);
   str.Format(_T("OEM ID: %u\n"), siSysInfo.dwOemId); Log(str);
   str.Format(_T("Number of Processors: %u\n"), siSysInfo.dwNumberOfProcessors); Log(str);
   str.Format(_T("Page Size: %u\n"), siSysInfo.dwPageSize); Log(str);
   str.Format(_T("Processor Type: %u\n"), siSysInfo.dwProcessorType); Log(str);
   str.Format(_T("Active processor mask: 0x%lX\n"), siSysInfo.dwActiveProcessorMask); Log(str);

}

void CMainFrame::OnNewDoc(PINITYPE ini)
{
   CString     parms, parm1;
   CStdioFile  pParm;
   TCHAR       szPath[MAX_PATH], szDrive[MAX_PATH], szDir[MAX_PATH];

   // Execution Directory
   GetCurrentDirectory(MAX_PATH, szPath);
   m_cwd.Format(_T("%s"), szPath);
   GetModuleFileName(NULL, szPath, MAX_PATH); 
   m_exe.Format(_T("%s"), szPath);
   _wsplitpath_s(szPath, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, NULL, 0, NULL, 0);
   swprintf_s(m_exePath, MAX_PATH, _T("%s%s"), szDrive, szDir);

   if (ini != NULL) {
      GetOutputWnd().GetLog().m_bStamp       = ini->apLogStamp;
      GetOutputWnd().GetLog().m_bEnable      = ini->apLogEnable;
      GetOutputWnd().GetTraffic().m_bStamp   = ini->apTrafficStamp;
      GetOutputWnd().GetTraffic().m_bEnable  = ini->apTrafficEnable;
      GetOutputWnd().GetTraffic().m_bLocal   = ini->apTrafficLocal;
      GetOutputWnd().GetTraffic().m_bPipe    = ini->apTrafficPipe;
      GetOutputWnd().GetDebug().m_bStamp     = ini->apDebugStamp;
      GetOutputWnd().GetDebug().m_bEnable    = ini->apDebugEnable;
      m_lastTree                             = ini->apLastTree;
      //
      // Init the Dockable Panes
      //
      GetGraphWnd().OnNewDoc(ini);
      GetProjectWnd().OnNewDoc(ini);
      GetParmsWnd().OnNewDoc(ini);
      GetImageWnd().OnNewDoc(ini);
   }
}

void CMainFrame::Serialize(CArchive& ar)
{
   // We use the CArchive document because the
   // GetActiveDocument() will not be valid when
   // first reading the document from disk.
   CappDoc  *pDoc = (CappDoc *)ar.m_pDocument;

   //
   // Serialize the MainFrame data to storage
   //
   if (ar.IsStoring()) {
      pDoc->m_ini->apLogStamp        = GetOutputWnd().GetLog().m_bStamp;
      pDoc->m_ini->apLogEnable       = GetOutputWnd().GetLog().m_bEnable;
      pDoc->m_ini->apTrafficStamp    = GetOutputWnd().GetTraffic().m_bStamp;
      pDoc->m_ini->apTrafficEnable   = GetOutputWnd().GetTraffic().m_bEnable;
      pDoc->m_ini->apTrafficLocal    = GetOutputWnd().GetTraffic().m_bLocal;
      pDoc->m_ini->apTrafficPipe     = GetOutputWnd().GetTraffic().m_bPipe;
      pDoc->m_ini->apDebugStamp      = GetOutputWnd().GetDebug().m_bStamp;
      pDoc->m_ini->apDebugEnable     = GetOutputWnd().GetDebug().m_bEnable;
      pDoc->m_ini->apLastTree        = m_lastTree;
   }
   //
   // De-serialize the MainFrame data from storage
   //
   else {
      GetOutputWnd().GetLog().m_bStamp       = pDoc->m_ini->apLogStamp;
      GetOutputWnd().GetLog().m_bEnable      = pDoc->m_ini->apLogEnable;
      GetOutputWnd().GetTraffic().m_bStamp   = pDoc->m_ini->apTrafficStamp;
      GetOutputWnd().GetTraffic().m_bEnable  = pDoc->m_ini->apTrafficEnable;
      GetOutputWnd().GetTraffic().m_bLocal   = pDoc->m_ini->apTrafficLocal;
      GetOutputWnd().GetTraffic().m_bPipe    = pDoc->m_ini->apTrafficPipe;
      GetOutputWnd().GetDebug().m_bStamp     = pDoc->m_ini->apDebugStamp;
      GetOutputWnd().GetDebug().m_bEnable    = pDoc->m_ini->apDebugEnable;
      m_lastTree                             = pDoc->m_ini->apLastTree;
      m_bLogTraffic                          = pDoc->m_ini->apTrafficEnable;
   }

   //
   // Serialize the Dockable Panes
   //
   GetGraphWnd().Serialize(ar);
   GetProjectWnd().Serialize(ar);
   GetParmsWnd().Serialize(ar);
   GetImageWnd().Serialize(ar);
}

void CMainFrame::WriteSBar(UINT pane, BOOL show)
{
   if (show == TRUE) GetStatusBar().SetPaneText(pane, m_str_status);
}

void CMainFrame::Log(CString str, COLORREF color)
{
   GetOutputWnd().LogStr(str, color);
}

void CMainFrame::Traffic(CString str, COLORREF color)
{
   GetOutputWnd().TrafficStr(str, color);
}

void CMainFrame::Debug(CString str, COLORREF color)
{
   GetOutputWnd().DebugStr(str, color);
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
   CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
   CFrameWndEx::Dump(dc);
}
#endif //_DEBUG

//
// CMainFrame Message Handlers
//

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
   CString  str, parms;
   bool     bRegExist = theApp.m_bRegExist;

   if (nIDEvent == ID_TIMER_ONE_SEC) {
      if (m_nSysTimeMS == 0) m_nSysTimeMS = timeGetTime();
      if (m_nIntTimeMS == 0) m_nIntTimeMS = timeGetTime();
      m_nSysTime = (timeGetTime() - m_nSysTimeMS) / 1000;
      m_nIntTime = (timeGetTime() - m_nIntTimeMS) / 1000;
      //m_nSysTime++;
      //m_nIntTime++;
      // One-Time  Init
      if (m_nSysTime == 1) GetProjectWnd().SelectPage(m_pPage[0]);
      
      if (m_nShowTime == MF_TIMER_SYS) {
         if (m_nSysTime > 3600) 
            str.Format(_T("\t%2d:%02d:%02d"), (m_nSysTime/3600), (m_nSysTime/60) % 60, m_nSysTime%60);
         else
            str.Format(_T("\t%02d:%02d"), m_nSysTime/60, m_nSysTime%60);
      }
      else if (m_nShowTime == MF_TIMER_INT) {
         if (m_nIntTime > 3600) 
            str.Format(_T("\t%2d:%02d:%02d"), (m_nIntTime/3600), (m_nIntTime/60) % 60, m_nIntTime%60);
         else
            str.Format(_T("\t%02d:%02d"), m_nIntTime/60, m_nIntTime%60);
      }
      else {
         str.Format(_T(""));
      }
      // Update Interval Timer Pane
      GetStatusBar().SetPaneText(2, str);
      // Report Errors to Log
      if (m_nError != 0) OnError(m_nError);
      // One Time Initialization after Document is Available
      // Parameters could be loaded here!!!
      if (m_bOnce == FALSE) {
         CappDoc *pDoc = (CappDoc *)GetActiveDocument();
         if (pDoc != NULL) {
            m_bOnce = TRUE;
            // Set modified flag if indicated
            if (m_bModifiedFlag == TRUE) CappDoc::GetDoc()->SetModifiedFlag(TRUE);
            // Cycle through all tree items for expansion state restore
            for (UINT i=0;i<m_pageLen;i++) {
               if (*m_pPage[i].hItem != NULL && m_lastTree & (1 << i)) {
                  GetProjectWnd().Expand(*m_pPage[i].hItem, TVE_EXPAND);
               }
            }
            // Report App Detail
            ReportStartup();
            // CM Traffic Log
            if (pDoc->m_ini->apLogCMTraffic) {
               cm_log(NULL, CM_LOG_ENABLE);
               str.Format(_T("Warning : CM Traffic Log is Enabled\n"));
               Log(str,APP_MSG_WARNING);
            }
            // Initialize GUI Parameter List
            GetParmsWnd().InitParamList();
            // Cycle through all tree items for startup flags
            for (UINT i=0;i<m_pageLen;i++) {
               if (*m_pPage[i].hItem != NULL && (m_pPage[i].flags & 1)) {
                  OnCreateView(m_pPage[i].id);
               }
            }
         }
      }
   }

   CFrameWndEx::OnTimer(nIDEvent);
}

void CMainFrame::OnClose()
{
   TVITEM   tvi = {0};
   UINT     state = 0;

   CappDoc *pDoc = (CappDoc *)GetActiveDocument();

   // Remove Device Change Registration
   if (m_hDevNotify) {
      UnregisterDeviceNotification(m_hDevNotify);
      m_hDevNotify = NULL;
   }

   // Cycle through all tree items for expansion state capture
   for (UINT i=0;i<m_pageLen;i++) {
      if (*m_pPage[i].hItem != NULL) {
         tvi.hItem = *m_pPage[i].hItem;
         tvi.mask = TVIF_STATE;
         tvi.stateMask = TVIS_EXPANDED;
         GetProjectWnd().GetItem(&tvi);
         if (tvi.state & TVIS_EXPANDED) {
            state |= (1 << i);
         }
      }
   }
   if (m_lastTree != state) {
      pDoc->SetModifiedFlag(TRUE);
      m_lastTree = state;
   }

   // Notify each View we're closing
   // Special case for the default view
   if (*m_pPage[0].pView == NULL) {
      *m_pPage[0].pView = GetActiveView();
   }
   // Cycle through all active views
   for (UINT i=0;i<m_pageLen;i++) {
      if (*m_pPage[i].pView != NULL) {
         (*m_pPage[i].pView)->SendMessage(WM_CLOSING, 0, 0);
      }
   }

   cm_final();

   CFrameWndEx::OnClose();
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // Current Screen Size
    int cx = ::GetSystemMetrics(SM_CXSCREEN);
    int cy = ::GetSystemMetrics(SM_CYSCREEN);

    // Set the maximum size. Used while maximizing.
    lpMMI->ptMaxSize.x = cx;
    lpMMI->ptMaxSize.y = cy;

    // Set the Minimum Track Size. Used while resizing.
    lpMMI->ptMinTrackSize.x = APP_MIN_SIZE_X;
    lpMMI->ptMinTrackSize.y = APP_MIN_SIZE_Y;

    // Set the Maximum Track Size. Used while resizing.
    lpMMI->ptMaxTrackSize.x = cx;
    lpMMI->ptMaxTrackSize.y = cy;

   CFrameWndEx::OnGetMinMaxInfo(lpMMI);
}

void CMainFrame::OnViewProjectWnd()
{
   if (GetProjectWnd().IsWindowVisible()) {
      GetProjectWnd().ShowPane(FALSE, FALSE, FALSE);
   }
   else {
      GetProjectWnd().ShowPane(TRUE, FALSE, TRUE);
      GetProjectWnd().SetFocus();
   }
}

void CMainFrame::OnUpdateViewProjectWnd(CCmdUI* pCmdUI)
{
   pCmdUI->SetCheck(GetProjectWnd().IsWindowVisible());
   pCmdUI->Enable(TRUE);
}

void CMainFrame::OnViewGraphWnd()
{
   if (GetGraphWnd().IsWindowVisible()) {
      GetGraphWnd().ShowPane(FALSE, FALSE, FALSE);
   }
   else {
      GetGraphWnd().ShowPane(TRUE, FALSE, TRUE);
      GetGraphWnd().SetFocus();
   }
}

void CMainFrame::OnUpdateViewGraphWnd(CCmdUI* pCmdUI)
{
   pCmdUI->SetCheck(GetGraphWnd().IsWindowVisible());
   pCmdUI->Enable(TRUE);
}

void CMainFrame::OnViewImageWnd()
{
   if (GetImageWnd().IsWindowVisible()) {
      GetImageWnd().ShowPane(FALSE, FALSE, FALSE);
   }
   else {
      GetImageWnd().ShowPane(TRUE, FALSE, TRUE);
      GetImageWnd().SetFocus();
   }
}

void CMainFrame::OnViewOutputWnd()
{
   if (GetOutputWnd().IsWindowVisible()) {
      GetOutputWnd().ShowPane(FALSE, FALSE, FALSE);
   }
   else {
      GetOutputWnd().ShowPane(TRUE, FALSE, TRUE);
      GetOutputWnd().SetFocus();
   }
}

void CMainFrame::OnUpdateFileSave(CCmdUI *pCmdUI)
{
   CappDoc *pDoc = (CappDoc *)GetActiveDocument();
   if (pDoc != NULL) {
      pCmdUI->Enable(pDoc->IsModified());
   }
}

void CMainFrame::OnUpdateFileNew(CCmdUI *pCmdUI)
{
   pCmdUI->Enable(m_nCom == APP_CON_NONE);
}

void CMainFrame::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
   pCmdUI->Enable(m_nCom == APP_CON_NONE);
}

void CMainFrame::OnUpdateViewOutputWnd(CCmdUI* pCmdUI)
{
   pCmdUI->SetCheck(GetOutputWnd().IsWindowVisible());
   pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateIndCom(CCmdUI *pCmdUI)
{
   CString  str;
   switch (m_nCom) {
   case APP_CON_NONE :
      pCmdUI->SetText(_T("NONE"));
      break;
   case APP_CON_OPTO :
      str.Format(_T("OPTO.%d"), m_nPortId);
      pCmdUI->SetText(str);
      break;
   case APP_CON_FIFO :
      str.Format(_T("FIFO.%d"), m_nPortId);
      pCmdUI->SetText(str);
      break;
   case APP_CON_COM :
      str.Format(_T("COM.%d"), m_nPortId);
      pCmdUI->SetText(str);
      break;
   case APP_CON_LAN :
      str.Format(_T("LAN.%d"), m_nPortId);
      pCmdUI->SetText(str);
      break;
   case APP_CON_UDP :
      str.Format(_T("UDP.%d"), m_nPortId);
      pCmdUI->SetText(str);
      break;
   case APP_CON_TCP :
      str.Format(_T("TCP.%d"), m_nPortId);
      pCmdUI->SetText(str);
      break;
   default :
      pCmdUI->SetText(_T("NONE"));
      break;
   }
}

void CMainFrame::OnUpdateIndConnect(CCmdUI *pCmdUI)
{
   if (m_nCom == APP_CON_NONE) {
      GetStatusBar().SetPaneIcon(4, m_hBmpBlu, APP_DEF_TRANS);
   }
   else if (m_nComErr != 0) {
      GetStatusBar().SetPaneIcon(4, m_hBmpRed, APP_DEF_TRANS);
   }
   else if (m_nComAct == 1) {
      GetStatusBar().SetPaneIcon(4, m_hBmpAct, APP_DEF_TRANS);
   }
   else {
      GetStatusBar().SetPaneIcon(4, m_hBmpGrn, APP_DEF_TRANS);
   }
}

void CMainFrame::OnError(UINT &nError)
{
}

afx_msg LRESULT CMainFrame::OnChangeView(WPARAM wp, LPARAM lp)
{
   PPAGEENTRY  pPage = { 0 };

   // Capture Initial View, this prevents re-creation
   if (*m_pPage[0].pView == NULL) {
      *m_pPage[0].pView = GetActiveView();
   }

   // Load View Description
   m_str_status.LoadString((UINT)wp + 1);

   // Check Current View
   if (m_nView == wp) {
      WriteSBar(MF_SBAR_STATUS);
      return 0;
   }

   // Create View when first selected
   if (m_pageMap.Lookup((WORD)wp, (void *&)pPage)) {
      if (*pPage->pView == NULL) {
         CCreateContext ctx;
         ctx.m_pCurrentDoc = NULL;
         ctx.m_pCurrentFrame = NULL;
         ctx.m_pLastView = NULL;
         ctx.m_pNewDocTemplate = NULL;
         ctx.m_pNewViewClass = pPage->pRTClass;
         *pPage->pView = static_cast<CView*>(CreateView(&ctx));
         CappDoc::GetDoc()->AddView(*pPage->pView);
         (*pPage->pView)->OnInitialUpdate();
      }
   }

   // View was Created
   if (*pPage->pView != NULL) {
      CappDoc::GetDoc()->SwitchToView(*pPage->pView);
      m_nView = (WORD)wp;
      WriteSBar(MF_SBAR_STATUS);
   }

   return 0;
}

void CMainFrame::OnSelectView(UINT itemID)
{
   PPAGEENTRY     pPage = { 0 };
   // Select the View based on Dialog ID
   if (m_pageMap.Lookup((WORD)itemID, (void *&)pPage)) {
      if (*pPage->pView != NULL) {
         GetProjectWnd().SelectPage(*pPage);
      }
   }
}

void CMainFrame::OnCreateView(UINT itemID)
{
   PPAGEENTRY  pPage = { 0 };
   if (m_pageMap.Lookup((WORD)itemID, (void *&)pPage)) {
      if (*pPage->pView == NULL) {
         CCreateContext ctx;
         ctx.m_pCurrentDoc = NULL;
         ctx.m_pCurrentFrame = NULL;
         ctx.m_pLastView = NULL;
         ctx.m_pNewDocTemplate = NULL;
         ctx.m_pNewViewClass = pPage->pRTClass;
         *pPage->pView = static_cast<CView*>(CreateView(&ctx));
         CappDoc::GetDoc()->AddView(*pPage->pView);
         (*pPage->pView)->OnInitialUpdate();
         ::SetWindowLong((*pPage->pView)->m_hWnd, GWL_ID, 0);
      }
   }
}

void CMainFrame::OnMessageView(UINT itemID, UINT msgID, WPARAM wParam, LPARAM lParam)
{
   PPAGEENTRY     pPage = NULL;
   // Use Dialog ID to send View a Message
   if (m_pageMap.Lookup((WORD)itemID, (void *&)pPage)) {
      if (*pPage->pView == NULL) 
         *pPage->pView = GetActiveView();
      (*pPage->pView)->SendMessage(msgID, wParam, lParam);
   }
}

afx_msg LRESULT CMainFrame::OnExpandAll(WPARAM wParam, LPARAM lParam)
{
   // Cycle through all tree items and expand
   for (UINT i=0;i<m_pageLen;i++) {
      if (*m_pPage[i].hItem != NULL) {
         GetProjectWnd().Expand(*m_pPage[i].hItem, TVE_EXPAND);
      }
   }
   return 0;
}

afx_msg LRESULT CMainFrame::OnCollapseAll(WPARAM wParam, LPARAM lParam)
{
   // Cycle through all tree items and expand
   for (UINT i=0;i<m_pageLen;i++) {
      if (*m_pPage[i].hItem != NULL) {
         GetProjectWnd().Expand(*m_pPage[i].hItem, TVE_COLLAPSE);
      }
   }
   return 0;
}

afx_msg LRESULT CMainFrame::OnPlot(WPARAM wParam, LPARAM lParam)
{
   GetGraphWnd().Plot((PPLOT)wParam);
   return 0;
}

afx_msg LRESULT CMainFrame::OnLogMsg(WPARAM wParam, LPARAM lParam)
{
   GetOutputWnd().LogStr(*(CString *)wParam, (COLORREF)lParam);
   delete (CString *)wParam;
   return 0;
}

afx_msg LRESULT CMainFrame::OnStatusBar(WPARAM wParam, LPARAM lParam)
{
   WriteSBar((UINT)wParam, (BOOL)lParam);
   return 0;
}

afx_msg LRESULT CMainFrame::OnTrafficMsg(WPARAM wParam, LPARAM lParam)
{
   GetOutputWnd().TrafficStr(*(CString *)wParam, (COLORREF)lParam);
   delete (CString *)wParam;
   return 0;
}

afx_msg LRESULT CMainFrame::OnDebugMsg(WPARAM wParam, LPARAM lParam)
{
   GetOutputWnd().DebugStr(*(CString *)wParam, (COLORREF)lParam);
   delete (CString *)wParam;
   return 0;
}

BOOL CMainFrame::OnDeviceChange(UINT EventType, DWORD_PTR dwData)
{
   CString  str, strDev;
   CString  dev, id, serial, guid;
   int      curPos = 0;

   PDEV_BROADCAST_DEVICEINTERFACE pdbci = (PDEV_BROADCAST_DEVICEINTERFACE)dwData;
   PDEV_BROADCAST_HDR             pdbch = (PDEV_BROADCAST_HDR)dwData;
   PDEV_BROADCAST_HANDLE          pdbcd = (PDEV_BROADCAST_HANDLE)dwData;

   // Device Arrival, Interfaces Only
   if (EventType == DBT_DEVICEARRIVAL && 
       pdbci->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
      strDev.Format(_T("%s"), pdbci->dbcc_name);
      // Parse the name string
      if (!strDev.IsEmpty()) {
         dev    = strDev.Tokenize(_T("#"), curPos);
         id     = strDev.Tokenize(_T("#"), curPos);
         serial = strDev.Tokenize(_T("#"), curPos);
         guid   = strDev.Tokenize(_T("#"), curPos);
      }
      // FTDI Devices
      if ((CString(_T(OPTO_VID_PID_STR)) == id) ||
          (CString(_T(OPTO_VID_PID_STR_ALT)) == id)) {
         str.Format(_T("Device Arrival  :  %s\n"), strDev);
         Log(str);
         str.Format(_T("Device FTDI ID and Serial :  %s, %s\n"), id, serial);
         Log(str);
         CW2A dev(serial);
         if ((dev.m_psz[0] == 'O' && dev.m_psz[1] == '1') ||
             (dev.m_psz[0] == 'O' && dev.m_psz[1] == '2')) {
            OnMessageView(IDD_CONNECT, WM_DEV_ARRIVE,  APP_CON_OPTO, 0);
         }
      }
   }

   // Device Removal, Interfaces Only
   if (EventType == DBT_DEVICEREMOVECOMPLETE && 
       pdbci->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
      strDev.Format(_T("%s"), pdbci->dbcc_name);
      // Parse the name string
      if (!strDev.IsEmpty()) {
         dev    = strDev.Tokenize(_T("#"), curPos);
         id     = strDev.Tokenize(_T("#"), curPos);
         serial = strDev.Tokenize(_T("#"), curPos);
         guid   = strDev.Tokenize(_T("#"), curPos);
      }
      // FTDI Devices
      if ((CString(_T(OPTO_VID_PID_STR)) == id) ||
         (CString(_T(OPTO_VID_PID_STR_ALT)) == id)) {
         str.Format(_T("Device Removal  :  %s\n"), strDev);
         Log(str);
         str.Format(_T("Device FTDI ID and Serial :  %s, %s\n"), id, serial);
         Log(str);
         // Compare Location ID
         if (serial == CString(m_chSerial)) {
            CW2A dev(serial);
            if ((dev.m_psz[0] == 'O' && dev.m_psz[1] == '1') ||
                (dev.m_psz[0] == 'O' && dev.m_psz[1] == '2')) {
                OnMessageView(IDD_CONNECT, WM_DEV_REMOVE,  APP_CON_OPTO, TRUE);
            }
         }
         else {
            CW2A dev(serial);
            if ((dev.m_psz[0] == 'O' && dev.m_psz[1] == '1') ||
                (dev.m_psz[0] == 'O' && dev.m_psz[1] == '2')) {
                OnMessageView(IDD_CONNECT, WM_DEV_REMOVE,  APP_CON_OPTO, FALSE);
            }
         }
      }
   }

   return TRUE;
}

void CMainFrame::OnUpdateCustomize(CCmdUI *pCmdUI)
{
   // Remove "Customize" from the Toolbars Menu
   if (pCmdUI->m_pMenu!=NULL)
      pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID, ID_VIEW_CUSTOMIZE);
}

afx_msg LRESULT CMainFrame::OnLogClear(WPARAM wParam, LPARAM lParam)
{
   GetOutputWnd().OnChangeActiveTab(wParam, lParam);
   return 0;
}

afx_msg LRESULT CMainFrame::OnCmTraffic(WPARAM wParam, LPARAM lParam)
{
   CString  str, str1, dir;
   UINT     i,j;

   // Check if Enabled ?
   if (!GetOutputWnd().GetTraffic().m_bEnable) return 0;

   // Check for Local ?
   if (!GetOutputWnd().GetTraffic().m_bLocal && lParam == APP_TRAFFIC_LOCAL) return 0;

   // Check for Pipe ?
   if (!GetOutputWnd().GetTraffic().m_bPipe && lParam == APP_TRAFFIC_IN_PIPE) return 0;
   if (!GetOutputWnd().GetTraffic().m_bPipe && lParam == APP_TRAFFIC_OUT_PIPE) return 0;

   CappDoc *pDoc = (CappDoc *)GetActiveDocument();
   UCHAR    *pMsg = (UCHAR *)wParam;

   if (pMsg == NULL) return 0;

   // Enter Critical Section
   EnterCriticalSection(&m_hTrafficMutex);

   COLORREF clr;
   UINT     msgLen = ((pMsg[7] & 0x0F) << 8) | pMsg[6];
   
   //
   // Traffic Direction
   //
   // Inbound
   if (lParam == APP_TRAFFIC_INBOUND) {
      dir.Format(_T(">"));
      clr = APP_MSG_INBOUND;
   }
   // Outbound
   else if (lParam == APP_TRAFFIC_OUTBOUND) {
      dir.Format(_T("<"));
      clr = APP_MSG_OUTBOUND;
   }
   // Local
   else if (lParam == APP_TRAFFIC_LOCAL) {
      dir.Format(_T("*"));
      clr = APP_MSG_LOCAL;
   }
   // In Pipe
   else if (lParam == APP_TRAFFIC_IN_PIPE) {
      dir.Format(_T(">#"));
      clr = APP_MSG_IN_PIPE;
   }
   // Out Pipe
   else if (lParam == APP_TRAFFIC_OUT_PIPE) {
      dir.Format(_T("<#"));
      clr = APP_MSG_OUT_PIPE;
   }
   // Error
   else if (lParam == APP_TRAFFIC_ERROR) {
      dir.Format(_T("!"));
      clr = APP_MSG_ERROR;
   }

   if (lParam == APP_TRAFFIC_IN_PIPE) {
      for (i=0;i<64;i+=16) {
         str1.Format(_T("%s\t%03X : "), dir, i); str += str1;
         dir.Format(_T(""));
         for (j=0;j<16;j++) {
            if ((i+j) == 64) break;
            str1.Format(_T(" %02X"), pMsg[i+j]); str += str1;
         }
         str1.Format(_T("\n")); str += str1;
      }
      if ((i%16) != 0) {
         str1.Format(_T("\n")); str += str1;
      }
      for (i=0;i<64;i+=16) {
         str1.Format(_T("\t%03X : "), i+4032); str += str1;
         dir.Format(_T(""));
         for (j=0;j<16;j++) {
            if ((i+j) == 64) break;
            str1.Format(_T(" %02X"), pMsg[i+j+4032]); str += str1;
         }
         str1.Format(_T("\n")); str += str1;
      }
      if ((i%16) != 0) {
         str1.Format(_T("\n")); str += str1;
      }
   }
   else {
      // CM Header is 8-Bytes
      if (GetOutputWnd().GetTraffic().m_bStamp)
         str1.Format(_T("\n")); str += str1;
      // CM Header is 8-Bytes, see cmConst.h
      str1.Format(_T("%s\t000 : %02X %02X %02X %02X %02X %02X %1X %03X(%d)\n"),
            dir, pMsg[0], pMsg[1], pMsg[2], pMsg[3], pMsg[4], pMsg[5],
            (pMsg[7] & 0xF0) >> 4, msgLen, msgLen); str += str1;
      // CM Parms are 4-Bytes minimum, Payload includes CM Parms + Body
      str1.Format(_T("\t008 : ")); str += str1;
      if (msgLen < CM_MAX_MSG_INT8U && msgLen > sizeof(cm_hdr_t)) {
         // Message Body
         for (i=0;i<(msgLen-8);i++) {
            str1.Format(_T("%02X "), pMsg[i+8]); str += str1;
            if (((i+1)%16) == 0) {
               if (i != (msgLen-9)) {
                  str1.Format(_T("\n")); str += str1;
                  str1.Format(_T("\t%03X : "),(i+1)+8); str += str1;
               }
               else {
                  str1.Format(_T("\n")); str += str1;
               }
            }
         }
         if ((i%16) != 0) {
            str1.Format(_T("\n")); str += str1;
         }
      }
      else {
         str1.Format(_T("Message Length Invalid %03X(%d)\n"), msgLen, msgLen); str += str1;
      }
   }

   // Output the String
   GetOutputWnd().TrafficStr(str, clr);

   // Leave Critical Section
   LeaveCriticalSection(&m_hTrafficMutex);

   return 0;
}

afx_msg LRESULT CMainFrame::OnCmLog(WPARAM wParam, LPARAM lParam)
{

   pcm_msg_t    msg = (pcm_msg_t)wParam;

   char    *dir;
   char     line[512], cat[64];
   uint16_t i,j;
   uint16_t len = msg->h.msglen;
   char    *msgid = "-", *cmid = "-";
   CString  str, str1;
   COLORREF clr;

   uint32_t now;
   double delta;

   uint16_t table_len = (sizeof(msg_table)/sizeof(*(msg_table)));

   pcm_pipe_t  pipe = (pcm_pipe_t)msg;

   // Check if Enabled ?
   if (!GetOutputWnd().GetTraffic().m_bEnable) return 0;

   // Check for Local ?
   if (!GetOutputWnd().GetTraffic().m_bLocal && lParam == APP_TRAFFIC_LOCAL) return 0;

   // Check for Pipe ?
   if (!GetOutputWnd().GetTraffic().m_bPipe && lParam == APP_TRAFFIC_IN_PIPE) return 0;
   if (!GetOutputWnd().GetTraffic().m_bPipe && lParam == APP_TRAFFIC_OUT_PIPE) return 0;

   CappDoc *pDoc = (CappDoc *)GetActiveDocument();

   if (msg == NULL) return 0;

   // Enter Critical Section
   EnterCriticalSection(&m_hTrafficMutex);

   now   = (uint32_t)m_timer.GetElapsedAsMicroSeconds();
   if (m_last_us == 0) m_last_us = now;
   delta = (double)(now - m_last_us);
   if (delta != 0) delta = delta / (1E6);

   // Control Message
   if (msg->h.dst_cmid != CM_ID_PIPE) {
      uint8_t *in_u8 = (uint8_t *)msg;
      // traffic origin, local timer
      if (msg->h.event == CM_EVENT_TIMER) {
         dir = "#";
         clr = APP_MSG_INBOUND;
      }
      // traffic origin, local
      else if (msg->h.dst_devid == msg->h.src_devid) {
         dir = "*";
         clr = APP_MSG_LOCAL;
      }
      // inbound
      else if (msg->h.dst_devid == CM_DEV_WIN) {
         dir = ">";
         clr = APP_MSG_INBOUND;
      }
      // outbound
      else  {
         dir = "<";
         clr = APP_MSG_OUTBOUND;
      }
      // cm_hdr_t is 8 bytes
      sprintf_s(line, sizeof(line), "%s\t000 : ", dir);
      for (i=0;i<8;i++) {
         sprintf_s(cat, sizeof(cat), "%02X", in_u8[i]);
         strcat_s(line, sizeof(line), cat);
      }
      // find cmid and msgid strings
      for (i=0;i<table_len; i++) {
         if ((msg_table[i].cmid  == msg->p.srvid) &&
             (msg_table[i].msgid == msg->p.msgid)) {
            msgid = msg_table[i].msg_str;
            cmid  = msg_table[i].cmid_str;
         }
      }
      sprintf_s(cat, sizeof(cat), " %6d  %10.4lf  %s:%s\n", msg->h.msglen, delta, cmid, msgid);
      strcat_s(line, sizeof(line), cat);
      CA2W wline(line);
      str1.Format(_T("%s"), wline.m_psz); str += str1;
      // msg_parms_t is 4 bytes + body
      if (msg->h.msglen < CM_MAX_MSG_INT8U &&
          msg->h.msglen > sizeof(cm_hdr_t)) {
         len -= 8;
         for (i=0;i<len;i+=8) {
            sprintf_s(line, "\t%03X : ", i+8);
            for (j=0;j<8;j++) {
               if ((i+j) >= (len))
                  strcat_s(line, "  ");
               else {
                  sprintf_s(cat, sizeof(cat), "%02X", in_u8[i+j+8]);
                  strcat_s(line, sizeof(line), cat);
               }
            }
            strcat_s(line, sizeof(line), "\n");
            CA2W wline(line);
            str1.Format(_T("%s"), wline.m_psz); str += str1;
         }
         sprintf_s(line, sizeof(line), "\n");
         CA2W wline(line);
         str1.Format(_T("%s"), wline.m_psz); str += str1;
      }
   }
   // Pipe Message
   else {
      uint8_t  *in_u8   = (uint8_t *)pipe;
      uint8_t *in_pipe  = (uint8_t *)&pipe->msglen;
      //  8 bytes of pipe header
      sprintf_s(line, sizeof(line), "%s\t000 : ", "@");
      for (i=0;i<8;i++) {
         sprintf_s(cat, "%02X", in_u8[i]);
         strcat_s(line, sizeof(line), cat);
      }
      // find cmid and msgid strings
      for (i=0;i<table_len;i++) {
         if ((msg_table[i].cmid  == pipe->dst_cmid) &&
               (msg_table[i].msgid == pipe->msgid)) {
            msgid = msg_table[i].msg_str;
            cmid  = msg_table[i].cmid_str;
         }
      }
      sprintf_s(cat, sizeof(cat), " %6d  %10.4lf  %s:%s\n", 1024, delta, cmid, msgid);
      strcat_s(line, sizeof(line), cat);
      CA2W wline(line);
      str1.Format(_T("%s"), wline.m_psz); str += str1;
      // the next 48 bytes of pipe
      len = 48;
      for (i=0;i<len;i+=8) {
         sprintf_s(line, sizeof(line), "\t%03X : ", i+8);
         for (j=0;j<8;j++) {
            if ((i+j) >= (len))
               strcat_s(line, sizeof(line), "  ");
            else {
               sprintf_s(cat, sizeof(cat), "%02X", in_pipe[i+j+8]);
               strcat_s(line, sizeof(line), cat);
            }
         }
         strcat_s(line, sizeof(line), "\n");
         CA2W wline(line);
         str1.Format(_T("%s"), wline.m_psz); str += str1;
      }
      sprintf_s(line, sizeof(line), "\n");
      CA2W wline1(line);
      str1.Format(_T("%s"), wline1.m_psz); str += str1;
   }

   m_last_us = now;

   // Output the String
   GetOutputWnd().TrafficStr(str, clr);

   // Leave Critical Section
   LeaveCriticalSection(&m_hTrafficMutex);

   return 0;
}

//
// POPUP WARNING IF MESSAGE DROPPED
//
afx_msg LRESULT CMainFrame::OnDropMsg(WPARAM wParam, LPARAM lParam)
{
   CappDoc *pDoc = (CappDoc *)GetActiveDocument();

      if (pDoc->m_ini->apMessageDropWarn) {
         AfxMessageBox(IDS_MESSAGE_DROPPED, MB_OK|MB_ICONSTOP);
      }

   return 0;
}

void CMainFrame::ProgressBarEnable(INT nIndex, LONG nTotal)
{
   GetStatusBar().EnablePaneProgressBar(nIndex, nTotal);
}

void CMainFrame::ProgressBarSet(INT nIndex, LONG nCurr)
{
   GetStatusBar().SetPaneProgress(nIndex, nCurr);
}

void CMainFrame::SetParam(VOID * param) {
   CappDoc *pDoc = (CappDoc *)GetActiveDocument();
   GetParmsWnd().SetParam(param);
}

void  cm_traffic_log(uint8_t op_code, pcm_msg_t msg) {
   CMainFrame *pMainFrm = (CMainFrame *)AfxGetApp()->GetMainWnd();
   pMainFrm->PostMessage(WM_CM_LOG, (WPARAM)msg, (LPARAM)op_code);
}