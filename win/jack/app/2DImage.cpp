// 2DImage.cpp : implementation file//

#include "stdafx.h"
#include "MainFrm.h"
#include "math.h"
#include "2DImage.h"

#define IMAGE_CLASSNAME    _T("2DImage")

C2DImage::C2DImage()
{
	RegisterWindowClass();
   BackColor = GetSysColor(COLOR_BTNFACE);
	m_bMemDCCreated = false;
   m_Pal = NULL;
}

C2DImage::~C2DImage()
{
   if (m_Pal != NULL)
      delete m_Pal;
}

BEGIN_MESSAGE_MAP(C2DImage, CWnd)
   ON_WM_PAINT()
	ON_WM_ERASEBKGND()
   ON_WM_SIZE()
   ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

bool C2DImage::RegisterWindowClass()
{
   WNDCLASS wndcls;
   HINSTANCE hInst = AfxGetInstanceHandle();

   if (!(::GetClassInfo(hInst, IMAGE_CLASSNAME, &wndcls))) {
	   memset(&wndcls, 0, sizeof(WNDCLASS));   
      wndcls.hInstance     = hInst;
      wndcls.lpfnWndProc   = ::DefWindowProc;
      wndcls.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wndcls.hIcon         = 0;
      wndcls.lpszMenuName  = NULL;
      wndcls.hbrBackground = (HBRUSH) ::GetStockObject(WHITE_BRUSH);
      wndcls.style         = CS_GLOBALCLASS;
      wndcls.cbClsExtra    = 0;
      wndcls.cbWndExtra    = 0;
      wndcls.lpszClassName = IMAGE_CLASSNAME;

      if (!RegisterClass(&wndcls)) {
          //  AfxThrowResourceException();
         return false;
      }
   }

   return true;
}

int C2DImage::Create(CWnd *pParentWnd, const RECT &rect, UINT nID, DWORD dwStyle)
{
   LOGPALETTE *pLogPal = (LOGPALETTE *) new BYTE[sizeof(LOGPALETTE)
                           + 255 * sizeof(PALETTEENTRY)];

   unsigned char   grayt[] = {
     #include "ctgray.inc"
   };

	int Result = CWnd::Create(IMAGE_CLASSNAME, _T(""), dwStyle, rect, pParentWnd, nID);
	
   class CapMain *m_pDlg = (CapMain*)AfxGetApp()->m_pMainWnd;
  
   // Create Grey Scale Palette
   pLogPal->palVersion = 0x0300;
   pLogPal->palNumEntries = 256;

   for (int i=0;i<256;i++) {
     PALETTEENTRY entry = {grayt[(i>>1)+128*0],
                           grayt[(i>>1)+128*0],
                           grayt[(i>>1)+128*0],0};
     pLogPal->palPalEntry[i] = entry;
   }

   m_Pal = new CPalette;

   m_Pal->CreatePalette(pLogPal);

   delete [] (BYTE *)pLogPal;

   if (Result)
      RefreshCtrl();

   // Data Tip
   EnableToolTips(TRUE);
   m_tip.Create(this);
   m_tip.Activate(TRUE);
   m_tip.AddTool(this, _T(""));
   m_tip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
   m_tip.SendMessage(TTM_SETDELAYTIME, TTDT_AUTOPOP, SHRT_MAX);
   m_tip.SendMessage(TTM_SETDELAYTIME, TTDT_INITIAL, 1000);
   m_tip.SendMessage(TTM_SETDELAYTIME, TTDT_RESHOW, 100);

   m_tipPoint.x = 0;
   m_tipPoint.y = 0;

   return Result;
}

BOOL C2DImage::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

CDC* C2DImage::GetDC()
{
	return &m_dcImage;
}

void C2DImage::ReleaseDC(CDC* pDC)
{
}

void C2DImage::RefreshCtrl()
{
	CClientDC dc(this) ;  

   m_bMemDCCreated = true;

   CRect rect;
   GetClientRect(rect);

	CBrush m_BrushBack;
	m_BrushBack.CreateSolidBrush(BackColor) ;

   if (!m_dcImage.GetSafeHdc()) {
		CBitmap m_bitmapImage;
		m_dcImage.CreateCompatibleDC(&dc) ;
		m_bitmapImage.CreateCompatibleBitmap(&dc, rect.Width(),rect.Height()) ;
		m_dcImage.SelectObject(&m_bitmapImage) ;
	}
    
	m_dcImage.SetBkColor(BackColor);
	m_dcImage.FillRect(rect,&m_BrushBack);

	Invalidate();
}

void C2DImage::OnPaint()
{
   if (!m_bMemDCCreated) {
      RefreshCtrl();
      m_bMemDCCreated = true;
   }
   CPaintDC dc(this);
   CRect rect;
   GetClientRect(&rect);
   dc.BitBlt(0, 0, rect.Width(), rect.Height(),&m_dcImage, 0, 0, SRCCOPY) ;
}

void C2DImage::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	// Force recreation of Image DC
	if (m_dcImage.GetSafeHdc())
		m_dcImage.DeleteDC();
	
	RefreshCtrl();
}

void C2DImage::OnMouseMove(UINT nFlags, CPoint point)
{
   CString str;
   COLORREF rgb;
   CClientDC dc(this);

   rgb = dc.GetPixel(point);

   if ((point.x != m_tipPoint.x) || (point.y != m_tipPoint.y)) {
      str.Format(_T("R:%d  G:%d  B:%d"), GetRValue(rgb), GetGValue(rgb), GetBValue(rgb)); 
      m_tip.UpdateTipText(str, this);
      m_tipPoint.x = point.x;
      m_tipPoint.y = point.y;
   }

   CWnd::OnMouseMove(nFlags, point);
}

void C2DImage::Reset()
{
   RefreshCtrl() ;
}

void C2DImage::Redraw()
{
	Invalidate();
}

void C2DImage::DrawLine(INT *aLine, UINT scanLine, UINT offset)
{
   UINT  i,j;
   CRect rect;
   GetClientRect(&rect);
   if ((offset + (rect.Width()-37)) < 8192) {
      for (i=0;i<(UINT)rect.Width()-37;i++) {
         j = (UINT)aLine[i+offset];
         m_dcImage.SetPixelV(i+24, scanLine+4, RGB(j,j,j));
      }
   }
}

void C2DImage::FillSolidRect(INT x, INT y, COLORREF color, INT size)
{
   m_dcImage.FillSolidRect(x, y, size, size, color);
}

static UINT greyScale[9] = {0,32,64,96,128,160,192,224,255};

void C2DImage::DrawGreyScale()
{
   UINT  i;
   // Draw Gray Scale
   for (i=0;i<9;i++) {
      m_dcImage.FillSolidRect(4, (i*8)+4, 16, 8, 
         RGB(greyScale[i], greyScale[i], greyScale[i]));
   }
	Invalidate();
}

void C2DImage::DrawImage(UCHAR *im, INT numRows, INT numCols, INT x, INT y)
{
   INT   i,j;
   UINT  k;
   for (i=0;i<numRows;i++) {
      for (j=0;j<numCols;j++) {
         k = (UINT)im[(i*numCols)+j];
         m_dcImage.SetPixelV(i+x, (numCols - j) + y, RGB(k,k,k));
      }
   }
}

void C2DImage::DrawRect(CRect rect, COLORREF rgb)
{
   m_dcImage.FillSolidRect(rect, rgb);
}

void C2DImage::DrawOrder(UCHAR *im, INT numRows, INT numCols, INT x, INT y)
{
   INT   i,j;
   UINT  k;
   for (i=0;i<numRows;i++) {
      for (j=0;j<numCols;j++) {
         k = (UINT)im[(i*numCols)+j];
         if (k == 0) {
            m_dcImage.SetPixelV(i + x, (numCols - j) + y, APP_DEF_IMAGE_BACK);
         }
         else {
            m_dcImage.SetPixelV(i + x, (numCols - j) + y, RGB(  0,   0, 192));
         }
      }
   }
}

BOOL C2DImage::PreTranslateMessage(MSG* pMsg) 
{
    m_tip.RelayEvent(pMsg);
    return CWnd::PreTranslateMessage(pMsg);
}
