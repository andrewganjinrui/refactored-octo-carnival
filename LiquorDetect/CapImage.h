#pragma once


// CCapImage

class CCapImage : public CStatic
{
	DECLARE_DYNAMIC(CCapImage)

public:
	CCapImage();
	virtual ~CCapImage();

	CString m_Msg;
	CPoint m_StartP;
	CPoint m_EndP;

	CBitmap* m_pBitmap;

protected:
	DECLARE_MESSAGE_MAP()
private:
	void DrawImage(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
//	afx_msg void OnMove(int x, int y);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	void DrawRoi(CDC* pDC);
};


