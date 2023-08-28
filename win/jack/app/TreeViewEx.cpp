#include "stdafx.h"
#include "TreeViewEx.h"
#include "MainFrm.h"

CTreeViewEx::CTreeViewEx()
{
}

CTreeViewEx::~CTreeViewEx()
{
}

IMPLEMENT_DYNAMIC(CTreeViewEx, CTreeCtrl)

BEGIN_MESSAGE_MAP(CTreeViewEx, CTreeCtrl)
   ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CTreeViewEx::OnTvnSelchanged)
	ON_WM_PAINT()
END_MESSAGE_MAP()


void CTreeViewEx::SetItemFont(HTREEITEM hItem, LOGFONT& logfont)
{
	Color_Font cf;
	if( !m_mapColorFont.Lookup( hItem, cf ) )
		cf.color = (COLORREF)-1;
	cf.logfont = logfont;
	m_mapColorFont[hItem] = cf;
}


void CTreeViewEx::SetItemBold(HTREEITEM hItem, BOOL bBold)
{
	SetItemState(hItem, bBold ? TVIS_BOLD: 0, TVIS_BOLD);
}


void CTreeViewEx::SetItemColor(HTREEITEM hItem, COLORREF color)
{
	Color_Font cf;
	if(!m_mapColorFont.Lookup(hItem, cf))
		cf.logfont.lfFaceName[0] = '\0';
	cf.color = color;
	m_mapColorFont[hItem] = cf;
}


BOOL CTreeViewEx::GetItemFont(HTREEITEM hItem, LOGFONT * plogfont)
{
	Color_Font cf;
	if(!m_mapColorFont.Lookup(hItem, cf))
		return FALSE;
	if(cf.logfont.lfFaceName[0] == '\0') 
		return FALSE;
	*plogfont = cf.logfont;
	return TRUE;

}


BOOL CTreeViewEx::GetItemBold(HTREEITEM hItem)
{
	return GetItemState(hItem, TVIS_BOLD) & TVIS_BOLD;
}


COLORREF CTreeViewEx::GetItemColor(HTREEITEM hItem)
{
	// Returns (COLORREF)-1 if color was not set
	Color_Font cf;
	if(!m_mapColorFont.Lookup(hItem, cf))
		return (COLORREF) - 1;
	return cf.color;

}


void CTreeViewEx::OnPaint() 
{
	CPaintDC dc(this);

	// Create a memory DC compatible with the paint DC
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CRect rcClip, rcClient;
	dc.GetClipBox( &rcClip );
	GetClientRect(&rcClient);

	// Select a compatible bitmap into the memory DC
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap( &dc, rcClient.Width(), rcClient.Height() );
	memDC.SelectObject( &bitmap );
	
	// Set clip region to be same as that in paint DC
	CRgn rgn;
	rgn.CreateRectRgnIndirect( &rcClip );
	memDC.SelectClipRgn(&rgn);
	rgn.DeleteObject();
	
	// First let the control do its default drawing.
	CWnd::DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);

	HTREEITEM hItem = GetFirstVisibleItem();

	int iItemCount = GetVisibleCount() + 1;
	while(hItem && iItemCount--)
	{		
		CRect rect;

		// Do not meddle with selected items or drop highlighted items
		UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;
		Color_Font cf;
	
		//if ( !(GetTreeCtrl().GetItemState( hItem, selflag ) & selflag ) 
		//	&& m_mapColorFont.Lookup( hItem, cf ))
		
		if ((GetItemState(hItem, selflag) & selflag) 
			&& ::GetFocus() == m_hWnd)
			;
		else if (m_mapColorFont.Lookup(hItem, cf))
		{
			CFont *pFontDC;
			CFont fontDC;
			LOGFONT logfont;

			if(cf.logfont.lfFaceName[0] != '\0') 
				logfont = cf.logfont;
			else {
				// No font specified, so use window font
				CFont *pFont = GetFont();
				pFont->GetLogFont( &logfont );
			}

			if(GetItemBold(hItem))
				logfont.lfWeight = 700;

			fontDC.CreateFontIndirect(&logfont);
			pFontDC = memDC.SelectObject(&fontDC );

			if(cf.color != (COLORREF) - 1)
				memDC.SetTextColor(cf.color);
			else
				memDC.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));


			CString sItem = GetItemText(hItem);

			GetItemRect(hItem, &rect, TRUE);

         if (GetItemState(hItem, selflag) & TVIS_SELECTED) {
            //memDC.SetBkColor(GetSysColor(COLOR_BTNFACE));
            memDC.SetBkColor(GetSysColor(COLOR_WINDOW));
         }
         else {
            if (GetBkColor() == (COLORREF)-1) {
               memDC.SetBkColor(GetSysColor(COLOR_WINDOW));
            }
            else {
               memDC.SetBkColor(GetBkColor());
            }
         }

			memDC.TextOut(rect.left + 2, rect.top + 1, sItem);
			
			memDC.SelectObject(pFontDC);
		}
		hItem = GetNextVisibleItem(hItem);
	}


	dc.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC, 
				rcClip.left, rcClip.top, SRCCOPY);

	memDC.DeleteDC();
}

BOOL CTreeViewEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != NULL);

	if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL)
	{
		GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}

void CTreeViewEx::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
   HTREEITEM hItem = GetSelectedItem();   
   DWORD_PTR dw = GetItemData(hItem);
   AfxGetMainWnd()->SendMessage(WM_CHANGE_VIEW, dw, 0);
   *pResult = 0;
}
