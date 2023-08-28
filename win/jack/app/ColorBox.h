#if !defined(AFX_COLORBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_)
#define AFX_COLORBOX_H__5529A6B1_584A_11D2_A41A_006097BD277B__INCLUDED_

// ColorBox.h : header file

class CColorBox : public CListBox
{
public:
	CColorBox();

public:
	int AddString(LPCTSTR lpszItem);
	int AddString(LPCTSTR lpszItem, COLORREF rgb);
	int InsertString(int nIndex, LPCTSTR lpszItem);
	int InsertString(int nIndex, LPCTSTR lpszItem, COLORREF rgb);
	void SetItemColor(int nIndex, COLORREF rgb);
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

public:
	virtual ~CColorBox();

protected:

	DECLARE_MESSAGE_MAP()
};

#endif
