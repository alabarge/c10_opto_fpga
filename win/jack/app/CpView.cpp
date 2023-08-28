
// CpView.cpp : implementation of the CCpView class
//

#include "stdafx.h"
#include "cpView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Traffic Log
static   inline void  logTraffic(BOOL flag, WPARAM wParam, WPARAM pMsg, LPARAM lParam);

// CM Registrations
static const cm_sub_t subs[] = {
   {CM_ID_NULL, NULL, NULL}
};

// CCpView

IMPLEMENT_DYNCREATE(CCpView, CViewEx)

BEGIN_MESSAGE_MAP(CCpView, CViewEx)
   ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_CP_ADDR, IDC_CP_TRACE, OnRangeUpdateED)
   ON_CONTROL_RANGE(BN_CLICKED, IDC_CP_WR, IDC_CP_UART, OnCheckUpdateED)
   ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_CP_DATA_TYPE, IDC_CP_DATA_TYPE, OnComboUpdateED)
   ON_BN_CLICKED(IDC_CP_GET, &CCpView::OnBnClickedCpGet)
   ON_BN_CLICKED(IDC_CP_SET, &CCpView::OnBnClickedCpSet)
   ON_BN_CLICKED(IDC_CP_TRACE_GET, &CCpView::OnBnClickedCpTraceGet)
   ON_BN_CLICKED(IDC_CP_TRACE_SET, &CCpView::OnBnClickedCpTraceSet)
   ON_BN_CLICKED(IDC_CP_TRACE_ON, &CCpView::OnBnClickedCpTraceOn)
   ON_BN_CLICKED(IDC_CP_TRACE_OFF, &CCpView::OnBnClickedCpTraceOff)
   ON_MESSAGE(WM_UPDATE_CONFIG, &CCpView::OnUpdate)
   ON_WM_TIMER()
   ON_MESSAGE(WM_CLOSING, &CCpView::OnClosing)
END_MESSAGE_MAP()

// CCpView construction/destruction

CCpView::CCpView()
	: CViewEx(CCpView::IDD)
{
   m_cpAddr    = _T("");
   m_cpValue   = _T("");
   m_cpType    = CFG_INT8U;
   m_cpWR      = 0;
   m_cpTrace   = _T("");
   m_cpCI      = 0;
   m_cpCM      = 0;
   m_cpIND     = 0;
   m_cpIRQ     = 0;
   m_cpMEM     = 0;
   m_cpDVR     = 0;
   m_cpRUN     = 0;
   m_cpERR     = 0;
   m_cpID      = 0;

   memset(&m_rxq, 0, sizeof(rxq_t));
   m_rxq.thread = NULL;
}

CCpView::~CCpView()
{
}

void CCpView::DoDataExchange(CDataExchange* pDX)
{
	CViewEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_CP_EDIT, m_Edit);
   DDX_Text(pDX, IDC_CP_ADDR, m_cpAddr);
   DDX_Text(pDX, IDC_CP_VALUE, m_cpValue);
   DDX_CBIndex(pDX, IDC_CP_DATA_TYPE, m_cpType);
   DDX_Check(pDX, IDC_CP_WR, m_cpWR);
   DDX_Text(pDX, IDC_CP_TRACE, m_cpTrace);
   DDX_Check(pDX, IDC_CP_CI, m_cpCI);
   DDX_Check(pDX, IDC_CP_CM, m_cpCM);
   DDX_Check(pDX, IDC_CP_IRQ, m_cpIRQ);
   DDX_Check(pDX, IDC_CP_DVR, m_cpDVR);
   DDX_Check(pDX, IDC_CP_RUN, m_cpRUN);
   DDX_Check(pDX, IDC_CP_ERR, m_cpERR);
   DDX_Check(pDX, IDC_CP_ID, m_cpID);
   DDX_Check(pDX, IDC_CP_ROUTE, m_cpERR);
   DDX_Check(pDX, IDC_CP_UART, m_cpERR);
}

LRESULT CCpView::OnClosing(WPARAM wParam, LPARAM lParam)
{
   // Cancel Thread
   m_rxq.halt = TRUE;
   Sleep(100);
   return 0;
}

void CCpView::OnTimer(UINT_PTR nIDEvent)
{
   cm_send_t   ps = {0};
   CappDoc     *pDoc = GetDoc();
   CMainFrame* pMainFrm = (CMainFrame *)AfxGetApp()->GetMainWnd();

   if (nIDEvent == ID_TIMER_PING_TIMEOUT) {
       KillTimer(ID_TIMER_PING_TIMEOUT);
      // If connected then disconnect
      if (pMainFrm->m_nCom != APP_CON_NONE) {
         // Send disconnect to ConView()
         pMainFrm->OnMessageView(IDD_CONNECT, WM_DISCONNECT,  0, 0);
         // Issue Warning
         if (pDoc->m_ini->apPingWarn) {
            AfxMessageBox(IDS_PING_WARN, MB_OK|MB_ICONSTOP);
         }
      }
   }
   else if (nIDEvent == ID_TIMER_PING) {
      // If not connected then kill timer
      if (pMainFrm->m_nCom == APP_CON_NONE) 
         KillTimer(ID_TIMER_PING);
      // Send Ping Request
      else {
         pcmq_t slot = cm_alloc();
         if (slot != NULL) {
            pcp_ping_msg_t msg = (pcp_ping_msg_t)slot->buf;
            msg->p.srvid   = CM_ID_CP_SRV;
            msg->p.msgid   = CP_PING_REQ;
            msg->p.flags   = CP_NO_FLAGS;
            msg->p.status  = CP_OK;
            ps.msg         = (pcm_msg_t)msg;
            ps.dst_cmid    = CM_ID_CP_SRV;
            ps.src_cmid    = CM_ID_CP_CLI;
            ps.msglen      = sizeof(cp_ping_msg_t);
            // Send the Request
            cm_send(CM_MSG_REQ, &ps);
         }
      }
   }
   CViewEx::OnTimer(nIDEvent);
}

LRESULT CCpView::OnUpdate(WPARAM wParam, LPARAM lParam)
{
   OnUpdateConfig((BOOL)wParam);
   return 0;
}

void CCpView::OnInitialUpdate()
{
   CString str, str1;

   CViewEx::OnInitialUpdate();

   // Set the Anchors
   m_sizer.SetAnchor(IDC_CP_EDIT, ANCHOR_ALL);
   m_sizer.SetAnchor(IDC_CP_TRACE_STATIC, ANCHOR_VERTICALLY);

   m_sizer.InvokeOnResized();

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

   // Report the NIOS Address Map from the "system.h" header.
   str.Format(_T("NIOS Address Map, from system.h :\r\n")); str1 += str;
   str.Format(_T("Module            Address   \r\n")); str1 += str;
   str.Format(_T("============      ==========\r\n")); str1 += str;

#ifdef SDRAM_BASE
   str.Format(_T("  sdram           0x%08X\r\n"), SDRAM_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), (UINT)SDRAM_BASE + (UINT)SDRAM_SPAN - 1); str1 += str;
#endif

#ifdef SDRAM_FIFO_REGION_BASE
   str.Format(_T("  sdram_fifo      0x%08X\r\n"), SDRAM_FIFO_REGION_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), (UINT)SDRAM_FIFO_REGION_BASE + (UINT)SDRAM_FIFO_REGION_SPAN - 1); str1 += str;
#endif

#ifdef ONCHIP_CPU_BASE
   str.Format(_T("  onchip_cpu      0x%08X\r\n"), ONCHIP_CPU_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), (UINT)ONCHIP_CPU_BASE + (UINT)ONCHIP_CPU_SPAN - 1); str1 += str;
#endif

#ifdef ONCHIP_BASE
   str.Format(_T("  onchip          0x%08X\r\n"), ONCHIP_CPU_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), (UINT)ONCHIP_CPU_BASE + (UINT)ONCHIP_CPU_SPAN - 1); str1 += str;
#endif

#ifdef ONCHIP_ADC_BASE
   str.Format(_T("  onchip_adc      0x%08X\r\n"), ONCHIP_ADC_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), (UINT)ONCHIP_ADC_BASE + (UINT)ONCHIP_ADC_SPAN - 1); str1 += str;
#endif

   str.Format(_T("  stamp           0x%08X\r\n"), STAMP_BASE); str1 += str;
   str.Format(_T("  gpx             0x%08X\r\n"), GPX_BASE); str1 += str;
   str.Format(_T("  gpi             0x%08X\r\n"), GPI_BASE); str1 += str;

#ifdef COM_BASE
   str.Format(_T("  com             0x%08X\r\n"), COM_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), COM_BASE + COM_SPAN - 1); str1 += str;
#endif
#ifdef OPTO_BASE
   str.Format(_T("  opto            0x%08X\r\n"), OPTO_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), OPTO_BASE + OPTO_SPAN - 1); str1 += str;
#endif
#ifdef FIFO_BASE
   str.Format(_T("  fifo            0x%08X\r\n"), FIFO_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), FIFO_BASE + FIFO_SPAN - 1); str1 += str;
#endif
#ifdef LAN_BASE
   str.Format(_T("  lan             0x%08X\r\n"), LAN_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), LAN_BASE + LAN_SPAN - 1); str1 += str;
#endif

   str.Format(_T("  adc             0x%08X\r\n"), ADC_BASE); str1 += str;

#ifdef UPDATE_BASE
   str.Format(_T("  update          0x%08X\r\n"), UPDATE_BASE); str1 += str;
   str.Format(_T("                  0x%08X\r\n"), UPDATE_BASE + UPDATE_SPAN - 1); str1 += str;
#endif

   str.Format(_T("  pid             %d\r\n"), FPGA_PID); str1 += str;
   str.Format(_T("  stamp           %d\r\n"), FPGA_EPOCH); str1 += str;
   str.Format(_T("\r\n")); str1 += str;

   str.Format(_T("CM Constants (cmConst.h):\r\n")); str1 += str;
   str.Format(_T("  CM_ID_CP_SRV       0x%02X\r\n"), CM_ID_CP_SRV); str1 += str;
   str.Format(_T("  CM_DEV_WIN         0x%02X\r\n"), CM_DEV_WIN); str1 += str;
   str.Format(_T("  CM_ENDIAN          0x%02X\r\n"), CM_ENDIAN); str1 += str;
   str.Format(_T("  CM_MAX_MSG_INT8U   0x%04X\r\n"), CM_MAX_MSG_INT8U); str1 += str;
   str.Format(_T("  CM_MAX_OBJS        0x%02X\r\n\r\n"), CM_MAX_OBJS); str1 += str;

   ToEdit(m_Edit, str1);

   // Connect to CM
   cp_init();

   // Prevent Connect Focus
   GetDlgItem(IDC_CP_MEM_STATIC)->SetFocus();  
}

void CCpView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   // Transfer data to dialog items
   OnUpdateConfig(FALSE);
}

void CCpView::OnRangeUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
   if (nID == IDC_CP_TRACE) {
      ControlToCheck();
      OnUpdateConfig(FALSE);
   }
}

void CCpView::OnComboUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
   OnUpdateConfig(FALSE);
}

void CCpView::OnCheckUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   UpdateData(TRUE);
   CheckToControl();
   UpdateData(FALSE);
   OnUpdateConfig(TRUE);
}

void CCpView::OnUpdateConfig(BOOL bDir)
{
   INT     cpINT = 0;
   UINT    cpUINT = 0;

   CappDoc *pDoc = GetDoc();

   if (bDir == TRUE) {
      // Retrieve data from Form
      UpdateData(TRUE);
      swscanf_s(m_cpAddr, _T("%x"), &pDoc->m_ini->cpAddr);
      swscanf_s(m_cpTrace, _T("%x"), &pDoc->m_ini->cpTrace);
      pDoc->m_ini->cpWR = m_cpWR;
      pDoc->m_ini->cpType = m_cpType;
      switch(m_cpType) {
      case CFG_INT8U:
      case CFG_INT16U:
      case CFG_INT32U:
         swscanf_s(m_cpValue, _T("%x"), &cpUINT);
         pDoc->m_ini->cpValue = (DOUBLE)cpUINT;
         break;
      case CFG_INT8S:
      case CFG_INT16S:
      case CFG_INT32S:
         swscanf_s(m_cpValue, _T("%d"), &cpINT);
         pDoc->m_ini->cpValue = (DOUBLE)cpINT;
      case CFG_FP32:
      case CFG_FP64:
         swscanf_s(m_cpValue, _T("%lf"), &pDoc->m_ini->cpValue);
         break;
      default:
         swscanf_s(m_cpValue, _T("%x"), &cpUINT);
         pDoc->m_ini->cpValue = (DOUBLE)cpUINT;
         break;
      }
   }
   else {
      m_cpAddr.Format(_T("0x%08X"), pDoc->m_ini->cpAddr);
      m_cpTrace.Format(_T("0x%08X"), pDoc->m_ini->cpTrace);
      m_cpWR = pDoc->m_ini->cpWR;
      m_cpType = pDoc->m_ini->cpType;
      switch(m_cpType) {
         case CFG_INT8U:
             m_cpValue.Format(_T("0x%02X"), (UCHAR)pDoc->m_ini->cpValue);
            break;
         case CFG_INT8S:
            m_cpValue.Format(_T("%d"), (CHAR)pDoc->m_ini->cpValue);
            break;
         case CFG_INT16U:
            m_cpValue.Format(_T("0x%04X"), (USHORT)pDoc->m_ini->cpValue);
            break;
         case CFG_INT16S:
            m_cpValue.Format(_T("%d"), (SHORT)pDoc->m_ini->cpValue);
            break;
         case CFG_INT32U:
            m_cpValue.Format(_T("0x%08X"), (UINT)pDoc->m_ini->cpValue);
            break;
         case CFG_INT32S:
            m_cpValue.Format(_T("%d"), (INT)pDoc->m_ini->cpValue);
            break;
         case CFG_FP32:
            m_cpValue.Format(_T("%.E"), (DOUBLE)pDoc->m_ini->cpValue);
            break;
         case CFG_INT64U:
            m_cpValue.Format(_T("0x%016X"), (UINT64)pDoc->m_ini->cpValue);
            break;
         case CFG_INT64S:
            m_cpValue.Format(_T("%d"), (INT64)pDoc->m_ini->cpValue);
            break;
         case CFG_FP64:
            m_cpValue.Format(_T("%.E"), (DOUBLE)pDoc->m_ini->cpValue);
            break;
         default:
            m_cpValue.Format(_T("0x%08X"), (UINT)pDoc->m_ini->cpValue);
            break;
      }
      ControlToCheck();
      // Update Form
      UpdateData(FALSE);
   }
}

void CCpView::CheckToControl()
{
   UINT  trace;

   swscanf_s(m_cpTrace, _T("%x"), &trace);

   if (m_cpCI == TRUE)
      trace |= CFG_TRACE_CI;
   else
      trace &= ~CFG_TRACE_CI;
   if (m_cpCM == TRUE)
      trace |= CFG_TRACE_CM;
   else
      trace &= ~CFG_TRACE_CM;
   if (m_cpDVR == TRUE)
      trace |= CFG_TRACE_DRIVER;
   else
      trace &= ~CFG_TRACE_DRIVER;
   if (m_cpERR == TRUE)
      trace |= CFG_TRACE_ERROR;
   else
      trace &= ~CFG_TRACE_ERROR;
   if (m_cpID == TRUE)
      trace |= CFG_TRACE_ID;
   else
      trace &= ~CFG_TRACE_ID;
   if (m_cpRUN == TRUE)
      trace |= CFG_TRACE_RUN;
   else
      trace &= ~CFG_TRACE_RUN;
   if (m_cpIRQ == TRUE)
      trace |= CFG_TRACE_IRQ;
   else
      trace &= ~CFG_TRACE_IRQ;
   if (m_cpROUTE == TRUE)
      trace |= CFG_TRACE_ROUTE;
   else
      trace &= ~CFG_TRACE_ROUTE;
   if (m_cpUART == TRUE)
      trace |= CFG_TRACE_UART;
   else
      trace &= ~CFG_TRACE_UART;

   m_cpTrace.Format(_T("0x%08X"),trace);

}

void CCpView::ControlToCheck()
{
   UINT  trace;
   swscanf_s(m_cpTrace, _T("%x"), &trace);

   m_cpCM   = (trace & CFG_TRACE_CM) ? TRUE : FALSE;
   m_cpCI   = (trace & CFG_TRACE_CI) ? TRUE : FALSE;
   m_cpDVR  = (trace & CFG_TRACE_DRIVER) ? TRUE : FALSE;
   m_cpERR  = (trace & CFG_TRACE_ERROR) ? TRUE : FALSE;
   m_cpID   = (trace & CFG_TRACE_ID) ? TRUE : FALSE;
   m_cpRUN  = (trace & CFG_TRACE_RUN) ? TRUE : FALSE;
   m_cpIRQ  = (trace & CFG_TRACE_IRQ) ? TRUE : FALSE;
   m_cpROUTE  = (trace & CFG_TRACE_ROUTE) ? TRUE : FALSE;
   m_cpUART  = (trace & CFG_TRACE_UART) ? TRUE : FALSE;

}

// CCpView diagnostics

#ifdef _DEBUG
void CCpView::AssertValid() const
{
	CViewEx::AssertValid();
}

void CCpView::Dump(CDumpContext& dc) const
{
	CViewEx::Dump(dc);
}

#endif //_DEBUG

//
// CCpView Message Handlers
//

void CCpView::OnBnClickedCpGet()
{
   cm_send_t   ps = {0};

   CappDoc *pDoc = GetDoc();

   pcmq_t slot = cm_alloc();

   if (slot != NULL) {
      pcp_mem_msg_t msg = (pcp_mem_msg_t)slot->buf;
      msg->p.srvid   = CM_ID_CP_SRV;
      msg->p.msgid   = CP_MEM_REQ;
      msg->p.flags   = CP_MEM_RD;
      msg->p.status  = CP_OK;
      msg->b.address = pDoc->m_ini->cpAddr;
      msg->b.value   = 0;
      msg->b.type    = pDoc->m_ini->cpType;
      ps.msg         = (pcm_msg_t)msg;
      ps.dst_cmid    = CM_ID_CP_SRV;
      ps.src_cmid    = CM_ID_CP_CLI;
      ps.msglen      = sizeof(cp_mem_msg_t);
      // Send the Request
      cm_send(CM_MSG_REQ, &ps);
   }
}

void CCpView::OnBnClickedCpSet()
{
   cm_send_t   ps = {0};

   CappDoc *pDoc = GetDoc();

   pcmq_t slot = cm_alloc();

   if (slot != NULL) {
      pcp_mem_msg_t msg = (pcp_mem_msg_t)slot->buf;
      msg->p.srvid   = CM_ID_CP_SRV;
      msg->p.msgid   = CP_MEM_REQ;
      msg->p.flags   = CP_MEM_WR;
      msg->p.status  = CP_OK;
      msg->b.address = pDoc->m_ini->cpAddr;
      msg->b.value   = (uint32_t)pDoc->m_ini->cpValue;
      msg->b.type    = pDoc->m_ini->cpType;
      ps.msg         = (pcm_msg_t)msg;
      ps.dst_cmid    = CM_ID_CP_SRV;
      ps.src_cmid    = CM_ID_CP_CLI;
      ps.msglen      = sizeof(cp_mem_msg_t);
      // Send the Request
      cm_send(CM_MSG_REQ, &ps);
   }
}

void CCpView::OnBnClickedCpTraceGet()
{
   cm_send_t   ps = {0};

   CappDoc *pDoc = GetDoc();

   pcmq_t slot = cm_alloc();

   if (slot != NULL) {
      pcp_trace_msg_t msg = (pcp_trace_msg_t)slot->buf;
      msg->p.srvid   = CM_ID_CP_SRV;
      msg->p.msgid   = CP_TRACE_REQ;
      msg->p.flags   = CP_TRACE_GET;
      msg->p.status  = CP_OK;
      msg->b.trace   = pDoc->m_ini->cpTrace;
      msg->b.debug   = 0;
      ps.msg         = (pcm_msg_t)msg;
      ps.dst_cmid    = CM_ID_CP_SRV;
      ps.src_cmid    = CM_ID_CP_CLI;
      ps.msglen      = sizeof(cp_trace_msg_t);
      // Send the Request
      cm_send(CM_MSG_REQ, &ps);
   }
}

void CCpView::OnBnClickedCpTraceSet()
{
   cm_send_t   ps = {0};

   CappDoc *pDoc = GetDoc();

   pcmq_t slot = cm_alloc();

   if (slot != NULL) {
      pcp_trace_msg_t msg = (pcp_trace_msg_t)slot->buf;
      msg->p.srvid   = CM_ID_CP_SRV;
      msg->p.msgid   = CP_TRACE_REQ;
      msg->p.flags   = CP_TRACE_SET;
      msg->p.status  = CP_OK;
      msg->b.trace   = pDoc->m_ini->cpTrace;
      msg->b.debug   = 0;
      ps.msg         = (pcm_msg_t)msg;
      ps.dst_cmid    = CM_ID_CP_SRV;
      ps.src_cmid    = CM_ID_CP_CLI;
      ps.msglen      = sizeof(cp_trace_msg_t);
      // Send the Request
      cm_send(CM_MSG_REQ, &ps);
   }
}

void CCpView::OnBnClickedCpTraceOn()
{
   CappDoc *pDoc = GetDoc();
   m_cpTrace.Format(_T("0x%08X"),0xFFFFFFFF);
   pDoc->m_ini->cpTrace = 0xFFFFFFFF;
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(FALSE);
}

void CCpView::OnBnClickedCpTraceOff()
{
   CappDoc *pDoc = GetDoc();
   m_cpTrace.Format(_T("0x%08X"),0x0);
   pDoc->m_ini->cpTrace = 0x0;
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(FALSE);
}

//
// LOG TRAFFIC TO MAINFRAME
//
inline void logTraffic(BOOL flag, WPARAM wParam, WPARAM pMsg, LPARAM lParam)
{
   if (flag)
      AfxGetMainWnd()->SendMessage((UINT)wParam, (WPARAM)pMsg, (UINT)lParam);
}

//
// CM RELATED FUNCTIONS
//

uint32_t CCpView::cp_init(void) {
      
   // Register this Client
   m_cmid   = CM_ID_CP_CLI;
   m_handle = cm_register(m_cmid, cp_qmsg_static, (void *)this, NULL);

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

   //// Start the Message Delivery Thread
   //m_rxq.thread = CreateThread(NULL, 0, cp_thread_start, (void *)this, 0, &m_rxq.tid);

   AfxBeginThread(cp_thread_start, (void *)this);

   return CP_OK;
}

uint32_t CCpView::cp_msg(pcm_msg_t msg) {

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
   //    TRACE RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_TRACE_RESP)) {
      pcp_trace_msg_t rsp = (pcp_trace_msg_t)msg;
      pDoc->m_ini->cpTrace  = rsp->b.trace;
      // Update the View
      //OnUpdateConfig(FALSE);
      SendMessage(WM_UPDATE_CONFIG, FALSE, 0);
      pDoc->SetModifiedFlag(TRUE);
   }
   // 
   //    MEMORY RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_MEM_RESP)) {
      pcp_mem_msg_t rsp = (pcp_mem_msg_t)msg;
      pDoc->m_ini->cpAddr  = rsp->b.address;
      pDoc->m_ini->cpValue = rsp->b.value;
      // Update the View
      //OnUpdateConfig(FALSE);
      SendMessage(WM_UPDATE_CONFIG, FALSE, 0);
      pDoc->SetModifiedFlag(TRUE);
   }
   //
   //    PING RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_PING_RESP)) {
      // Re-Start Ping Timeout
      SetTimer(ID_TIMER_PING_TIMEOUT, 30000, NULL);
   }
   //
   //    VERSION RESPONSE
   //
   else if (cm_msg == MSG(CM_ID_CP_SRV, CP_VER_RESP)) {
      pcp_ver_msg_t rsp = (pcp_ver_msg_t)msg;
      m_fw_ver      = rsp->b.fw_ver;
      m_sysid       = rsp->b.sysid;
      m_stamp_epoch = rsp->b.stamp_epoch;
      m_stamp_date  = rsp->b.stamp_date;
      m_stamp_time  = rsp->b.stamp_time;
      m_trace       = rsp->b.trace;
      m_feature     = rsp->b.feature;
      m_debug       = rsp->b.debug;
      // Start Ping Timeout, If enabled in F/W
      if (m_feature & CFG_FEATURE_PING) {
         SetTimer(ID_TIMER_PING_TIMEOUT, 30000, NULL);
         SetTimer(ID_TIMER_PING, 10000, NULL);
      }
      // Send Version Info to IDD_CONNECT for display
      pcmq_t slot = cm_alloc();
      if (slot != NULL) {
         pcp_ver_msg_t ver = (pcp_ver_msg_t)slot->buf;
         memcpy(ver, msg, sizeof(cp_ver_msg_t));
         pMainFrm->OnMessageView(IDD_CONNECT, WM_CM_MESSAGE, (WPARAM)ver, 0);
      }
   }

   // Release the Slot
   cm_free(msg);

   return result;
}

uint32_t CCpView::cp_timer(pcm_msg_t msg) {

   uint32_t    result = CP_OK;
   uint16_t    cm_msg  = MSG(msg->p.srvid, msg->p.msgid);

   // Release the Slot
   cm_free(msg);

   return result;

}

uint32_t CCpView::cp_qmsg_static(pcm_msg_t msg, void *member) {
   CCpView *This = (CCpView *)member;
   return This->cp_qmsg(msg);
}

uint32_t CCpView::cp_qmsg(pcm_msg_t msg) {

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

//DWORD WINAPI CCpView::cp_thread_start(LPVOID data) {
//   CCpView *This = (CCpView *)data;
//   return This->cp_thread();
//}

UINT CCpView::cp_thread_start(LPVOID data) {
   CCpView *This = (CCpView *)data;
   return This->cp_thread();
}

DWORD CCpView::cp_thread(void) {

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
      if (msg != NULL) cp_msg(msg);

   }

   return 0;

} // end cp_thread()
