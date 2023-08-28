
#pragma once

#include "TreeViewEx.h"
#include "ProjectSet.h"

class CProjectToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CProjectWnd : public CDockablePane
{
// Construction
public:
	CProjectWnd();

   friend class CProjectSet;

	void AdjustLayout();
   void OnNewDoc(PINITYPE ini);
   UINT AddPage(PAGEENTRY &pPage);
   BOOL SetItem(TVITEM* pItem);
   BOOL GetItem(TVITEM* pItem);
   UINT SelectPage(PAGEENTRY &pPage);
   BOOL Expand(HTREEITEM hItem, UINT nCode);

// Attributes
protected:

	CTreeViewEx m_Tree;
	CImageList m_ProjectViewImages;
	CProjectToolBar m_ToolBar;

   UINT     m_apAutoSave;
   UINT     m_apReloadLast;
   UINT     m_apSysIDWarn;
   UINT     m_apPingWarn;
   UINT     m_apOverwriteWarn;
   UINT     m_apMessageDropWarn;
   UINT     m_apLogCMTraffic;

protected:

// Implementation
public:
	virtual ~CProjectWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettings();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	DECLARE_MESSAGE_MAP()

public:
   CMFCToolBar&    GetToolBar()  { return m_ToolBar; }
   CTreeViewEx&    GetTree()     { return m_Tree; }
   afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
   virtual void Serialize(CArchive& ar);
};

