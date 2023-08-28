
// CpMemView.h : interface of the CCpMemView class
//

#pragma once

#include "MainFrm.h"
#include "resource.h"
#include "RichEx.h"

#define  CP_MAX_BUFFER      APP_MAX_CP_MEM_SIZE
#define  CP_MAX_INT8U      (CP_MAX_BUFFER)
#define  CP_MAX_INT16U     (CP_MAX_BUFFER / sizeof(USHORT))
#define  CP_MAX_INT32U     (CP_MAX_BUFFER / sizeof(UINT))

double rand_normal(double mean, double stddev);
void rand_seed(unsigned int seed);

class CCpMemView : public CViewEx
{
protected: // create from serialization only
	CCpMemView();
	DECLARE_DYNCREATE(CCpMemView)

public:
	enum{ IDD = IDD_CP_MEM };

   CString  m_cpMemFile;
   CString  m_cpMemStart;
   CString  m_cpMemLength;
   INT      m_cpMemType;
   INT      m_cpMemBin;
   INT      m_cpMemHex;
   INT      m_cpMemAll;
   INT      m_cpMemCycle;
   INT      m_cpMemAWGN;
   INT      m_cpMemVerify;
   INT      m_cpMemShow;
   INT      m_cpMemToFile;

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
	virtual ~CCpMemView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
   void OnUpdateConfig(BOOL bDir);
   void OnRangeUpdateED(UINT nID);
   void OnComboUpdateED(UINT nID);
   void OnCheckUpdateED(UINT nID);
   void MemToEditBox(void);
   void ReadMemData(void);
   void WriteMemData(void);
   void OnBlkRdResp(pcm_msg_t pMsg);
   void OnBlkWrResp(pcm_msg_t pMsg);
   void OnMemValidate(void);
   void OnPlot(void);
   void OnPlotPipe(WPARAM wParam, LPARAM lParam);

   CHAR     m_hCM;
   CRichEx  m_Edit;
   CMFCEditEx m_File;
   CStdioFile m_outFile;

   CString  m_xferRate;

   UINT     m_nXfer;
   UINT     m_numCollect;
   UINT     m_numRequest;
   UINT     m_memIndex;
   BOOL     m_bGetting;
   BOOL     m_bSetting;
   BOOL     m_bTesting;
   BOOL     m_bStreaming;
   UINT     m_passNumber;
   UINT     m_lenDel;
   BOOL     m_bShow;
   UINT     m_nTimeStart;
   UINT     m_samCnt;

   UINT     m_nError;
   BOOL     m_bOnce;
   UINT     m_nPktCnt;
   UINT     m_nSeqID;

   FLOAT    m_samData[2048];

   UCHAR    m_msgBody[CM_MAX_MSG_PAYLOAD_INT8U];

   // CM Related
   rxq_t m_rxq;
   uint8_t  m_handle;
   uint8_t  m_cmid;
   uint32_t cpm_init(void);
   uint32_t cpm_qmsg(pcm_msg_t msg);
   static uint32_t cpm_qmsg_static(pcm_msg_t msg, void *data);
   uint32_t cpm_msg(pcm_msg_t msg);
   uint32_t cpm_timer(pcm_msg_t msg);
   static UINT cpm_thread_start(LPVOID data);
//   static DWORD WINAPI cpm_thread_start(LPVOID data);
   DWORD cpm_thread(void);
   
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   afx_msg void OnTimer(UINT_PTR nIDEvent);
   virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
   afx_msg LRESULT OnClosing(WPARAM wParam, LPARAM lParam);
   afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
   afx_msg void OnBnClickedCpMemRead();
   afx_msg void OnBnClickedCpMemWrite();
   afx_msg void OnBnClickedCpMemGet();
   afx_msg void OnBnClickedCpMemSet();
   afx_msg void OnBnClickedCpMemClear();
   afx_msg void OnBnClickedCpMemTest();
};

