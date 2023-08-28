#pragma once

class CTreeViewEx : public CTreeCtrl
{
	DECLARE_DYNAMIC(CTreeViewEx)

public:
					CTreeViewEx();
	virtual			~CTreeViewEx();

	void			SetItemFont(HTREEITEM, LOGFONT&);
	void			SetItemBold(HTREEITEM, BOOL);
	void			SetItemColor(HTREEITEM, COLORREF);
	BOOL			GetItemFont(HTREEITEM, LOGFONT *);
	BOOL			GetItemBold(HTREEITEM);
	COLORREF		GetItemColor(HTREEITEM);

protected:

	struct Color_Font {
		COLORREF color;
		LOGFONT  logfont;
	};

	CMap <void*, void*, Color_Font, Color_Font&> m_mapColorFont;

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	protected:
		afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

public:
   afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
};
