
#include "stdafx.h"
#include "MainFrm.h"
#include "ParmsWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma warning(disable: 4800) 

static CString  parmTitle[] = DOC_TAG_TITLES;

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CParmsWnd::CParmsWnd()
{
}

CParmsWnd::~CParmsWnd()
{
}

BEGIN_MESSAGE_MAP(CParmsWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
   ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

int CParmsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!GetParmList().Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
		return -1;

	GetToolBar().Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PARMS);
	GetToolBar().LoadToolBar(IDR_PARMS, 0, 0, TRUE);
	GetToolBar().SetPaneStyle(GetToolBar().GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	GetToolBar().SetPaneStyle(GetToolBar().GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	GetToolBar().SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	GetToolBar().SetRouteCommandsViaFrame(FALSE);
	AdjustLayout();

   return 0;
}

void CParmsWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CParmsWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL) return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = GetToolBar().CalcFixedLayout(FALSE, TRUE).cy;
	GetToolBar().SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	GetParmList().SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CParmsWnd::InitParamList()
{
   UINT     i,j = 0;
   CString  str, group, combo, strMin, strMax;
   CString  subItem, str1, ext, filt, desc;
   INT      curPos;
   INT      min, max, val;
   INT      title, lastTitle = -1;

   CMFCPropertyGridProperty* pGrp = NULL;

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();
   
   SetPropListFont();

   GetParmList().RemoveAll();
	GetParmList().EnableHeaderCtrl(FALSE);
	GetParmList().EnableDescriptionArea();
	GetParmList().SetVSDotNetLook();
	GetParmList().MarkModifiedProperties();

   m_parmLen = 0;

   // Load Parameters to Property Grid
   for (i=0;i<pDoc->m_iniLen;i++) {
      curPos = 0;
      if (pDoc->m_pIni[i].tag & DOC_TAG_PARM) {
         str.Format(_T("%s"), pDoc->m_pIni[i].key);
         group = str.Tokenize(_T("."), curPos);
         // New Group Title
         title = (pDoc->m_pIni[i].tag & DOC_TAG_TITLE) >> 4;
         if (lastTitle != title) {
            if (lastTitle != -1) GetParmList().AddProperty(pGrp);
            lastTitle = title;
      	   pGrp = new CMFCPropertyGridProperty(parmTitle[title]);
         }
         // Format the Fields
         str.Format(_T(""));
         str.Format(_T("%s"), pDoc->m_pIni[i].name);
         pDoc->GetValIni(i, str);
         // Filename Read Property
         if (pDoc->m_pIni[i].tag & DOC_TAG_FILE_RD) {
            curPos = 0;
            str1.Format(_T("%s"), pDoc->m_pIni[i].desc);
            desc = str1.Tokenize(_T("&"), curPos);
            ext  = str1.Tokenize(_T("&"), curPos);
            filt = str1.Tokenize(_T("&"), curPos);
	         pGrp->AddSubItem(new CMFCPropertyGridFileProperty(pDoc->m_pIni[i].name, TRUE, str, ext, 0, filt, desc, i));
         }
         // Filename Write Property
         else if (pDoc->m_pIni[i].tag & DOC_TAG_FILE_WR) {
            curPos = 0;
            str1.Format(_T("%s"), pDoc->m_pIni[i].desc);
            desc = str1.Tokenize(_T("&"), curPos);
            ext  = str1.Tokenize(_T("&"), curPos);
            filt = str1.Tokenize(_T("&"), curPos);
	         pGrp->AddSubItem(new CMFCPropertyGridFileProperty(pDoc->m_pIni[i].name, TRUE, str, ext, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filt, desc, i));
         }
         // ComboBox Property
         else if (pDoc->m_pIni[i].tag & DOC_TAG_COMBO) {
            curPos = 0;
            str1.Format(_T("%s"), pDoc->m_pIni[i].desc);
            desc = str1.Tokenize(_T("&"), curPos);
            CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(pDoc->m_pIni[i].name, (_variant_t)str, desc, i);
            combo = str1.Tokenize(_T("&"), curPos);
            while (combo.IsEmpty() == FALSE) {
            	pProp->AddOption(combo);
               combo = str1.Tokenize(_T("&"), curPos);
            }
            pGrp->AddSubItem(pProp);
         	pProp->AllowEdit(FALSE);
         }
         // Spin Control Property
         else if (pDoc->m_pIni[i].tag & DOC_TAG_SPIN) {
            curPos = 0;
            str1.Format(_T("%s"), pDoc->m_pIni[i].desc);
            desc = str1.Tokenize(_T("&"), curPos);
            swscanf_s(str, _T("%d"), &val);
            CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(pDoc->m_pIni[i].name, (_variant_t)val, desc, i);
            strMin = str1.Tokenize(_T("&"), curPos);
            swscanf_s(strMin, _T("%d"), &min);
            strMax = str1.Tokenize(_T("&"), curPos);
            swscanf_s(strMax, _T("%d"), &max);
            pProp->EnableSpinControl(TRUE, min, max);
            pGrp->AddSubItem(pProp);
         }
         // True-False
         else if (pDoc->m_pIni[i].tag & DOC_TAG_BOOL) {
            if (str == _T("0"))
               pGrp->AddSubItem(new CMFCPropertyGridProperty(pDoc->m_pIni[i].name, (_variant_t)false, pDoc->m_pIni[i].desc, i));
            else
               pGrp->AddSubItem(new CMFCPropertyGridProperty(pDoc->m_pIni[i].name, (_variant_t)true, pDoc->m_pIni[i].desc, i));
         }
         // Normal String/Number
         else {
            pGrp->AddSubItem(new CMFCPropertyGridProperty(pDoc->m_pIni[i].name, (_variant_t)str, pDoc->m_pIni[i].desc, i));
         }
         m_parmLen++;
      }
   }

   // Last Group
   if (pGrp != NULL) GetParmList().AddProperty(pGrp);
   
   // Expand All
	GetParmList().ExpandAll(TRUE);
   //GetParmList().GetProperty(0)->Expand(TRUE);
	GetParmList().SetAlphabeticMode(FALSE);

}

void CParmsWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	GetParmList().SetFocus();
}

void CParmsWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CParmsWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	GetParmList().SetFont(&m_fntPropList);
}

LRESULT CParmsWnd::OnPropertyChanged(WPARAM, LPARAM lParam )
{
   UINT     entry;
   CString  str;

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();

   CMFCPropertyGridProperty* pProp = (CMFCPropertyGridProperty*) lParam;

   // Update the Parameter
   entry = (UINT)pProp->GetData();
   if (pDoc->m_pIni[entry].tag & DOC_TAG_BOOL) {
      bool b = pProp->GetValue().boolVal;
      if (b == true)
         str.Format(_T("1"));
      else 
         str.Format(_T("0"));
   }
   else if (pDoc->m_pIni[entry].tag & DOC_TAG_SPIN) {
      str.Format(_T("%d"), pProp->GetValue().intVal);
   }
   else {
      str.Format(_T("%s"), pProp->GetValue().bstrVal);
   }
   pDoc->SetValIni(entry, str);
   pDoc->SetModifiedFlag(TRUE);

   return 0;
}

void CParmsWnd::SetParam(VOID *param)
{
   UINT     i;
   INT      val;
   CString  str;

   CMFCPropertyGridProperty* pProp;

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();

   // Load Parameters to Property Grid
   for (i=0;i<pDoc->m_iniLen;i++) {
      // Search for Match in INI list
      if (param == pDoc->m_pIni[i].parm) {
         pProp = GetParmList().FindItemByData(i, TRUE);
         str.Format(_T(""));
         pDoc->GetValIni(i, str);
         // Filename Read Property
         if (pDoc->m_pIni[i].tag & DOC_TAG_FILE_RD) {
            pProp->SetValue((_variant_t)str);
         }
         // Filename Write Property
         else if (pDoc->m_pIni[i].tag & DOC_TAG_FILE_WR) {
            pProp->SetValue((_variant_t)str);
         }
         // ComboBox Property
         else if (pDoc->m_pIni[i].tag & DOC_TAG_COMBO) {
            pProp->SetValue((_variant_t)str);
         }
         // Spin Control Property
         else if (pDoc->m_pIni[i].tag & DOC_TAG_SPIN) {
            swscanf_s(str, _T("%d"), &val);
            pProp->SetValue((_variant_t)val);
         }
         // True-False
         else if (pDoc->m_pIni[i].tag & DOC_TAG_BOOL) {
            if (str == _T("0"))
               pProp->SetValue((_variant_t)false);
            else
               pProp->SetValue((_variant_t)true);
         }
         // Normal String/Number
         else {
            pProp->SetValue((_variant_t)str);
         }
         break;
      }
   }
}

void CParmsWnd::Serialize(CArchive& ar)
{
   CappDoc* pDoc = (CappDoc *)ar.m_pDocument;

   if (ar.IsStoring()) {
   }
   //
   // De-serialize the MainFrame data from storage
   //
   else {
   }
}

void CParmsWnd::OnNewDoc(PINITYPE ini)
{
   if (ini != NULL) {
   }
}

void CParmsWnd::WriteFile(CStdioFile& fp)
{
   UINT     i;
   CString  str;
   CTime    t = CTime::GetCurrentTime();

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc* pDoc = (CappDoc*)pMainFrm->GetActiveDocument();

   str.Format(_T("file.creation =  %s\n"), t.Format(_T("%#c")));
   fp.WriteString(str);

   // Write Tagged Parameters to File
   for (i=0;i<pDoc->m_iniLen;i++) {
      if (pDoc->m_pIni[i].tag & DOC_TAG_PARM) {
         pDoc->GetIni(i, str);
         fp.WriteString(str);
      }
   }
   fp.WriteString(_T("\n"));
}