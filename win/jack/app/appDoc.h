
// appDoc.h : interface of the CappDoc class
//
//

#pragma once

#include "MainFrm.h"

#define  DOC_INT           0
#define  DOC_UINT          1
#define  DOC_HEX2          2
#define  DOC_HEX4          3
#define  DOC_HEX8          4
#define  DOC_DBL           5
#define  DOC_DBL0          6
#define  DOC_DBL2          7
#define  DOC_DBL4          8
#define  DOC_DBL6          9
#define  DOC_DBL8          10
#define  DOC_STR           11
#define  DOC_FLT           12

#define  DOC_TAG_PARM      0x00000001
#define  DOC_TAG_FILE_RD   0x00000002
#define  DOC_TAG_COMBO     0x00000004
#define  DOC_TAG_BOOL      0x00000008
#define  DOC_TAG_TITLE     0x000000F0
#define  DOC_TAG_FILE_WR   0x00000100
#define  DOC_TAG_SPIN      0x00000200

#define  DOC_TAG_TITLES    {_T("CPU Plot Settings"), _T("DAQ Plot Settings"), _T(""), _T(""), _T(""), _T(""), _T(""), _T("")}

#define  DOC_ERR_NONE      0x00000000
#define  DOC_ERR_BUF       0x00000001
#define  DOC_ERR_REV       0x00000002

class CappDoc : public CDocument
{
protected: // create from serialization only
   CappDoc();
   DECLARE_DYNCREATE(CappDoc)

// Attributes
public:

// Operations
public:

   static CappDoc *GetDoc();

   PINITYPE    m_ini;
   BYTE       *m_pCpBuf;
   BOOL        m_bStreaming;

// Overrides
public:
   virtual BOOL OnNewDocument();
   virtual void Serialize(CArchive& ar);
   virtual void SetTitle(LPCTSTR lpszTitle);
   virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

// Implementation
public:
   virtual ~CappDoc();

#ifdef _DEBUG
   virtual void AssertValid() const;
   virtual void Dump(CDumpContext& dc) const;
#endif

   CView*   SwitchToView(CView* pNewView);
   void     Log(CString str, COLORREF color = APP_MSG_NORMAL);
   void     Traffic(CString str, COLORREF color = APP_MSG_NORMAL);
   void     Debug(CString str, COLORREF color = APP_MSG_NORMAL);
   void     WriteLog(CString filename);

   void     SetValIni(UINT entry, CString &str);
   void     GetValIni(UINT entry, CString &str);
   void     SetIni(CString &str);
   void     GetIni(UINT entry, CString &str);


   // Non-perisitant Data
   UINT     m_nTest;
   UINT     m_iniCnt;
   UINT     m_iniLen;

   PINIENTRY   m_pIni;

protected:

   void  InitIni(void);

   BYTE       *m_docData;
   UINT        m_docLen;
   CString     m_strData;

   CMapStringToPtr m_iniMap;

// Generated message map functions
protected:
   DECLARE_MESSAGE_MAP()

   virtual BOOL SaveModified();
};
