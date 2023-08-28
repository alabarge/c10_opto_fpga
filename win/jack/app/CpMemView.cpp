
// CpMemView.cpp : implementation of the CCpMemView class
//

#include "stdafx.h"
#include "cpMemView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CM Registrations
static const cm_sub_t subs[] = {
   {CM_ID_NULL, NULL, NULL}
};

static TCHAR   szFilterTxt[] = _T("Memory Files (*.txt)|*.txt|All Files (*.*)|*.*||");
static TCHAR   szFilterBin[] = _T("Memory Files (*.bin)|*.bin|All Files (*.*)|*.*||");
static UINT    typeLen[]  = {256, 256, 128, 128, 64, 64, 64};
static DOUBLE  Mean[]     = {128.0, 0.0, 32768.0, 0.0, 2147483648.0, 0.0, 0.0};
static DOUBLE  STDEV[]    = {12.8, 12.8, 3276.8, 3276.8, 214748364.8, 214748364.8, 12.8};
static UINT    samLen[]   = {0, 1350, 1350, 1350, 1352, 1350, 1350, 1351, 1352};
static LASTREQ lastReq    = {0};

// CCpMemView

IMPLEMENT_DYNCREATE(CCpMemView, CViewEx)

BEGIN_MESSAGE_MAP(CCpMemView, CViewEx)
   ON_CONTROL_RANGE(EN_CHANGE, IDC_CP_MEM_FILE, IDC_CP_MEM_LENGTH, OnRangeUpdateED)
   ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_CP_MEM_START, IDC_CP_MEM_LENGTH, OnRangeUpdateED)
   ON_CONTROL_RANGE(BN_CLICKED, IDC_CP_MEM_BIN, IDC_CP_MEM_TO_FILE, OnCheckUpdateED)
   ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_CP_MEM_TYPE, IDC_CP_MEM_TYPE, OnComboUpdateED)
   ON_BN_CLICKED(IDC_CP_MEM_READ, &CCpMemView::OnBnClickedCpMemRead)
   ON_BN_CLICKED(IDC_CP_MEM_WRITE, &CCpMemView::OnBnClickedCpMemWrite)
   ON_BN_CLICKED(IDC_CP_MEM_GET, &CCpMemView::OnBnClickedCpMemGet)
   ON_BN_CLICKED(IDC_CP_MEM_SET, &CCpMemView::OnBnClickedCpMemSet)
   ON_BN_CLICKED(IDC_CP_MEM_CLEAR, &CCpMemView::OnBnClickedCpMemClear)
   ON_BN_CLICKED(IDC_CP_MEM_TEST, &CCpMemView::OnBnClickedCpMemTest)
   ON_MESSAGE(WM_CLOSING, &CCpMemView::OnClosing)
   ON_WM_TIMER()
   ON_WM_CTLCOLOR()
   ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CCpMemView construction/destruction

CCpMemView::CCpMemView()
	: CViewEx(CCpMemView::IDD)
{
   m_cpMemFile    = _T("");
   m_cpMemStart   = _T("");
   m_cpMemLength  = _T("");
   m_cpMemType    = CFG_INT8U;
   m_cpMemBin     = 0;
   m_cpMemHex     = 0;
   m_cpMemAll     = 0;
   m_cpMemCycle   = 0;
   m_cpMemAWGN    = 0;
   m_cpMemVerify  = 0;
   m_cpMemShow    = 0;
   m_cpMemToFile  = 0;

   m_bGetting = FALSE;
   m_bSetting = FALSE;
   m_bTesting = FALSE;
   m_bStreaming = FALSE;
   m_passNumber = 0;
   m_nXfer = 0;
   m_lenDel = 0;
   m_bShow = FALSE;
   m_bOnce = FALSE;
   m_nError = FALSE;
   m_nPktCnt = 0;
   m_nSeqID = 0;

   memset(&m_rxq, 0, sizeof(rxq_t));
   m_rxq.thread = NULL;
}

CCpMemView::~CCpMemView()
{
}

void CCpMemView::DoDataExchange(CDataExchange* pDX)
{
	CViewEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_CP_MEM_EDIT, m_Edit);
   DDX_Control(pDX, IDC_CP_MEM_FILE, m_File);
   DDX_Text(pDX, IDC_CP_MEM_FILE, m_cpMemFile);
   DDX_Text(pDX, IDC_CP_MEM_START, m_cpMemStart);
   DDX_Text(pDX, IDC_CP_MEM_LENGTH, m_cpMemLength);
   DDX_CBIndex(pDX, IDC_CP_MEM_TYPE, m_cpMemType);
   DDX_Check(pDX, IDC_CP_MEM_BIN, m_cpMemBin);
   DDX_Check(pDX, IDC_CP_MEM_HEX, m_cpMemHex);
   DDX_Check(pDX, IDC_CP_MEM_ALL, m_cpMemAll);
   DDX_Check(pDX, IDC_CP_MEM_CYCLE, m_cpMemCycle);
   DDX_Check(pDX, IDC_CP_MEM_AWGN, m_cpMemAWGN);
   DDX_Check(pDX, IDC_CP_MEM_VERIFY, m_cpMemVerify);
   DDX_Check(pDX, IDC_CP_MEM_SHOW, m_cpMemShow);
   DDX_Check(pDX, IDC_CP_MEM_TO_FILE, m_cpMemToFile);
}

LRESULT CCpMemView::OnClosing(WPARAM wParam, LPARAM lParam)
{
   // Cancel Thread
   m_rxq.halt = TRUE;
   Sleep(100);
   return 0;
}

void CCpMemView::OnTimer(UINT_PTR nIDEvent)
{
   CString  str;
   CMainFrame *pMainFrm = (CMainFrame *)AfxGetApp()->GetMainWnd();
   cm_send_t   ps   = {0};

   if (nIDEvent == ID_TIMER_MSG_TIMEOUT) {
      //pcmq_t      slot = cm_alloc();
      //pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;
      //if (slot != NULL) {
      //   pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;
      //   msg->p.srvid   = lastReq.srvID;
      //   msg->p.msgid   = lastReq.msgID;
      //   msg->p.flags   = lastReq.flags;
      //   msg->p.status  = lastReq.status;
      //   ps.msg         = (pcm_msg_t)msg;
      //   ps.dst_cmid    = lastReq.srvID;
      //   ps.src_cmid    = lastReq.cliID;
      //   ps.msglen      = sizeof(cp_block_msg_t);
      //   // Send the Request
      //   cm_send(CM_MSG_REQ, &ps);
      //}
      str.Format(_T("Warning : Response Timeout, srvID:msgID = %02X:%02X\n"), lastReq.srvID, lastReq.msgID);
      GetDoc()->Log(str, APP_MSG_WARNING);
      KillTimer(ID_TIMER_MSG_TIMEOUT);
   }

   CViewEx::OnTimer(nIDEvent);
}

void CCpMemView::OnInitialUpdate()
{
   CViewEx::OnInitialUpdate();

   m_sizer.SetAnchor(IDC_CP_MEM_EDIT, ANCHOR_ALL);
   m_sizer.SetAnchor(IDC_CP_MEM_BLK_STATIC, ANCHOR_VERTICALLY);

   // RichEdit Font, Clear and Update
   m_Edit.SetFont(&m_font);
   m_Edit.SetWindowText(_T(""));

   // RichEdit Margins
   PARAFORMAT pf;
   pf.cbSize = sizeof(PARAFORMAT);
   pf.dwMask = PFM_STARTINDENT | PFM_RIGHTINDENT;
   pf.dxStartIndent = APP_CEDIT_MARGINS;
   pf.dxRightIndent = APP_CEDIT_MARGINS;
   m_Edit.SetParaFormat(pf);

   // Browsing Mode
   m_File.EnableFileBrowseButton();

   MemToEditBox();

   // Connect to CM
   cpm_init();

   // Prevent Focus
   GetDlgItem(IDC_CP_MEM_BLK_STATIC)->SetFocus();
}

void CCpMemView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   // Transfer data to dialog items
   OnUpdateConfig(FALSE);
}

void CCpMemView::OnRangeUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CCpMemView::OnComboUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
   if (nID == IDC_CP_MEM_TYPE) {
      MemToEditBox();
      OnPlot();
   }

}

void CCpMemView::OnCheckUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
   if ((nID == IDC_CP_MEM_HEX) ||
       (nID == IDC_CP_MEM_ALL)) {
      MemToEditBox();
   }
}

void CCpMemView::OnUpdateConfig(BOOL bDir)
{
   INT64   cpINT = 0;
   UINT64  cpUINT = 0;

   CappDoc *pDoc = GetDoc();

   if (bDir == TRUE) {
      // Retrieve data from Form
      UpdateData(TRUE);
      wcscpy_s(pDoc->m_ini->cpMemFile, m_cpMemFile);
      pDoc->m_ini->cpMemBin = m_cpMemBin;
      pDoc->m_ini->cpMemHex = m_cpMemHex;
      pDoc->m_ini->cpMemAll = m_cpMemAll;
      pDoc->m_ini->cpMemCycle = m_cpMemCycle;
      pDoc->m_ini->cpMemAWGN = m_cpMemAWGN;
      pDoc->m_ini->cpMemVerify = m_cpMemVerify;
      pDoc->m_ini->cpMemShow = m_cpMemShow;
      pDoc->m_ini->cpMemToFile = m_cpMemToFile;
      swscanf_s(m_cpMemStart, _T("%x"), &pDoc->m_ini->cpMemStart);
      swscanf_s(m_cpMemLength, _T("%x"), &pDoc->m_ini->cpMemLength);
      pDoc->m_ini->cpMemType = m_cpMemType;
   }
   else {
      m_cpMemFile.Format(_T("%s"), pDoc->m_ini->cpMemFile);
      m_cpMemBin = pDoc->m_ini->cpMemBin;
      m_cpMemHex = pDoc->m_ini->cpMemHex;
      m_cpMemAll = pDoc->m_ini->cpMemAll;
      m_cpMemCycle = pDoc->m_ini->cpMemCycle;
      m_cpMemAWGN = pDoc->m_ini->cpMemAWGN;
      m_cpMemVerify = pDoc->m_ini->cpMemVerify;
      m_cpMemShow = pDoc->m_ini->cpMemShow;
      m_cpMemToFile = pDoc->m_ini->cpMemToFile;
      m_cpMemStart.Format(_T("0x%08X"), pDoc->m_ini->cpMemStart);
      m_cpMemLength.Format(_T("0x%08X"), pDoc->m_ini->cpMemLength);
      m_cpMemType = pDoc->m_ini->cpMemType;
      // Update Form
      UpdateData(FALSE);
   }
}

void CCpMemView::MemToEditBox()
{
   CappDoc *pDoc = GetDoc();

	CHARFORMAT cf;
	INT nVisible = 0;

   CString  str1,str2, strEdit;
   UINT     i,j;
   UINT     memCount, cols;
   UINT     length = pDoc->m_ini->cpMemLength;
   TCHAR    indexHex[10] = {0}, indexDec[10] = {0};

   UCHAR    *pINT8U  = (UCHAR *)pDoc->m_pCpBuf;
   CHAR     *pINT8S  = (CHAR *)pDoc->m_pCpBuf;
   USHORT   *pINT16U = (USHORT *)pDoc->m_pCpBuf;
   SHORT    *pINT16S = (SHORT *)pDoc->m_pCpBuf;
   UINT     *pINT32U = (UINT *)pDoc->m_pCpBuf;
   INT      *pINT32S = (INT *)pDoc->m_pCpBuf;
   FLOAT    *pFP32   = (FLOAT *)pDoc->m_pCpBuf;

   if (!m_cpMemShow) return;

   if (pDoc->m_ini->cpMemLength > CP_MAX_BUFFER) {
      str1.Format(_T("Error : Memory Length Exceeds Max Buffer, 0x%08X > 0x%08X\n"), pDoc->m_ini->cpMemLength, CP_MAX_BUFFER);
      GetDoc()->Log(str1, APP_MSG_ERROR);
      return;
   }

   cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;

   if (m_cpMemAll != TRUE) {
      length = 256;
      wcscpy_s(indexHex, _T(" %04X  "));
      wcscpy_s(indexDec, _T(" %5d  "));
   }
   else {
      wcscpy_s(indexHex, _T(" %08X  "));
      wcscpy_s(indexDec, _T(" %10d  "));
   }

   switch(m_cpMemType) {
      case  CFG_INT8U:
      case  CFG_INT8S:
         memCount = length;
         cols = 8;
         break;
      case  CFG_INT16U:
      case  CFG_INT16S:
         memCount = length / 2;
         cols = 8;
         break;
      case  CFG_INT32U:
      case  CFG_INT32S:
      case  CFG_FP32:
         memCount = length / 4;
         cols = 4;
         break;
      default:
         m_cpMemType = CFG_INT8U;
         memCount = length;
         cols = 8;
         break;
   }

   // Clear EditBox
   if (m_lenDel != length) {
      m_lenDel = length;
      m_Edit.SetWindowText(_T(""));
   }

   for (i=0;i<memCount;i+=cols) {
      if (m_cpMemHex == TRUE) {
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               str1.Format(indexHex,i);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %02X"), pINT8U[i+j]);
                  str1 += str2;
               }
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               str1.Format(indexHex,i*2);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %04X"), pINT16U[i+j]);
                  str1 += str2;
               }
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               str1.Format(indexHex,i*4);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %08X"), pINT32U[i+j]);
                  str1 += str2;
               }
               break;
         }
      }
      else {
         switch(m_cpMemType) {
            case CFG_INT8U:
               str1.Format(indexDec,i);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %4u"), pINT8U[i+j]);
                  str1 += str2;
               }
               break;
            case CFG_INT8S:
               str1.Format(indexDec,i);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %4d"), pINT8S[i+j]);
                  str1 += str2;
               }
               break;
            case CFG_INT16U:
               str1.Format(indexDec,i*2);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %6u"), pINT16U[i+j]);
                  str1 += str2;
               }
               break;
            case CFG_INT16S:
               str1.Format(indexDec,i*2);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %6d"), pINT16S[i+j]);
                  str1 += str2;
               }
               break;
            case CFG_INT32U:
               str1.Format(indexDec,i*4);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %12u"), pINT32U[i+j]);
                  str1 += str2;
               }
               break;
            case CFG_INT32S:
               str1.Format(indexDec,i*4);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %12d"), pINT32S[i+j]);
                  str1 += str2;
               }
               break;
            case CFG_FP32:
               str1.Format(indexDec,i*4);
               for (j=0;j<cols;j++) {
                  str2.Format(_T(" %10.3f"), pFP32[i+j]);
                  str1 += str2;
               }
               break;
         }
      }
      strEdit += str1 + _T("\r\n");
   }
   // First Line Visible
   cf.crTextColor = APP_CEDIT_LOG;
   m_Edit.SetDefaultCharFormat(cf);
   m_Edit.SetWindowText(strEdit);
   m_Edit.SetSel(0, 0);
}

void CCpMemView::ReadMemData(void)
{
   CappDoc *pDoc = GetDoc();

   CString str1;
   CStdioFile pFile;

   UCHAR    *pINT8U  = (UCHAR *)pDoc->m_pCpBuf;
   USHORT   *pINT16U = (USHORT *)pDoc->m_pCpBuf;
   UINT     *pINT32U = (UINT *)pDoc->m_pCpBuf;

   UINT     k,l;

   if (pDoc->m_ini->cpMemLength > CP_MAX_BUFFER) {
      str1.Format(_T("Error : Memory Length Exceeds Max Buffer, 0x%08X > 0x%08X\n"), pDoc->m_ini->cpMemLength, CP_MAX_BUFFER);
      GetDoc()->Log(str1, APP_MSG_ERROR);
      return;
   }

   if (!CString(pDoc->m_ini->cpMemFile).IsEmpty()) {
      if (m_cpMemBin)
         pFile.Open(pDoc->m_ini->cpMemFile, CFile::typeBinary | CFile::modeRead);
      else
         pFile.Open(pDoc->m_ini->cpMemFile, CFile::modeRead);

      if (pFile.m_hFile != CFile::hFileNull) {

         // Clear Buffer
         memset(pDoc->m_pCpBuf,0,CP_MAX_BUFFER);

         if (m_cpMemBin) {
            pFile.Read(pINT8U, pDoc->m_ini->cpMemLength);
         }
         else {
            if (m_cpMemHex == TRUE) {
               switch(m_cpMemType) {
                  case CFG_INT8U:
                  case CFG_INT8S:
                     k = 0;
                     while (pFile.ReadString(str1)) {
                        str1.TrimRight();
                        if (str1[0] == '[') continue;
                        swscanf_s(str1.GetBuffer(255), _T("%x"), &l);
                        str1.ReleaseBuffer();
                        pINT8U[k++] = l;
                        if (k==CP_MAX_INT8U)
                           break;
                     }
                     break;
                  case CFG_INT16U:
                  case CFG_INT16S:
                     k = 0;
                     while (pFile.ReadString(str1)) {
                        str1.TrimRight();
                        if (str1[0] == '[') continue;
                        swscanf_s(str1.GetBuffer(255), _T("%x"), &l);
                        str1.ReleaseBuffer();
                        pINT16U[k++] = l;
                        if (k==CP_MAX_INT16U)
                           break;
                     }
                     break;
                  case CFG_INT32U:
                  case CFG_INT32S:
                  case CFG_FP32:
                     k = 0;
                     while (pFile.ReadString(str1)) {
                        str1.TrimRight();
                        if (str1[0] == '[') continue;
                        swscanf_s(str1.GetBuffer(255), _T("%x"), &l);
                        str1.ReleaseBuffer();
                        pINT32U[k++] = l;
                        if (k==CP_MAX_INT32U)
                           break;
                     }
                     break;
               }
            }
            else {
               switch(m_cpMemType) {
                  case CFG_INT8U:
                  case CFG_INT8S:
                     k = 0;
                     while (pFile.ReadString(str1)) {
                        str1.TrimRight();
                        if (str1[0] == '[') continue;
                        swscanf_s(str1.GetBuffer(255), _T("%d"), &l);
                        str1.ReleaseBuffer();
                        pINT8U[k++] = l;
                        if (k==CP_MAX_INT8U)
                           break;
                     }
                     break;
                  case CFG_INT16U:
                  case CFG_INT16S:
                     k = 0;
                     while (pFile.ReadString(str1)) {
                        str1.TrimRight();
                        if (str1[0] == '[') continue;
                        swscanf_s(str1.GetBuffer(255), _T("%d"), &l);
                        str1.ReleaseBuffer();
                        pINT16U[k++] = l;
                        if (k==CP_MAX_INT16U)
                           break;
                     }
                     break;
                  case CFG_INT32U:
                  case CFG_INT32S:
                  case CFG_FP32:
                     k = 0;
                     while (pFile.ReadString(str1)) {
                        str1.TrimRight();
                        if (str1[0] == '[') continue;
                        swscanf_s(str1.GetBuffer(255), _T("%d"), &l);
                        str1.ReleaseBuffer();
                        pINT32U[k++] = l;
                        if (k==CP_MAX_INT32U)
                           break;
                     }
                     break;
               }
            }
         }
         pFile.Close();
         MemToEditBox();
      }
      else {
         str1.Format(_T("Error : Unable to Open file %s\n"), pDoc->m_ini->cpMemFile);
         GetDoc()->Log(str1, APP_MSG_ERROR);
      }
   }
   else {
      str1.Format(_T("Error : File name is empty\n"));
      GetDoc()->Log(str1, APP_MSG_ERROR);
   }
}

void CCpMemView::WriteMemData(void)
{
   CappDoc *pDoc = GetDoc();

   CString     str1;
   CStdioFile  pFile;
   UINT        i, memLen = pDoc->m_ini->cpMemLength;

   UCHAR    *pINT8U  = (UCHAR *)pDoc->m_pCpBuf;
   CHAR     *pINT8S  = (CHAR *)pDoc->m_pCpBuf;
   USHORT   *pINT16U = (USHORT *)pDoc->m_pCpBuf;
   SHORT    *pINT16S = (SHORT *)pDoc->m_pCpBuf;
   UINT     *pINT32U = (UINT *)pDoc->m_pCpBuf;
   INT      *pINT32S = (INT *)pDoc->m_pCpBuf;
   FLOAT    *pFP32 = (FLOAT *)pDoc->m_pCpBuf;

   if (pDoc->m_ini->cpMemLength > CP_MAX_BUFFER) {
      str1.Format(_T("Error : Memory Length Exceeds Max Buffer, 0x%08X > 0x%08X\n"), pDoc->m_ini->cpMemLength, CP_MAX_BUFFER);
      GetDoc()->Log(str1, APP_MSG_ERROR);
      return;
   }

   if (!CString(pDoc->m_ini->cpMemFile).IsEmpty()) {
      if (m_cpMemBin)
         pFile.Open(pDoc->m_ini->cpMemFile, CFile::typeBinary | CFile::modeCreate | CFile::modeWrite);
      else
         pFile.Open(pDoc->m_ini->cpMemFile, CFile::modeCreate | CFile::modeWrite);

      if (pFile.m_hFile != CFile::hFileNull) {
         if (m_cpMemBin) {
           pFile.Write(pINT8U, memLen);
         }
         else {
            if (m_cpMemHex == TRUE) {
               switch(m_cpMemType) {
                  case CFG_INT8U:
                  case CFG_INT8S:
                     for (i=0;i<memLen;i++) {
                        str1.Format(_T(" 0x%02X\n"), pINT8U[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
                  case CFG_INT16U:
                  case CFG_INT16S:
                     for (i=0;i<memLen/2;i++) {
                        str1.Format(_T(" 0x%04X\n"), pINT16U[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
                  case CFG_INT32U:
                  case CFG_INT32S:
                  case CFG_FP32:
                     for (i=0;i<memLen/4;i++) {
                        str1.Format(_T(" 0x%08X\n"), pINT32U[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
               }
            }
            else {
               switch(m_cpMemType) {
                  case CFG_INT8U:
                     for (i=0;i<memLen;i++) {
                        str1.Format(_T(" %4u\n"), pINT8U[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
                  case CFG_INT8S:
                     for (i=0;i<memLen;i++) {
                        str1.Format(_T(" %4d\n"), pINT8S[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
                  case CFG_INT16U:
                     for (i=0;i<memLen/2;i++) {
                        str1.Format(_T(" %6u\n"), pINT16U[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
                  case CFG_INT16S:
                     for (i=0;i<memLen/2;i++) {
                        str1.Format(_T(" %6d\n"), pINT16S[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
                  case CFG_INT32U:
                     for (i=0;i<memLen/4;i++) {
                        str1.Format(_T(" %12u\n"), pINT32U[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
                  case CFG_INT32S:
                     for (i=0;i<memLen/4;i++) {
                        str1.Format(_T(" %12d\n"), pINT32S[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
                  case CFG_FP32:
                     for (i=0;i<memLen/4;i++) {
                        str1.Format(_T(" %12.5f\n"), pFP32[i]);
                        pFile.WriteString(str1.GetBuffer(255));
                        str1.ReleaseBuffer();
                     }
                     break;
               }
            }
         }
         pFile.Close();
      }
      else {
         str1.Format(_T("Error : Unable to Open file %s\n"), pDoc->m_ini->cpMemFile);
         GetDoc()->Log(str1, APP_MSG_ERROR);
      }
   }
   else {
      str1.Format(_T("Error : File name is empty\n"));
      GetDoc()->Log(str1, APP_MSG_ERROR);
   }
}

void CCpMemView::OnBlkRdResp(pcm_msg_t pMsg)
{
   CappDoc *pDoc = GetDoc();

   CString  str;
   USHORT   cmMsg = (pMsg->p.srvid << 8) | pMsg->p.msgid;
   UINT     i,j;
   UINT     address, length, type, index;
   UCHAR    *pUCHAR;
   CHAR     *pCHAR;
   USHORT   *pUSHORT;
   SHORT    *pSHORT;
   INT      *pINT;
   UINT     *pUINT;
   FLOAT    *pFLOAT;

   UCHAR    *pINT8U  = (UCHAR *)pDoc->m_pCpBuf;
   CHAR     *pINT8S  = (CHAR *)pDoc->m_pCpBuf;
   USHORT   *pINT16U = (USHORT *)pDoc->m_pCpBuf;
   SHORT    *pINT16S = (SHORT *)pDoc->m_pCpBuf;
   UINT     *pINT32U = (UINT *)pDoc->m_pCpBuf;
   INT      *pINT32S = (INT *)pDoc->m_pCpBuf;
   FLOAT    *pFP32 = (FLOAT *)pDoc->m_pCpBuf;

   cm_send_t   ps   = {0};
   pcp_block_msg_t rsp = (pcp_block_msg_t)pMsg;

   switch(m_cpMemType) {
      case CFG_INT8U:
         pUCHAR = (UCHAR *)rsp->b.data;
         break;
      case CFG_INT8S:
         pCHAR = (CHAR *)rsp->b.data;
         break;
      case CFG_INT16U:
         pUSHORT = (USHORT *)rsp->b.data;
         break;
      case CFG_INT16S:
         pSHORT = (SHORT *)rsp->b.data;
         break;
      case CFG_INT32U:
         pUINT = (UINT *)rsp->b.data;
         break;
      case CFG_INT32S:
         pINT = (INT *)rsp->b.data;
         break;
      case CFG_FP32:
         pFLOAT = (FLOAT *)rsp->b.data;
         pUINT = (UINT *)rsp->b.data;
         break;
   }

   index    = rsp->b.index;
   address  = rsp->b.address;
   length   = rsp->b.length;
   type     = rsp->b.type;

   j = index * length;

   switch(m_cpMemType) {
      case CFG_INT8U:
         if ((length + j) <= CP_MAX_INT8U) {
            for (i=0;i<length;i++) {
               pINT8U[i+j] = pUCHAR[i];
            }
         }
         break;
      case CFG_INT8S:
         if ((length + j) <= CP_MAX_INT8U) {
            for (i=0;i<length;i++) {
               pINT8S[i+j] = pCHAR[i];
            }
         }
         break;
      case CFG_INT16U:
         if ((length + j) <= CP_MAX_INT16U) {
            for (i=0;i<length;i++) {
               pINT16U[i+j] = pUSHORT[i];
            }
         }
         break;
      case CFG_INT16S:
         if ((length + j) <= CP_MAX_INT16U) {
            for (i=0;i<length;i++) {
               pINT16S[i+j] = pSHORT[i];
            }
         }
         break;
      case CFG_INT32U:
         if ((length + j) <= CP_MAX_INT32U) {
            for (i=0;i<length;i++) {
               pINT32U[i+j] = pUINT[i];
            }
         }
         break;
      case CFG_INT32S:
         if ((length + j) <= CP_MAX_INT32U) {
            for (i=0;i<length;i++) {
               pINT32S[i+j] = pINT[i];
            }
         }
         break;
      case CFG_FP32:
         if ((length + j) <= CP_MAX_INT32U) {
            for (i=0;i<length;i++) {
               pFP32[i+j] = pFLOAT[i];
            }
         }
         break;
   }

   // Increment our Received Response
   // Counter
   m_memIndex++;

   // If not Last Response, send the next Request
   if ((m_memIndex != m_numRequest) &&
       (m_bGetting == TRUE)) {

      pcmq_t slot = cm_alloc();

      if (slot != NULL) {
         pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;
         // Fill Body
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               msg->b.type = CFG_INT8U;
               msg->b.length   = 256;
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               msg->b.type = CFG_INT16U;
               msg->b.length   = 128;
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               msg->b.type = CFG_INT32U;
               msg->b.length   = 64;
               break;
         }
         msg->b.index    = m_memIndex;
         msg->b.address  = pDoc->m_ini->cpMemStart + (m_memIndex * 256);
         msg->p.srvid   = CM_ID_CP_SRV;
         msg->p.msgid   = CP_BLOCK_REQ;
         msg->p.flags   = CP_BLOCK_RD;
         msg->p.status  = CP_OK;
         ps.msg         = (pcm_msg_t)msg;
         ps.dst_cmid    = CM_ID_CP_SRV;
         ps.src_cmid    = CM_ID_CP_MEM_CLI;
         ps.msglen      = sizeof(cp_block_msg_t) - CP_BLOCK_MAX;
         // Send the Request
         cm_send(CM_MSG_REQ, &ps);
         // store for retry
         memcpy_s(&lastReq, sizeof(cp_block_msg_t), msg, sizeof(cp_block_msg_t));
         SetTimer(ID_TIMER_MSG_TIMEOUT, 1000, NULL);
      }
   }
   // Final Response Received
   else {
      m_bGetting = FALSE;
      GetDlgItem(IDC_CP_MEM_GET)->SetWindowText(_T("Get"));
      if (m_cpMemAll == TRUE) {
         MemToEditBox();
      }
      if (m_bTesting == TRUE) {
         OnMemValidate();
      }
      // Plot the Data
      OnPlot();
   }

   // Show the Current Received Count
   str.Format(_T("0x%08X"),m_memIndex*256);
   GetDlgItem(IDC_CP_MEM_XFER)->SetWindowText(str);

   // Show the first block
   if (m_cpMemAll != TRUE) {
      if (m_memIndex == 1) {
         MemToEditBox();
      }
   }
}

void CCpMemView::OnBlkWrResp(pcm_msg_t pMsg)
{
   CappDoc *pDoc = GetDoc();

   UINT        result = CP_OK;
   UINT        i,j;
   UINT        address, length, type, index;
   UINT        memLen = pDoc->m_ini->cpMemLength;
   CString     str;

   cm_send_t   ps   = {0};
   pcp_block_msg_t rsp = (pcp_block_msg_t)pMsg;

   UCHAR    *pINT8U  = (UCHAR *)pDoc->m_pCpBuf;
   USHORT   *pINT16U = (USHORT *)pDoc->m_pCpBuf;
   UINT     *pINT32U = (UINT *)pDoc->m_pCpBuf;

   index    = rsp->b.index;
   address  = rsp->b.address;
   length   = rsp->b.length;
   type     = rsp->b.type;

   // Increment our Received Response
   // Counter
   m_memIndex++;

   // If not Last Response, send the next Request
   if ((m_memIndex != m_numRequest) &&
       (m_bSetting == TRUE)) {
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;
         // Fill Body
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               msg->b.type = CFG_INT8U;
               msg->b.length = 256;
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               msg->b.type = CFG_INT16U;
               msg->b.length = 128;
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               msg->b.type = CFG_INT32U;
               msg->b.length = 64;
               break;
         }
         msg->b.index    = m_memIndex;
         msg->b.address  = pDoc->m_ini->cpMemStart + (m_memIndex * 256);
         j = m_memIndex * msg->b.length;
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               for (i=0;i<256;i++) {
                  msg->b.data[i] = pINT8U[i+j];
               }
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               for (i=0;i<128;i++) {
                  ((USHORT*)msg->b.data)[i] = pINT16U[i+j];
               }
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               for (i=0;i<64;i++) {
                  ((UINT*)msg->b.data)[i] = pINT32U[i+j];
               }
               break;
         }
         msg->p.srvid   = CM_ID_CP_SRV;
         msg->p.msgid   = CP_BLOCK_REQ;
         msg->p.flags   = CP_BLOCK_WR;
         msg->p.status  = CP_OK;
         ps.msg         = (pcm_msg_t)msg;
         ps.dst_cmid    = CM_ID_CP_SRV;
         ps.src_cmid    = CM_ID_CP_MEM_CLI;
         ps.msglen      = sizeof(cp_block_msg_t);
         // Send the Request
         cm_send(CM_MSG_REQ, &ps);
         // store for retry
         memcpy_s(&lastReq, sizeof(cp_block_msg_t), msg, sizeof(cp_block_msg_t));
         SetTimer(ID_TIMER_MSG_TIMEOUT, 1000, NULL);
      }
   }
   // Final Response Received
   else {
      m_bSetting = FALSE;
      GetDlgItem(IDC_CP_MEM_SET)->SetWindowText(_T("Set"));
      // If Testing then Get the data that
      // was just written
      if (m_bTesting) {
         OnBnClickedCpMemGet();
      }
   }

   // Show the Current Sent Count
   str.Format(_T("0x%08X"),m_memIndex*256);
   GetDlgItem(IDC_CP_MEM_XFER)->SetWindowText(str);
}

void CCpMemView::OnMemValidate()
{
   CappDoc *pDoc = GetDoc();

   UCHAR*   pINT8U  = (UCHAR *)pDoc->m_pCpBuf;
   CHAR*    pINT8S  = (CHAR *)pDoc->m_pCpBuf;
   USHORT*  pINT16U = (USHORT *)pDoc->m_pCpBuf;
   SHORT*   pINT16S = (SHORT *)pDoc->m_pCpBuf;
   UINT*    pINT32U = (UINT *)pDoc->m_pCpBuf;
   INT*     pINT32S = (INT *)pDoc->m_pCpBuf;
   FP32*    pFP32   = (FP32 *)pDoc->m_pCpBuf;

   UINT     memLen = pDoc->m_ini->cpMemLength;
   UINT     memCnt;
   USHORT   memValue;
   CString  str;
   BOOL     memPassed = TRUE;
   UINT     i,j;
   DOUBLE   stdev, mean, val;

   // New Random Seed
   srand(m_passNumber);
   rand_seed(m_passNumber);

   // Default is Cycling
   m_bTesting = FALSE;

   // Memory Length
   switch(m_cpMemType) {
      case CFG_INT8U:
      case CFG_INT8S:
         memCnt = memLen;
         break;
      case CFG_INT16U:
      case CFG_INT16S:
         memCnt = memLen/2;
         break;
      case CFG_INT32U:
      case CFG_INT32S:
      case CFG_FP32:
         memCnt = memLen/4;
         break;
   }

   // Verify Memory Contents
   if (m_cpMemVerify) {
      // AWGN
      if (m_cpMemAWGN) {
         mean = Mean[m_cpMemType];
         stdev = STDEV[m_cpMemType];
         for (i=0;i<memCnt;i++) {
            val = rand_normal(mean, stdev);
            if (memPassed == FALSE) break;
            switch(m_cpMemType) {
            case CFG_INT8U:
               if (i < CP_MAX_INT8U) {
                  if (pINT8U[i] != (UCHAR)val) {
                     j = pDoc->m_ini->cpMemStart+i;
                     str.Format(_T("Memory Test Error: Pass = %d, Location = %08X, read = %02X, wrote = %02X\n"),
                           m_passNumber, j, pINT8U[i], (UCHAR)val);
                     GetDoc()->Log(str, APP_MSG_ERROR);
                     memPassed = FALSE;
                     // Break for failure
                     m_bTesting = TRUE;
                     break;
                  }
               }
               break;
            case CFG_INT8S:
               if (pINT8S[i] != (CHAR)val) {
                  j = pDoc->m_ini->cpMemStart+i;
                  str.Format(_T("Memory Test Error: Pass = %d, Location = %08X, read = %02X, wrote = %02X\n"),
                        m_passNumber, j, pINT8S[i], (CHAR)val);
                  GetDoc()->Log(str, APP_MSG_ERROR);
                  memPassed = FALSE;
                  // Break for failure
                  m_bTesting = TRUE;
                  break;
               }
               break;
            case CFG_INT16U:
               if (pINT16U[i] != (USHORT)val) {
                  j = pDoc->m_ini->cpMemStart+(i<<1);
                  str.Format(_T("Memory Test Error: Pass = %d, Location = %08X, read = %04X, wrote = %04X\n"),
                        m_passNumber, j, pINT16U[i], (USHORT)val);
                  GetDoc()->Log(str, APP_MSG_ERROR);
                  memPassed = FALSE;
                  // Break for failure
                  m_bTesting = TRUE;
                  break;
               }
               break;
            case CFG_INT16S:
               if (pINT16S[i] != (SHORT)val) {
                  j = pDoc->m_ini->cpMemStart+(i<<1);
                  str.Format(_T("Memory Test Error: Pass = %d, Location = %08X, read = %04X, wrote = %04X\n"),
                        m_passNumber, j, pINT16S[i], (SHORT)val);
                  GetDoc()->Log(str, APP_MSG_ERROR);
                  memPassed = FALSE;
                  // Break for failure
                  m_bTesting = TRUE;
                  break;
               }
               break;
            case CFG_INT32U:
               if (pINT32U[i] != (UINT)val) {
                  j = pDoc->m_ini->cpMemStart+(i<<2);
                  str.Format(_T("Memory Test Error: Pass = %d, Location = %08X, read = %08X, wrote = %08X\n"),
                        m_passNumber, j, pINT32U[i], (UINT)val);
                  GetDoc()->Log(str, APP_MSG_ERROR);
                  memPassed = FALSE;
                  // Break for failure
                  m_bTesting = TRUE;
                  break;
               }
               break;
            case CFG_INT32S:
               if (pINT32S[i] != (INT)val) {
                  j = pDoc->m_ini->cpMemStart+(i<<2);
                  str.Format(_T("Memory Test Error: Pass = %d, Location = %08X, read = %08X, wrote = %08X\n"),
                        m_passNumber, j, pINT32S[i], (INT)val);
                  GetDoc()->Log(str, APP_MSG_ERROR);
                  memPassed = FALSE;
                  // Break for failure
                  m_bTesting = TRUE;
                  break;
               }
               break;
            case CFG_FP32:
               if (pFP32[i] != (FP32)val) {
                  j = pDoc->m_ini->cpMemStart+(i<<2);
                  str.Format(_T("Memory Test Error: Pass = %d, Location = %08X, read = %E, wrote = %E\n"),
                        m_passNumber, j, pFP32[i], (FP32)val);
                  GetDoc()->Log(str, APP_MSG_ERROR);
                  memPassed = FALSE;
                  // Break for failure
                  m_bTesting = TRUE;
                  break;
               }
               break;
            }
         }
      }
      // 16-Bit Unsigned Random
      else {
         // Cycle over memLen
         for (i=0;i<memLen/2;i++) {
            if (i < CP_MAX_INT16U) {
               memValue = (USHORT)(((float)(rand()) / (float)(RAND_MAX)) * 65535.0);
               if (pINT16U[i] != memValue) {
                  j = pDoc->m_ini->cpMemStart+(i<<1);
                  str.Format(_T("Memory Test Error: Pass = %d, Location = %08X, read = %04X, wrote = %04X\n"),
                        m_passNumber, j, pINT16U[i], memValue);
                  GetDoc()->Log(str, APP_MSG_ERROR);
                  memPassed = FALSE;
                  // Break for failure
                  m_bTesting = TRUE;
                  break;
               }
            }
         }
      }
   }

   if (m_cpMemCycle && memPassed == TRUE) {
      OnBnClickedCpMemTest();
   }
   else {
      m_bTesting = FALSE;
      m_bSetting = FALSE;
      m_bGetting = FALSE;
      GetDlgItem(IDC_CP_MEM_TEST)->SetWindowText(_T("Test"));
   }
}

void CCpMemView::OnPlot()
{
   CString  str;
   CappDoc  *pDoc = GetDoc();
   UINT     len,i;

   FLOAT   *pDatX;
   FLOAT   *pDatY;

   INT8U    *p8U   = (INT8U *)pDoc->m_pCpBuf;
   INT8     *p8S   = (INT8 *)pDoc->m_pCpBuf;
   INT16U   *p16U  = (INT16U *)pDoc->m_pCpBuf;
   INT16    *p16S  = (INT16 *)pDoc->m_pCpBuf;
   INT32U   *p32U  = (INT32U *)pDoc->m_pCpBuf;
   INT32    *p32S  = (INT32 *)pDoc->m_pCpBuf;
   FLOAT    *pFP32 = (FLOAT *)pDoc->m_pCpBuf;

   // Validate Maximum Length
   if (pDoc->m_ini->cpMemLength > CP_MAX_BUFFER) {
      str.Format(_T("Error : Memory Length Exceeds Max Buffer, %08X > %08X\n"), pDoc->m_ini->cpMemLength, CP_MAX_BUFFER);
      GetDoc()->Log(str, APP_MSG_ERROR);
      return;
   }

   // Allocate Plot Buffer
   switch(m_cpMemType) {
      case CFG_INT8U:
      case CFG_INT8S:
         len = pDoc->m_ini->cpMemLength;
         break;
      case CFG_INT16U:
      case CFG_INT16S:
         len = pDoc->m_ini->cpMemLength/2;
         break;
      case CFG_INT32U:
      case CFG_INT32S:
      case CFG_FP32:
         len = pDoc->m_ini->cpMemLength/4;
         break;
   }

   pDatX = new FLOAT[len];
   pDatY = new FLOAT[len];

   // Build the Plot Message
   PPLOT pPlot = new PLOT;
   ::ZeroMemory(pPlot, sizeof(PLOT));

   // Add Points to Plot Array
   switch(m_cpMemType) {
      case CFG_INT8U:
         for (i=0;i<len;i++) {
            pDatY[i] = (FLOAT)p8U[i];
            pDatX[i] = (FLOAT)i;
         }
         break;
      case CFG_INT8S:
         for (i=0;i<len;i++) {
            pDatY[i] = (FLOAT)p8S[i];
            pDatX[i] = (FLOAT)i;
         }
         break;
      case CFG_INT16U:
         for (i=0;i<len;i++) {
            pDatY[i] = (FLOAT)p16U[i];
            pDatX[i] = (FLOAT)i;
         }
         break;
      case CFG_INT16S:
         for (i=0;i<len;i++) {
            pDatY[i] = (FLOAT)p16S[i];
            pDatX[i] = (FLOAT)i;
         }
         break;
      case CFG_INT32U:
         for (i=0;i<len;i++) {
            pDatY[i] = (FLOAT)p32U[i];
            pDatX[i] = (FLOAT)i;
         }
         break;
      case CFG_INT32S:
         for (i=0;i<len;i++) {
            pDatY[i] = (FLOAT)p32S[i];
            pDatX[i] = (FLOAT)i;
         }
         break;
      case CFG_FP32:
         for (i=0;i<len;i++) {
            pDatY[i] = (FLOAT)pFP32[i];
            pDatX[i] = (FLOAT)i;
         }
         break;
   }

   pPlot->plot = GRAPH_TYPE_CPU_MEM;
   pPlot->type = m_cpMemType;

   pPlot->lanes[0] = 1;
   pPlot->lanes[1] = 0;
   pPlot->length = len;
   pPlot->window = len;
   pPlot->freq   = 0;
   pPlot->chan   = 0;

   pPlot->legend = pDoc->m_ini->cplotLegend;
   pPlot->pDatX  = pDatX;
   pPlot->pDatY  = pDatY;
   pPlot->record = m_nPktCnt;
   pPlot->autoScale[0] = pDoc->m_ini->cplotAutoA;
   pPlot->autoScale[1] = pDoc->m_ini->cplotAutoB;

   pPlot->xMin[0] = 0.0;
   pPlot->xMin[1] = 0.0;
   pPlot->xMax[0] = len;
   pPlot->xMax[1] = len;

   pPlot->yMin[0] = pDoc->m_ini->cplotYminA;
   pPlot->yMin[1] = pDoc->m_ini->cplotYminB;
   pPlot->yMax[0] = pDoc->m_ini->cplotYmaxA;
   pPlot->yMax[1] = pDoc->m_ini->cplotYmaxB;

   // Send Message
   AfxGetMainWnd()->PostMessage(WM_PLOT_MSG, (WPARAM)pPlot, (LPARAM)0);
}

void CCpMemView::OnPlotPipe(WPARAM wParam, LPARAM lParam)
{
}

// CCpMemView diagnostics

#ifdef _DEBUG
void CCpMemView::AssertValid() const
{
	CViewEx::AssertValid();
}

void CCpMemView::Dump(CDumpContext& dc) const
{
	CViewEx::Dump(dc);
}

#endif //_DEBUG

//
// CCpMemView Message Handlers
//

HBRUSH CCpMemView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   HBRUSH hbr = CViewEx::OnCtlColor(pDC, pWnd, nCtlColor);
   // Text Color for CEdit
   if (pWnd->GetDlgCtrlID() == IDC_CP_MEM_EDIT) {
      pDC->SetTextColor(APP_CEDIT_LOG);
      hbr = m_bgBrush;
   }
   // Grey Text for Transfer
   if (pWnd->GetDlgCtrlID() == IDC_CP_MEM_XFER) {
      pDC->SetTextColor(APP_READ_ONLY_TEXT);
      hbr = m_bgBrush;
   }
   return hbr;
}

void CCpMemView::OnShowWindow(BOOL bShow, UINT nStatus)
{
   CViewEx::OnShowWindow(bShow, nStatus);
   m_bShow = bShow;
   // Update CEdit only if Document Attached to View
   if (bShow == TRUE && m_pDocument != NULL) MemToEditBox();
}

void CCpMemView::OnBnClickedCpMemRead()
{
   TCHAR*   filter = szFilterTxt;
   TCHAR*   ext = _T("txt");
   CappDoc* pDoc = GetDoc();

   if (CString(pDoc->m_ini->cpMemFile).IsEmpty()) {
      if (m_cpMemBin) {
         filter = szFilterBin;
         ext = _T("bin");
      }
      CFileDialog dlg(TRUE, ext, pDoc->m_ini->cpMemFile, OFN_HIDEREADONLY, filter);
      if (dlg.DoModal() == IDOK) {
         CFile pFile;
         if (pFile.Open(dlg.GetPathName(), CFile::modeRead)) {
            pFile.Close();
            wcscpy_s(pDoc->m_ini->cpMemFile, dlg.GetPathName());
            OnUpdateConfig(FALSE);
            GetDoc()->SetModifiedFlag(TRUE);
            ReadMemData();
         }
      }
   }
   else
      ReadMemData();
}

void CCpMemView::OnBnClickedCpMemWrite()
{
   TCHAR*   filter = szFilterTxt;
   TCHAR*   ext = _T("txt");
   CappDoc* pDoc = GetDoc();

   if (CString(pDoc->m_ini->cpMemFile).IsEmpty()) {
      if (m_cpMemBin) {
         filter = szFilterBin;
         ext = _T("bin");
      }
      CFileDialog dlg(FALSE, ext, pDoc->m_ini->cpMemFile, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);
      if (dlg.DoModal() == IDOK) {
         CFile pFile;
         wcscpy_s(pDoc->m_ini->cpMemFile, dlg.GetPathName());
         OnUpdateConfig(FALSE);
         GetDoc()->SetModifiedFlag(TRUE);
         WriteMemData();
      }
   }
   else
      WriteMemData();
}

void CCpMemView::OnBnClickedCpMemGet()
{
   CappDoc *pDoc = GetDoc();

   UINT        result = CP_OK;
   UINT        memLen = pDoc->m_ini->cpMemLength;
   CString     str;

   cm_send_t   ps   = {0};
   pcmq_t      slot = cm_alloc();
   if (slot == NULL) return;

   pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;

   if (m_bGetting) {
      m_bGetting = FALSE;
      GetDlgItem(IDC_CP_MEM_GET)->SetWindowText(_T("Get"));
      KillTimer(ID_TIMER_MSG_TIMEOUT);
   }
   else {
      if (memLen != 0) {
         // Clear Transfer Count
         GetDlgItem(IDC_CP_MEM_XFER)->SetWindowText(_T(""));
         m_bGetting = TRUE;
         if (!m_bTesting) {
            GetDlgItem(IDC_CP_MEM_GET)->SetWindowText(_T("Abort"));
         }
         m_numRequest = 0;

         // Divide the collection into Number
         // of Requests, per Memory Type
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               m_numCollect = memLen;
               m_numRequest = ((m_numCollect + 255) / 256);
               // Insure we don't overflow the CP Buffer
               if ((m_numRequest * 256) > CP_MAX_INT8U) {
                  m_numRequest = CP_MAX_INT8U / 256;
               }
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               m_numCollect = memLen/2;
               m_numRequest = ((m_numCollect + 127) / 128);
               // Insure we don't overflow the DC Buffer
               if ((m_numRequest * 128) > CP_MAX_INT16U) {
                  m_numRequest = CP_MAX_INT16U / 128;
               }
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               m_numCollect = memLen/4;
               m_numRequest = ((m_numCollect + 63) / 64);
               // Insure we don't overflow the DC Buffer
               if ((m_numRequest * 64) > CP_MAX_INT32U) {
                  m_numRequest = CP_MAX_INT32U / 64;
               }
               break;
         }

         // This is the first request
         m_memIndex = 0;

         // Clear the DC Buffer
         ::ZeroMemory(pDoc->m_pCpBuf, CP_MAX_BUFFER);

         // Fill Body
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               msg->b.type = CFG_INT8U;
               msg->b.length   = 256;
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               msg->b.type = CFG_INT16U;
               msg->b.length   = 128;
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               msg->b.type = CFG_INT32U;
               msg->b.length   = 64;
               break;
         }

         msg->b.index    = m_memIndex;
         msg->b.address  = pDoc->m_ini->cpMemStart;

         pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;
         msg->p.srvid   = CM_ID_CP_SRV;
         msg->p.msgid   = CP_BLOCK_REQ;
         msg->p.flags   = CP_BLOCK_RD;
         msg->p.status  = CP_OK;
         ps.msg         = (pcm_msg_t)msg;
         ps.dst_cmid    = CM_ID_CP_SRV;
         ps.src_cmid    = CM_ID_CP_MEM_CLI;
         ps.msglen      = sizeof(cp_block_msg_t) - CP_BLOCK_MAX;
         // Send the Request
         cm_send(CM_MSG_REQ, &ps);
         // store for retry
         memcpy_s(&lastReq, sizeof(cp_block_msg_t), msg, sizeof(cp_block_msg_t));
         SetTimer(ID_TIMER_MSG_TIMEOUT, 1000, NULL);
      }
      else {
         str.Format(_T("Error : Block Read Length is Zero\n"));
         GetDoc()->Log(str, APP_MSG_ERROR);
      }
   }
}

void CCpMemView::OnBnClickedCpMemSet()
{
   CappDoc *pDoc = GetDoc();

   UINT        result = CP_OK;
   UINT        i;
   UCHAR       flags;
   UINT        memLen = pDoc->m_ini->cpMemLength;
   CString     str;

   cm_send_t   ps   = {0};
   pcmq_t      slot = cm_alloc();
   if (slot == NULL) return;

   pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;

   UCHAR    *pINT8U  = (UCHAR *)pDoc->m_pCpBuf;
   USHORT   *pINT16U = (USHORT *)pDoc->m_pCpBuf;
   UINT     *pINT32U = (UINT *)pDoc->m_pCpBuf;

   if (m_bSetting) {
      m_bSetting = FALSE;
      GetDlgItem(IDC_CP_MEM_SET)->SetWindowText(_T("Set"));
      KillTimer(ID_TIMER_MSG_TIMEOUT);
   }
   else {
      if (memLen != 0) {
         m_bSetting = TRUE;
         GetDlgItem(IDC_CP_MEM_SET)->SetWindowText(_T("Abort"));
         m_numRequest = 0;

         // Divide the collection into Number
         // of Requests, per Memory Type
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               m_numCollect = memLen;
               m_numRequest = ((m_numCollect + 255) / 256);
               // Insure we don't overflow the DC Buffer
               if ((m_numRequest * 256) > CP_MAX_INT8U) {
                  m_numRequest = CP_MAX_INT8U / 256;
               }
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               m_numCollect = memLen/2;
               m_numRequest = ((m_numCollect + 127) / 128);
               // Insure we don't overflow the DC Buffer
               if ((m_numRequest * 128) > CP_MAX_INT16U) {
                  m_numRequest = CP_MAX_INT16U / 128;
               }
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               m_numCollect = memLen/4;
               m_numRequest = ((m_numCollect + 63) / 64);
               // Insure we don't overflow the DC Buffer
               if ((m_numRequest * 64) > CP_MAX_INT32U) {
                  m_numRequest = CP_MAX_INT32U / 64;
               }
               break;
         }

         // This is the first request
         m_memIndex = 0;

         // Enable Readback
         flags = CP_BLOCK_WR;

         // Fill Body
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               msg->b.type = CFG_INT8U;
               msg->b.length   = 256;
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               msg->b.type = CFG_INT16U;
               msg->b.length   = 128;
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               msg->b.type = CFG_INT32U;
               msg->b.length   = 64;
               break;
         }

         msg->b.index    = m_memIndex;
         msg->b.address  = pDoc->m_ini->cpMemStart;

         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               for (i=0;i<256;i++) {
                  msg->b.data[i] = pINT8U[i];
               }
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               for (i=0;i<128;i++) {
                  ((USHORT*)msg->b.data)[i] = pINT16U[i];
               }
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               for (i=0;i<64;i++) {
                  ((UINT*)msg->b.data)[i] = pINT32U[i];
               }
               break;
         }

         pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;
         msg->p.srvid   = CM_ID_CP_SRV;
         msg->p.msgid   = CP_BLOCK_REQ;
         msg->p.flags   = CP_BLOCK_WR;
         msg->p.status  = CP_OK;
         ps.msg         = (pcm_msg_t)msg;
         ps.dst_cmid    = CM_ID_CP_SRV;
         ps.src_cmid    = CM_ID_CP_MEM_CLI;
         ps.msglen      = sizeof(cp_block_msg_t);
         // Send the Request
         cm_send(CM_MSG_REQ, &ps);
         // store for retry
         memcpy_s(&lastReq, sizeof(cp_block_msg_t), msg, sizeof(cp_block_msg_t));
         SetTimer(ID_TIMER_MSG_TIMEOUT, 1000, NULL);
      }
      else {
         str.Format(_T("Error : Block Write Length is Zero\n"));
         GetDoc()->Log(str, APP_MSG_ERROR);
      }
   }
}

void CCpMemView::OnBnClickedCpMemClear()
{
   CappDoc *pDoc = GetDoc();
   ::ZeroMemory(pDoc->m_pCpBuf, CP_MAX_BUFFER);
   MemToEditBox();
   m_passNumber = 0;
   m_nXfer = 0;
}

void CCpMemView::OnBnClickedCpMemTest()
{
   CappDoc *pDoc = GetDoc();
   CMainFrame *pMainFrm = (CMainFrame *)AfxGetApp()->GetMainWnd();

   UINT        result = CP_OK;
   UINT        i;
   UINT        memLen = pDoc->m_ini->cpMemLength;
   DOUBLE      stdev, mean, val;
   CString     str;

   cm_send_t   ps   = {0};
   pcmq_t      slot = cm_alloc();
   if (slot == NULL) return;

   pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;

   UCHAR*   pINT8U  = (UCHAR *)pDoc->m_pCpBuf;
   CHAR*    pINT8S  = (CHAR *)pDoc->m_pCpBuf;
   USHORT*  pINT16U = (USHORT *)pDoc->m_pCpBuf;
   SHORT*   pINT16S = (SHORT *)pDoc->m_pCpBuf;
   UINT*    pINT32U = (UINT *)pDoc->m_pCpBuf;
   INT*     pINT32S = (INT *)pDoc->m_pCpBuf;
   FP32*    pFP32   = (FP32 *)pDoc->m_pCpBuf;

   if (m_bTesting) {
      m_bTesting = FALSE;
      m_bSetting = FALSE;
      m_bGetting = FALSE;
      GetDlgItem(IDC_CP_MEM_TEST)->SetWindowText(_T("Test"));
      KillTimer(ID_TIMER_MSG_TIMEOUT);
   }
   else {
      if (memLen != 0) {
         m_bTesting = TRUE;
         m_bSetting = TRUE;
         GetDlgItem(IDC_CP_MEM_TEST)->SetWindowText(_T("Abort"));
         m_numRequest = 0;


         // Clear the CP Buffer
         ::ZeroMemory(pDoc->m_pCpBuf, CP_MAX_BUFFER);

         m_passNumber++;

         srand(m_passNumber);
         rand_seed(m_passNumber);

         pMainFrm->m_str_status.Format(_T("%5d"), m_passNumber);
         pMainFrm->PostMessage(WM_STATUS_BAR, (WPARAM)MF_SBAR_COUNT, (LPARAM)m_bShow);

         // Divide the collection into Number
         // of Requests, per Memory Type
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               m_numCollect = memLen;
               m_numRequest = ((m_numCollect + 255) / 256);
               // Insure we don't overflow the DC Buffer
               if ((m_numRequest * 256) > CP_MAX_INT8U) {
                  m_numRequest = CP_MAX_INT8U / 256;
               }
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               m_numCollect = memLen/2;
               m_numRequest = ((m_numCollect + 127) / 128);
               // Insure we don't overflow the DC Buffer
               if ((m_numRequest * 128) > CP_MAX_INT16U) {
                  m_numRequest = CP_MAX_INT16U / 128;
               }
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               m_numCollect = memLen/4;
               m_numRequest = ((m_numCollect + 63) / 64);
               // Insure we don't overflow the DC Buffer
               if ((m_numRequest * 64) > CP_MAX_INT32U) {
                  m_numRequest = CP_MAX_INT32U / 64;
               }
               break;
         }

         // Additive White Gaussian Noise
         if (m_cpMemAWGN) {
            mean = Mean[m_cpMemType];
            stdev = STDEV[m_cpMemType];
            for (i=0;i<m_numCollect;i++) {
               val = rand_normal(mean, stdev);
               switch(m_cpMemType) {
               case CFG_INT8U:
                  if (i < CP_MAX_INT8U)
                     pINT8U[i] = (UCHAR)val;
                  break;
               case CFG_INT8S:
                     pINT8S[i] = (CHAR)val;
                  break;
               case CFG_INT16U:
                     pINT16U[i] = (USHORT)val;
                  break;
               case CFG_INT16S:
                     pINT16S[i] = (SHORT)val;
                  break;
               case CFG_INT32U:
                     pINT32U[i] = (UINT)val;
                  break;
               case CFG_INT32S:
                     pINT32S[i] = (INT)val;
                  break;
               case CFG_FP32:
                     pFP32[i] = (FP32)val;
                  break;
               }
            }
         }
         // 16-Bit Pseudorandom  Numbers
         else {
            for (i=0;i<memLen/2;i++) {
               if (i < CP_MAX_INT16U)
                  pINT16U[i] = (USHORT)(((float)(rand()) / (float)(RAND_MAX)) * 65535.0);
            }
         }

         // Update EditBox
         MemToEditBox();

         // This is the first request
         m_memIndex = 0;

         // Fill Body
         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               msg->b.type = CFG_INT8U;
               msg->b.length   = 256;
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               msg->b.type = CFG_INT16U;
               msg->b.length   = 128;
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               msg->b.type = CFG_INT32U;
               msg->b.length   = 64;
               break;
         }

         msg->b.index    = m_memIndex;
         msg->b.address  = pDoc->m_ini->cpMemStart;

         switch(m_cpMemType) {
            case CFG_INT8U:
            case CFG_INT8S:
               for (i=0;i<256;i++) {
                  msg->b.data[i] = pINT8U[i];
               }
               break;
            case CFG_INT16U:
            case CFG_INT16S:
               for (i=0;i<128;i++) {
                  ((USHORT*)msg->b.data)[i] = pINT16U[i];
               }
               break;
            case CFG_INT32U:
            case CFG_INT32S:
            case CFG_FP32:
               for (i=0;i<64;i++) {
                  ((UINT*)msg->b.data)[i] = pINT32U[i];
               }
               break;
         }

         pcp_block_msg_t msg = (pcp_block_msg_t)slot->buf;
         msg->p.srvid   = CM_ID_CP_SRV;
         msg->p.msgid   = CP_BLOCK_REQ;
         msg->p.flags   = CP_BLOCK_WR;
         msg->p.status  = CP_OK;
         ps.msg         = (pcm_msg_t)msg;
         ps.dst_cmid    = CM_ID_CP_SRV;
         ps.src_cmid    = CM_ID_CP_MEM_CLI;
         ps.msglen      = sizeof(cp_block_msg_t);
         // Send the Request
         cm_send(CM_MSG_REQ, &ps);
         // store for retry
         memcpy_s(&lastReq, sizeof(cp_block_msg_t), msg, sizeof(cp_block_msg_t));
         SetTimer(ID_TIMER_MSG_TIMEOUT, 1000, NULL);
      }
      else {
         str.Format(_T("Error : Block Write Length is Zero\n"));
         GetDoc()->Log(str, APP_MSG_ERROR);
      }
   }
}

//
// CM RELATED FUNCTIONS
//

uint32_t CCpMemView::cpm_init(void) {

   // Register this Client
   m_cmid   = CM_ID_CP_MEM_CLI;
   m_handle = cm_register(m_cmid, cpm_qmsg_static, (void *)this, NULL);

   // Initialize the RX Queue
   memset(&m_rxq, 0, sizeof(rxq_t));
   for (int i=0;i<APP_RX_QUE;i++) {
      m_rxq.buf[i] = NULL;
   }
   m_rxq.head  = 0;
   m_rxq.tail  = 0;
   m_rxq.slots = APP_RX_QUE;
   m_rxq.halt  = FALSE;
   InitializeConditionVariable (&m_rxq.cv);
   InitializeCriticalSection (&m_rxq.mutex);

   // Start the Message Delivery Thread
   //m_rxq.thread = CreateThread(NULL, 0, &cpm_thread_start, (void *)this, 0, &m_rxq.tid);

   AfxBeginThread(cpm_thread_start, (void *)this);

   return CP_OK;
}

uint32_t CCpMemView::cpm_msg(pcm_msg_t msg) {

   uint32_t    result = CP_OK;
   uint16_t    cm_msg  = MSG(msg->p.srvid, msg->p.msgid);

   CString     str;
   CMainFrame *pMainFrm = (CMainFrame *)AfxGetApp()->GetMainWnd();
   CappDoc    *pDoc = GetDoc();

   // Update Connection Status Indicator
   pMainFrm->m_nComErr = msg->p.status;

   //
   //    NON-ZERO STATUS
   //
   if (msg->p.status != CM_OK) {
      str.Format(_T("Error : CM Message status %02X, srvid %02X, msgid %02X\n"),
         msg->p.status, msg->p.srvid, msg->p.msgid);
      GetDoc()->Log(str, APP_MSG_ERROR);
   }
   //
   //    BLOCK RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_BLOCK_RESP)) {
      if (msg->p.flags & CP_BLOCK_RD) {
            KillTimer(ID_TIMER_MSG_TIMEOUT);
            OnBlkRdResp(msg);
      }
      else if (msg->p.flags & CP_BLOCK_WR) {
            KillTimer(ID_TIMER_MSG_TIMEOUT);
            OnBlkWrResp(msg);
      }
   }

   // Release the Slot
   cm_free(msg);

   return result;
}

uint32_t CCpMemView::cpm_timer(pcm_msg_t msg) {

   uint32_t    result = CP_OK;
   uint16_t    cm_msg  = MSG(msg->p.srvid, msg->p.msgid);

   // Release the Slot
   cm_free(msg);

   return result;

}

uint32_t CCpMemView::cpm_qmsg_static(pcm_msg_t msg, void *member) {
   CCpMemView *This = (CCpMemView *)member;
   return This->cpm_qmsg(msg);
}

uint32_t CCpMemView::cpm_qmsg(pcm_msg_t msg) {

   uint32_t    result = CP_OK;

   // validate message
   if (msg != NULL) {

      // Enter Critical Section
      EnterCriticalSection(&m_rxq.mutex);

      // place in receive queue
      m_rxq.buf[m_rxq.head] = (uint32_t *)msg;
      if (++m_rxq.head == m_rxq.slots) m_rxq.head = 0;

      // Leave Critical Section
      LeaveCriticalSection(&m_rxq.mutex);

      // signal the thread
      WakeConditionVariable (&m_rxq.cv);
   }

   return result;
}

UINT CCpMemView::cpm_thread_start(LPVOID data) {
   CCpMemView *This = (CCpMemView *)data;
   return This->cpm_thread();
}

//DWORD WINAPI CCpMemView::cpm_thread_start(LPVOID data) {
//   CCpMemView *This = (CCpMemView *)data;
//   return This->cpm_thread();
//}

DWORD CCpMemView::cpm_thread(void) {

   pcm_msg_t   msg;

   // Message Receive Loop
   while (m_rxq.halt != TRUE) {

      // Enter Critical Section
      EnterCriticalSection(&m_rxq.mutex);

      // Wait on condition variable,
      // this unlocks the mutex while waiting
      while ((m_rxq.head == m_rxq.tail) && (m_rxq.halt != TRUE)) {
         SleepConditionVariableCS(&m_rxq.cv, &m_rxq.mutex, 50);
      }

      // cancel thread
      if (m_rxq.halt == TRUE) break;

      // clear previous message
      msg = NULL;

      if (m_rxq.head != m_rxq.tail) {
         // Validate message
         if (m_rxq.buf[m_rxq.tail] != NULL) {
            msg = (pcm_msg_t)m_rxq.buf[m_rxq.tail];
            if (++m_rxq.tail == m_rxq.slots) m_rxq.tail = 0;
         }
         // silently drop
         else {
            if (++m_rxq.tail == m_rxq.slots) m_rxq.tail = 0;
         }
      }

      // Leave Critical Section
      LeaveCriticalSection(&m_rxq.mutex);

      // Deliver Message using this thread
      if (msg != NULL) cpm_msg(msg);

   }

   return 0;

} // end cp_thread()
