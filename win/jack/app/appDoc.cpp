
// appDoc.cpp : implementation of the CappDoc class
//

#include "stdafx.h"
#include "MainFrm.h"
#include "appDoc.h"
#include "build.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CappDoc

IMPLEMENT_DYNCREATE(CappDoc, CDocument)

BEGIN_MESSAGE_MAP(CappDoc, CDocument)
END_MESSAGE_MAP()


// CappDoc construction/destruction

CappDoc::CappDoc()
{
   // Allocate Beam Update List data type
   m_ini = (PINITYPE) new BYTE [sizeof(INITYPE)];

   //
   // INI Key/Value pairs, Type and parameter ptr
   // These are default safe values for all application data.
   // When a new document is created, these are the initialization values.
   // When a document is loaded, these values are used to initialize the
   // application data, then the loaded document is used to overwrite these
   // default values.
   //
   // Ini Elements are added fist by creating a new entry into the INITYPE
   // data type defined in appType.h, then creating an initialization entry
   // in this table. Generally the elements should be partitioned by View.
   //
   // The DOC_STR data type must be handled with care. The length of this
   // data type is always APP_STR_LEN, so any variables that are used with
   // this data type must be declared using the APP_STR_LEN macro.
   //
   // The Text file created from these items uses Unicode Character Format.
   //
   INIENTRY iniConst[] = {
      //
      //
      // Variable Text Name      Default Value              Data Type      Address to Variable        Dim      Tags           Name                 Parameter Hint
      // ==================      =============              =========      ===================        =====    ==========     ================     ==============
      //
      // MainFrame
      //
      {_T("ap.logStamp"),        _T("1"),                   DOC_UINT,      &m_ini->apLogStamp,        1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.trafficStamp"),    _T("0"),                   DOC_UINT,      &m_ini->apTrafficStamp,    1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.trafficEnable"),   _T("0"),                   DOC_UINT,      &m_ini->apTrafficEnable,   1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.trafficLocal"),    _T("0"),                   DOC_UINT,      &m_ini->apTrafficLocal,    1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.trafficPipe"),     _T("0"),                   DOC_UINT,      &m_ini->apTrafficPipe,     1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.debugStamp"),      _T("0"),                   DOC_UINT,      &m_ini->apDebugStamp,      1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.debugEnable"),     _T("0"),                   DOC_UINT,      &m_ini->apDebugEnable,     1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.lastTree"),        _T("0"),                   DOC_HEX8,      &m_ini->apLastTree,        1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.autoSave"),        _T("1"),                   DOC_UINT,      &m_ini->apAutoSave,        1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.sysIDWarn"),       _T("1"),                   DOC_UINT,      &m_ini->apSysIDWarn,       1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.pingWarn"),        _T("0"),                   DOC_UINT,      &m_ini->apPingWarn,        1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.overwriteWarn"),   _T("0"),                   DOC_UINT,      &m_ini->apOverwriteWarn,   1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.messageDropWarn"), _T("1"),                   DOC_UINT,      &m_ini->apMessageDropWarn, 1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.logCMTraffic"),    _T("0"),                   DOC_UINT,      &m_ini->apLogCMTraffic,    1,       0x00000000,    _T(""),              _T("")},
      {_T("ap.logEnable"),       _T("0"),                   DOC_UINT,      &m_ini->apLogEnable,       1,       0x00000000,    _T(""),              _T("")},
      //                                                                                                                      ,
      // Connection                                                                                                           ,
      //                                                                                                                      ,
      {_T("con.comCon"),         _T("2"),                   DOC_INT,       &m_ini->conComCon,         1,       0x00000000,    _T(""),              _T("")},
      {_T("con.ipAddr"),         _T("192.168.1.70"),        DOC_STR,       &m_ini->conIpAddr,         1,       0x00000000,    _T(""),              _T("")},
      {_T("con.macAddr"),        _T("00:02:C9:4E:7F:C8"),   DOC_STR,       &m_ini->conMacAddr,        1,       0x00000000,    _T(""),              _T("")},
      {_T("con.autoConnect"),    _T("0"),                   DOC_UINT,      &m_ini->conAutoConnect,    1,       0x00000000,    _T(""),              _T("")},
      {_T("con.autoPort"),       _T("0"),                   DOC_UINT,      &m_ini->conAutoPort,       1,       0x00000000,    _T(""),              _T("")},
      {_T("con.ipPort"),         _T("2781"),                DOC_UINT,      &m_ini->conIpPort,         1,       0x00000000,    _T(""),                _T("")},
      //                                                                                                                      ,
      // Control Panel (CP)                                                                                                   ,
      //                                                                                                                      ,
      {_T("cp.addr"),            _T("0"),                   DOC_HEX8,      &m_ini->cpAddr,            1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.value"),           _T("0"),                   DOC_DBL0,      &m_ini->cpValue,           1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.type"),            _T("4"),                   DOC_UINT,      &m_ini->cpType,            1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.wr"),              _T("1"),                   DOC_UINT,      &m_ini->cpWR,              1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.trace"),           _T("0"),                   DOC_UINT,      &m_ini->cpTrace,           1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memFile"),         _T("D:\\mem.txt"),         DOC_STR,       &m_ini->cpMemFile,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memStart"),        _T("0x00100000"),          DOC_HEX8,      &m_ini->cpMemStart,        1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.mem_length"),      _T("0x200"),               DOC_HEX8,      &m_ini->cpMemLength,       1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.mem_type"),        _T("1"),                   DOC_UINT,      &m_ini->cpMemType,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memBin"),          _T("0"),                   DOC_UINT,      &m_ini->cpMemBin,          1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memHex"),          _T("1"),                   DOC_UINT,      &m_ini->cpMemHex,          1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memAll"),          _T("1"),                   DOC_UINT,      &m_ini->cpMemAll,          1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memCycle"),        _T("0"),                   DOC_UINT,      &m_ini->cpMemCycle,        1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memAWGN"),         _T("1"),                   DOC_UINT,      &m_ini->cpMemAWGN,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memVerify"),       _T("1"),                   DOC_UINT,      &m_ini->cpMemVerify,       1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memShow"),         _T("1"),                   DOC_UINT,      &m_ini->cpMemShow,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.memToFile"),       _T("0"),                   DOC_UINT,      &m_ini->cpMemToFile,       1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.gphType"),         _T("2"),                   DOC_UINT,      &m_ini->cpGphType,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.gphXmin"),         _T("0"),                   DOC_DBL4,      &m_ini->cpGphXmin,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.gphXmax"),         _T("512"),                 DOC_DBL4,      &m_ini->cpGphXmax,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.gphYmin"),         _T("-60"),                 DOC_DBL4,      &m_ini->cpGphYmin,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.gphYmax"),         _T("+60"),                 DOC_DBL4,      &m_ini->cpGphYmax,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.gphAuto"),         _T("0"),                   DOC_UINT,      &m_ini->cpGphAuto,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.gphCursor"),       _T("1"),                   DOC_UINT,      &m_ini->cpGphCursor,       1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.gphPoints"),       _T("1"),                   DOC_UINT,      &m_ini->cpGphPoints,       1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.imgType"),         _T("0"),                   DOC_UINT,      &m_ini->cpImgType,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.imgRows"),         _T("256"),                 DOC_UINT,      &m_ini->cpImgRows,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.imgCols"),         _T("256"),                 DOC_UINT,      &m_ini->cpImgCols,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.imgDepth"),        _T("1"),                   DOC_UINT,      &m_ini->cpImgDepth,        1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.imgData"),         _T("0"),                   DOC_UINT,      &m_ini->cpImgData,         1,       0x00000000,    _T(""),              _T("")},
      {_T("cp.imgAuto"),         _T("1"),                   DOC_UINT,      &m_ini->cpImgAuto,         1,       0x00000000,    _T(""),              _T("")},
      //
      // DAQ Settings
      //
      {_T("daq.filename"),       _T("D:\\daq.txt"),         DOC_STR,       &m_ini->daqFile,           1,       0x00000000,    _T(""),                _T("")},
      {_T("daq.toFile"),         _T("0"),                   DOC_UINT,      &m_ini->daqToFile,         1,       0x00000000,    _T(""),                _T("")},
      {_T("daq.hex"),            _T("0"),                   DOC_UINT,      &m_ini->daqHex,            1,       0x00000000,    _T(""),                _T("")},
      {_T("daq.show"),           _T("0"),                   DOC_UINT,      &m_ini->daqShow,           1,       0x00000000,    _T(""),                _T("")},
      {_T("daq.ramp"),           _T("0"),                   DOC_UINT,      &m_ini->daqRamp,           1,       0x00000000,    _T(""),                _T("")},
      {_T("daq.float"),          _T("0"),                   DOC_UINT,      &m_ini->daqFloat,          1,       0x00000000,    _T(""),                _T("")},
      {_T("daq.plot"),           _T("0"),                   DOC_UINT,      &m_ini->daqPlot,           1,       0x00000000,    _T(""),                _T("")},
      {_T("daq.raw"),            _T("0"),                   DOC_UINT,      &m_ini->daqRaw,            1,       0x00000000,    _T(""),                _T("")},
      {_T("daq.packets"),        _T("32"),                  DOC_UINT,      &m_ini->daqPackets,         1,       0x00000000,    _T(""),                _T("")},
      //
      // Parameter Pane "CPU Plot Settings"
      //
      {_T("cplot.legend"),        _T("0"),                  DOC_UINT,      &m_ini->cplotLegend,       1,       0x00000009,    _T("Legend"),   _T("Enable or Disable Plot Legend")},
      {_T("cplot.autoA"),         _T("0"),                  DOC_UINT,      &m_ini->cplotAutoA,        1,       0x00000009,    _T("Auto A"),   _T("Plot A Auto Scale Y Axes")},
      {_T("cplot.yMinA"),         _T("-80.0"),              DOC_DBL2,      &m_ini->cplotYminA,        1,       0x00000001,    _T("Y Min A"),  _T("Plot A Minimum Y Axes")},
      {_T("cplot.yMaxA"),         _T("+80.0"),              DOC_DBL2,      &m_ini->cplotYmaxA,        1,       0x00000001,    _T("Y Max A"),  _T("Plot A Maximum Y Axes")},
      {_T("cplot.autoB"),         _T("0"),                  DOC_UINT,      &m_ini->cplotAutoB,        1,       0x00000009,    _T("Auto B"),   _T("Plot B Auto Scale Y Axes")},
      {_T("cplot.yMinB"),         _T("-80.0"),              DOC_DBL2,      &m_ini->cplotYminB,        1,       0x00000001,    _T("Y Min B"),  _T("Plot B Minimum Y Axes")},
      {_T("cplot.yMaxB"),         _T("+80.0"),              DOC_DBL2,      &m_ini->cplotYmaxB,        1,       0x00000001,    _T("Y Max B"),  _T("Plot B Maximum Y Axes")},
      //
      // Parameter Pane "DAQ Plot Settings"
      //
      {_T("qplot.legend"),        _T("0"),                  DOC_UINT,      &m_ini->qplotLegend,       1,       0x00000019,    _T("Legend"),   _T("Enable or Disable Plot Legend")},
      {_T("qplot.autoA"),         _T("1"),                  DOC_UINT,      &m_ini->qplotAutoA,        1,       0x00000019,    _T("Auto A"),   _T("Plot A Auto Scale Y Axes")},
      {_T("qplot.yMinA"),         _T("-60"),                DOC_DBL2,      &m_ini->qplotYminA,        1,       0x00000011,    _T("Y Min A"),  _T("Plot A Minimum Y Axes")},
      {_T("qplot.yMaxA"),         _T("+60"),                DOC_DBL2,      &m_ini->qplotYmaxA,        1,       0x00000011,    _T("Y Max A"),  _T("Plot A Maximum Y Axes")},
      {_T("qplot.autoB"),         _T("1"),                  DOC_UINT,      &m_ini->qplotAutoB,        1,       0x00000019,    _T("Auto B"),   _T("Plot B Auto Scale Y Axes")},
      {_T("qplot.yMinB"),         _T("-60"),                DOC_DBL2,      &m_ini->qplotYminB,        1,       0x00000011,    _T("Y Min B"),  _T("Plot B Minimum Y Axes")},
      {_T("qplot.yMaxB"),         _T("+60"),                DOC_DBL2,      &m_ini->qplotYmaxB,        1,       0x00000011,    _T("Y Max B"),  _T("Plot B Maximum Y Axes")},
      {_T("qplot.length"),        _T("4080"),               DOC_UINT,      &m_ini->qplotLength,       1,       0x00000011,    _T("Length"),   _T("Plot X Length in Points")},
   };

   m_pIni = (PINIENTRY) new BYTE [sizeof(iniConst)];

   m_iniLen = DIM(iniConst);

   m_bStreaming = FALSE;

   for (UINT i=0;i<m_iniLen;i++) {
      m_pIni[i] = iniConst[i];
   }

   // Control Panel Memory Buffer
   m_pCpBuf = new BYTE[APP_MAX_CP_MEM_SIZE];
   ::ZeroMemory(m_pCpBuf, APP_MAX_CP_MEM_SIZE);

}

CappDoc::~CappDoc()
{
   if (m_pIni != NULL) delete m_pIni;
   if (m_ini != NULL) delete m_ini;
   if (m_pCpBuf != NULL) delete m_pCpBuf;
}

BOOL CappDoc::OnNewDocument()
{
   CString str;
   CMainFrame* m_pMainFrm = (CMainFrame*)AfxGetMainWnd();

   if (!CDocument::OnNewDocument())
      return FALSE;

   // Initialize the document data
   InitIni();

   // Ensure Initial View Visible
   m_pMainFrm->OnSelectView(IDD_CONNECT);

   // Clear Logs
   m_pMainFrm->PostMessage(WM_LOG_CLEAR, OUTPUT_WND_ALL, 0);

   return TRUE;
}

BOOL CappDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
   CMainFrame* m_pMainFrm = (CMainFrame*)AfxGetMainWnd();

   // First, Safe Init the document data
   InitIni();

   // De-serialize
   if (!CDocument::OnOpenDocument(lpszPathName))
      return FALSE;

   // Ensure Initial View Visible
   m_pMainFrm->OnSelectView(IDD_CONNECT);

   return TRUE;
}

// CappDoc serialization

void CappDoc::Serialize(CArchive& ar)
{
   UINT     i;
   CString  inLine, str;
   CTime    t = CTime::GetCurrentTime();
   CFile    *fp = ar.GetFile();

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

   //
   // Serialize the document data to storage
   //
   if (ar.IsStoring()) {
      // Write Edit Warning Banner to File
      ar.WriteString(_T("#\r\n"));
      ar.WriteString(_T("#   NOTE: This file is automatically updated,\r\n"));
      ar.WriteString(_T("#         manual edits may be lost.\r\n"));
      ar.WriteString(_T("#\r\n"));
      str.Format(_T("#   File created %s, from\r\n"), t.Format(_T("%#c")));
      ar.WriteString(str);
      str.Format(_T("#   %s\r\n"), pMainFrm->m_exe);
      ar.WriteString(str);
      ar.WriteString(_T("#\r\n"));
      str.Format(_T("#   %d parameters written to file.\r\n"), m_iniLen);
      ar.WriteString(str);
      ar.WriteString(_T("#\r\n"));
      // Update MainFrame Data
      pMainFrm->Serialize(ar);
      // Cycle over all Ini entries and write to file
      for (i=0;i<m_iniLen;i++) {
         GetIni(i, str);
         ar.WriteString(str);
      }
      // Last Entry in File
      ar.WriteString(_T("@EOF\r\n"));
      // Report Data File Stored
      str.Format(_T("Write Data File : %s, wrote %d items\r\n"), fp->GetFilePath(), m_iniLen);
      Log(str);
   }
   //
   // De-serialize the document data from storage
   //
   else {
      // Read from the file
      while (ar.ReadString(inLine) == TRUE) {
         // Remove whitespace
         inLine.Trim(_T(" "));
         // Ignore Comments
         if (inLine[0] == '#') continue;
         // Ignore Empty lines
         if (inLine.IsEmpty()) continue;
         // Check for End-Of-File
         if (inLine[0] == '@') break;
         // Only process lines with equate symbol
         if (inLine.Find(_T("=")) != -1) SetIni(inLine);
      }
      // Check Number of Entries
      if (m_iniLen != m_iniCnt) {
         str.Format(_T("Error : INI Table Entries (%d) and File Entries (%d) Do Not Match\n"), m_iniLen, m_iniCnt);
         Log(str, APP_MSG_ERROR);
         pMainFrm->m_bModifiedFlag = TRUE;
      }
      // Update MainFrame Data
      pMainFrm->Serialize(ar);
      // Report Data File Loaded
      str.Format(_T("Read Data File : %s, read %d items\r\n"), fp->GetFilePath(), m_iniCnt);
      Log(str);
   }
}

BOOL CappDoc::SaveModified()
{
   if (!IsModified()) return TRUE;
   if (m_ini->apAutoSave) {
      DoFileSave();
      // Prevent the case when a new file
      // is not saved and DoFileSave() returns
      // FALSE, which crashes the program for
      // some reason.
      return TRUE;
   }
   return CDocument::SaveModified();
}

void CappDoc::InitIni(void)
{
   int      valPos;
   UINT     i,j;
   CString  str, vector, value;

   CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();

   // Clear and Init the Map
   m_iniMap.RemoveAll();
   m_iniMap.InitHashTable(m_iniLen + (m_iniLen / 4));
   m_iniCnt = 0;

   // Cycle over all data entries
   for (i=0;i<m_iniLen;i++) {
      // Create Map entry
      m_iniMap[m_pIni[i].key] = &m_pIni[i];
      value.Format(_T("%s"), m_pIni[i].value);
      valPos = 0;
      switch (m_pIni[i].type) {
      case DOC_INT :
         if (m_pIni[i].dim == 1)
            *(INT *)m_pIni[i].parm = _wtoi(value);
         else {
            for (j=0;j<m_pIni[i].dim;j++) {
               vector = value.Tokenize(_T("[ "), valPos);
               ((INT *)m_pIni[i].parm)[j] = _wtoi(vector);
            }
         }
         break;
      case DOC_UINT :
         if (m_pIni[i].dim == 1)
            *(UINT *)m_pIni[i].parm = _wtoi(value);
         else {
            for (j=0;j<m_pIni[i].dim;j++) {
               vector = value.Tokenize(_T("[ "), valPos);
               ((UINT *)m_pIni[i].parm)[j] = _wtoi(vector);
            }
         }
         break;
      case DOC_HEX2 :
      case DOC_HEX4 :
      case DOC_HEX8 :
         if (m_pIni[i].dim == 1)
            swscanf_s(m_pIni[i].value, _T("%x"), (UINT *)(m_pIni[i].parm));
         else {
            for (j=0;j<m_pIni[i].dim;j++) {
               vector = value.Tokenize(_T("[ "), valPos);
               swscanf_s(vector, _T("%x"), &((UINT *)m_pIni[i].parm)[j]);
            }
         }
         break;
      case DOC_DBL :
      case DOC_DBL0 :
      case DOC_DBL2 :
      case DOC_DBL4 :
      case DOC_DBL6 :
      case DOC_DBL8 :
         if (m_pIni[i].dim == 1)
            *(DOUBLE *)m_pIni[i].parm = _wtof(m_pIni[i].value);
         else {
            for (j=0;j<m_pIni[i].dim;j++) {
               vector = value.Tokenize(_T("[ "), valPos);
               ((DOUBLE *)m_pIni[i].parm)[j] = _wtof(vector);
            }
         }
         break;
      case DOC_FLT :
         if (m_pIni[i].dim == 1)
            *(FLOAT *)m_pIni[i].parm = (FLOAT)_wtof(m_pIni[i].value);
         else {
            for (j=0;j<m_pIni[i].dim;j++) {
               vector = value.Tokenize(_T("[ "), valPos);
               ((FLOAT *)m_pIni[i].parm)[j] = (FLOAT)_wtof(vector);
            }
         }
         break;
      case DOC_STR :
         wcscpy_s((TCHAR *)m_pIni[i].parm, APP_STR_LEN, m_pIni[i].value);
         break;
      default :
         str.Format(_T("Error : Invalid Data Type in Parameter List\n"));
         Log(str, APP_MSG_ERROR);
         break;
      }
   }

   // Init the MainFrame Data
   pMainFrm->OnNewDoc(m_ini);
}

void CappDoc::SetIni(CString &str)
{
   int         curPos = 0;
   int         valPos = 0;
   int         i = 0;
   CString     key, value, strErr;
   CString     vector;
   PINIENTRY   par;

   // Get Key
   key = str.Tokenize(_T("="), curPos);
   key.Trim();
   if (key != _T("")) {
      // Get Value
      if (curPos != -1) {
         value = str.Tokenize(_T("=;"), curPos);
         value.Trim();
      }
      else {
         key = _T("");
      }
      // Get Ini Entry
      if (m_iniMap.Lookup(key, (void *&)par)) {
         switch (par->type) {
         case DOC_INT :
            if (par->dim == 1)
               *(INT *)par->parm = _wtoi(value);
            else {
               for (i=0;i<par->dim;i++) {
                  if (valPos == -1) break;
                  vector = value.Tokenize(_T("[ "), valPos);
                  ((INT *)par->parm)[i] = _wtoi(vector);
               }
            }
            break;
         case DOC_UINT :
            if (par->dim == 1)
               *(UINT *)par->parm = _wtoi(value);
            else {
               for (i=0;i<par->dim;i++) {
                  if (valPos == -1) break;
                  vector = value.Tokenize(_T("[ "), valPos);
                  ((UINT *)par->parm)[i] = _wtoi(vector);
               }
            }
            break;
         case DOC_HEX2 :
         case DOC_HEX4 :
         case DOC_HEX8 :
            if (par->dim == 1)
               swscanf_s(value, _T("%x"), (UINT *)(par->parm));
            else {
               for (i=0;i<par->dim;i++) {
                  if (valPos == -1) break;
                  vector = value.Tokenize(_T("[ "), valPos);
                  swscanf_s(vector, _T("%x"), &((UINT *)par->parm)[i]);
               }
            }
            break;
         case DOC_DBL :
         case DOC_DBL0 :
         case DOC_DBL2 :
         case DOC_DBL4 :
         case DOC_DBL6 :
         case DOC_DBL8 :
            if (par->dim == 1)
               *(DOUBLE *)par->parm = _wtof(value);
            else {
               for (i=0;i<par->dim;i++) {
                  if (valPos == -1) break;
                  vector = value.Tokenize(_T("[ "), valPos);
                  ((DOUBLE *)par->parm)[i] = _wtof(vector);
               }
            }
            break;
         case DOC_FLT :
            if (par->dim == 1)
               *(FLOAT *)par->parm = (FLOAT)_wtof(value);
            else {
               for (i=0;i<par->dim;i++) {
                  if (valPos == -1) break;
                  vector = value.Tokenize(_T("[ "), valPos);
                  ((FLOAT *)par->parm)[i] = (FLOAT)_wtof(vector);
               }
            }
            break;
         case DOC_STR :
            if (value.GetLength() < APP_STR_LEN)
               wcscpy_s((TCHAR *)par->parm, APP_STR_LEN, value);
            else {
               str.Format(_T("Error : Invalid Data Length in Parameter List, %d\n"), value.GetLength());
               Log(str, APP_MSG_ERROR);
            }
            break;
         default :
            str.Format(_T("Error : Invalid Data Type in Parameter List, %d\n"), par->type);
            Log(str, APP_MSG_ERROR);
            break;
         }
         // Count the file entries
         m_iniCnt++;
         // Invalid Key
         if (valPos == -1) {
            str.Format(_T("Error : Invalid Data Type in Parameter List, %d\n"), par->type);
            Log(str, APP_MSG_ERROR);
         }
      }
      // Invalid Key
      else {
         str.Format(_T("Error : Invalid Key in Parameter List, %s\n"), key);
         Log(str, APP_MSG_ERROR);
      }
   }
   // Empty Key
   else {
      str.Format(_T("Error : Empty Key in Parameter List\n"));
      Log(str, APP_MSG_ERROR);
   }
}

void CappDoc::SetValIni(UINT entry, CString &str)
{
   int         valPos = 0;
   UINT        i = 0;
   CString     array;

   switch (m_pIni[entry].type) {
   case DOC_INT :
      if (m_pIni[entry].dim == 1)
         *(INT *)m_pIni[entry].parm = _wtoi(str);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            if (valPos == -1) break;
            array = str.Tokenize(_T("[ "), valPos);
            ((INT *)m_pIni[entry].parm)[i] = _wtoi(array);
         }
      }
      break;
   case DOC_UINT :
      if (m_pIni[entry].dim == 1)
         *(UINT *)m_pIni[entry].parm = _wtoi(str);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            if (valPos == -1) break;
            array = str.Tokenize(_T("[ "), valPos);
            ((UINT *)m_pIni[entry].parm)[i] = _wtoi(array);
         }
      }
      break;
   case DOC_HEX2 :
   case DOC_HEX4 :
   case DOC_HEX8 :
      if (m_pIni[entry].dim == 1)
         swscanf_s(str, _T("%x"), (UINT *)(m_pIni[entry].parm));
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            if (valPos == -1) break;
            array = str.Tokenize(_T("[ "), valPos);
            swscanf_s(array, _T("%x"), &((UINT *)m_pIni[entry].parm)[i]);
         }
      }
      break;
   case DOC_DBL :
   case DOC_DBL0 :
   case DOC_DBL2 :
   case DOC_DBL4 :
   case DOC_DBL6 :
   case DOC_DBL8 :
      if (m_pIni[entry].dim == 1)
         *(DOUBLE *)m_pIni[entry].parm = _wtof(str);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            if (valPos == -1) break;
            array = str.Tokenize(_T("[ "), valPos);
            ((DOUBLE *)m_pIni[entry].parm)[i] = _wtof(array);
         }
      }
      break;
   case DOC_FLT :
      if (m_pIni[entry].dim == 1)
         *(FLOAT *)m_pIni[entry].parm = (FLOAT)_wtof(str);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            if (valPos == -1) break;
            array = str.Tokenize(_T("[ "), valPos);
            ((FLOAT *)m_pIni[entry].parm)[i] = (FLOAT)_wtof(array);
         }
      }
      break;
   case DOC_STR :
      if (m_pIni[entry].dim == 1)
         wcscpy_s((TCHAR *)m_pIni[entry].parm, APP_STR_LEN, str);
      break;
   }
   // Invalid Key
   if (valPos == -1) {
      str.Format(_T("Error : Invalid Key in Parameter List, %d\n"), entry);
      Log(str, APP_MSG_ERROR);
   }
}

void CappDoc::GetIni(UINT entry, CString &str)
{
   UINT     i;
   CString  str1;

   switch (m_pIni[entry].type) {
   case DOC_INT :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %d;\r\n"), m_pIni[entry].key, *(INT *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%d "), ((INT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_UINT :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %d;\r\n"), m_pIni[entry].key, *(UINT *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%d "), ((UINT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_HEX2 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %02X;\r\n"), m_pIni[entry].key, *(UINT *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%02X "), ((UINT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_HEX4 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %04X;\r\n"), m_pIni[entry].key, *(UINT *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%04X "), ((UINT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_HEX8 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %08X;\r\n"), m_pIni[entry].key, *(UINT *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%08X "), ((UINT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_DBL :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %lf;\r\n"), m_pIni[entry].key, *(DOUBLE *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%lf "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_DBL0 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %.0lf;\r\n"), m_pIni[entry].key, *(DOUBLE *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.0lf "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_DBL2 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %.2lf;\r\n"), m_pIni[entry].key, *(DOUBLE *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.2lf "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_DBL4 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %.4lf;\r\n"), m_pIni[entry].key, *(DOUBLE *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.4lf "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_DBL6 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %.6lf;\r\n"), m_pIni[entry].key, *(DOUBLE *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.6lf "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_DBL8 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %.8lf;\r\n"), m_pIni[entry].key, *(DOUBLE *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.8lf "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_FLT :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%s = %f;\r\n"), m_pIni[entry].key, *(FLOAT *)m_pIni[entry].parm);
      else {
         str.Format(_T("%s = [ "), m_pIni[entry].key);
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%f "), ((FLOAT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
         str1.Format(_T("];\r\n"));
         str += str1;
      }
      break;
   case DOC_STR :
      str.Format(_T("%s = %s;\r\n"), m_pIni[entry].key, (CHAR *)m_pIni[entry].parm);
      break;
   default :
      str.Format(_T("Error : Invalid Data Type in Parameter List\n"));
      Log(str, APP_MSG_ERROR);
      break;
   }
}

void CappDoc::GetValIni(UINT entry, CString &str)
{
   UINT     i;
   CString  str1;

   switch (m_pIni[entry].type) {
   case DOC_INT :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%d"), *(INT *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%d  "), ((INT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_UINT :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%d"), *(UINT *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%d  "), ((UINT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_HEX2 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%02X"), *(UINT *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%02X  "), ((UINT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_HEX4 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%04X"), *(UINT *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%04X  "), ((UINT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_HEX8 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%08X"), *(UINT *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%08X  "), ((UINT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_DBL :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%lf"), *(DOUBLE *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%lf  "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_DBL0 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%.0lf"), *(DOUBLE *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.0lf  "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_DBL2 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%.2lf"), *(DOUBLE *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.2lf  "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_DBL4 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%.4lf"), *(DOUBLE *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.4lf  "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_DBL6 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%.6lf"), *(DOUBLE *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.6lf  "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_DBL8 :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%.8lf"), *(DOUBLE *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.8lf  "), ((DOUBLE *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_FLT :
      if (m_pIni[entry].dim == 1)
         str.Format(_T("%.2f"), *(FLOAT *)m_pIni[entry].parm);
      else {
         for (i=0;i<m_pIni[entry].dim;i++) {
            str1.Format(_T("%.2f  "), ((FLOAT *)m_pIni[entry].parm)[i]);
            str += str1;
         }
      }
      break;
   case DOC_STR :
      str.Format(_T("%s"), (CHAR *)m_pIni[entry].parm);
      break;
   default :
      str.Format(_T("Error : Invalid Data Type in Parameter List\n"));
      Log(str, APP_MSG_ERROR);
      break;
   }
}

CappDoc * CappDoc::GetDoc()
{
   CFrameWnd * pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
   return (CappDoc *) pFrame->GetActiveDocument();
}

CView* CappDoc::SwitchToView (CView* pNewView)
{
   CFrameWnd* m_pMainFrm = (CFrameWnd*)AfxGetMainWnd();
   CView* pOldView = (CView *)m_pMainFrm->GetActiveView();
   ASSERT(pOldView != NULL);
   ASSERT_VALID(pOldView);
   ASSERT(pOldView->GetDocument() == this);

   ::SetWindowLong(pOldView->m_hWnd, GWL_ID, 0);
   ::SetWindowLong(pNewView->m_hWnd, GWL_ID, AFX_IDW_PANE_FIRST);

   // Hide the inactive view.
   pOldView->ShowWindow(SW_HIDE);

   // Active View
   m_pMainFrm->SetActiveView(pNewView);
   m_pMainFrm->RecalcLayout();

   // Update the Form Items
   UpdateAllViews(pOldView);

   // [AEL] Force an OnSize Message to fix a WndResizer Bug
   CRect rect;
   m_pMainFrm->GetWindowRect(rect);
   rect.bottom++;
   m_pMainFrm->MoveWindow(rect);
   rect.bottom--;
   m_pMainFrm->MoveWindow(rect);

   // Show the newly active view
   pNewView->ShowWindow(SW_SHOW);

   return pOldView;
}

void CappDoc::Log(CString str, COLORREF color)
{
   CString *_str = new CString;
   *_str = str;
   AfxGetMainWnd()->SendMessage(WM_LOG_MSG, (WPARAM)_str, color);
}

void CappDoc::Traffic(CString str, COLORREF color)
{
   CString *_str = new CString;
   *_str = str;
   AfxGetMainWnd()->SendMessage(WM_TRAFFIC_MSG, (WPARAM)_str, color);
}

void CappDoc::Debug(CString str, COLORREF color)
{
   CString *_str = new CString;
   *_str = str;
   AfxGetMainWnd()->SendMessage(WM_DEBUG_MSG, (WPARAM)_str, color);
}

void CappDoc::WriteLog(CString filename)
{
   CString *_filename = new CString;
   *_filename = filename;
   AfxGetMainWnd()->SendMessage(WM_WRITE_LOG, (WPARAM)_filename);
}

// CappDoc diagnostics

#ifdef _DEBUG
void CappDoc::AssertValid() const
{
   CDocument::AssertValid();
}

void CappDoc::Dump(CDumpContext& dc) const
{
   CDocument::Dump(dc);
}

#endif //_DEBUG


// CappDoc commands

void CappDoc::SetTitle(LPCTSTR lpszTitle)
{
   CString strTitle, str;
   CString plat, config;

   // Show Exe Platform
   #if defined _M_IX86
      plat.Format(_T("x86"));
   #elif defined _M_X64
      plat.Format(_T("x64"));
   #else
      plat.Format(_T("*"));
   #endif

   // Show Debug State
   #ifdef _DEBUG
      config.Format(_T(" DEBUG.%s,"), plat);
   #else
      config.Format(_T(""));
   #endif

   // APPLICATION TITLE BAR
   strTitle.LoadString(AFX_IDS_APP_TITLE);
   if (CString(lpszTitle) == CString(_T("Untitled"))) {
      m_strTitle = _T("Untitled");
      str.Format(_T("%s build %d :%s  %s"), strTitle, BUILD_INC, config, lpszTitle);
   }
   else {
      str.Format(_T("%s build %d :%s  %s"), strTitle, BUILD_INC, config, m_strPathName);
   }
   AfxGetMainWnd()->SetWindowText(str);
}
