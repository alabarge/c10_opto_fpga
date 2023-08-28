
// app.h : main header file for the app application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "appCfg.h"
#include "appType.h"
#include "system.h"
#include "MainFrm.h"

// CappApp:
// See app.cpp for the implementation of this class
//

class CappApp : public CWinAppEx
{
public:
	CappApp();

   bool  m_bRegExist;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

   virtual BOOL ReloadWindowPlacement(CFrameWnd* pFrame);
   virtual BOOL StoreWindowPlacement(const CRect& rectNormalPosition, int nFlags, int nShowCmd);
   virtual BOOL SaveState(LPCTSTR lpszSectionName = NULL, CFrameImpl* pFrameImpl = NULL);
	virtual BOOL LoadState(LPCTSTR lpszSectionName = NULL, CFrameImpl* pFrameImpl = NULL);
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
   virtual BOOL LoadWindowPlacement(CRect& rectNormalPosition, int& nFflags, int& nShowCmd);

};

extern CappApp theApp;
