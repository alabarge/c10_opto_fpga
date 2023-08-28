
// ParmsWnd.h : interface of the CParmsWnd class
//
//

#pragma once

class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CParmsWnd : public CDockablePane
{
// Construction
public:
	CParmsWnd();

   friend class CParmsNew;

	void AdjustLayout();
   void OnNewDoc(PINITYPE ini);
   virtual void Serialize(CArchive& ar);
	void InitParamList();

// Attributes
public:

protected:
	CFont m_fntPropList;
	CPropertiesToolBar m_ToolBar;
	CMFCPropertyGridCtrl m_ParmList;

   UINT         m_parmLen;

// Implementation
public:
	virtual ~CParmsWnd();
   afx_msg void SetParam(VOID * param);
   void WriteFile(CStdioFile& fp);


protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

   LRESULT OnPropertyChanged(WPARAM,LPARAM lParam);

   CMFCToolBar&           GetToolBar()  { return m_ToolBar; }
   CMFCPropertyGridCtrl&  GetParmList() { return m_ParmList; }

	DECLARE_MESSAGE_MAP()

	void SetPropListFont();

};

