
// 2DImage.h : interface of the C2DImage class
//

#pragma once

class C2DImage : public CWnd
{
public:
   C2DImage();	
   virtual ~C2DImage();
   virtual BOOL PreTranslateMessage(MSG* pMsg);

   CDC* GetDC();
   void ReleaseDC(CDC* pDC);

   COLORREF GetBackColor() const       { return BackColor;   }
   void SetBackColor(COLORREF NewCol)  { BackColor = NewCol; }

   void RefreshCtrl();
   int Create(CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle=WS_VISIBLE);
	
   void  Reset();
   void  Redraw();
   void  DrawLine(INT *aLine, UINT scanLine, UINT offset);
   void  FillSolidRect(INT x, INT y, COLORREF color, INT size = 2);
   void  DrawGreyScale();
   void  DrawRect(CRect rect, COLORREF rgb);
   void  DrawImage(UCHAR *im, INT numRows, INT numCols, INT x = 0, INT y = 0);
   void  DrawOrder(UCHAR *im, INT numRows, INT numCols, INT x = 0, INT y = 0);

protected:
   afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnMouseMove(UINT nFlags, CPoint point);

   CDC      m_dcImage;
   CBitmap  m_bitmapImage;
   CPalette *m_Pal;
	COLORREF BackColor;		

   CToolTipCtrl   m_tip;
   CPoint         m_tipPoint;

	bool RegisterWindowClass();
	bool m_bMemDCCreated;

   DECLARE_MESSAGE_MAP()
public:
};
