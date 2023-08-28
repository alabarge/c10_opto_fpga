
// DAQView.cpp : implementation of the CDAQView class
//

#include "stdafx.h"
#include "DAQView.h"
#include "Path.h"

using namespace nsPath;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CM Registrations
static cm_sub_t subs[] = {
   {CM_ID_DAQ_SRV, DAQ_DONE_IND, CM_ID_DAQ_SRV},
   {CM_ID_NULL, NULL, NULL}
};

static TCHAR   szFilterTxt[] = _T("DAQ Files (*.txt)|*.txt|All Files (*.*)|*.*||");
static TCHAR   szFilterBin[] = _T("DAQ Files (*.bin)|*.bin|All Files (*.*)|*.*||");

// CDAQView

IMPLEMENT_DYNCREATE(CDAQView, CViewEx)

BEGIN_MESSAGE_MAP(CDAQView, CViewEx)
   ON_CONTROL_RANGE(EN_CHANGE, IDC_DAQ_FILE, IDC_DAQ_FILE, OnRangeUpdateED)
   ON_CONTROL_RANGE(BN_CLICKED, IDC_DAQ_TO_FILE, IDC_DAQ_RAW, OnCheckUpdateED)
   ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_DAQ_PACKETS, IDC_DAQ_PACKETS, OnRangeUpdateED)
   ON_BN_CLICKED(IDC_DAQ_START, &CDAQView::OnBnClickedDAQStart)
   ON_MESSAGE(WM_CLOSING, &CDAQView::OnClosing)
   ON_WM_TIMER()
   ON_WM_CTLCOLOR()
   ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CDAQView construction/destruction

CDAQView::CDAQView()
	: CViewEx(CDAQView::IDD)
{
   m_daqFile     = _T("");
   m_daqToFile   = 0;
   m_daqHex      = 0;
   m_daqShow     = 0;
   m_daqRamp     = 0;
   m_daqFloat    = 0;
   m_daqPlot     = 0;
   m_daqRaw      = 0;
   m_daqPackets  = _T("");

   m_bRunning = FALSE;

   m_nPktCnt = 0;
   m_nBlkCnt = 0;
   m_seqID = 0;
   m_nXfer = 0;
   m_lastStamp_us = 0;

   m_hrTimer.Start();

   // Space for ADC Samples
   m_pADC = new INT[DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM * DAQ_MAX_CH];

   ::ZeroMemory(m_pADC, (DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM * DAQ_MAX_CH) * sizeof(INT));

   memset(&m_rxq, 0, sizeof(rxq_t));
   m_rxq.thread = NULL;
}

CDAQView::~CDAQView()
{
   if (m_pADC != NULL) delete m_pADC;
}

void CDAQView::DoDataExchange(CDataExchange* pDX)
{
	CViewEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_DAQ_EDIT, m_Edit);
   DDX_Control(pDX, IDC_DAQ_FILE, m_File);
   DDX_Text(pDX, IDC_DAQ_FILE, m_daqFile);
   DDX_Check(pDX, IDC_DAQ_TO_FILE, m_daqToFile);
   DDX_Check(pDX, IDC_DAQ_HEX, m_daqHex);
   DDX_Check(pDX, IDC_DAQ_SHOW, m_daqShow);
   DDX_Check(pDX, IDC_DAQ_RAMP, m_daqRamp);
   DDX_Check(pDX, IDC_DAQ_FLOAT, m_daqFloat);
   DDX_Check(pDX, IDC_DAQ_PLOT, m_daqPlot);
   DDX_Check(pDX, IDC_DAQ_RAW, m_daqRaw);
   DDX_Text(pDX, IDC_DAQ_PACKETS, m_daqPackets);
}

void CDAQView::OnTimer(UINT_PTR nIDEvent)
{
   daq_run_body_t   sync = {0};

   if (nIDEvent == ID_TIMER_DONE_IND) {
      if (m_bRunning) OnBnClickedDAQStart();
      KillTimer(ID_TIMER_DONE_IND);
   }
   CViewEx::OnTimer(nIDEvent);
}

LRESULT CDAQView::OnClosing(WPARAM wParam, LPARAM lParam)
{
   if (m_bRunning) OnBnClickedDAQStart();
   // Cancel Thread
   m_rxq.halt = TRUE;
   Sleep(100);
   return 0;
}

void CDAQView::OnInitialUpdate()
{
   CViewEx::OnInitialUpdate();

   m_sizer.SetAnchor(IDC_DAQ_EDIT, ANCHOR_ALL);
   m_sizer.SetAnchor(IDC_DAQ_BLK_STATIC, ANCHOR_VERTICALLY);

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

   MemToEditRun();

   // Connect to CM
   daq_init();

   // Prevent Focus
   GetDlgItem(IDC_DAQ_BLK_STATIC)->SetFocus();
}

void CDAQView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   // Transfer data to dialog items
   OnUpdateConfig(FALSE);
}

void CDAQView::OnRangeUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CDAQView::OnComboUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CDAQView::OnCheckUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CDAQView::OnUpdateConfig(BOOL bDir)
{
   CappDoc *pDoc = GetDoc();

   if (bDir == TRUE) {
      // Retrieve data from Form
      UpdateData(TRUE);
      wcscpy_s(pDoc->m_ini->daqFile, m_daqFile);
      pDoc->m_ini->daqToFile = m_daqToFile;
      pDoc->m_ini->daqHex = m_daqHex;
      pDoc->m_ini->daqShow = m_daqShow;
      pDoc->m_ini->daqRamp = m_daqRamp;
      pDoc->m_ini->daqFloat = m_daqFloat;
      pDoc->m_ini->daqPlot = m_daqPlot;
      pDoc->m_ini->daqRaw = m_daqRaw;
      swscanf_s(m_daqPackets, _T("%d"), &pDoc->m_ini->daqPackets);
   }
   else {
      m_daqFile.Format(_T("%s"), pDoc->m_ini->daqFile);
      m_daqToFile = pDoc->m_ini->daqToFile;
      m_daqHex = pDoc->m_ini->daqHex;
      m_daqShow = pDoc->m_ini->daqShow;
      m_daqRamp = pDoc->m_ini->daqRamp;
      m_daqFloat = pDoc->m_ini->daqFloat;
      m_daqPlot = pDoc->m_ini->daqPlot;
      m_daqRaw = pDoc->m_ini->daqRaw;
      m_daqPackets.Format(_T("%d"), pDoc->m_ini->daqPackets);
      // Update Form
      UpdateData(FALSE);
   }
}

void CDAQView::OnPlotRun()
{
   CString str;
   CappDoc *pDoc = GetDoc();
   UINT     i,j,k,l,m;
   SHORT    adc;

   FLOAT   *pDatX;
   FLOAT   *pDatY;

   pDatX = new FLOAT[DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM * DAQ_MAX_CH];
   pDatY = new FLOAT[DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM * DAQ_MAX_CH];

   // Send the Plot Message
   PPLOT pPlot = new PLOT;
   ::ZeroMemory(pPlot, sizeof(PLOT));

   // ADC Sampling Frequency
   if (m_adcRate != 0)
      pPlot->freq   = (100E6 / m_adcRate) / DAQ_MAX_CH / 16.0;
   else
      pPlot->freq   = (100E6 / DAQ_RATE_MAX) / DAQ_MAX_CH / 16.0;
   pPlot->chan   = 8;

   //
   // Plot All Channels
   //
   // Fill Plot Array by Packed Channels
   // CH0[0..1983],CH1[0..1983], etc
   for (i=0,l=0,m=0;i<DAQ_MAX_CH;i++) {
      for (j=0;j<DAQ_MAX_PIPE_RUN;j++) {
         for (k=0;k<DAQ_MAX_SAM;k++) {
            if (m_daqRamp) {
               pDatX[m++] = (FLOAT)(((DAQ_MAX_SAM * j) + k) * (1.0 / pPlot->freq));
               pDatY[l++] = (FLOAT)(m_pADC[i + ((DAQ_MAX_LEN * j) + (k * DAQ_MAX_CH))]);
            }
            else {
               pDatX[m++] = (FLOAT)(((DAQ_MAX_SAM * j) + k) * (1.0 / pPlot->freq));
               adc = (SHORT)m_pADC[i + ((DAQ_MAX_LEN * j) + (k * DAQ_MAX_CH))];
               // Sign Extension
               // if (adc & 0x0800) adc |= 0xF800;
               // Convert offset binary Volts
               pDatY[l++] = (FLOAT)(adc * DAQ_LSB);
            }
         }
      }
   }
   pPlot->type     = 1;
   pPlot->lanes[0] = DAQ_MAX_CH / 2;
   pPlot->lanes[1] = DAQ_MAX_CH / 2;
   pPlot->length = (DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM);
   pPlot->window = (DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM);

   pPlot->plot = GRAPH_TYPE_DAQ;

   pPlot->legend = pDoc->m_ini->qplotLegend;
   pPlot->pDatX  = pDatX;
   pPlot->pDatY  = pDatY;
   pPlot->record = m_nPktCnt;
   pPlot->autoScale[0] = pDoc->m_ini->qplotAutoA;
   pPlot->autoScale[1] = pDoc->m_ini->qplotAutoB;

   pPlot->xMin[0] = 0.0;
   pPlot->xMin[1] = 0.0;
   pPlot->xMax[0] = ((DOUBLE)DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM) * ((FLOAT)1.0 / pPlot->freq);
   pPlot->xMax[1] = ((DOUBLE)DAQ_MAX_PIPE_RUN * DAQ_MAX_SAM) * ((FLOAT)1.0 / pPlot->freq);

   pPlot->yMin[0] = pDoc->m_ini->qplotYminA;
   pPlot->yMin[1] = pDoc->m_ini->qplotYminB;
   pPlot->yMax[0] = pDoc->m_ini->qplotYmaxA;
   pPlot->yMax[1] = pDoc->m_ini->qplotYmaxB;

   AfxGetMainWnd()->PostMessage(WM_PLOT_MSG, (WPARAM)pPlot, (LPARAM)0);
}

void CDAQView::MemToEditRun()
{
	CHARFORMAT cf;
   CString  str1,str2, strEdit;
	long     nVisible = 0;
   int      i,j,k,l;

   cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;

   if (!m_daqShow) return;

   if ((m_nPktCnt % 1024) == 0) m_Edit.SetWindowText(_T(""));

   if (m_daqHex) {
      if ((m_nPktCnt % 1024) == 0) {
         str1.Format(_T("CNT     CH0    CH1    CH2    CH3    CH4    CH5    CH6    CH7 "));
         strEdit += str1 + _T("\r\n");
         str1.Format(_T("======  ====== ====== ====== ====== ====== ====== ====== ======"));
         strEdit += str1 + _T("\r\n");
      }
      for (i=0;i<DAQ_MAX_PIPE_RUN;i++) {
         for (l=0,j=0;l<DAQ_MAX_LEN;l+=DAQ_MAX_CH) {
            k = (i * DAQ_MAX_LEN) + l;
            str1.Format(_T("%06X  %06X %06X %06X %06X %06X %06X %06X %06X"), 
               ((m_nPktCnt + i) * DAQ_MAX_SAM) + j++, m_pADC[k+0],  m_pADC[k+1],  m_pADC[k+2], m_pADC[k+3], 
               m_pADC[k+4],   m_pADC[k+5],  m_pADC[k+6], m_pADC[k+7]);
            strEdit += str1 + _T("\r\n");
         }
      }
   }
   else {
      if ((m_nPktCnt % 1024) == 0) {
         str1.Format(_T("CNT       CH0      CH1      CH2      CH3      CH4      CH5      CH6      CH7 "));
         strEdit += str1 + _T("\r\n");
         str1.Format(_T("========  ======== ======== ======== ======== ======== ======== ======== ========"));
         strEdit += str1 + _T("\r\n");
      }
      for (i=0;i<DAQ_MAX_PIPE_RUN;i++) {
         for (l=0,j=0;l<DAQ_MAX_LEN;l+=DAQ_MAX_CH) {
            k = (i * DAQ_MAX_LEN) + l;
            str1.Format(_T("%8d  %8d %8d %8d %8d %8d %8d %8d %8d"), 
               ((m_nPktCnt + i) * DAQ_MAX_SAM) + j++, (UINT)m_pADC[k+0],  (UINT)m_pADC[k+1],  (UINT)m_pADC[k+2],  (UINT)m_pADC[k+3], 
               (UINT)m_pADC[k+4],  (UINT)m_pADC[k+5],  (UINT)m_pADC[k+6],  (UINT)m_pADC[k+7]);
            strEdit += str1 + _T("\r\n");
         }
      }
   }

	m_Edit.SetSel(m_Edit.GetWindowTextLength(), -1);
   cf.crTextColor = APP_CEDIT_LOG;
   m_Edit.SetSelectionCharFormat(cf);
   m_Edit.ReplaceSel(strEdit);
   nVisible = GetLines(m_Edit);
	if (&m_Edit != m_Edit.GetFocus()) {
		m_Edit.LineScroll(INT_MAX);
		m_Edit.LineScroll(1 - nVisible);
	}
}

// CDAQView diagnostics

#ifdef _DEBUG
void CDAQView::AssertValid() const
{
	CViewEx::AssertValid();
}

void CDAQView::Dump(CDumpContext& dc) const
{
	CViewEx::Dump(dc);
}

#endif //_DEBUG

//
// CDAQView Message Handlers
//

HBRUSH CDAQView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   HBRUSH hbr = CViewEx::OnCtlColor(pDC, pWnd, nCtlColor);
   // Text Color for CEdit
   return hbr;
}

void CDAQView::OnShowWindow(BOOL bShow, UINT nStatus)
{
   CViewEx::OnShowWindow(bShow, nStatus);
   m_bShow = bShow;
   // Update CEdit only if Document Attached to View
   if (bShow == TRUE && m_pDocument != NULL) MemToEditRun();
}

void CDAQView::OnPipeMsgRun(pcm_pipe_daq_t pipe)
{
   CString  str,strTime;
   UINT     i,j,k,l,m;
   FLOAT    samf[16];
   INT      sami[16];

   DOUBLE   xferSum = 0.0;
   DOUBLE   deltaTSum = 0.0;
   DOUBLE   xferRate = 0.0;

   USHORT   *pBuf = (USHORT*)pipe;

   CappDoc *pDoc = GetDoc();
   CMainFrame* pMainFrm = (CMainFrame *)AfxGetApp()->GetMainWnd();

   if (m_bRunning == FALSE) return;
      
   // Track Total Transfer Count
   m_nXfer += (INT)(pipe->msglen * sizeof(uint32_t));

   // Store Transfer Rate to Window
   m_xferLen[m_nBlkCnt%10] = (DOUBLE)(pipe->msglen * sizeof(uint32_t));
   m_deltaT[m_nBlkCnt%10]  = (DOUBLE)(((pipe->stamp_us) - (m_lastStamp_us)) * 1E-6);
   m_lastStamp_us = pipe->stamp_us;

   // ADC Rate
   m_adcRate = pipe->rate;

   // Cycle over Pipe Messages
   for (i=0;i<DAQ_MAX_PIPE_RUN;i++) {
      // Cycle over Samples, Packed in channel order
      for (k=0,m=0;k<DAQ_MAX_SAM;k++) {
         for (l=0;l<DAQ_MAX_CH;l++) {
            m_pADC[(i*DAQ_MAX_LEN) + m++] = ((INT)pipe->samples[(k*DAQ_MAX_CH) + l]);
         }
      }
      //
      // Write to File, FORMAT TXT
      //
      if (m_daqToFile && !m_daqHex && !m_daqRaw) {
         for (l=0,j=0;l<DAQ_MAX_LEN;l+=DAQ_MAX_CH) {
            k = (i * DAQ_MAX_LEN) + l;
            if (m_daqFloat) {
               for (m=0;m<DAQ_MAX_CH;m++) samf[m] = (FLOAT)(m_pADC[k+m] * DAQ_LSB);
                  str.Format(_T("  %8d %8E %8E %8E %8E %8E %8E %8E %8E\n"), 
                  ((m_nPktCnt + i) * DAQ_MAX_SAM) + j++, 
                  samf[0],  samf[1],  samf[2],  samf[3],  samf[4],  samf[5],  samf[6],  samf[7]);
               m_pFile.WriteString(str);
            }
            else {
               for (m=0;m<DAQ_MAX_CH;m++) sami[m] = m_pADC[k+m];
               str.Format(_T("  %8d %8d %8d %8d %8d %8d %8d %8d %8d\n"), 
                  ((m_nPktCnt + i) * DAQ_MAX_SAM) + j++, 
                  sami[0],  sami[1],  sami[2],  sami[3],  sami[4],  sami[5],  sami[6],  sami[7]);
               m_pFile.WriteString(str);
            }
         }
      }
      else if (m_daqToFile && !m_daqRaw) {
         for (l=0,j=0;l<DAQ_MAX_LEN;l+=DAQ_MAX_CH) {
            k = (i * DAQ_MAX_LEN) + l;
            str.Format(_T("  %06X %06X %06X %06X %06X %06X %06X %06X %06X\n"), 
               ((m_nPktCnt + i) * DAQ_MAX_SAM) + j++, (UINT)m_pADC[k+0],  (UINT)m_pADC[k+1],  (UINT)m_pADC[k+2],  (UINT)m_pADC[k+3], 
               (UINT)m_pADC[k+4],  (UINT)m_pADC[k+5],  (UINT)m_pADC[k+6],  (UINT)m_pADC[k+7]);
            m_pFile.WriteString(str);
         }
      }
      else if (m_daqToFile) {
         for (l=0;l<sizeof(cm_pipe_daq_t)/2;l+=8) {
            str.Format(_T("%08X   %04X %04X %04X %04X   %04X %04X %04X %04X\n"),
               (UINT)(((m_nPktCnt + i) * sizeof(cm_pipe_daq_t)) + (l * 2)),
               pBuf[l+0], pBuf[l+1], pBuf[l+2], pBuf[l+3],
               pBuf[l+4], pBuf[l+5], pBuf[l+6], pBuf[l+7]);
            m_pFile.WriteString(str);
         }
      }
      // Next pipe message
      pipe  = (pcm_pipe_daq_t)((UCHAR *)pipe + OPTO_PIPELEN_UINT8);
      pBuf  = (USHORT *)pipe;
   }

   MemToEditRun();

   OnPlotRun();

   // Report Transfer Rate
   if (m_nBlkCnt > 10) {
      for (i=0;i<10;i++) {
         xferSum += m_xferLen[i];
         deltaTSum += m_deltaT[i];
      }
      // Compute Average Transfer Rate over Window
      xferRate = xferSum / deltaTSum;
      if (xferRate > 1E6) 
         str.Format(_T("%6.1lf MiB/s"), xferRate / 1E6);
      else if (xferRate > 1E3) 
         str.Format(_T("%6.1lf KiB/s"), xferRate / 1E3);
   }

   m_nBlkCnt++;
   m_nPktCnt+=DAQ_MAX_PIPE_RUN;

   pMainFrm->m_str_status.Format(_T("%8d  : %s"), m_nPktCnt, str);
   pMainFrm->PostMessage(WM_STATUS_BAR, (WPARAM)MF_SBAR_COUNT, (LPARAM)m_bShow);

}

void CDAQView::OnBnClickedDAQStart()
{
   CString     str, inLine;
   time_t      szClock;
   UINT        i;

   cm_send_t   ps = {0};
   pcmq_t slot = cm_alloc();
   pdaq_run_msg_t msg = (pdaq_run_msg_t)slot->buf;
   if (slot == NULL) return;

   CappDoc *pDoc = GetDoc();
   CMainFrame* pMainFrm = (CMainFrame *)AfxGetApp()->GetMainWnd();

   //
   //  STOP
   //
   if (m_bRunning) {
      m_bRunning = FALSE;
      GetDlgItem(IDC_DAQ_START)->SetWindowText(_T("Start"));

      // Send the Request
      msg->p.srvid   = CM_ID_DAQ_SRV;
      msg->p.msgid   = DAQ_RUN_REQ;
      msg->p.flags   = DAQ_NO_FLAGS;
      msg->p.status  = DAQ_OK;
      msg->b.opcode  = DAQ_CMD_STOP;
      ps.msg         = (pcm_msg_t)msg;
      ps.dst_cmid    = CM_ID_DAQ_SRV;
      ps.src_cmid    = CM_ID_DAQ_CLI;
      ps.msglen      = sizeof(daq_run_msg_t);
      cm_send(CM_MSG_REQ, &ps);

      // Un-Register for all Pipe Messages
      cm_pipe_reg(CM_ID_DAQ_CLI, 0, 0, CM_DEV_NULL);

      // Close the Output File
      if (m_pFile.m_hFile != CFile::hFileNull) {
         m_pFile.Close();
      }

   }
   //
   //  START
   //
   else {

      m_nPktCnt = 0;
      m_nBlkCnt = 0;
      m_nXfer = 0;
      m_lastStamp_us = 0;

      // Clear Rate Window
      for (i=0;i<10;i++) {
         m_xferLen[i] = 0.0;
         m_deltaT[i] = 0.0;
      }

      // Clear the Pipe Head index
      if (pMainFrm->m_nCom == APP_CON_FIFO) fifo_head();
      if (pMainFrm->m_nCom == APP_CON_LAN)  lan_head();
      if (pMainFrm->m_nCom == APP_CON_UDP)  udp_head();
      if (pMainFrm->m_nCom == APP_CON_OPTO) opto_head();
      if (pMainFrm->m_nCom == APP_CON_COM)  com_head();

      // Open the File for Writing
      if (m_daqToFile) {
         time(&szClock);
         localtime_s(&m_t, &szClock);
         CPath path(pDoc->m_ini->daqFile);

         // Time Stamp
         if (!CString(pDoc->m_ini->daqFile).IsEmpty()) {
            m_capFile.Format(_T("%s%02d%02d%02d_%02d%02d%02d_%s"), path.GetPath(), m_t.tm_year-100, m_t.tm_mon+1,
                              m_t.tm_mday, m_t.tm_hour, m_t.tm_min, m_t.tm_sec, path.GetName());
            m_pFile.Open(m_capFile, CFile::modeCreate | CFile::modeWrite);
         }
      }

      if (m_daqToFile && m_pFile.m_hFile == CFile::hFileNull) {
         str.Format(_T("Error : File %s did not open for writing\n"), pDoc->m_ini->daqFile);
         GetDoc()->Log(str, APP_MSG_ERROR);
      }
      else {
         // Fill Body
         msg->b.opcode  = DAQ_CMD_RUN | DAQ_CMD_SEND_IND;
         msg->b.opcode |= (m_daqRamp) ? DAQ_CMD_RAMP : 0;
         // scan can only be used for OPTO rates or higher
         // this is handled in firmware
         msg->b.opcode |= DAQ_CMD_SCAN;
         msg->b.opcode |= DAQ_CMD_HEAD;
         msg->b.opcode |= DAQ_CMD_CH_ALL;
         msg->b.packets = pDoc->m_ini->daqPackets;
         // When using a fast com port, it's single pipe message contains 32 1K packets
         // so set the pool count appropriately for ADC speed
         cm_pipe_reg(CM_ID_DAQ_CLI, CM_PIPE_DAQ_DATA, 1, CM_DEV_WIN);
         // Send the Request
         msg->p.srvid   = CM_ID_DAQ_SRV;
         msg->p.msgid   = DAQ_RUN_REQ;
         msg->p.flags   = DAQ_NO_FLAGS;
         msg->p.status  = DAQ_OK;
         ps.msg         = (pcm_msg_t)msg;
         ps.dst_cmid    = CM_ID_DAQ_SRV;
         ps.src_cmid    = CM_ID_DAQ_CLI;
         ps.msglen      = sizeof(daq_run_msg_t);
         cm_send(CM_MSG_REQ, &ps);
         // Send ADC Sync in 50 mS
         SetTimer(ID_TIMER_ADC_SYNC, 50, NULL);
         // Write Parameters to File if Not Scanning
         if (m_daqToFile && !m_daqHex) {
            str.Format(_T("# opCode : %08X\n"), msg->b.opcode);
            m_pFile.WriteString(str);
            if (m_daqFloat) {
               str.Format(_T("# CNT      CH0          CH1          CH2          CH3          CH4          CH5          CH6          CH7           \n"));
               m_pFile.WriteString(str);
               str.Format(_T("# ======== ============ ============ ============ ============ ============ ============ ============ ============ \n\n"));
               m_pFile.WriteString(str);
            }
            else {
               str.Format(_T("# CNT      CH0      CH1      CH2      CH3      CH4      CH5      CH6      CH7      \n"));
               m_pFile.WriteString(str);
               str.Format(_T("# ======== ======== ======== ======== ======== ======== ======== ======== ======== \n\n"));
               m_pFile.WriteString(str);
            }
         }
         m_bRunning = TRUE;
         GetDlgItem(IDC_DAQ_START)->SetWindowText(_T("Stop"));
      }
   }

   // Prevent Focus
   GetDlgItem(IDC_DAQ_BLK_STATIC)->SetFocus();

}

//
// CM RELATED FUNCTIONS
//

uint32_t CDAQView::daq_init(void) {
      
   // Register this Client
   m_cmid   = CM_ID_DAQ_CLI;
   m_handle = cm_register(m_cmid, daq_qmsg_static, (void *)this, subs);

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
//   m_rxq.thread = CreateThread(NULL, 0, &daq_thread_start, (void *)this, 0, &m_rxq.tid);

   AfxBeginThread(daq_thread_start, (void *)this);

   return CP_OK;
}

uint32_t CDAQView::daq_msg(pcm_msg_t msg) {

   uint32_t    result = CP_OK;
   uint16_t    cm_msg  = MSG(msg->p.srvid, msg->p.msgid);
   pcm_pipe_t  pipe    = (pcm_pipe_t)msg;

   CString     str;

   CMainFrame* pMainFrm = (CMainFrame *)AfxGetApp()->GetMainWnd();
   CappDoc *pDoc = GetDoc();

   //
   // HANDLE PIPE MESSAGES
   //
   if (pipe->dst_cmid == CM_ID_PIPE) {
      //
      // DAQ Pipe Messages
      //
      if (pipe->msgid == CM_PIPE_DAQ_DATA) {
         OnPipeMsgRun((pcm_pipe_daq_t)msg);
      }
   }
   //
   // HANDLE CTL MESSAGES
   //
   else {

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
      // Done Indication
      //
      else if (cm_msg == MSG(CM_ID_DAQ_SRV, DAQ_DONE_IND)) {
         SetTimer(ID_TIMER_DONE_IND, 20, NULL);
      }

      // Release the Slot
      cm_free(msg);
   }

   return result;
}

uint32_t CDAQView::daq_timer(pcm_msg_t msg) {

   uint32_t    result = CP_OK;
   uint16_t    cm_msg  = MSG(msg->p.srvid, msg->p.msgid);

   // Release the Slot
   cm_free(msg);

   return result;

}

uint32_t CDAQView::daq_qmsg_static(pcm_msg_t msg, void *member) {
   CDAQView *This = (CDAQView *)member;
   return This->daq_qmsg(msg);
}

uint32_t CDAQView::daq_qmsg(pcm_msg_t msg) {

   uint32_t    result = DAQ_OK;

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

UINT CDAQView::daq_thread_start(LPVOID data) {
   CDAQView *This = (CDAQView *)data;
   return This->daq_thread();
}

DWORD CDAQView::daq_thread(void) {

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
      if (msg != NULL) daq_msg(msg);

   }

   return 0;

} // end cp_thread()
