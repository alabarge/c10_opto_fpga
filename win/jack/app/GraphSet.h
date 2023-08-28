#pragma once

// CGraphSet dialog

class CGraphSet : public CDialog
{
	DECLARE_DYNAMIC(CGraphSet)

public:
	CGraphSet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraphSet();

// Dialog Data
	enum { IDD = IDD_GPH_SETTINGS };

   INT      m_cpGphType;
   CString  m_cpGphXmin;
   CString  m_cpGphXmax;
   CString  m_cpGphYmin;
   CString  m_cpGphYmax;
   INT      m_cpGphAuto;
   INT      m_cpGphCursor;
   INT      m_cpGphPoints;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   CToolTipCtrl m_toolTip;
   CBrush       m_bgBrush;
   CComboBox    m_Type;
   CWnd*        m_pParent;

	DECLARE_MESSAGE_MAP()
public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnUpdateGphAuto(CCmdUI* pCmdUI);
   virtual BOOL PreTranslateMessage(MSG* pMsg);
};
