// ConView.cpp : implementation of the CConView class
//

#include "stdafx.h"
#include "ConView.h"
#include "Dbt.h"
#include <time.h>
#include "build.h"
#include <iphlpapi.h>

// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static   DWORD fifo_speed[]  = {480000000};
static   DWORD ftdi_speed[] = {12000000, 3000000, 2000000, 1500000, 1250000, 1000000, 500000, 115200, 19200};
static   DWORD uart_speed[] = {12000000, 3000000, 2000000, 1500000, 1250000, 1000000, 500000, 115200, 19200};
static   DWORD libsp_speed[] = { 12000000, 3000000, 2000000, 1500000, 1250000, 1000000, 500000, 115200, 19200 };
static   DWORD com_speed[]  = {115200, 57600, 19200, 9600};
static   INT   devID;

// CConView

IMPLEMENT_DYNCREATE(CConView, CViewEx)

BEGIN_MESSAGE_MAP(CConView, CViewEx)
   ON_CONTROL_RANGE(EN_KILLFOCUS, IDC_CON_IP_ADDR, IDC_CON_MAC_ADDR, OnRangeUpdateED)
   ON_CONTROL_RANGE(BN_CLICKED, IDC_CON_STARTUP, IDC_CON_STARTUP, OnCheckUpdateED)
   ON_BN_CLICKED(IDC_CON_REFRESH, &CConView::OnBnClickedConRefresh)
   ON_BN_CLICKED(IDC_CON_CONNECT, &CConView::OnBnClickedConConnect)
   ON_UPDATE_COMMAND_UI(IDC_CON_PORT_SPEED, &CConView::OnUpdatePortSpeed)
   ON_UPDATE_COMMAND_UI(IDC_CON_CONNECT, &CConView::OnUpdateConnect)
   ON_UPDATE_COMMAND_UI(IDC_CON_PORT_TYPE, &CConView::OnUpdateType)
   ON_UPDATE_COMMAND_UI(IDC_CON_PORT_SEL, &CConView::OnUpdatePort)
   ON_UPDATE_COMMAND_UI(IDC_CON_IP_ADDR, &CConView::OnUpdateIpAddr)
   ON_UPDATE_COMMAND_UI(IDC_CON_MAC_ADDR, &CConView::OnUpdateMacAddr)
   ON_UPDATE_COMMAND_UI(IDC_CON_IP_PORT, &CConView::OnUpdateIpPort)
   ON_MESSAGE(WM_CLOSING, &CConView::OnClosing)
   ON_MESSAGE(WM_DEV_REMOVE, &CConView::OnDevRemove)
   ON_MESSAGE(WM_DEV_ARRIVE, &CConView::OnDevArrive)
   ON_MESSAGE(WM_DEV_ERROR, &CConView::OnDevError)
   ON_MESSAGE(WM_CM_MESSAGE, &CConView::OnCmMsg)
   ON_MESSAGE(WM_UDP_MESSAGE, &CConView::OnUDPMsg)
   ON_WM_TIMER()
   ON_WM_DEVICECHANGE()
   ON_WM_CTLCOLOR()
   ON_MESSAGE(WM_SOCK_NEW_MSG, &CConView::OnSockNewMsg)
   ON_MESSAGE(WM_SOCK_DONE, &CConView::OnSockDone)
   ON_MESSAGE(WM_SOCK_CONNECT, &CConView::OnSockConnect)
   ON_CBN_SELCHANGE(IDC_CON_PORT_TYPE, &CConView::OnCbnSelchangeConPortType)
   ON_CBN_SELCHANGE(IDC_CON_PORT_SPEED, &CConView::OnCbnSelchangeConPortSpeed)
   ON_CBN_SELCHANGE(IDC_CON_PORT_SEL, &CConView::OnCbnSelchangeConPortSel)
END_MESSAGE_MAP()

// CConView construction/destruction

CConView::CConView()
	: CViewEx(CConView::IDD)
{
   m_comCon       = APP_CON_FIFO;
   m_comPort      = APP_PORT_NONE;
   m_comSpeed     = APP_SPEED_NONE;
   m_ipAddress    = _T("");
   m_macAddress   = _T("");
   m_autoConnect  = FALSE;
   m_autoPort     = APP_PORT_NONE;
   m_ipAddress    = _T("");
   m_conIpPort    = _T("");

   m_nCom         = APP_CON_NONE;
   m_nComDelta    = APP_CON_NONE;

   m_id           = CM_PORT_COM0;
   m_autoRefresh  = FALSE;
}

CConView::~CConView()
{
}

void CConView::DoDataExchange(CDataExchange* pDX)
{
	CViewEx::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_CON_PORT_SEL, m_PortSel);
   DDX_Control(pDX, IDC_CON_PORT_SPEED, m_SpeedSel);
   DDX_Control(pDX, IDC_CON_REFRESH, m_Refresh);
   DDX_Control(pDX, IDC_CON_CONNECT, m_Connect);
   DDX_CBIndex(pDX, IDC_CON_PORT_TYPE, m_comCon);
   DDX_CBIndex(pDX, IDC_CON_PORT_SEL, m_comPort);
   DDX_CBIndex(pDX, IDC_CON_PORT_SPEED, m_comSpeed);
   DDX_Text(pDX, IDC_CON_IP_ADDR, m_ipAddress);
   DDX_Text(pDX, IDC_CON_MAC_ADDR, m_macAddress);
   DDX_Check(pDX, IDC_CON_STARTUP, m_autoConnect);
   DDX_Text(pDX, IDC_CON_IP_PORT, m_conIpPort);
}

void CConView::OnTimer(UINT_PTR nIDEvent)
{
   CappDoc *pDoc = GetDoc();
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

   if (nIDEvent == ID_TIMER_REFRESH) {
      // If not connected then refresh ports
      if (m_nCom == APP_CON_NONE) {
         m_autoRefresh = TRUE;
         OnBnClickedConRefresh();
      }
      KillTimer(ID_TIMER_REFRESH);
   }
   else if (nIDEvent == ID_TIMER_PORT_SEL) {
      // If not connected then select port
      if (m_nCom == APP_CON_NONE) {
         m_PortSel.SetCurSel(m_autoPort);
         OnUpdateConfig(TRUE);
      }
      KillTimer(ID_TIMER_PORT_SEL);
   }
   else if (nIDEvent == ID_TIMER_CONNECT) {
      // If not connected then Connect
      if (m_nCom == APP_CON_NONE)
         OnBnClickedConConnect();
      KillTimer(ID_TIMER_CONNECT);
   }
   else if (nIDEvent == ID_TIMER_TCP_CLOSE) {
      KillTimer(ID_TIMER_TCP_CLOSE);
      // Close the Open Connection
      m_client.Close();
      // No Connection
      m_nCom              = APP_CON_NONE;
      pMainFrm->m_nCom    = APP_CON_NONE;
      pMainFrm->m_nComErr = 0;
      pMainFrm->m_nPort   = APP_PORT_NONE;
      pMainFrm->m_nPortId = 0;
      pMainFrm->m_hCom    = NULL;
   }
   CViewEx::OnTimer(nIDEvent);
}

void CConView::OnInitialUpdate()
{
   CViewEx::OnInitialUpdate();

   // Set the Anchors
   m_sizer.SetAnchor(IDC_CON_MSG_STATIC, ANCHOR_VERTICALLY);
   m_sizer.SetAnchor(IDC_CON_DEV_STATIC, ANCHOR_VERTICALLY);
   m_sizer.SetAnchor(IDC_CON_SERVERS_STATIC, ANCHOR_VERTICALLY);
   m_sizer.SetAnchor(IDC_CON_CLIENTS_STATIC, ANCHOR_VERTICALLY);
   m_sizer.InvokeOnResized();

   HICON refreshHot  = static_cast<HICON>(LoadImage (theApp.m_hInstance, MAKEINTRESOURCE(IDI_REFRESH_HOT),
				               IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR));
	HICON refreshCold = static_cast<HICON>(LoadImage (theApp.m_hInstance, MAKEINTRESOURCE(IDI_REFRESH_COLD),
				               IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR));
   m_Refresh.SetImage(refreshHot, TRUE, refreshHot, refreshCold);
   
   // Auto Connect
   if (m_autoConnect == TRUE) {
      SetTimer(ID_TIMER_REFRESH,  250, NULL);
      SetTimer(ID_TIMER_PORT_SEL, 500, NULL);
      SetTimer(ID_TIMER_CONNECT,  750, NULL);
   }
   else {
      SetTimer(ID_TIMER_REFRESH, 1500, NULL);
   }
   // Update Port and Speed as per Connection Type
   OnCbnSelchangeConPortType();

   // Set Communications Device
#ifdef OPTO_BASE
   GetDlgItem(IDC_CON_FIFO_REV_STATIC)->SetWindowText(_T("OPTO"));
#endif
#ifdef FIFO_BASE
   GetDlgItem(IDC_CON_FIFO_REV_STATIC)->SetWindowText(_T("FIFO"));
#endif
#ifdef COM_BASE
   GetDlgItem(IDC_CON_FIFO_REV_STATIC)->SetWindowText(_T("COM"));
#endif
#ifdef LAN_BASE
   GetDlgItem(IDC_CON_FIFO_REV_STATIC)->SetWindowText(_T("LAN"));
#endif
#ifdef SIM_BASE
   GetDlgItem(IDC_CON_FIFO_REV_STATIC)->SetWindowText(_T("SIM"));
#endif

   m_timer.Start();

   // Prevent Connect Focus
   GetDlgItem(IDC_CON_MSG_STATIC)->SetFocus();
}

void CConView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   // Transfer data to dialog items
   OnUpdateConfig(FALSE);
}

void CConView::OnRangeUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CConView::OnComboUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CConView::OnCheckUpdateED(UINT nID)
{
   GetDoc()->SetModifiedFlag(TRUE);
   OnUpdateConfig(TRUE);
}

void CConView::OnUpdateConfig(BOOL bDir)
{
   CappDoc *pDoc = GetDoc();

   if (bDir == TRUE) {
      // Retrieve data from Form
      UpdateData(TRUE);
      pDoc->m_ini->conComCon = m_comCon;
      wcscpy_s(pDoc->m_ini->conIpAddr, m_ipAddress);
      wcscpy_s(pDoc->m_ini->conMacAddr, m_macAddress);
      pDoc->m_ini->conAutoConnect = m_autoConnect;
      pDoc->m_ini->conAutoPort = m_autoPort;
      swscanf_s(m_conIpPort, _T("%d"), &pDoc->m_ini->conIpPort);
   }
   else {
      m_comCon = pDoc->m_ini->conComCon;
      m_ipAddress.Format(_T("%s"), pDoc->m_ini->conIpAddr);
      m_macAddress.Format(_T("%s"), pDoc->m_ini->conMacAddr);
      m_autoConnect = pDoc->m_ini->conAutoConnect;
      m_autoPort = pDoc->m_ini->conAutoPort;
      m_conIpPort.Format(_T("%d"), pDoc->m_ini->conIpPort);
      // Update Form
      UpdateData(FALSE);
   }
}

// CConView diagnostics

#ifdef _DEBUG
void CConView::AssertValid() const
{
	CViewEx::AssertValid();
}

void CConView::Dump(CDumpContext& dc) const
{
	CViewEx::Dump(dc);
}

#endif //_DEBUG

//
// CConView Message Handlers
//

afx_msg LRESULT CConView::OnCmMsg(WPARAM wParam, LPARAM lParam)
{
   pcm_msg_t msg = (pcm_msg_t)wParam;
   USHORT  cm_msg = MSG(msg->p.srvid, msg->p.msgid);
   CString str;
   time_t  bTime;

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   CappDoc  *pDoc = GetDoc();

   // 
   // Control Panel Version Response
   //
   if (cm_msg == MSG(CM_ID_CP_SRV, CP_VER_RESP)) {
      // Show All Connected Service Providers
      if (cm_id_exists(CM_ID_INSTANCE)) {
         ((CButton *)(GetDlgItem(IDC_CON_SRV_CM)))->SetCheck(BST_CHECKED);
      }
      if (cm_id_exists(CM_ID_CP_SRV)) {
         ((CButton *)(GetDlgItem(IDC_CON_SRV_CP)))->SetCheck(BST_CHECKED);
      }
      if (cm_id_exists(CM_ID_DAQ_SRV)) {
         ((CButton *)(GetDlgItem(IDC_CON_SRV_DAQ)))->SetCheck(BST_CHECKED);
      }
      // Show All Connected Clients
      if (cm_id_exists(CM_ID_INSTANCE)) {
         ((CButton *)(GetDlgItem(IDC_CON_CLI_CM)))->SetCheck(BST_CHECKED);
      }
      if (cm_id_exists(CM_ID_CP_CLI)) {
         ((CButton *)(GetDlgItem(IDC_CON_CLI_CP)))->SetCheck(BST_CHECKED);
      }
      if (cm_id_exists(CM_ID_DAQ_CLI)) {
         ((CButton *)(GetDlgItem(IDC_CON_CLI_DAQ)))->SetCheck(BST_CHECKED);
      }
      // version response
      pcp_ver_msg_t  ver = (pcp_ver_msg_t)msg;
      // Set APP Version
      str.Format(_T("%d.%d.%d.%d"), (ver->b.fw_ver>>24) & 0xFF, (ver->b.fw_ver>>16) & 0xFF,
               (ver->b.fw_ver>>8) & 0xFF, ver->b.fw_ver & 0xFF);
      GetDlgItem(IDC_CON_FW_REV)->SetWindowText(str);
      // Set System ID
      str.Format(_T("%d"), ver->b.sysid);
      GetDlgItem(IDC_CON_SYS_ID)->SetWindowText(str);
      // Set System Time Stamp
      str.Format(_T("%d"), ver->b.stamp_epoch);
      GetDlgItem(IDC_CON_STAMP)->SetWindowText(str);
      // Convert timestamp to real time
      bTime = (time_t)ver->b.stamp_epoch;
      CTime t(bTime);
      str.Format(_T("Quartus Build : %s\n"), t.Format(_T("%#c")));
      GetDoc()->Log(str);
      // Set Connection Version
      str.Format(_T("%d"), ver->b.vhdl[0]);
      GetDlgItem(IDC_CON_FIFO_REV)->SetWindowText(str);
      // Set ADC Version
      str.Format(_T("%d"), ver->b.vhdl[1]);
      GetDlgItem(IDC_CON_ADC_REV)->SetWindowText(str);
      // Check SYSID
      if (ver->b.sysid != FPGA_PID && pDoc->m_ini->apSysIDWarn) {
         AfxMessageBox(IDS_SYSID_INVALID, MB_OK|MB_ICONSTOP);
      }
   }

   // Release the Slot
   cm_free(msg);

   return 0;
}

afx_msg LRESULT CConView::OnUDPMsg(WPARAM wParam, LPARAM lParam)
{
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   return 0;
}

void CConView::OnBnClickedConRefresh()
{
   uint32_t devCnt;
   BOOL     devFound = FALSE;
   INT      devPreferred = -1;
   UINT     i,j;
   CString  str;

   CappDoc  *pDoc = GetDoc();
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

   if (m_autoRefresh == FALSE) return;

   // Clear the Port Select ComboBox
   m_PortSel.ResetContent();
   m_comPort = APP_PORT_NONE;
   pMainFrm->m_nPort = APP_PORT_NONE;

   //
   // OPTO DEVICE
   //
   if (m_comCon == APP_CON_OPTO) {
      // available devices?
      devCnt = opto_query(&m_pOPTOInfo);
      if (devCnt == 0) {
         str.Format(_T("Warning : No OPTO Devices Available\n"));
         pDoc->Log(str, APP_MSG_WARNING);
      }
      // parse device info
      else {
         if (devCnt <= OPTO_MAX_DEVICES) {
            for (i=0,j=0;i<devCnt;i++) {
               CA2W serial(m_pOPTOInfo[i].serial);
               CA2W desc(m_pOPTOInfo[i].desc);
               // Check for OPTO Omniware Serial
               if (serial.m_psz[0] == 'O' && serial.m_psz[1] == '1') {
                  if ((m_pOPTOInfo[i].id == OPTO_VID_PID && m_pOPTOInfo[i].flags != 1) ||
                      (m_pOPTOInfo[i].id == OPTO_VID_PID_ALT && m_pOPTOInfo[i].flags != 1)) {
                     str.Format(_T("OPTO.%d \t:\n"), i); pDoc->Log(str, APP_MSG_HIGHLITE);
                  }
                  else {
                     str.Format(_T("OPTO.%d \t: Unavailable\n"), i); pDoc->Log(str, APP_MSG_ERROR);
                  }
               }
               else {
                  str.Format(_T("OPTO.%d \t: Unavailable\n"), i); pDoc->Log(str, APP_MSG_ERROR);
               }
               str.Format(_T(" flags \t: %08X\n"), m_pOPTOInfo[i].flags);  pDoc->Log(str);
               str.Format(_T(" type  \t: %08X\n"), m_pOPTOInfo[i].type);   pDoc->Log(str);
               str.Format(_T(" id    \t: %08X\n"), m_pOPTOInfo[i].id);     pDoc->Log(str);
               str.Format(_T(" locid \t: %08X\n"), m_pOPTOInfo[i].locid);  pDoc->Log(str);
               str.Format(_T(" serial\t: %s\n"),   serial.m_psz);          pDoc->Log(str);
               str.Format(_T(" desc  \t: %s\n"),   desc.m_psz);            pDoc->Log(str);
               str.Format(_T(" handle\t: %08X\n"), m_pOPTOInfo[i].handle); pDoc->Log(str);
               // Check for OPTO Omniware Serial
               if (serial.m_psz[0] == 'O' && serial.m_psz[1] == '1') {
                  // Add to ComboBox
                  if ((m_pOPTOInfo[i].handle == 0 && m_pOPTOInfo[i].id == OPTO_VID_PID) ||
                      (m_pOPTOInfo[i].handle == 0 && m_pOPTOInfo[i].id == OPTO_VID_PID_ALT)) {
                     str.Format(_T("OPTO.%d"), i);
                     m_PortSel.AddString(str);
                     m_PortSel.SetItemData(j++, i);
                     devFound = TRUE;
                  }
               }
            }
            // Show first selection, if any
            m_comPort = (devFound == TRUE) ? 0 : APP_PORT_NONE;
         }
         else {
            str.Format(_T("Error : Maximum Attached Devices Exceeded, %d\n"), devCnt);
            GetDoc()->Log(str, APP_MSG_ERROR);
         }
      }
   }
   //
   // FIFO DEVICE
   //
   else if (m_comCon == APP_CON_FIFO) {
      // available devices?
      devCnt = fifo_query(&m_pFIFOInfo);
      if (devCnt == 0) {
         str.Format(_T("Warning : No FIFO Devices Available\n"));
         pDoc->Log(str, APP_MSG_WARNING);
      }
      // parse device info
      else {
         if (devCnt <= APP_MAX_DEVICES) {
            for (i=0,j=0;i<devCnt;i++) {
               CA2W serial(m_pFIFOInfo[i].serial);
               CA2W desc(m_pFIFOInfo[i].desc);
               // Check for FIFO Omniware Serial
               if (serial.m_psz[0] == 'O' && serial.m_psz[1] == '2') {
                  if ((m_pFIFOInfo[i].id == FIFO_VID_PID && m_pFIFOInfo[i].flags != 1) ||
                      (m_pFIFOInfo[i].id == FIFO_VID_PID_ALT && m_pFIFOInfo[i].flags != 1)) {
                     str.Format(_T("FIFO.%d \t:\n"), i); pDoc->Log(str, APP_MSG_HIGHLITE);
                  }
                  else {
                     str.Format(_T("FIFO.%d \t: Unavailable\n"), i); pDoc->Log(str, APP_MSG_ERROR);
                  }
               }
               else {
                  str.Format(_T("FIFO.%d \t: Unavailable\n"), i); pDoc->Log(str, APP_MSG_ERROR);
               }
               str.Format(_T(" flags \t: %08X\n"), m_pFIFOInfo[i].flags);  pDoc->Log(str);
               str.Format(_T(" type  \t: %08X\n"), m_pFIFOInfo[i].type);   pDoc->Log(str);
               str.Format(_T(" id    \t: %08X\n"), m_pFIFOInfo[i].id);     pDoc->Log(str);
               str.Format(_T(" locID \t: %08X\n"), m_pFIFOInfo[i].locid);  pDoc->Log(str);
               str.Format(_T(" serial\t: %s\n"),   serial.m_psz);          pDoc->Log(str);
               str.Format(_T(" desc  \t: %s\n"),   desc.m_psz);            pDoc->Log(str);
               str.Format(_T(" handle\t: %08X\n"), m_pFIFOInfo[i].handle); pDoc->Log(str);
               // Check for FIFO Omniware Serial
               if (serial.m_psz[0] == 'O' && serial.m_psz[1] == '2') {
                  // Add to ComboBox
                  if ((m_pFIFOInfo[i].handle == 0 && m_pFIFOInfo[i].id == FIFO_VID_PID) ||
                      (m_pFIFOInfo[i].handle == 0 && m_pFIFOInfo[i].id == FIFO_VID_PID_ALT)) {
                     str.Format(_T("FIFO.%d"), i);
                     m_PortSel.AddString(str);
                     m_PortSel.SetItemData(j++, i);
                     devFound = TRUE;
                  }
               }
            }
            // Show first selection, if any
            m_comPort = (devFound == TRUE) ? 0 : APP_PORT_NONE;
         }
         else {
            str.Format(_T("Error : Maximum Attached Devices Exceeded, %d\n"), devCnt);
            GetDoc()->Log(str, APP_MSG_ERROR);
         }
      }
   }
   //
   // COM DEVICE
   //
   else if (m_comCon == APP_CON_COM) {
      // available devices?
      devCnt = com_query(&m_pCOMInfo);
      if (devCnt == 0) {
         str.Format(_T("Warning : No COM Devices Available\n"));
         pDoc->Log(str, APP_MSG_WARNING);
      }
      // parse device info
      else {
         if (devCnt <= COM_MAX_DEVICES) {
            for (i=0,j=0;i<COM_MAX_DEVICES;i++) {
               if (m_pCOMInfo[i].locid != 0) {
                  CA2W serial(m_pCOMInfo[i].serial);
                  CA2W desc(m_pCOMInfo[i].desc);
                  str.Format(_T("COM.%d \t:\n"), m_pCOMInfo[i].locid); pDoc->Log(str, APP_MSG_HIGHLITE);
                  str.Format(_T(" flags \t: %08X\n"), m_pCOMInfo[i].flags);  pDoc->Log(str);
                  str.Format(_T(" type  \t: %08X\n"), m_pCOMInfo[i].type);   pDoc->Log(str);
                  str.Format(_T(" id    \t: %08X\n"), m_pCOMInfo[i].id);     pDoc->Log(str);
                  str.Format(_T(" locID \t: %08X\n"), m_pCOMInfo[i].locid);  pDoc->Log(str);
                  str.Format(_T(" serial\t: %s\n"),   serial.m_psz);          pDoc->Log(str);
                  str.Format(_T(" desc  \t: %s\n"),   desc.m_psz);            pDoc->Log(str);
                  str.Format(_T(" handle\t: %08X\n"), m_pCOMInfo[i].handle); pDoc->Log(str);
                  // Add to ComboBox
                  str.Format(_T("COM.%d"), m_pCOMInfo[i].locid);
                  m_PortSel.AddString(str);
                  m_PortSel.SetItemData(j++, m_pCOMInfo[i].locid);
                  devFound = TRUE;
               }
            }
            // Show first selection, if any
            m_comPort = (devFound == TRUE) ? 0 : APP_PORT_NONE;
         }
         else {
            str.Format(_T("Error : Maximum Attached Devices Exceeded, %d\n"), devCnt);
            GetDoc()->Log(str, APP_MSG_ERROR);
         }
      }
   }
   //
   // LAN DEVICE
   //
   if (m_comCon == APP_CON_LAN) {
      // available devices?
      devCnt = lan_query(&m_pLANInfo);
      if (devCnt == 0) {
         str.Format(_T("Warning : No FIFO Devices Available\n"));
         pDoc->Log(str, APP_MSG_WARNING);
      }
      // parse device info
      else {
         if (devCnt <= LAN_MAX_DEVICES) {
            for (i=0,j=0;i<devCnt;i++) {
               CA2W serial(m_pLANInfo[i].serial);
               CA2W desc(m_pLANInfo[i].desc);
               str.Format(_T("LAN.%d \t:\n"), i); pDoc->Log(str, APP_MSG_HIGHLITE);
               str.Format(_T(" serial\t: %s\n"),   serial.m_psz);         pDoc->Log(str);
               str.Format(_T(" desc  \t: %s\n"),   desc.m_psz);           pDoc->Log(str);
               str.Format(_T(" flags \t: %08X\n"), m_pLANInfo[i].flags);  pDoc->Log(str);
               str.Format(_T(" type  \t: %08X\n"), m_pLANInfo[i].type);   pDoc->Log(str);
               str.Format(_T(" id    \t: %08X\n"), m_pLANInfo[i].id);     pDoc->Log(str);
               str.Format(_T(" locID \t: %08X\n"), m_pLANInfo[i].locid);  pDoc->Log(str);
               str.Format(_T(" handle\t: %08X\n"), m_pLANInfo[i].handle); pDoc->Log(str);
               // Select Preferred Device, NULL Protocols
               if (devPreferred == -1 && m_pLANInfo[i].flags == 1) devPreferred = i;
               // Check for Control Channel
               if (m_pLANInfo[i].type == LAN_CHAN_CTL) {
                  // Add to ComboBox
                  str.Format(_T("LAN.%d"), i);
                  m_PortSel.AddString(str);
                  m_PortSel.SetItemData(j++, i);
                  devFound = TRUE;
               }
            }
            // Show first selection, if any
            m_comPort = (devFound == TRUE) ? 0 : APP_PORT_NONE;
            // Preferred Selection
            if (devPreferred != -1) m_comPort = devPreferred;
         }
         else {
            str.Format(_T("Error : Maximum Attached Devices Exceeded, %d\n"), devCnt);
            GetDoc()->Log(str, APP_MSG_ERROR);
         }
      }
   }
   //
   // UDP DEVICE, NO REAL QUERY
   //
   else if (m_comCon == APP_CON_UDP) {
      str.Format(_T("UDP.%d \t:\n"), 0); pDoc->Log(str, APP_MSG_HIGHLITE);
      str.Format(_T(" ip    \t: %s\n"),   pDoc->m_ini->conIpAddr); pDoc->Log(str);
      str.Format(_T(" port  \t: %04d\n"), pDoc->m_ini->conIpPort); pDoc->Log(str);
      // Add to ComboBox
      str.Format(_T("UDP.%d"), 0);
      m_PortSel.AddString(str);
      m_PortSel.SetItemData(0, 0);
      m_comPort = 0;
   }
   //
   // TCP DEVICE (USING CMSOCK), NO REAL QUERY
   //
   else if (m_comCon == APP_CON_TCP) {
      str.Format(_T("TCP.%d \t:\n"), 0); pDoc->Log(str, APP_MSG_HIGHLITE);
      str.Format(_T(" ip    \t: %s\n"),   pDoc->m_ini->conIpAddr); pDoc->Log(str);
      str.Format(_T(" port  \t: %04d\n"), pDoc->m_ini->conIpPort); pDoc->Log(str);
      // Add to ComboBox
      str.Format(_T("TCP.%d"), 0);
      m_PortSel.AddString(str);
      m_PortSel.SetItemData(0, 0);
      m_comPort = 0;
   }
   //
   // UART DEVICE
   //
   else if (m_comCon == APP_CON_UART) {
      // available devices?
      devCnt = uart_query(&m_pUARTInfo);
      if (devCnt == 0) {
         str.Format(_T("Warning : No UART Devices Available\n"));
         pDoc->Log(str, APP_MSG_WARNING);
      }
      // parse device info
      else {
         if (devCnt <= UART_MAX_DEVICES) {
            for (i=0,j=0;i<devCnt;i++) {
               CA2W serial(m_pUARTInfo[i].serial);
               CA2W desc(m_pUARTInfo[i].desc);
               // Check for UART Omniware Serial
               if (serial.m_psz[0] == 'O' && serial.m_psz[1] == '5') {
                  if ((m_pUARTInfo[i].id == OPTO_VID_PID && m_pUARTInfo[i].flags != 1) ||
                      (m_pUARTInfo[i].id == OPTO_VID_PID_ALT && m_pUARTInfo[i].flags != 1)) {
                     str.Format(_T("UART.%d \t:\n"), i); pDoc->Log(str, APP_MSG_HIGHLITE);
                  }
                  else {
                     str.Format(_T("UART.%d \t: Unavailable\n"), i); pDoc->Log(str, APP_MSG_ERROR);
                  }
               }
               else {
                  str.Format(_T("UART.%d \t: Unavailable\n"), i); pDoc->Log(str, APP_MSG_ERROR);
               }
               str.Format(_T(" flags \t: %08X\n"), m_pUARTInfo[i].flags);  pDoc->Log(str);
               str.Format(_T(" type  \t: %08X\n"), m_pUARTInfo[i].type);   pDoc->Log(str);
               str.Format(_T(" id    \t: %08X\n"), m_pUARTInfo[i].id);     pDoc->Log(str);
               str.Format(_T(" locid \t: %08X\n"), m_pUARTInfo[i].locid);  pDoc->Log(str);
               str.Format(_T(" serial\t: %s\n"),   serial.m_psz);          pDoc->Log(str);
               str.Format(_T(" desc  \t: %s\n"),   desc.m_psz);            pDoc->Log(str);
               str.Format(_T(" handle\t: %08X\n"), m_pUARTInfo[i].handle); pDoc->Log(str);
               // Check for UART Omniware Serial
               if (serial.m_psz[0] == 'O' && serial.m_psz[1] == '5') {
                  // Add to ComboBox
                  if ((m_pUARTInfo[i].handle == 0 && m_pUARTInfo[i].id == UART_VID_PID) ||
                      (m_pUARTInfo[i].handle == 0 && m_pUARTInfo[i].id == UART_VID_PID_ALT)) {
                     str.Format(_T("UART.%d"), i);
                     m_PortSel.AddString(str);
                     m_PortSel.SetItemData(j++, i);
                     devFound = TRUE;
                  }
               }
            }
            // Show first selection, if any
            m_comPort = (devFound == TRUE) ? 0 : APP_PORT_NONE;
         }
         else {
            str.Format(_T("Error : Maximum Attached Devices Exceeded, %d\n"), devCnt);
            GetDoc()->Log(str, APP_MSG_ERROR);
         }
      }
   }

   // Update Form Items
   OnUpdateConfig(FALSE);

   // Prevent Refresh Focus
   GetDlgItem(IDC_CON_MSG_STATIC)->SetFocus();

}

void CConView::OnBnClickedConConnect()
{
   CString  str, value;
   uint32_t port = 0, sysid, stamp, cmDat;
   uint32_t libRev, sysRev, apiRev;
   int32_t  result = 0;
   INT      speed = 0;
   UCHAR    rev[64] = {0};
   CHAR    *pcap_rev = NULL;
   CHAR    *csc_rev = NULL;
   UCHAR    macip[20] = { 0x6C, 0x2B, 0x59, 0xD6, 0xF5, 0x04, 0xC0, 0xA8, 0x01, 0x3E };
   INT      i;
   INT      curPos = 0;
   UINT     j;

   CappDoc  *pDoc = GetDoc();
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

   CString strTitle;
   strTitle.LoadString(AFX_IDS_APP_TITLE);
   str.Format(_T("%s build %d"), strTitle, BUILD_INC);
   CW2A dev(str);

   // Clear All Current Versions
   str.Format(_T(""));
   GetDlgItem(IDC_CON_FW_REV)->SetWindowText(str);
   GetDlgItem(IDC_CON_SYS_ID)->SetWindowText(str);
   GetDlgItem(IDC_CON_STAMP)->SetWindowText(str);
   GetDlgItem(IDC_CON_FIFO_REV)->SetWindowText(str);
   GetDlgItem(IDC_CON_ADC_REV)->SetWindowText(str);

   // Uncheck All Service Providers
   ((CButton *)GetDlgItem(IDC_CON_SRV_CM))->SetCheck(FALSE);
   ((CButton *)GetDlgItem(IDC_CON_SRV_CP))->SetCheck(FALSE);
   ((CButton *)GetDlgItem(IDC_CON_SRV_DAQ))->SetCheck(FALSE);

   // Uncheck All Clients
   ((CButton *)GetDlgItem(IDC_CON_CLI_CM))->SetCheck(FALSE);
   ((CButton *)GetDlgItem(IDC_CON_CLI_CP))->SetCheck(FALSE);
   ((CButton *)GetDlgItem(IDC_CON_CLI_DAQ))->SetCheck(FALSE);

   //
   // CONNECT
   //
   if (pMainFrm->m_nCom == APP_CON_NONE) {
      // Validate Port Selection, can happen if device
      // is unplugged after a refresh
      if (m_comPort == APP_PORT_NONE) {
         str.Format(_T("Error : No Devices Attached or Selected\n"));
         GetDoc()->Log(str, APP_MSG_ERROR);
      }
      //
      // OPTO DEVICE
      //
      else if (m_comCon == APP_CON_OPTO) {
         // Open the OPTO Port
         m_nCom              = APP_CON_OPTO;
         pMainFrm->m_nCom    = APP_CON_OPTO;
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         // Check for Fast Serial OPTO Device, Baudrate fixed at 12 Mbps
         speed = (m_pOPTOInfo[port].type == 8) ? 0 : ftdi_speed[m_comSpeed];
         result = opto_init(speed, CM_PORT_COM0, port);
         // Check Device ID
         if (result != OPTO_OK) {
            str.Format(_T("Error : Failed to Open OPTO.%d Device\n"), port);
            GetDoc()->Log(str, APP_MSG_ERROR);
            m_nCom              = APP_CON_NONE;
            pMainFrm->m_nCom    = APP_CON_NONE;
            pMainFrm->m_nPort   = APP_PORT_NONE;
            pMainFrm->m_nPortId = 0;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
         }
         else {
            CA2W serial(m_pOPTOInfo[port].serial);
            str.Format(_T("Opened OPTO.%d (%s) for Messaging\n"), port, serial.m_psz);
            GetDoc()->Log(str);
            opto_rev(&libRev, &sysRev, &apiRev);
            str.Format(_T("OPTO.%d : ftd2xx.lib %08X, ftd2xx.sys %08X, optoapi.dll %08X\n"), port, libRev, sysRev, apiRev);
            GetDoc()->Log(str);
            opto_sysid(&sysid, &stamp, &cmDat);
            str.Format(_T("OPTO.%d : sysID %d, stamp %d, cm %08X\n"), port, sysid, stamp, cmDat);
            GetDoc()->Log(str);
            // Assign the Type and Port
            pMainFrm->m_nPort   = m_comPort;
            pMainFrm->m_nPortId = port;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
            // Save Serial for Device Removal Check
            memcpy(pMainFrm->m_chSerial, m_pOPTOInfo[port].serial, 16);
            // Send CM Registration Request
            cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_OPEN, (UCHAR *)dev.m_psz);
            // Check for AutoConnect Port Change
            if (m_autoPort != m_comPort) {
               m_autoPort = m_comPort;
               GetDoc()->SetModifiedFlag(TRUE);
               OnUpdateConfig(TRUE);
            }
         }
      }
      //
      // FIFO DEVICE
      //
      else if (m_comCon == APP_CON_FIFO) {
         // Open the FIFO Port
         m_nCom              = APP_CON_FIFO;
         pMainFrm->m_nCom    = APP_CON_FIFO;
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         result = fifo_init(speed, CM_PORT_COM0, port);
         // Check Device ID
         if (result != FIFO_OK) {
            str.Format(_T("Error : Failed to Open FIFO.%d Device\n"), port);
            GetDoc()->Log(str, APP_MSG_ERROR);
            m_nCom              = APP_CON_NONE;
            pMainFrm->m_nCom    = APP_CON_NONE;
            pMainFrm->m_nPort   = APP_PORT_NONE;
            pMainFrm->m_nPortId = 0;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
         }
         else {
            CA2W serial(m_pFIFOInfo[port].serial);
            str.Format(_T("Opened FIFO.%d (%s) for Messaging\n"), port, serial.m_psz);
            GetDoc()->Log(str);
            fifo_rev(&libRev, &sysRev, &apiRev);
            str.Format(_T("FIFO.%d : ftd2xx.lib %08X, ftd2xx.sys %08X, fifoapi.dll %08X\n"), port, libRev, sysRev, apiRev);
            GetDoc()->Log(str);
            fifo_sysid(&sysid, &stamp, &cmDat);
            str.Format(_T("FIFO.%d : sysID %d, stamp %d, cm %08X\n"), port, sysid, stamp, cmDat);
            GetDoc()->Log(str);
            // Assign the Type and Port
            pMainFrm->m_nPort   = m_comPort;
            pMainFrm->m_nPortId = port;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
            // Save Serial for Device Removal Check
            memcpy(pMainFrm->m_chSerial, m_pFIFOInfo[port].serial, 16);
            // Send CM Registration Request
            cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_OPEN, (UCHAR *)dev.m_psz);
            // Check for AutoConnect Port Change
            if (m_autoPort != m_comPort) {
               m_autoPort = m_comPort;
               GetDoc()->SetModifiedFlag(TRUE);
               OnUpdateConfig(TRUE);
            }
         }
      }
      //
      // COM DEVICE
      //
      else if (m_comCon == APP_CON_COM) {
         // Open the COM Port
         m_nCom              = APP_CON_COM;
         pMainFrm->m_nCom    = APP_CON_COM;
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         speed = com_speed[m_comSpeed];
         result = com_init(speed, CM_PORT_COM0, port);
         // Check Device ID
         if (result != COM_OK) {
            str.Format(_T("Error : Failed to Open COM.%d Device\n"), port);
            GetDoc()->Log(str, APP_MSG_ERROR);
            m_nCom              = APP_CON_NONE;
            pMainFrm->m_nCom    = APP_CON_NONE;
            pMainFrm->m_nPort   = APP_PORT_NONE;
            pMainFrm->m_nPortId = 0;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
         }
         else {
            CA2W serial(m_pCOMInfo[port].desc);
            str.Format(_T("Opened COM.%d (%s) for Messaging\n"), port, serial.m_psz);
            GetDoc()->Log(str);
            com_rev(&libRev, &sysRev, &apiRev);
            str.Format(_T("COM.%d : wsc64.lib %08X, comapi.dll %08X\n"), port, libRev, apiRev);
            GetDoc()->Log(str);
            com_sysid(&sysid, &stamp, &cmDat);
            str.Format(_T("COM.%d : sysID %d, stamp %d, cm %08X\n"), port, sysid, stamp, cmDat);
            GetDoc()->Log(str);
            // Assign the Type and Port
            pMainFrm->m_nPort   = m_comPort;
            pMainFrm->m_nPortId = port;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
            // Save Serial for Device Removal Check
            memcpy(pMainFrm->m_chSerial, m_pCOMInfo[port].desc, 16);
            // Send CM Registration Request
            cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_OPEN, (UCHAR *)dev.m_psz);
            // Check for AutoConnect Port Change
            if (m_autoPort != m_comPort) {
               m_autoPort = m_comPort;
               GetDoc()->SetModifiedFlag(TRUE);
               OnUpdateConfig(TRUE);
            }
         }
      }
      //
      // LAN DEVICE
      //
      else if (m_comCon == APP_CON_LAN) {
         // Open the LAN Port
         m_nCom              = APP_CON_LAN;
         pMainFrm->m_nCom    = APP_CON_LAN;
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         // Load the MAC & IP Addresses
         for (i=0,curPos=0;i<6;i++) {
            value = m_macAddress.Tokenize(_T(":"), curPos);
            swscanf_s(value, _T("%x"), &j);
            macip[i+10] = (UCHAR)j;
         }
         for (i=0,curPos=0;i<4;i++) {
            value = m_ipAddress.Tokenize(_T("."), curPos);
            swscanf_s(value, _T("%d"), &j);
            macip[i+16] = (UCHAR)j;
         }
         result = lan_init(speed, CM_PORT_COM0, port, pDoc->m_ini->conIpPort, macip);
         // Check Device ID
         if (result != LAN_OK) {
            str.Format(_T("Error : Failed to Open LAN.%d Device\n"), port);
            GetDoc()->Log(str, APP_MSG_ERROR);
            m_nCom              = APP_CON_NONE;
            pMainFrm->m_nCom    = APP_CON_NONE;
            pMainFrm->m_nPort   = APP_PORT_NONE;
            pMainFrm->m_nPortId = 0;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
         }
         else {
            CA2W desc(m_pLANInfo[port].desc);
            str.Format(_T("Opened LAN.%d (%s) for Messaging\n"), port, desc.m_psz);
            GetDoc()->Log(str);
            lan_rev(&pcap_rev, &sysRev, &apiRev);
            str.Format(_T("LAN.%d : lanapi.dll, %08X\n"), port, apiRev);
            GetDoc()->Log(str);
            CA2W rev(pcap_rev);
            str.Format(_T("LAN.%d : wpcap64.lib, %s\n"), port, rev.m_psz);
            GetDoc()->Log(str);
            str.Format(_T("LAN.%d : npcap filter, dst %d.%d.%d.%d\n"), port, macip[6], macip[7], macip[8], macip[9]);
            GetDoc()->Log(str);
            lan_sysid(&sysid, &stamp, &cmDat);
            str.Format(_T("LAN.%d : sysID %d, stamp %d, cm %08X\n"), port, sysid, stamp, cmDat);
            GetDoc()->Log(str);
            // Assign the Type and Port
            pMainFrm->m_nPort   = m_comPort;
            pMainFrm->m_nPortId = port;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
            // Send CM Registration Request
            cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_OPEN, (UCHAR *)dev.m_psz);
            // Check for AutoConnect Port Change
            if (m_autoPort != m_comPort) {
               m_autoPort = m_comPort;
               GetDoc()->SetModifiedFlag(TRUE);
               OnUpdateConfig(TRUE);
            }
         }
      }
      //
      // UDP DEVICE
      //
      else if (m_comCon == APP_CON_UDP) {
         // Open the LAN Port
         m_nCom              = APP_CON_UDP;
         pMainFrm->m_nCom    = APP_CON_UDP;
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         // Load the MAC & IP Addresses
         for (i=0,curPos=0;i<6;i++) {
            value = m_macAddress.Tokenize(_T(":"), curPos);
            swscanf_s(value, _T("%x"), &j);
            macip[i+10] = (UCHAR)j;
         }
         for (i=0,curPos=0;i<4;i++) {
            value = m_ipAddress.Tokenize(_T("."), curPos);
            swscanf_s(value, _T("%d"), &j);
            macip[i+16] = (UCHAR)j;
         }
         result = udp_init(speed, CM_PORT_COM0, port, pDoc->m_ini->conIpPort, macip);
         // Check Device ID
         if (result != UDP_OK) {
            str.Format(_T("Error : Failed to Open UDP.%d Device\n"), port);
            GetDoc()->Log(str, APP_MSG_ERROR);
            m_nCom              = APP_CON_NONE;
            pMainFrm->m_nCom    = APP_CON_NONE;
            pMainFrm->m_nPort   = APP_PORT_NONE;
            pMainFrm->m_nPortId = 0;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
         }
         else {
            str.Format(_T("Opened UDP.%d for Messaging, Connecting to %s @ %d\n"), 
                  (INT)0, pDoc->m_ini->conIpAddr, pDoc->m_ini->conIpPort);
            GetDoc()->Log(str);
            udp_rev(&csc_rev, &sysRev, &apiRev);
            str.Format(_T("UDP.%d : lanapi.dll, %08X\n"), port, apiRev);
            GetDoc()->Log(str);
            // Assign the Type and Port
            pMainFrm->m_nPort   = m_comPort;
            pMainFrm->m_nPortId = port;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
            // Send CM Registration Request
            cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_OPEN, (UCHAR *)dev.m_psz);
         }
      }
      //
      // TCP DEVICE (USING CMSOCK)
      //
      else if (m_comCon == APP_CON_TCP) {
         // Open the TCP Port
         m_nCom              = APP_CON_TCP;
         pMainFrm->m_nCom    = APP_CON_TCP;
         // Start Message Client
         if (m_client.Open(pDoc->m_ini->conIpAddr, pDoc->m_ini->conIpPort)) {
            m_client.ListenEx(4, 0, &m_queue, this, m_id);
            str.Format(_T("Opened TCP.%d for Messaging, Connecting to %s @ %d\n"), 
                  (INT)0, pDoc->m_ini->conIpAddr, pDoc->m_ini->conIpPort);
            GetDoc()->Log(str);
            // Assign the Type and Port
            m_comPort = 0;
            pMainFrm->m_nPort   = m_comPort;
            pMainFrm->m_nPortId = 0;
            pMainFrm->m_nComErr = 0;
            // Send CM Registration Request
            cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_OPEN, (UCHAR *)dev.m_psz);
         }
         else {
            m_client.Close();
            str.Format(_T("Error : TCP Client Not Started, %08X\n"), GetLastError());
            GetDoc()->Log(str, APP_MSG_ERROR);
            m_nCom              = APP_CON_NONE;
            pMainFrm->m_nCom    = APP_CON_NONE;
            pMainFrm->m_nComErr = 0;
            pMainFrm->m_nPort   = APP_PORT_NONE;
            pMainFrm->m_nPortId = 0;
            pMainFrm->m_hCom    = NULL;
         }
      }
      //
      // UART DEVICE
      //
      else if (m_comCon == APP_CON_UART) {
         // Open the OPTO Port
         m_nCom              = APP_CON_UART;
         pMainFrm->m_nCom    = APP_CON_UART;
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         // Check for UART Device, Baudrate selectable
         speed = (m_pUARTInfo[port].type == 8) ? 0 : uart_speed[m_comSpeed];
         result = uart_init(speed, CM_PORT_COM0, port);
         // Check Device ID
         if (result != UART_OK) {
            str.Format(_T("Error : Failed to Open UART.%d Device\n"), port);
            GetDoc()->Log(str, APP_MSG_ERROR);
            m_nCom              = APP_CON_NONE;
            pMainFrm->m_nCom    = APP_CON_NONE;
            pMainFrm->m_nPort   = APP_PORT_NONE;
            pMainFrm->m_nPortId = 0;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
         }
         else {
            CA2W serial(m_pUARTInfo[port].serial);
            str.Format(_T("Opened UART.%d (%s) for Messaging\n"), port, serial.m_psz);
            GetDoc()->Log(str);
            opto_rev(&libRev, &sysRev, &apiRev);
            str.Format(_T("UART.%d : ftd2xx.lib %08X, ftd2xx.sys %08X, optoapi.dll %08X\n"), port, libRev, sysRev, apiRev);
            GetDoc()->Log(str);
            opto_sysid(&sysid, &stamp, &cmDat);
            str.Format(_T("UART.%d : sysID %d, stamp %d, cm %08X\n"), port, sysid, stamp, cmDat);
            GetDoc()->Log(str);
            // Assign the Type and Port
            pMainFrm->m_nPort   = m_comPort;
            pMainFrm->m_nPortId = port;
            pMainFrm->m_hCom    = NULL;
            pMainFrm->m_nComErr = 0;
            // Save Serial for Device Removal Check
            memcpy(pMainFrm->m_chSerial, m_pUARTInfo[port].serial, 16);
            // Send CM Registration Request
            cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_OPEN, (UCHAR *)dev.m_psz);
            // Check for AutoConnect Port Change
            if (m_autoPort != m_comPort) {
               m_autoPort = m_comPort;
               GetDoc()->SetModifiedFlag(TRUE);
               OnUpdateConfig(TRUE);
            }
         }
      }
   }
   //
   // DISCONNECT
   //
   else {
      // Validate Port Selection, can happen if device
      // is unplugged after a refresh
      if (m_comPort == APP_PORT_NONE) {
         str.Format(_T("Error : No Devices Attached or Selected\n"));
         GetDoc()->Log(str, APP_MSG_ERROR);
      }
      //
      // OPTO DEVICE
      //
      else if (m_comCon == APP_CON_OPTO) {
         // Close Registration to Hardware
         cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_CLOSE, (UCHAR *)dev.m_psz);
         // Close the Open Port
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         CA2W serial(m_pOPTOInfo[port].serial);
         str.Format(_T("Closed OPTO.%d (%s)\n"), port, serial.m_psz);
         GetDoc()->Log(str);
         // Delete Serial for Device Removal Check
         memset(pMainFrm->m_chSerial, 0, 16);
         opto_final();
         // No Connection
         m_nCom              = APP_CON_NONE;
         pMainFrm->m_nCom    = APP_CON_NONE;
         pMainFrm->m_nComErr = 0;
         pMainFrm->m_nPort   = APP_PORT_NONE;
         pMainFrm->m_nPortId = 0;
         pMainFrm->m_hCom    = NULL;
      }
      //
      // FIFO DEVICE
      //
      else if (m_comCon == APP_CON_FIFO) {
         // Close Registration to Hardware
         cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_CLOSE, (UCHAR *)dev.m_psz);
         // Close the Open Port
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         CA2W serial(m_pFIFOInfo[port].serial);
         str.Format(_T("Closed OPTO.%d (%s)\n"), port, serial.m_psz);
         GetDoc()->Log(str);
         // Delete Serial for Device Removal Check
         memset(pMainFrm->m_chSerial, 0, 16);
         fifo_final();
         // No Connection
         m_nCom              = APP_CON_NONE;
         pMainFrm->m_nCom    = APP_CON_NONE;
         pMainFrm->m_nComErr = 0;
         pMainFrm->m_nPort   = APP_PORT_NONE;
         pMainFrm->m_nPortId = 0;
         pMainFrm->m_hCom    = NULL;
      }
      //
      // COM DEVICE
      //
      else if (m_comCon == APP_CON_COM) {
         // Close Registration to Hardware
         cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_CLOSE, (UCHAR *)dev.m_psz);
         // Close the Open Port
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         CA2W serial(m_pCOMInfo[port].serial);
         str.Format(_T("Closed COM.%d (%s)\n"), port, serial.m_psz);
         GetDoc()->Log(str);
         // Delete Serial for Device Removal Check
         memset(pMainFrm->m_chSerial, 0, 16);
         com_final();
         // No Connection
         m_nCom              = APP_CON_NONE;
         pMainFrm->m_nCom    = APP_CON_NONE;
         pMainFrm->m_nComErr = 0;
         pMainFrm->m_nPort   = APP_PORT_NONE;
         pMainFrm->m_nPortId = 0;
         pMainFrm->m_hCom    = NULL;
      }
      //
      // LAN DEVICE
      //
      else if (m_comCon == APP_CON_LAN) {
         // Close Registration to Hardware
         cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_CLOSE, (UCHAR *)dev.m_psz);
         // Close the Open Port
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         CA2W desc(m_pLANInfo[port].desc);
         str.Format(_T("Closed LAN.%d (%s)\n"), port, desc.m_psz);
         GetDoc()->Log(str);
         lan_final();
         // No Connection
         m_nCom              = APP_CON_NONE;
         pMainFrm->m_nCom    = APP_CON_NONE;
         pMainFrm->m_nComErr = 0;
         pMainFrm->m_nPort   = APP_PORT_NONE;
         pMainFrm->m_nPortId = 0;
         pMainFrm->m_hCom    = NULL;
      }
      //
      // UDP DEVICE
      //
      else if (m_comCon == APP_CON_UDP) {
         // Close Registration to Hardware
         cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_CLOSE, (UCHAR *)dev.m_psz);
         str.Format(_T("Closed UDP.%d, %s @ %d\n"),
               (INT)0, pDoc->m_ini->conIpAddr, pDoc->m_ini->conIpPort);
         GetDoc()->Log(str);
         // Close the Open Port
         udp_final();
         // No Connection
         m_nCom              = APP_CON_NONE;
         pMainFrm->m_nCom    = APP_CON_NONE;
         pMainFrm->m_nComErr = 0;
         pMainFrm->m_nPort   = APP_PORT_NONE;
         pMainFrm->m_nPortId = 0;
         pMainFrm->m_hCom    = NULL;
      }
      //
      // TCP DEVICE
      //
      else if (m_comCon == APP_CON_TCP) {
         // Close Registration to Hardware
         cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_CLOSE, (UCHAR *)dev.m_psz);
         // Set timer to close connection
         SetTimer(ID_TIMER_TCP_CLOSE, 50, NULL);
      }
      //
      // UART DEVICE
      //
      else if (m_comCon == APP_CON_UART) {
         // Close Registration to Hardware
         cm_send_reg_req(APP_DEVID, CM_PORT_COM0, CM_REG_CLOSE, (UCHAR *)dev.m_psz);
         // Close the Open Port
         port = (UINT)m_PortSel.GetItemData(m_comPort);
         CA2W serial(m_pUARTInfo[port].serial);
         str.Format(_T("Closed UART.%d (%s)\n"), port, serial.m_psz);
         GetDoc()->Log(str);
         // Delete Serial for Device Removal Check
         memset(pMainFrm->m_chSerial, 0, 16);
         uart_final();
         // No Connection
         m_nCom              = APP_CON_NONE;
         pMainFrm->m_nCom    = APP_CON_NONE;
         pMainFrm->m_nComErr = 0;
         pMainFrm->m_nPort   = APP_PORT_NONE;
         pMainFrm->m_nPortId = 0;
         pMainFrm->m_hCom    = NULL;
      }
   }

   // Remove Focus Square
   GetDlgItem(IDC_CON_MSG_STATIC)->SetFocus();

}

//
// INBOUND MESSAGE CALLBACK FROM TCP DEVICE
//
afx_msg LRESULT CConView::OnSockNewMsg(WPARAM wParam, LPARAM lParam)
{
	CCMsg *pQMsg = NULL;
   pcm_msg_t pMsg;
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

	while (pQMsg = m_queue.Remove()) {
      pMsg = (pcm_msg_t)pQMsg->m_pBody;
      //
      // CM PIPE DIRECT ROUTING
      //
      if (pMsg->h.dst_cmid == CM_ID_PIPE) {
         cm_pipe_send((pcm_pipe_t)pMsg, (UINT)pQMsg->m_len);
      }
      //
      // CM MESSAGE NORMAL ROUTING
      //
      else {
         // mark message for deletion
         pMsg->h.keep = 0;
         cm_route(pMsg);
      }
      // Delete the Header, not the Body
		delete pQMsg;
	}

	return 0;
}

afx_msg LRESULT CConView::OnSockDone(WPARAM wParam, LPARAM lParam)
{
   CString str;
   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
   m_client.Close();
   m_nCom              = APP_CON_NONE;
   pMainFrm->m_nCom    = APP_CON_NONE;
   pMainFrm->m_nComErr = 0;
   pMainFrm->m_nPort   = APP_PORT_NONE;
   pMainFrm->m_nPortId = 0;
   pMainFrm->m_hCom    = NULL;
   str.Format(_T("Closed TCP.%d (%d)\n"), (INT)0, GetDoc()->m_ini->conIpPort);
   GetDoc()->Log(str);
	return 0;
}

afx_msg LRESULT CConView::OnSockConnect(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void CConView::OnUpdatePortSpeed(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_comCon == APP_CON_COM && m_nCom == APP_CON_NONE);
}

void CConView::OnUpdateConnect(CCmdUI* pCmdUI)
{
   if  (m_nCom != m_nComDelta) {
      m_nComDelta = m_nCom;
      if (m_nCom == APP_CON_NONE) {
         m_Connect.SetWindowText(_T("Connect"));
         m_Refresh.EnableWindow(TRUE);
      }
      else {
         m_Connect.SetWindowText(_T("Disconnect"));
         m_Refresh.EnableWindow(FALSE);
      }
   }
	pCmdUI->Enable(m_comPort != APP_PORT_NONE);
}

void CConView::OnUpdateType(CCmdUI* pCmdUI)
{
   pCmdUI->Enable(m_nCom == APP_CON_NONE);
}

void CConView::OnUpdatePort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nCom == APP_CON_NONE);
}

void CConView::OnUpdateIpAddr(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nCom == APP_CON_NONE);
}

void CConView::OnUpdateMacAddr(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nCom == APP_CON_NONE && m_comCon != APP_CON_UDP);
}

void CConView::OnUpdateIpPort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nCom == APP_CON_NONE);
}

afx_msg LRESULT CConView::OnClosing(WPARAM wParam, LPARAM lParam)
{
   // Close the Open Connection
   if (m_nCom != APP_CON_NONE) {
      OnBnClickedConConnect();
      Sleep(50);
   }
   return 0;
}

afx_msg LRESULT CConView::OnDevRemove(WPARAM wParam, LPARAM lParam)
{
   // If not connected then refresh ports
   if (m_nCom == APP_CON_NONE) {
      // Allow Time for Device Removal
      SetTimer(ID_TIMER_REFRESH, 1000, NULL);
   }
   // Close the Open Connection
   else if (lParam == TRUE) {
      if ((m_nCom == APP_CON_OPTO && wParam == APP_CON_OPTO) ||
          (m_nCom == APP_CON_FIFO && wParam == APP_CON_FIFO)) {
         // Close
         OnBnClickedConConnect();
         // Refresh
         SetTimer(ID_TIMER_REFRESH, 1000, NULL);
      }
   }
   else {
      // Refresh
      SetTimer(ID_TIMER_REFRESH, 1000, NULL);
   }
   return 0;
}

afx_msg LRESULT CConView::OnDevArrive(WPARAM wParam, LPARAM lParam)
{
   // If not connected then refresh ports
   if (m_nCom == APP_CON_NONE) {
      // Allow Time for Device to Attach
      SetTimer(ID_TIMER_REFRESH, 3500, NULL);
   }
   return 0;
}

afx_msg LRESULT CConView::OnDevError(WPARAM wParam, LPARAM lParam)
{
   // If not connected then refresh ports
   if (m_nCom == APP_CON_NONE) {
      // Allow Time for Device Removal
      SetTimer(ID_TIMER_REFRESH, 2000, NULL);
   }
   // Close the Open Connection
   else {
      // Close
      OnBnClickedConConnect();
      // Refresh
      SetTimer(ID_TIMER_REFRESH, 2000, NULL);
   }
   return 0;
}

BOOL CConView::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
   BOOL bReturn = CWnd::OnDeviceChange (nEventType, dwData);

   PDEV_BROADCAST_DEVICEINTERFACE devInterface;
   PDEV_BROADCAST_HANDLE devHandle;

	devInterface = (PDEV_BROADCAST_DEVICEINTERFACE)dwData;

	if (devInterface != NULL && devInterface->dbcc_devicetype == DBT_DEVTYP_HANDLE) {
      devHandle = (PDEV_BROADCAST_HANDLE)dwData;
      if (nEventType == DBT_DEVICEREMOVECOMPLETE) {
         if (m_nCom == APP_CON_COM) {
            // Close
            OnBnClickedConConnect();
            // Refresh
            SetTimer(ID_TIMER_REFRESH, 1000, NULL);
         }
      }
	}

   return bReturn;
}

void CConView::OnCbnSelchangeConPortType()
{
   UINT     i,j;
   CString  str;
   CappDoc *pDoc = GetDoc();
   // Retrieve data from Form
   UpdateData(TRUE);
   if (pDoc->m_ini->conComCon != m_comCon) {
      pDoc->SetModifiedFlag(TRUE);
      OnUpdateConfig(TRUE);
   }
   // Clear ComboBoxes
   m_PortSel.ResetContent();
   m_SpeedSel.ResetContent();
   m_comPort = APP_PORT_NONE;
   m_comSpeed = APP_SPEED_NONE;
   // Load available Speeds
   if (m_comCon == APP_CON_COM) {
      for (i=0,j=0;i<DIM(com_speed);i++) {
         if (com_speed[i] < 1000000)
            str.Format(_T("%.1f Kbps"), (double)com_speed[i] / 1000);
         else
            str.Format(_T("%.1f Mbps"), (double)com_speed[i] / 1000000);
         m_SpeedSel.AddString(str);
         j = (CFG_BAUD_RATE == com_speed[i]) ? i : j;
      }
      // Select Preferred Speed, default is 0
      m_comSpeed = j;
   }
   else if (m_comCon == APP_CON_OPTO) {
      for (i=0,j=0;i<DIM(ftdi_speed);i++) {
         if (ftdi_speed[i] < 1000000)
            str.Format(_T("%.1f Kbps"), (double)ftdi_speed[i] / 1000);
         else
            str.Format(_T("%.1f Mbps"), (double)ftdi_speed[i] / 1000000);
         m_SpeedSel.AddString(str);
         j = (CFG_BAUD_RATE == ftdi_speed[i]) ? i : j;
      }
      // Select Preferred Speed, default is 0
      m_comSpeed = j;
   }
   else if (m_comCon == APP_CON_FIFO) {
      for (i=0,j=0;i<DIM(fifo_speed);i++) {
         if (fifo_speed[i] < 1000000)
            str.Format(_T("%.1f Kbps"), (double)fifo_speed[i] / 1000);
         else
            str.Format(_T("%.1f Mbps"), (double)fifo_speed[i] / 1000000);
         m_SpeedSel.AddString(str);
         j = (CFG_BAUD_RATE == fifo_speed[i]) ? i : j;
      }
      // Select Preferred Speed, default is 0
      m_comSpeed = j;
   }
   else if (m_comCon == APP_CON_UART) {
      for (i=0,j=0;i<DIM(uart_speed);i++) {
         if (uart_speed[i] < 1000000)
            str.Format(_T("%.1f Kbps"), (double)uart_speed[i] / 1000);
         else
            str.Format(_T("%.1f Mbps"), (double)uart_speed[i] / 1000000);
         m_SpeedSel.AddString(str);
         j = (CFG_BAUD_RATE == uart_speed[i]) ? i : j;
      }
      // Select Preferred Speed, default is 0
      m_comSpeed = j;
   }
   OnUpdateConfig(FALSE);

   OnBnClickedConRefresh();
}

void CConView::OnCbnSelchangeConPortSpeed()
{
   UpdateData(TRUE);
}

void CConView::OnCbnSelchangeConPortSel()
{
   UpdateData(TRUE);
}

