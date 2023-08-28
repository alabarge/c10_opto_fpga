// GraphWnd.h : interface of the CGraphWnd class
//

#pragma once

#include "GraphSet.h"
#include "Pegrpapi.h"

#define  GRAPH_TYPE_NONE       0
#define  GRAPH_TYPE_CPU_MEM    1
#define  GRAPH_TYPE_DAQ        2

#define  GRAPH_NAME_NONE       _T("None")
#define  GRAPH_NAME_CPU_MEM    _T("CPU Memory")
#define  GRAPH_NAME_DAQ        _T("DAQ")

#define  GRAPH_XDATA_RANGE     512

#define  GRAPH_NILL           -9999999.0F;

class CGraphToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CGraphWnd : public CDockablePane
{
// Construction
public:
	CGraphWnd();

   friend class CGraphSet;

	void AdjustLayout();
   void Plot(PPLOT pPlot);
   void OnNewDoc(PINITYPE ini);

// Attributes
protected:

	CGraphToolBar m_ToolBar;

   UINT     m_cpGphType;
   DOUBLE   m_cpGphXmin;
   DOUBLE   m_cpGphXmax;
   DOUBLE   m_cpGphYmin;
   DOUBLE   m_cpGphYmax;
   UINT     m_cpGphAuto;
   UINT     m_cpGphCursor;
   UINT     m_cpGphPoints;

protected:

// Implementation
public:
	virtual ~CGraphWnd();

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettings();
	afx_msg void OnPrint();

   void ResetGraph(HWND m_hPE);
   void ResetGraphDAQ(HWND m_hPE);
   void SetGraph(FLOAT xStep);
   void PlotGraph(void);
   void PlotMemory(void);
   void PlotDAQ(void);

   HWND     m_hPE;
   UINT     m_index;

   PLOT     m_plot;

   CRITICAL_SECTION m_hMutex;

   float    *m_XData;
   float    *m_YData;
   INT      m_XDataLen;
   INT      m_lanes[2];
   INT      m_type;
   INT      m_XIndex;

	DECLARE_MESSAGE_MAP()

public:
   CMFCToolBar&    GetToolBar()  { return m_ToolBar; }

   afx_msg BOOL OnEraseBkgnd(CDC* pDC);
   virtual BOOL OnCommand(WPARAM wp, LPARAM lp);
   virtual void Serialize(CArchive& ar);
   afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

