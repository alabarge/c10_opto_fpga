// GraphWnd.cpp : implementation of the CGraphWnd class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "GraphWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define  GRAPH_CLR1   RGB(0x33, 0x99, 0xff)
#define  GRAPH_CLR2   RGB(0xff, 0x42, 0x42)
#define  GRAPH_CLR3   RGB(0x00, 0xdc, 0x4f) 
#define  GRAPH_CLR4   RGB(0xab, 0x84, 0x8a) 
#define  GRAPH_CLR5   RGB(0xff, 0xdc, 0x00) 
#define  GRAPH_CLR6   RGB(0xcd, 0x59, 0xff) 
#define  GRAPH_CLR7   RGB(0xff, 0xb3, 0x12) 
#define  GRAPH_CLR8   RGB(0x00, 0x2c, 0x94) 

#define  GRAPH_CLR9   RGB(0x00, 0xcd, 0xd4) 
#define  GRAPH_CLR10  RGB(0x9a, 0x8a, 0x74) 
#define  GRAPH_CLR11  RGB(0x29, 0x29, 0x29) 
#define  GRAPH_CLR12  RGB(0x00, 0x5b, 0x17) 
#define  GRAPH_CLR13  RGB(0xb4, 0x11, 0x00) 
#define  GRAPH_CLR14  RGB(0xf9, 0x5b, 0xcd) 
#define  GRAPH_CLR15  RGB(0x68, 0x00, 0x95) 
#define  GRAPH_CLR16  RGB(0x00, 0x8a, 0x8a) 

#define  GRAPH_CLR17  RGB(220, 220,   0) // Gold

static   DWORD  clrList[]     = {GRAPH_CLR3,  GRAPH_CLR2,  GRAPH_CLR1,  GRAPH_CLR4,
                                 GRAPH_CLR5,  GRAPH_CLR6,  GRAPH_CLR7,  GRAPH_CLR8,
                                 GRAPH_CLR9,  GRAPH_CLR10, GRAPH_CLR11, GRAPH_CLR12,
                                 GRAPH_CLR13, GRAPH_CLR14, GRAPH_CLR15, GRAPH_CLR16};

static   wchar_t  *chnList[]  = {L"CH00", L"CH01", L"CH02", L"CH03",
                                 L"CH04", L"CH05", L"CH06", L"CH07",
                                 L"CH08", L"CH09", L"CH10", L"CH11",
                                 L"CH12", L"CH13", L"CH14", L"CH15"};

// CGraphWnd

BEGIN_MESSAGE_MAP(CGraphWnd, CDockablePane)
	ON_COMMAND(ID_GRAPH_1, OnSettings)
	ON_COMMAND(ID_GRAPH_2, OnPrint)
	ON_WM_CREATE()
	ON_WM_SIZE()
   ON_WM_ERASEBKGND()
   ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

//
// CGraphWnd construction/destruction
//

CGraphWnd::CGraphWnd()
{
   m_hPE   = NULL;
   m_index = 0;
   ::ZeroMemory(&m_plot, sizeof(PLOT));

   // Create Mutex for Plotting
   InitializeCriticalSection(&m_hMutex);

   m_cpGphType = 0;
   m_cpGphXmin = 0.0;
   m_cpGphXmax = 0.0;
   m_cpGphYmin = 0.0;
   m_cpGphYmax = 0.0;
   m_cpGphAuto = 0;
   m_cpGphCursor = 0;
   m_cpGphPoints = 0;

   m_XData = NULL;
   m_YData = NULL;

   m_XDataLen = GRAPH_XDATA_RANGE;
   m_lanes[0] = 0;
   m_lanes[1] = 0;
   m_XIndex = 0;
}

CGraphWnd::~CGraphWnd()
{
   if (m_XData) delete m_XData;
   if (m_YData) delete m_YData;
}

//
// CGraphWnd Overides
//

int CGraphWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1) return -1;

   GetToolBar().Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_GRAPH);
	GetToolBar().LoadToolBar(IDR_GRAPH, 0, 0, TRUE);
	GetToolBar().SetPaneStyle(GetToolBar().GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	GetToolBar().SetPaneStyle(GetToolBar().GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	GetToolBar().SetOwner(this);

	// All commands will be routed via this control, not via the parent frame:
	GetToolBar().SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();

   // OnSize handles re-sizing
   RECT  rect;
   rect.bottom = rect.left = rect.right = rect.top = 0;
   m_hPE = PEcreate(PECONTROL_SGRAPH, WS_VISIBLE, &rect, m_hWnd, 1001);

   SetWindowText(_T("Graph"));

	return 0;
}

void CGraphWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustLayout();
   CRect rect;

   GetToolBar().GetWindowRect(rect);

   if (m_hPE) ::MoveWindow(m_hPE, 0, rect.Height(), cx, cy-rect.Height(), FALSE);
}

BOOL CGraphWnd::OnEraseBkgnd(CDC* pDC)
{
   if (::IsWindow(m_hPE)) return TRUE;
   return CDockablePane::OnEraseBkgnd(pDC);
}

void CGraphWnd::Serialize(CArchive& ar)
{
   CappDoc* pDoc = (CappDoc *)ar.m_pDocument;

   if (ar.IsStoring()) {
      pDoc->m_ini->cpGphType    = m_cpGphType;
      pDoc->m_ini->cpGphXmin    = m_cpGphXmin;
      pDoc->m_ini->cpGphXmax    = m_cpGphXmax;
      pDoc->m_ini->cpGphYmin    = m_cpGphYmin;
      pDoc->m_ini->cpGphYmax    = m_cpGphYmax;
      pDoc->m_ini->cpGphAuto    = m_cpGphAuto;
      pDoc->m_ini->cpGphCursor  = m_cpGphCursor;
      pDoc->m_ini->cpGphPoints  = m_cpGphPoints;
   }
   //
   // De-serialize the MainFrame data from storage
   //
   else {
      m_cpGphType    = pDoc->m_ini->cpGphType;
      m_cpGphXmin    = pDoc->m_ini->cpGphXmin;
      m_cpGphXmax    = pDoc->m_ini->cpGphXmax;
      m_cpGphYmin    = pDoc->m_ini->cpGphYmin;
      m_cpGphYmax    = pDoc->m_ini->cpGphYmax;
      m_cpGphAuto    = pDoc->m_ini->cpGphAuto;
      m_cpGphCursor  = pDoc->m_ini->cpGphCursor;
      m_cpGphPoints  = pDoc->m_ini->cpGphPoints;
      // Init the Graph
      SetGraph(1.0);
      PEreinitialize(m_hPE);
      PEresetimage(m_hPE, 0, 0);
      if (m_cpGphType == GRAPH_TYPE_NONE)
         SetWindowText(GRAPH_NAME_NONE);
      else if (m_cpGphType == GRAPH_TYPE_CPU_MEM)
         SetWindowText(GRAPH_NAME_CPU_MEM);
      else if (m_cpGphType == GRAPH_TYPE_DAQ)
         SetWindowText(GRAPH_NAME_DAQ);
   }
}

void CGraphWnd::OnNewDoc(PINITYPE ini)
{
   if (ini != NULL) {
      m_cpGphType    = ini->cpGphType;
      m_cpGphXmin    = ini->cpGphXmin;
      m_cpGphXmax    = ini->cpGphXmax;
      m_cpGphYmin    = ini->cpGphYmin;
      m_cpGphYmax    = ini->cpGphYmax;
      m_cpGphAuto    = ini->cpGphAuto;
      m_cpGphCursor  = ini->cpGphCursor;
      m_cpGphPoints  = ini->cpGphPoints;
      // Init the Graph
      SetGraph(1.0);
      PEreinitialize(m_hPE);
      PEresetimage(m_hPE, 0, 0);
      if (m_cpGphType == GRAPH_TYPE_NONE)
         SetWindowText(GRAPH_NAME_NONE);
      else if (m_cpGphType == GRAPH_TYPE_CPU_MEM)
         SetWindowText(GRAPH_NAME_CPU_MEM);
      else if (m_cpGphType == GRAPH_TYPE_DAQ)
         SetWindowText(GRAPH_NAME_DAQ);
   }
}

BOOL CGraphWnd::OnCommand(WPARAM wp, LPARAM lp)
{
   return CDockablePane::OnCommand(wp, lp);
}

//
// CGraphWnd Implementation
//

void CGraphWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL) return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = GetToolBar().CalcFixedLayout(FALSE, TRUE).cy;
	GetToolBar().SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CGraphWnd::OnSettings()
{
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();
   CGraphSet dlg(this);
   if (dlg.DoModal() == IDOK) {
      pDoc->SetModifiedFlag(TRUE);
      SetGraph(1.0);
      PEreinitialize(m_hPE);
      PEresetimage(m_hPE, 0, 0);
      ::InvalidateRect(m_hPE, NULL, FALSE);
      if (m_cpGphType == GRAPH_TYPE_NONE)
         SetWindowText(GRAPH_NAME_NONE);
      else if (m_cpGphType == GRAPH_TYPE_CPU_MEM)
         SetWindowText(GRAPH_NAME_CPU_MEM);
      else if (m_cpGphType == GRAPH_TYPE_DAQ)
         SetWindowText(GRAPH_NAME_DAQ);
   }
}

void CGraphWnd::OnPrint()
{
   CRect    rect;
   POINT    p;

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();

   GetWindowRect(rect);
   p.x = rect.Width();
   p.y = rect.Height();
   GetToolBar().GetWindowRect(rect);
   p.y -= rect.Height();

   p.x *= 36;
   p.y *= 36;

   PEnset(m_hPE, PEP_nPRINTDPI, 150);
   PEnset(m_hPE, PEP_bHIDEPRINTDPI, TRUE);

   PElaunchprintdialog(m_hPE, FALSE, &p);

}

void CGraphWnd::SetGraph(FLOAT xStep) 
{
   if (m_XData) delete m_XData;
   if (m_YData) delete m_YData;
   m_XData = new float[m_XDataLen];
   m_YData = new float[m_XDataLen];
   for (int i=0;i<m_XDataLen;i++) m_XData[i] = (float)(i * xStep);
   for (int i=0;i<m_XDataLen;i++) m_YData[i] = (float)0;
   PEreset(m_hPE);
   if (m_cpGphType == GRAPH_TYPE_DAQ)
      ResetGraphDAQ(m_hPE);
   else
      ResetGraph(m_hPE);
}

void CGraphWnd::ResetGraph(HWND m_hPE) 
{
   if (m_hPE) {

      // Set Lanes if Empty
      if (m_lanes[0] == 0 && m_lanes[1] == 0) {
         m_lanes[0] = 1;
         m_lanes[1] = 0;
      }

      // Enable ZoomWindow
      PEnset(m_hPE, PEP_bZOOMWINDOW, TRUE);

      // Enable middle mouse dragging
      PEnset(m_hPE, PEP_bMOUSEDRAGGINGX, TRUE);
      PEnset(m_hPE, PEP_bMOUSEDRAGGINGY, TRUE);

      PEnset(m_hPE, PEP_nSUBSETS, m_lanes[0] + m_lanes[1]);
      PEnset(m_hPE, PEP_nPOINTS, m_XDataLen);

      // Auto Data Scaling
      PEnset(m_hPE, PEP_bAUTOSCALEDATA, TRUE);

      // Set X-Y Auto scale
      if (m_cpGphAuto) {
         PEnset(m_hPE, PEP_nMANUALSCALECONTROLX, PEMSC_NONE);
         PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_NONE);
      }
      // Set X-Y Manual Scale
      else {
         PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);
         PEnset(m_hPE, PEP_nMANUALSCALECONTROLX, PEMSC_MINMAX);
         PEvset(m_hPE, PEP_fMANUALMINX, &m_cpGphXmin, 1);
         PEvset(m_hPE, PEP_fMANUALMAXX, &m_cpGphXmax, 1);
         PEvset(m_hPE, PEP_fMANUALMINY, &m_cpGphYmin, 1);
         PEvset(m_hPE, PEP_fMANUALMAXY, &m_cpGphYmax, 1);
      }

      PEnset(m_hPE, PEP_nPOINTGRADIENTSTYLE, PEPGS_VERTICAL_ASCENT_INVERSE);
      PEnset(m_hPE, PEP_dwPOINTBORDERCOLOR, PERGB(100, 0, 0, 0));
      PEnset(m_hPE, PEP_nLINESYMBOLTHICKNESS, 3);
      PEnset(m_hPE, PEP_nAREABORDER, 1);
      PEnset(m_hPE, PEP_bALLOWSVGEXPORT, 1);

      PEnset(m_hPE, PEP_bSIMPLEPOINTLEGEND, TRUE);
      PEnset(m_hPE, PEP_bSIMPLELINELEGEND, TRUE);
      PEnset(m_hPE, PEP_nLEGENDSTYLE, PELS_1_LINE);

      // Clear out default data
      float val = 0;
      // Init X/Y data
      for (int i=0;i<m_lanes[0] + m_lanes[1];i++) {
         for (int j=0;j<4;j++) {
            PEvsetcellEx(m_hPE, PEP_faXDATA, i, j, &val);
            PEvsetcellEx(m_hPE, PEP_faYDATA, i, j, &val);
         }
      }

      // Small or Medium Data Points
      if (m_cpGphPoints) {
         PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_SMALL);
         PEnset(m_hPE, PEP_nMAXIMUMPOINTSIZE, PEPS_SMALL);
         PEnset(m_hPE, PEP_nMINIMUMPOINTSIZE, PEPS_SMALL);
      }
      else {
         PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_MEDIUM);
         PEnset(m_hPE, PEP_nMAXIMUMPOINTSIZE, PEMPS_MEDIUM);
         PEnset(m_hPE, PEP_nMINIMUMPOINTSIZE, PEMPS_MEDIUM);
      }

      PEnset(m_hPE, PEP_bPREPAREIMAGES, TRUE);
      PEnset(m_hPE, PEP_nDATAPRECISION, 3);
      PEnset(m_hPE, PEP_nGRIDSTYLE, PEGS_DOT);
      PEnset(m_hPE, PEP_nPLOTTINGMETHOD, PEGPM_LINE);

      // subset colors
      PEvsetEx(m_hPE, PEP_dwaSUBSETCOLORS, 0, m_lanes[0] + m_lanes[1], clrList, 0);

      // subset line types
      int nLineTypes[] = {PELT_THINSOLID, PELT_THINSOLID,
                          PELT_THINSOLID, PELT_THINSOLID,
                          PELT_THINSOLID, PELT_THINSOLID,
                          PELT_THINSOLID, PELT_THINSOLID, 
                          PELT_THINSOLID, PELT_THINSOLID, 
                          PELT_THINSOLID, PELT_THINSOLID, 
                          PELT_THINSOLID, PELT_THINSOLID, 
                          PELT_THINSOLID, PELT_THINSOLID};
      PEvset(m_hPE, PEP_naSUBSETLINETYPES, nLineTypes, m_lanes[0] + m_lanes[1]);

      // subset point types
      int nPointTypes[] = {PEPT_DOTSOLID, PEPT_DOTSOLID,
                           PEPT_DOTSOLID, PEPT_DOTSOLID,
                           PEPT_DOTSOLID, PEPT_DOTSOLID,
                           PEPT_DOTSOLID, PEPT_DOTSOLID, 
                           PEPT_DOTSOLID, PEPT_DOTSOLID, 
                           PEPT_DOTSOLID, PEPT_DOTSOLID, 
                           PEPT_DOTSOLID, PEPT_DOTSOLID, 
                           PEPT_DOTSOLID, PEPT_DOTSOLID};
      PEvset(m_hPE, PEP_naSUBSETPOINTTYPES, nPointTypes, m_lanes[0] + m_lanes[1]);

      // Allow zooming and panning
      PEnset(m_hPE, PEP_nALLOWZOOMING, PEAZ_HORZANDVERT);
      PEnset(m_hPE, PEP_bSCROLLINGHORZZOOM, TRUE);

      // Cursor prompting in top right corner
      PEnset(m_hPE, PEP_nCURSORMODE, PECM_NOCURSOR);
      PEnset(m_hPE, PEP_bCURSORPROMPTTRACKING, m_cpGphCursor);
      PEnset(m_hPE, PEP_nCURSORPROMPTLOCATION, PECPL_TOP_RIGHT);
      PEnset(m_hPE, PEP_nCURSORPROMPTSTYLE, PECPS_XYVALUES);
      PEnset(m_hPE, PEP_bALLOWDATAHOTSPOTS, TRUE);
      PEnset(m_hPE, PEP_bMOUSECURSORCONTROL, TRUE);

      // Set Various Other Properties
      PEnset(m_hPE, PEP_nGRIDBANDALPHA, 45);
      PEnset(m_hPE, PEP_bBITMAPGRADIENTMODE, FALSE);
      PEnset(m_hPE, PEP_nQUICKSTYLE, PEQS_NO_STYLE);
      PEnset(m_hPE, PEP_bMARKDATAPOINTS, TRUE);
      PEnset(m_hPE, PEP_bFOCALRECT, FALSE);
      PEnset(m_hPE, PEP_nZOOMSTYLE, PEZS_RO2_NOT);
      PEnset(m_hPE, PEP_bFIXEDFONTS, TRUE);
      PEnset(m_hPE, PEP_bPREPAREIMAGES, TRUE);
      PEnset(m_hPE, PEP_bCACHEBMP, TRUE);
      PEnset(m_hPE, PEP_bAUTOIMAGERESET, FALSE);
      PEnset(m_hPE, PEP_nBORDERTYPES, PETAB_NO_BORDER);
      PEnset(m_hPE, PEP_bALLOWPLOTCUSTOMIZATION, TRUE);

      PEnset(m_hPE, PEP_nTEXTSHADOWS, PETS_BOLD_TEXT);
      PEnset(m_hPE, PEP_nDATASHADOWS, PEDS_NONE);
      PEnset(m_hPE, PEP_bMAINTITLEBOLD, TRUE);
      PEnset(m_hPE, PEP_bSUBTITLEBOLD, FALSE);
      PEnset(m_hPE, PEP_bLABELBOLD, FALSE);
      PEnset(m_hPE, PEP_bLINESHADOWS, FALSE);
      //PEnset(m_hPE, PEP_nFONTSIZE, PEFS_SMALL);
      PEnset(m_hPE, PEP_nFONTSIZE, PEFS_MEDIUM);

      PEnset(m_hPE, PEP_nDPIX, 600);
      PEnset(m_hPE, PEP_nDPIY, 600);
      PEnset(m_hPE, PEP_nEXPORTSIZEDEF, PEESD_NO_SIZE_OR_PIXEL );
      PEnset(m_hPE, PEP_nEXPORTTYPEDEF, PEETD_PNG );
      PEnset(m_hPE, PEP_nEXPORTDESTDEF, PEEDD_CLIPBOARD );
      PEszset(m_hPE, PEP_szEXPORTUNITXDEF, TEXT("1280"));
      PEszset(m_hPE, PEP_szEXPORTUNITYDEF, TEXT("768"));
      PEnset(m_hPE, PEP_nEXPORTIMAGEDPI, 300 );

      PEszset(m_hPE, PEP_szMAINTITLEFONT, TEXT("Arial"));
      PEszset(m_hPE, PEP_szSUBTITLEFONT, TEXT("Arial"));
      PEszset(m_hPE, PEP_szMAINTITLE, TEXT(""));
      PEszset(m_hPE, PEP_szSUBTITLE, TEXT(""));
      //float f = 1.2F;
      //PEvset(m_hPE, PEP_fFONTSIZEALCNTL, &f, 1);

      // This allows plotting of zero values
      double dNill = GRAPH_NILL;
      PEvset(m_hPE, PEP_fNULLDATAVALUE, &dNill, 1);

      // Sub-divide 8 subsets into two axes
      int nArray[2] = {1,0};
      PEvset (m_hPE, PEP_naMULTIAXESSUBSETS, nArray, 2);

      // Subset labels
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 0,  TEXT("CH00"));

      int nSTL[] = {0};
      PEvset(m_hPE, PEP_naSUBSETSTOLEGEND, nSTL, m_lanes[0] + m_lanes[1]);

      PEnset(m_hPE, PEP_nLEGENDLOCATION, PELL_RIGHT);
      PEnset(m_hPE, PEP_nLEGENDSTYLE, PELS_1_LINE_INSIDE_AXIS);
      PEnset(m_hPE, PEP_bSHOWLEGEND, TRUE);
      PEnset(m_hPE, PEP_nMULTIAXESSIZING, TRUE);
      PEnset(m_hPE, PEP_nMULTIAXISSTYLE, PEMAS_SEPARATE_AXES);

      PEszset(m_hPE, PEP_szXAXISLABEL, TEXT("Counts"));

      PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Memory"));
      PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Memory"));

      // Add Some Padding on Right and Bottom
      PEnset(m_hPE, PEP_nIMAGEADJUSTLEFT, 50);
      PEnset(m_hPE, PEP_nIMAGEADJUSTRIGHT, 50);
      PEnset(m_hPE, PEP_nIMAGEADJUSTBOTTOM, 50);
      PEnset(m_hPE, PEP_nIMAGEADJUSTTOP, 50);

      // Improves metafile export
      PEnset(m_hPE, PEP_nDPIX, 600);
      PEnset(m_hPE, PEP_nDPIY, 600);

      // Set RenderEngine to Gdi Plus
      PEnset(m_hPE, PEP_nRENDERENGINE, PERE_HYBRID);
		PEnset(m_hPE, PEP_bANTIALIASTEXT, TRUE);
		PEnset(m_hPE, PEP_bANTIALIASGRAPHICS, TRUE);
      PEnset(m_hPE, PEP_nCOMPOSITE2D3D, PEC2D_FOREGROUND);

      ::InvalidateRect(m_hPE, NULL, FALSE);
   }
}


void CGraphWnd::ResetGraphDAQ(HWND m_hPE) 
{
   if (m_hPE) {
      // Set Lanes if Empty
      if (m_lanes[0] == 0 && m_lanes[1] == 0) {
         m_lanes[0] = 1;
         m_lanes[1] = 0;
      }

      // Enable ZoomWindow
      PEnset(m_hPE, PEP_bZOOMWINDOW, TRUE);

      // Enable middle mouse dragging
      PEnset(m_hPE, PEP_bMOUSEDRAGGINGX, TRUE);
      PEnset(m_hPE, PEP_bMOUSEDRAGGINGY, TRUE);

      PEnset(m_hPE, PEP_nSUBSETS, m_lanes[0] + m_lanes[1]);
      PEnset(m_hPE, PEP_nPOINTS, m_XDataLen);

      // Auto Data Scaling
      PEnset(m_hPE, PEP_bAUTOSCALEDATA, TRUE);

      // Set X-Y Auto scale
      if (m_cpGphAuto) {
         PEnset(m_hPE, PEP_nMANUALSCALECONTROLX, PEMSC_NONE);
         PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_NONE);
      }
      // Set X-Y Manual Scale
      else {
         PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);
         PEnset(m_hPE, PEP_nMANUALSCALECONTROLX, PEMSC_MINMAX);
         PEvset(m_hPE, PEP_fMANUALMINX, &m_cpGphXmin, 1);
         PEvset(m_hPE, PEP_fMANUALMAXX, &m_cpGphXmax, 1);
         PEvset(m_hPE, PEP_fMANUALMINY, &m_cpGphYmin, 1);
         PEvset(m_hPE, PEP_fMANUALMAXY, &m_cpGphYmax, 1);
      }

      PEnset(m_hPE, PEP_nPOINTGRADIENTSTYLE, PEPGS_VERTICAL_ASCENT_INVERSE);
      PEnset(m_hPE, PEP_dwPOINTBORDERCOLOR, PERGB(100, 0, 0, 0));
      PEnset(m_hPE, PEP_nLINESYMBOLTHICKNESS, 3);
      PEnset(m_hPE, PEP_nAREABORDER, 1);
      PEnset(m_hPE, PEP_bALLOWSVGEXPORT, 1);

      PEnset(m_hPE, PEP_bSIMPLEPOINTLEGEND, TRUE);
      PEnset(m_hPE, PEP_bSIMPLELINELEGEND, TRUE);
      PEnset(m_hPE, PEP_nLEGENDSTYLE, PELS_1_LINE);

      // Clear out default data
      float val = 0;
      // Init X/Y data
      for (int i=0;i<m_lanes[0] + m_lanes[1];i++) {
         for (int j=0;j<4;j++) {
            PEvsetcellEx(m_hPE, PEP_faXDATA, i, j, &val);
            PEvsetcellEx(m_hPE, PEP_faYDATA, i, j, &val);
         }
      }

      // Small or Medium Data Points
      if (m_cpGphPoints) {
         PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_SMALL);
         PEnset(m_hPE, PEP_nMAXIMUMPOINTSIZE, PEPS_SMALL);
         PEnset(m_hPE, PEP_nMINIMUMPOINTSIZE, PEPS_SMALL);
      }
      else {
         PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_MEDIUM);
         PEnset(m_hPE, PEP_nMAXIMUMPOINTSIZE, PEMPS_MEDIUM);
         PEnset(m_hPE, PEP_nMINIMUMPOINTSIZE, PEMPS_MEDIUM);
      }

      PEnset(m_hPE, PEP_bPREPAREIMAGES, TRUE);
      PEnset(m_hPE, PEP_nDATAPRECISION, 3);
      PEnset(m_hPE, PEP_nGRIDSTYLE, PEGS_DOT);
      PEnset(m_hPE, PEP_nPLOTTINGMETHOD, PEGPM_LINE);

      // subset colors
      PEvsetEx(m_hPE, PEP_dwaSUBSETCOLORS, 0, m_lanes[0] + m_lanes[1], clrList, 0);

      // subset line types
      int nLineTypes[] = {PELT_THINSOLID, PELT_THINSOLID,
                          PELT_THINSOLID, PELT_THINSOLID,
                          PELT_THINSOLID, PELT_THINSOLID,
                          PELT_THINSOLID, PELT_THINSOLID, 
                          PELT_THINSOLID, PELT_THINSOLID, 
                          PELT_THINSOLID, PELT_THINSOLID, 
                          PELT_THINSOLID, PELT_THINSOLID, 
                          PELT_THINSOLID, PELT_THINSOLID};
      PEvset(m_hPE, PEP_naSUBSETLINETYPES, nLineTypes, m_lanes[0] + m_lanes[1]);

      // subset point types
      int nPointTypes[] = {PEPT_DOTSOLID, PEPT_DOTSOLID,
                           PEPT_DOTSOLID, PEPT_DOTSOLID,
                           PEPT_DOTSOLID, PEPT_DOTSOLID,
                           PEPT_DOTSOLID, PEPT_DOTSOLID, 
                           PEPT_DOTSOLID, PEPT_DOTSOLID, 
                           PEPT_DOTSOLID, PEPT_DOTSOLID, 
                           PEPT_DOTSOLID, PEPT_DOTSOLID, 
                           PEPT_DOTSOLID, PEPT_DOTSOLID};
      PEvset(m_hPE, PEP_naSUBSETPOINTTYPES, nPointTypes, m_lanes[0] + m_lanes[1]);

      // Allow zooming and panning
      PEnset(m_hPE, PEP_nALLOWZOOMING, PEAZ_HORZANDVERT);
      PEnset(m_hPE, PEP_bSCROLLINGHORZZOOM, TRUE);

      // Cursor prompting in top right corner
      PEnset(m_hPE, PEP_nCURSORMODE, PECM_NOCURSOR);
      PEnset(m_hPE, PEP_bCURSORPROMPTTRACKING, m_cpGphCursor);
      PEnset(m_hPE, PEP_nCURSORPROMPTLOCATION, PECPL_TOP_RIGHT);
      PEnset(m_hPE, PEP_nCURSORPROMPTSTYLE, PECPS_XYVALUES);
      PEnset(m_hPE, PEP_bALLOWDATAHOTSPOTS, TRUE);
      PEnset(m_hPE, PEP_bMOUSECURSORCONTROL, TRUE);

      // Set Various Other Properties
      PEnset(m_hPE, PEP_nGRIDBANDALPHA, 45);
      PEnset(m_hPE, PEP_bBITMAPGRADIENTMODE, FALSE);
      PEnset(m_hPE, PEP_nQUICKSTYLE, PEQS_NO_STYLE);
      PEnset(m_hPE, PEP_bMARKDATAPOINTS, TRUE);
      PEnset(m_hPE, PEP_bFOCALRECT, FALSE);
      PEnset(m_hPE, PEP_nZOOMSTYLE, PEZS_RO2_NOT);
      PEnset(m_hPE, PEP_bFIXEDFONTS, TRUE);
      PEnset(m_hPE, PEP_bPREPAREIMAGES, TRUE);
      PEnset(m_hPE, PEP_bCACHEBMP, TRUE);
      PEnset(m_hPE, PEP_bAUTOIMAGERESET, FALSE);
      PEnset(m_hPE, PEP_nBORDERTYPES, PETAB_NO_BORDER);
      PEnset(m_hPE, PEP_bALLOWPLOTCUSTOMIZATION, TRUE);

      PEnset(m_hPE, PEP_nTEXTSHADOWS, PETS_BOLD_TEXT);
      PEnset(m_hPE, PEP_nDATASHADOWS, PEDS_NONE);
      PEnset(m_hPE, PEP_bMAINTITLEBOLD, TRUE);
      PEnset(m_hPE, PEP_bSUBTITLEBOLD, FALSE);
      PEnset(m_hPE, PEP_bLABELBOLD, FALSE);
      PEnset(m_hPE, PEP_bLINESHADOWS, FALSE);
      //PEnset(m_hPE, PEP_nFONTSIZE, PEFS_SMALL);
      PEnset(m_hPE, PEP_nFONTSIZE, PEFS_MEDIUM);

      PEnset(m_hPE, PEP_nDPIX, 600);
      PEnset(m_hPE, PEP_nDPIY, 600);
      PEnset(m_hPE, PEP_nEXPORTSIZEDEF, PEESD_NO_SIZE_OR_PIXEL );
      PEnset(m_hPE, PEP_nEXPORTTYPEDEF, PEETD_PNG );
      PEnset(m_hPE, PEP_nEXPORTDESTDEF, PEEDD_CLIPBOARD );
      PEszset(m_hPE, PEP_szEXPORTUNITXDEF, TEXT("1280"));
      PEszset(m_hPE, PEP_szEXPORTUNITYDEF, TEXT("768"));
      PEnset(m_hPE, PEP_nEXPORTIMAGEDPI, 300 );

      PEszset(m_hPE, PEP_szMAINTITLEFONT, TEXT("Arial"));
      PEszset(m_hPE, PEP_szSUBTITLEFONT, TEXT("Arial"));
      PEszset(m_hPE, PEP_szMAINTITLE, TEXT(""));
      PEszset(m_hPE, PEP_szSUBTITLE, TEXT(""));
      //float f = 1.2F;
      //PEvset(m_hPE, PEP_fFONTSIZEALCNTL, &f, 1);

      // This allows plotting of zero values
      double dNill = GRAPH_NILL;
      PEvset(m_hPE, PEP_fNULLDATAVALUE, &dNill, 1);

      // Sub-divide 8 subsets into two axes
      int nArray[2] = {8,8};
      PEvset (m_hPE, PEP_naMULTIAXESSUBSETS, nArray, 2);

      // Subset labels
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 0,  TEXT("CH00"));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 1,  TEXT("CH01" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 2,  TEXT("CH02" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 3,  TEXT("CH03" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 4,  TEXT("CH04"));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 5,  TEXT("CH05" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 6,  TEXT("CH06" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 7,  TEXT("CH07" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 8,  TEXT("CH08"));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 9,  TEXT("CH09" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 10, TEXT("CH10" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 11, TEXT("CH11" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 12, TEXT("CH12"));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 13, TEXT("CH13" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 14, TEXT("CH14" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 15, TEXT("CH15" ));

      int nSTL[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
      PEvset(m_hPE, PEP_naSUBSETSTOLEGEND, nSTL, m_lanes[0] + m_lanes[1]);

      PEnset(m_hPE, PEP_nLEGENDLOCATION, PELL_RIGHT);
      PEnset(m_hPE, PEP_nLEGENDSTYLE, PELS_1_LINE_INSIDE_AXIS);
      PEnset(m_hPE, PEP_bSHOWLEGEND, TRUE);
      PEnset(m_hPE, PEP_nMULTIAXESSIZING, TRUE);
      PEnset(m_hPE, PEP_nMULTIAXISSTYLE, PEMAS_SEPARATE_AXES);

      PEszset(m_hPE, PEP_szXAXISLABEL, TEXT("Seconds (s)"));

      PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Amplitude (V)"));
      PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Amplitude (V)"));

      // Add Some Padding on Right and Bottom
      PEnset(m_hPE, PEP_nIMAGEADJUSTLEFT, 50);
      PEnset(m_hPE, PEP_nIMAGEADJUSTRIGHT, 50);
      PEnset(m_hPE, PEP_nIMAGEADJUSTBOTTOM, 50);
      PEnset(m_hPE, PEP_nIMAGEADJUSTTOP, 50);

      // Improves metafile export
      PEnset(m_hPE, PEP_nDPIX, 600);
      PEnset(m_hPE, PEP_nDPIY, 600);

      // Set RenderEngine to Gdi Plus
      PEnset(m_hPE, PEP_nRENDERENGINE, PERE_HYBRID);
		PEnset(m_hPE, PEP_bANTIALIASTEXT, TRUE);
		PEnset(m_hPE, PEP_bANTIALIASGRAPHICS, TRUE);
      PEnset(m_hPE, PEP_nCOMPOSITE2D3D, PEC2D_FOREGROUND);

      ::InvalidateRect(m_hPE, NULL, FALSE);
   }
}

//
// CConView Message Handlers
//

void CGraphWnd::Plot(PPLOT pPlot)
{
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();

   // Enter Critical Section
   EnterCriticalSection(&m_hMutex);

   memcpy_s(&m_plot, sizeof(PLOT), pPlot, sizeof(PLOT));
   delete pPlot;

   if (IsWindowVisible() == TRUE) {
      PlotGraph();
   }
   else {
      // Delete Plot Data otherwise
      switch(m_plot.plot) {
      case GRAPH_TYPE_NONE :
         break;
      case GRAPH_TYPE_CPU_MEM :
         delete m_plot.pDatX;
         delete m_plot.pDatY;
         break;
      case GRAPH_TYPE_DAQ :
         delete m_plot.pDatX;
         delete m_plot.pDatY;
         break;
      }
   }

   // Leave Critical Section
   LeaveCriticalSection(&m_hMutex);

}

void CGraphWnd::PlotGraph(void)
{
   // OK to Plot
   if (m_cpGphType == m_plot.plot) {
      // Select Plot Type
      switch(m_plot.plot) {
      case GRAPH_TYPE_NONE :
         break;
      case GRAPH_TYPE_CPU_MEM :
         PlotMemory();
         delete m_plot.pDatX;
         delete m_plot.pDatY;
         break;
      case GRAPH_TYPE_DAQ :
         PlotDAQ();
         delete m_plot.pDatX;
         delete m_plot.pDatY;
         break;
      }
   }
   else {
      // Delete Plot Data otherwise
      switch(m_plot.plot) {
      case GRAPH_TYPE_NONE :
         break;
      case GRAPH_TYPE_CPU_MEM :
         delete m_plot.pDatX;
         delete m_plot.pDatY;
         break;
      case GRAPH_TYPE_DAQ :
         delete m_plot.pDatX;
         delete m_plot.pDatY;
         break;
      }
   }
}

void CGraphWnd::PlotMemory(void)
{
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();

   double   xMin, xMax, yMin, yMax;

   SetWindowText(GRAPH_NAME_CPU_MEM);

   // Reset X Index and Subsets
   if (m_plot.record == 0 || m_lanes[0] != m_plot.lanes[0] || 
       m_lanes[0] != m_plot.lanes[0] || m_XDataLen != m_plot.window || 
       m_plot.type != m_type) {
      m_XIndex   = 0;
      m_XDataLen = m_plot.window;
      m_lanes[0] = m_plot.lanes[0];
      m_lanes[1] = m_plot.lanes[1];
      m_type     = m_plot.type;
      SetGraph(1.0);
      // subset colors
      PEvsetEx(m_hPE, PEP_dwaSUBSETCOLORS, 0, m_lanes[0] + m_lanes[1], clrList, 0);
      // Sub-divide 8 subsets into two axes
      int nArray[2] = {m_lanes[0], m_lanes[1]};
      PEvset (m_hPE, PEP_naMULTIAXESSUBSETS, nArray, 2);
      // Subset labels
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 0,  TEXT("CH00"));
      int nSTL[] = {0};
      PEvset(m_hPE, PEP_naSUBSETSTOLEGEND, nSTL, m_lanes[0] + m_lanes[1]);
      PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Memory"));
      PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Memory"));
   }

   // Show Legends
   if (m_plot.legend) 
      PEnset(m_hPE, PEP_bSHOWLEGEND, TRUE);
   else
      PEnset(m_hPE, PEP_bSHOWLEGEND, FALSE);

   // Show Annotations
   PEnset(m_hPE, PEP_bSHOWANNOTATIONS, TRUE);

   PEnset(m_hPE, PEP_nMANUALSCALECONTROLX, PEMSC_MINMAX);
   xMin = m_plot.xMin[0]; xMax = m_plot.xMax[0];
   PEvset(m_hPE, PEP_fMANUALMINX, &xMin, 1);
   PEvset(m_hPE, PEP_fMANUALMAXX, &xMax, 1);

   PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
   if (m_plot.autoScale[1]) {
      PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_NONE);
   }
   else {
      // Manual Scale Y
      PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);
      yMin = m_plot.yMin[1]; yMax = m_plot.yMax[1];
      PEvset(m_hPE, PEP_fMANUALMINY, &yMin, 1);
      PEvset(m_hPE, PEP_fMANUALMAXY, &yMax, 1);
   }

   PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
   if (m_plot.autoScale[0]) {
      PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_NONE);
   }
   else {
      // Manual Scale Y
      PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);
      yMin = m_plot.yMin[0]; yMax = m_plot.yMax[0];
      PEvset(m_hPE, PEP_fMANUALMINY, &yMin, 1);
      PEvset(m_hPE, PEP_fMANUALMAXY, &yMax, 1);
   }

   PEnset(m_hPE, PEP_bCURSORPROMPTTRACKING, m_cpGphCursor);

   if (m_cpGphPoints) {
      PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_SMALL);
      PEnset(m_hPE, PEP_nMAXIMUMPOINTSIZE, PEMPS_SMALL);
      PEnset(m_hPE, PEP_nMINIMUMPOINTSIZE, PEMPS_SMALL);
   }
   else {
      PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_MEDIUM);
      PEnset(m_hPE, PEP_nMAXIMUMPOINTSIZE, PEMPS_MEDIUM);
      PEnset(m_hPE, PEP_nMINIMUMPOINTSIZE, PEMPS_MEDIUM);
   }

   // Set X-Data
   PEvsetW(m_hPE, PEP_faXDATA, m_plot.pDatX, m_plot.length * (m_lanes[0] + m_lanes[1]));

   // Set Y-Data
   PEvsetW(m_hPE, PEP_faYDATA, m_plot.pDatY, m_plot.length * (m_lanes[0] + m_lanes[1]));

   PEreinitialize(m_hPE);
   PEresetimage(m_hPE, 0, 0);
   ::InvalidateRect(m_hPE, NULL, FALSE);

}

void CGraphWnd::PlotDAQ(void)
{
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();

   double   xMin, xMax, yMin, yMax;
   float    period = (float)1.0 / (float)m_plot.freq;

   SetWindowText(GRAPH_NAME_DAQ);

   // Reset X Index and Subsets
   if ((m_lanes[0] != m_plot.lanes[0]) || 
       (m_XDataLen != m_plot.window) || 
       (m_plot.type != m_type)) {
      m_XIndex   = 0;
      m_XDataLen = m_plot.window;
      m_lanes[0] = m_plot.lanes[0];
      m_lanes[1] = m_plot.lanes[1];
      m_type     = m_plot.type;
      SetGraph(period);
   }
   if (m_plot.type == 1) {
      // subset colors
      PEvsetEx(m_hPE, PEP_dwaSUBSETCOLORS, 0, m_lanes[0] + m_lanes[1], clrList, 0);
      // Sub-divide 8 subsets into two axes
      int nArray[2] = {m_lanes[0], m_lanes[1]};
      PEvset (m_hPE, PEP_naMULTIAXESSUBSETS, nArray, 2);
      // Subset labels
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 0,  TEXT("CH00"));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 1,  TEXT("CH01" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 2,  TEXT("CH02" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 3,  TEXT("CH03" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 4,  TEXT("CH04"));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 5,  TEXT("CH05" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 6,  TEXT("CH06" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 7,  TEXT("CH07" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 8,  TEXT("CH08"));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 9,  TEXT("CH09" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 10, TEXT("CH10" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 11, TEXT("CH11" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 12, TEXT("CH12"));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 13, TEXT("CH13" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 14, TEXT("CH14" ));
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 15, TEXT("CH15" ));
      int nSTL[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
      PEvset(m_hPE, PEP_naSUBSETSTOLEGEND, nSTL, m_lanes[0] + m_lanes[1]);
      PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Amplitude (V)"));
      PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Amplitude (V)"));
      PEszset(m_hPE, PEP_szXAXISLABEL, TEXT("Seconds (s)"));
   }
   else if (m_plot.type == 2) {
      // channel color
      PEvsetEx(m_hPE, PEP_dwaSUBSETCOLORS, 0, 1, &clrList[m_plot.chan], 0);
      // Sub-divide 8 subsets into two axes
      int nArray[2] = {m_lanes[0], m_lanes[1]};
      PEvset (m_hPE, PEP_naMULTIAXESSUBSETS, nArray, 2);
      // Subset labels
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 0,  chnList[m_plot.chan]);
      int nSTL[] = {0};
      PEvset(m_hPE, PEP_naSUBSETSTOLEGEND, nSTL, m_lanes[0] + m_lanes[1]);
      PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Amplitude (V)"));
      PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Amplitude (V)"));
      PEszset(m_hPE, PEP_szXAXISLABEL, TEXT("Seconds (s)"));
   }
   else if (m_plot.type == 3) {
      // subset colors
      PEvsetEx(m_hPE, PEP_dwaSUBSETCOLORS, 0, m_lanes[0] + m_lanes[1], clrList, 0);
      // Sub-divide 8 subsets into two axes
      int nArray[2] = {m_lanes[0], m_lanes[1]};
      PEvset (m_hPE, PEP_naMULTIAXESSUBSETS, nArray, 2);
      // Subset labels
      PEvsetcell( m_hPE, PEP_szaSUBSETLABELS, 0,  TEXT("LINE 0"));
      int nSTL[] = {0};
      PEvset(m_hPE, PEP_naSUBSETSTOLEGEND, nSTL, m_lanes[0] + m_lanes[1]);
      PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Amplitude (V)"));
      PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
      PEszset(m_hPE, PEP_szYAXISLABEL, TEXT("Amplitude (V)"));
      PEszset(m_hPE, PEP_szXAXISLABEL, TEXT("Pixels"));
   }

   // Show Legends
   if (m_plot.legend) 
      PEnset(m_hPE, PEP_bSHOWLEGEND, TRUE);
   else
      PEnset(m_hPE, PEP_bSHOWLEGEND, FALSE);

   // Show Annotations
   PEnset(m_hPE, PEP_bSHOWANNOTATIONS, TRUE);

   PEnset(m_hPE, PEP_nMANUALSCALECONTROLX, PEMSC_MINMAX);
   xMin = m_plot.xMin[0]; xMax = m_plot.xMax[0];
   PEvset(m_hPE, PEP_fMANUALMINX, &xMin, 1);
   PEvset(m_hPE, PEP_fMANUALMAXX, &xMax, 1);

   PEnset(m_hPE, PEP_nWORKINGAXIS, 1);
   if (m_plot.autoScale[1]) {
      PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_NONE);
   }
   else {
      // Manual Scale Y
      PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);
      yMin = m_plot.yMin[1]; yMax = m_plot.yMax[1];
      PEvset(m_hPE, PEP_fMANUALMINY, &yMin, 1);
      PEvset(m_hPE, PEP_fMANUALMAXY, &yMax, 1);
   }

   PEnset(m_hPE, PEP_nWORKINGAXIS, 0);
   if (m_plot.autoScale[0]) {
      PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_NONE);
   }
   else {
      // Manual Scale Y
      PEnset(m_hPE, PEP_nMANUALSCALECONTROLY, PEMSC_MINMAX);
      yMin = m_plot.yMin[0]; yMax = m_plot.yMax[0];
      PEvset(m_hPE, PEP_fMANUALMINY, &yMin, 1);
      PEvset(m_hPE, PEP_fMANUALMAXY, &yMax, 1);
   }

   PEnset(m_hPE, PEP_bCURSORPROMPTTRACKING, m_cpGphCursor);

   if (m_cpGphPoints) {
      PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_SMALL);
      PEnset(m_hPE, PEP_nMAXIMUMPOINTSIZE, PEMPS_SMALL);
      PEnset(m_hPE, PEP_nMINIMUMPOINTSIZE, PEMPS_SMALL);
   }
   else {
      PEnset(m_hPE, PEP_nPOINTSIZE, PEPS_MEDIUM);
      PEnset(m_hPE, PEP_nMAXIMUMPOINTSIZE, PEMPS_MEDIUM);
      PEnset(m_hPE, PEP_nMINIMUMPOINTSIZE, PEMPS_MEDIUM);
   }

   // Set X-Data
   PEvsetW(m_hPE, PEP_faXDATA, m_plot.pDatX, m_plot.length * (m_lanes[0] + m_lanes[1]));

   // Set Y-Data
   PEvsetW(m_hPE, PEP_faYDATA, m_plot.pDatY, m_plot.length * (m_lanes[0] + m_lanes[1]));

   PEreinitialize(m_hPE);
   PEresetimage(m_hPE, 0, 0);
   ::InvalidateRect(m_hPE, NULL, FALSE);

}

void CGraphWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
   CDockablePane::OnShowWindow(bShow, nStatus);
}
