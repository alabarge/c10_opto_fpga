// ConView.h : interface of the CConView class
//

#pragma once

#include "MainFrm.h"
#include "comapi.h"
#include "optoapi.h"
#include "uartapi.h"
#include "fifoapi.h"
#include "lanapi.h"
#include "udpapi.h"
#include "CMSock.h"
#include "timer.h"

class CConView : public CViewEx
{
protected: // create from serialization only
	CConView();
	DECLARE_DYNCREATE(CConView)

public:
	enum{ IDD = IDD_CONNECT };

   INT      m_comCon;
   CString  m_ipAddress;
   CString  m_macAddress;
   INT      m_autoConnect;
   INT      m_autoPort;
   CString  m_conIpPort;

// Attributes
public:

// Operations
public:

// Overrides
public:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CConView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

   void OnUpdateConfig(BOOL bDir);
   void OnRangeUpdateED(UINT nID);
   void OnComboUpdateED(UINT nID);
   void OnCheckUpdateED(UINT nID);

   BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

   CComboBox      m_PortSel;
   CComboBox      m_SpeedSel;
   CMFCButton     m_Refresh;
   CButton        m_Connect;
   CFont          m_font;
            
   uart_dev_info_t  *m_pUARTInfo;
   opto_dev_info_t  *m_pOPTOInfo;
   fifo_dev_info_t  *m_pFIFOInfo;
   com_dev_info_t   *m_pCOMInfo;
   lan_dev_info_t   *m_pLANInfo;

   INT            m_comPort;
   INT            m_comSpeed;
   BOOL           m_autoRefresh;
   CHRTimer       m_timer;

   CCMQueue       m_queue;
	CCMClient      m_client;
   UINT           m_id;

   INT            m_devcnt;
   INT            m_nCom;
   INT            m_nComDelta;
   CHAR           m_hCM;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
   afx_msg LRESULT OnSockNewMsg(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnSockDone(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnSockConnect(WPARAM wParam, LPARAM lParam);
public:
   afx_msg void OnBnClickedConRefresh();
   virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   afx_msg void OnBnClickedConConnect();
	afx_msg void OnUpdatePortSpeed(CCmdUI* pCmdUI);
	afx_msg void OnUpdateConnect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateType(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePort(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIpAddr(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMacAddr(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIpPort(CCmdUI* pCmdUI);

   afx_msg LRESULT OnClosing(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnDevRemove(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnDevArrive(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnDevError(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnCmMsg(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnUDPMsg(WPARAM wParam, LPARAM lParam);

   afx_msg void OnTimer(UINT_PTR nIDEvent);
   afx_msg void OnCbnSelchangeConPortType();
   afx_msg void OnCbnSelchangeConPortSpeed();
   afx_msg void OnCbnSelchangeConPortSel();
};
