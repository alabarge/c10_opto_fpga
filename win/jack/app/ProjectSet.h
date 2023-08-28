#pragma once

// CProjectSet dialog

class CProjectSet : public CDialog
{
	DECLARE_DYNAMIC(CProjectSet)

public:
	CProjectSet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProjectSet();

// Dialog Data
	enum { IDD = IDD_PRJ_SETTINGS };

   INT      m_apAutoSave;
   INT      m_apReloadLast;
   INT      m_apSysIDWarn;
   INT      m_apPingWarn;
   INT      m_apOverwriteWarn;
   INT      m_apMessageDropWarn;
   INT      m_apLogCMTraffic;
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   CToolTipCtrl m_toolTip;
   CBrush       m_bgBrush;
   CWnd*        m_pParent;

	DECLARE_MESSAGE_MAP()
public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   virtual BOOL PreTranslateMessage(MSG* pMsg);
};
