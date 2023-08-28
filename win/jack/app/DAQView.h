
// DAQView.h : interface of the CDAQView class
//

#pragma once

#include "MainFrm.h"
#include "resource.h"
#include "timer.h"
#include "daq_msg.h"
#include "lanapi.h"

class CDAQView : public CViewEx
{
protected: // create from serialization only
	CDAQView();
	DECLARE_DYNCREATE(CDAQView)

public:
	enum{ IDD = IDD_DAQ };

   CString  m_daqFile;
   INT      m_daqToFile;
   INT      m_daqHex;
   INT      m_daqShow;
   INT      m_daqRamp;
   INT      m_daqFloat;
   INT      m_daqPlot;
   INT      m_daqRaw;
   CString  m_daqPackets;

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
	virtual ~CDAQView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   void OnUpdateConfig(BOOL bDir);
   void OnRangeUpdateED(UINT nID);
   void OnComboUpdateED(UINT nID);
   void OnCheckUpdateED(UINT nID);
   void MemToEditRun(void);
   void OnPlotRun(void);
   void OnPipeMsgRun(pcm_pipe_daq_t pipe);

   CHAR     m_hCM;
   CRichEx  m_Edit;
   CMFCEditEx m_File;
   CStdioFile m_pFile;
   CStdioFile m_pInFile;
   CString  m_capFile;

   struct   tm   m_t;

   BOOL     m_bRunning;
   BOOL     m_bShow;

   UINT     m_nPktCnt;
   UINT     m_nBlkCnt;
   UINT     m_seqID;

   INT     *m_pADC;
   INT      m_nXfer;
   DOUBLE   m_xferLen[10];
   DOUBLE   m_deltaT[10];
   UINT     m_lastStamp_us;
   UINT     m_adcRate;

   UCHAR    m_msgBody[CM_MAX_MSG_PAYLOAD_INT8U];

   CHRTimer m_hrTimer;

   // CM Related
   rxq_t    m_rxq;
   uint8_t  m_handle;
   uint8_t  m_cmid;
   uint32_t daq_init(void);
   uint32_t daq_qmsg(pcm_msg_t msg);
   static uint32_t daq_qmsg_static(pcm_msg_t msg, void *data);
   uint32_t daq_msg(pcm_msg_t msg);
   uint32_t daq_timer(pcm_msg_t msg);
   static UINT daq_thread_start(LPVOID data);
   DWORD daq_thread(void);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   afx_msg void OnTimer(UINT_PTR nIDEvent);
   virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
   afx_msg void OnBnClickedDAQStart();
   afx_msg LRESULT OnClosing(WPARAM wParam, LPARAM lParam);
};

