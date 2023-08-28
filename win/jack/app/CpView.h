
// CpView.h : interface of the CCpView class
//

#pragma once

#include "MainFrm.h"
#include "resource.h"

class CCpView : public CViewEx
{
protected: // create from serialization only
	CCpView();
	DECLARE_DYNCREATE(CCpView)

public:
	enum{ IDD = IDD_CP };

   CString  m_cpAddr;
   CString  m_cpValue;
   INT      m_cpType;
   INT      m_cpWR;
   CString  m_cpTrace;
   INT      m_cpCI;
   INT      m_cpCM;
   INT      m_cpIND;
   INT      m_cpIRQ;
   INT      m_cpMEM;
   INT      m_cpDVR;
   INT      m_cpRUN;
   INT      m_cpERR;
   INT      m_cpID;
   INT      m_cpROUTE;
   INT      m_cpUART;

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
	virtual ~CCpView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   void OnUpdateConfig(BOOL bDir);
   void OnRangeUpdateED(UINT nID);
   void OnComboUpdateED(UINT nID);
   void OnCheckUpdateED(UINT nID);

   void CheckToControl();
   void ControlToCheck();

   CHAR           m_hCM;
   CRichEx        m_Edit;

   uint32_t       m_fw_ver;
   uint32_t       m_sysid;
   uint32_t       m_stamp_epoch;
   uint32_t       m_stamp_date;
   uint32_t       m_stamp_time;
   uint32_t       m_trace;
   uint32_t       m_feature;
   uint32_t       m_debug;

   // CM Related
   rxq_t    m_rxq;
   uint8_t  m_handle;
   uint8_t  m_cmid;
   uint32_t cp_init(void);
   uint32_t cp_qmsg(pcm_msg_t msg);
   static uint32_t cp_qmsg_static(pcm_msg_t msg, void *data);
   uint32_t cp_msg(pcm_msg_t msg);
   uint32_t cp_timer(pcm_msg_t msg);
//   static DWORD WINAPI cp_thread_start(LPVOID data);
   static UINT cp_thread_start(LPVOID data);
   DWORD    cp_thread(void);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
   virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   afx_msg void OnBnClickedCpGet();
   afx_msg void OnBnClickedCpSet();
   afx_msg void OnBnClickedCpTraceGet();
   afx_msg void OnBnClickedCpTraceSet();
   afx_msg void OnBnClickedCpTraceOn();
   afx_msg void OnBnClickedCpTraceOff();
   afx_msg void OnTimer(UINT_PTR nIDEvent);

   afx_msg LRESULT OnClosing(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnUpdate(WPARAM wParam, LPARAM lParam);
};

