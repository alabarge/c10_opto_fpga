
#pragma once

#include "2DImage.h"
#include "ImageSet.h"

class CImageToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CImageWnd : public CDockablePane
{
// Construction
public:
	CImageWnd();

   friend class CImageSet;

	void AdjustLayout();
   void OnNewDoc(PINITYPE ini);
   void OnPlot(void);

// Attributes
protected:

	CImageToolBar m_ToolBar;

protected:

// Implementation
public:
	virtual ~CImageWnd();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettings();

   C2DImage    m_2DImage;

	DECLARE_MESSAGE_MAP()

public:
   CMFCToolBar&    GetToolBar()  { return m_ToolBar; }
   afx_msg BOOL OnEraseBkgnd(CDC* pDC);
   virtual void Serialize(CArchive& ar);
};

