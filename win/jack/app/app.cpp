
// app.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "app.h"
#include "About.h"

#include "appDoc.h"
#include "ConView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CappApp

BEGIN_MESSAGE_MAP(CappApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CappApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CappApp construction

CappApp::CappApp()
{
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Omniware.DE0.1.4.1"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

   m_bRegExist = false;
}

// The one and only CappApp object

CappApp theApp;


// CappApp initialization

BOOL CappApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Omniware"));

	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CappDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CConView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

   // Load last document if Enabled
   if ((cmdInfo.m_nShellCommand != cmdInfo.FileOpen) &&
       (GetProfileInt(_T("Settings"), _T("ReloadLast"), 1))) {
      if (m_pRecentFileList && m_pRecentFileList->GetSize() > 0) {
         // File Exists?
         DWORD dwAttrib = GetFileAttributes((*m_pRecentFileList)[0]);
         if (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
            cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
            cmdInfo.m_strFileName = (*m_pRecentFileList)[0];
         }
      }
   }

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	return TRUE;
}

int CappApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// App command to run the dialog
void CappApp::OnAppAbout()
{
	CAbout aboutDlg;
	aboutDlg.DoModal();
}

// CappApp customization load/save methods

BOOL CappApp::ReloadWindowPlacement(CFrameWnd* pFrame)
{
   return CWinAppEx::ReloadWindowPlacement(pFrame);
}

BOOL CappApp::StoreWindowPlacement(const CRect& rectNormalPosition, int nFlags, int nShowCmd)
{
   return CWinAppEx::StoreWindowPlacement(rectNormalPosition, nFlags, nShowCmd);
}

BOOL CappApp::SaveState(LPCTSTR lpszSectionName, CFrameImpl* pFrameImpl)
{
   return CWinAppEx::SaveState(lpszSectionName, pFrameImpl);
}

BOOL CappApp::LoadState(LPCTSTR lpszSectionName, CFrameImpl* pFrameImpl)
{
   return CWinAppEx::LoadState(lpszSectionName, pFrameImpl);
}

void CappApp::PreLoadState()
{
}

void CappApp::LoadCustomState()
{
}

void CappApp::SaveCustomState()
{
}

BOOL CappApp::LoadWindowPlacement(CRect& rectNormalPosition, int& nFflags, int& nShowCmd)
{
   BOOL result = CWinAppEx::LoadWindowPlacement(rectNormalPosition, nFflags, nShowCmd);

   // If the window placement isn't in the registry then
   // center the window on the screen with it's minimum size
   if (result == FALSE) {
      int cx = ::GetSystemMetrics(SM_CXSCREEN) / 2;
      int cy = ::GetSystemMetrics(SM_CYSCREEN) / 2;
      // Ensure mainframe is fully visible otherwise let
      // the framework position the window
      if ((cy - (APP_MIN_SIZE_Y / 2) < 0) ||
          (cx - (APP_MIN_SIZE_X / 2) < 0)) {
         return FALSE;
      }
      else {
         rectNormalPosition.top = cy - (APP_MIN_SIZE_Y / 2);
         rectNormalPosition.bottom = cy + (APP_MIN_SIZE_Y / 2);
         rectNormalPosition.left = cx - (APP_MIN_SIZE_X / 2);
         rectNormalPosition.right = cx + (APP_MIN_SIZE_X / 2);
         return TRUE;
      }
   }
   // Flag the existence of a registry entry
   else {
      m_bRegExist = true;
   }

   return result;
}
