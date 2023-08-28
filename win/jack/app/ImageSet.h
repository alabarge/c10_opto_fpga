#pragma once

// CImageSet dialog

class CImageSet : public CDialog
{
	DECLARE_DYNAMIC(CImageSet)

public:
	CImageSet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImageSet();

   //friend class CImageWnd;

// Dialog Data
	enum { IDD = IDD_IMG_SETTINGS };

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
