// CapImage.cpp : 实现文件
//

#include "stdafx.h"
#include "LiquorDetect.h"
#include "CapImage.h"
#include "ImData.h"
#include "LiquorDetectDlg.h"

extern BOOL g_bLive;
// CCapImage

IMPLEMENT_DYNAMIC(CCapImage, CStatic)

CCapImage::CCapImage()
{
	m_Msg = _T("");
	m_pBitmap = NULL;
}

CCapImage::~CCapImage()
{
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
}


BEGIN_MESSAGE_MAP(CCapImage, CStatic)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
//	ON_WM_MOVE()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CCapImage 消息处理程序



void CCapImage::DrawImage(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	CDC MemDC;
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
	m_pBitmap = new CBitmap;
	MemDC.CreateCompatibleDC(pDC);
	m_pBitmap->CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	m_pBitmap->SetBitmapDimension(rect.Width(), rect.Height());
	MemDC.SelectObject(m_pBitmap);

	CBrush br;
	br.CreateSolidBrush(RGB(0, 0, 0));
	MemDC.FillRect(rect, &br);

	MemDC.SetTextColor(RGB(255, 255, 255));
	MemDC.SetBkMode(TRANSPARENT);
	MemDC.DrawText(m_Msg, &rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();
	if (dlg->m_Image.IsEmpty())
	{
		pDC->BitBlt(0,0,rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);
		return;
	}

	CPoint point;
	int h, w, sw, sh;
	

	// 计算显示位置
	float dx, dy;
	w = dlg->m_Image.m_Width;
	h = dlg->m_Image.m_Height;
	dx = ((float)w)/rect.Width();
	dy = ((float)h)/rect.Height();
	if (dx > dy)
	{
		if (dx <= 1)
		{
			sw = w;
			sh = h;
		}
		else
		{
			sw = (int)(w/dx);
			sh = (int)(h/dx);
		}
	}
	else
	{
		if (dy < 1)
		{
			sw = w;
			sh = h;
		}
		else
		{
			sw = (int)(w/dy);
			sh = (int)(h/dy);
		}
	}
	point.x = rect.CenterPoint().x - sw/2;
	point.y = rect.CenterPoint().y - sh/2;
	dlg->m_Image.Draw(&MemDC, point, CSize(sw,sh));

	DrawRoi(&MemDC);

	pDC->BitBlt(0,0,rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);
}

void CCapImage::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	DrawImage(&dc);

	// 不为绘图消息调用 CStatic::OnPaint()
}

void CCapImage::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();
	if (dlg->m_CannyImage && !g_bLive)
	{
		m_StartP = point;
		m_EndP = point;
	}	
	CStatic::OnLButtonDown(nFlags, point);
}

void CCapImage::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();
	if (nFlags == MK_LBUTTON && dlg->m_CannyImage && !g_bLive)
	{

		CDC* pDC = GetDC();
		int mode = pDC->SetROP2(R2_NOT);
		pDC->SelectStockObject(NULL_BRUSH);
		pDC->Rectangle(m_StartP.x, m_StartP.y, m_EndP.x, m_EndP.y);
		m_EndP = point;

		pDC->Rectangle(m_StartP.x, m_StartP.y, m_EndP.x, m_EndP.y);
		pDC->SetROP2(mode);
		ReleaseDC(pDC);
	}
	CStatic::OnMouseMove(nFlags, point);
}

void CCapImage::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();
	if (dlg->m_CannyImage && !g_bLive)
	{
		m_EndP = point;

		if (m_StartP != m_EndP)
		{
			CRect rect(m_StartP, m_EndP);
			rect.NormalizeRect();
			dlg->m_CurrRoi.index = (int)dlg->m_RoiQue.size()+1;
			dlg->m_CurrRoi.tl = rect.TopLeft();
			dlg->m_CurrRoi.br = rect.BottomRight();
			dlg->m_RoiQue.push_back(dlg->m_CurrRoi);
			dlg->m_RoiQue[dlg->m_RoiQue.size()-1].bDetect = FALSE;

			//插入数据
			CString str;
			str.Format(_T("%d"), dlg->m_CurrRoi.index);
			int nRow = dlg->m_ctrData.InsertItem(dlg->m_ctrData.GetItemCount(), str);

			//插入ROI尺寸
			int RoiW, RoiH;
			RoiW = (int)dlg->ClientToImage((double)rect.Width());
			RoiH = (int)dlg->ClientToImage((double)rect.Height());
			
			str.Format(_T("%d * %d"), RoiW, RoiH);
			dlg->m_ctrData.SetItemText(nRow, 3, str);

			//设置稀释溶液浓度N--1
			dlg->m_ctrData.SetItemText(nRow, 4, _T("1"));

			//设置检测半径最大值、最小值
			dlg->SetRidusMaxMin(RoiW, RoiH);

			Invalidate(FALSE);
		}	
	}
	CStatic::OnLButtonUp(nFlags, point);
}

void CCapImage::DrawRoi(CDC* pDC)
{
	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();
	if (dlg->m_CannyImage && !g_bLive)
	{
		CRect rect;
		CPen pen, focuspen;
		pen.CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
		focuspen.CreatePen(PS_DASH, 1, RGB(255, 0, 0));
		pDC->SelectObject(&pen);
		pDC->SetTextColor(RGB(255, 0, 255));

		for (int i = 0; i < dlg->m_RoiQue.size(); i++)
		{
			rect = CRect(dlg->m_RoiQue[i].tl, dlg->m_RoiQue[i].br);
			pDC->SelectStockObject(NULL_BRUSH);
			pDC->Rectangle(rect);
			pDC->SetBkMode(TRANSPARENT);

			CString str;
			str.Format(_T("%d"), dlg->m_RoiQue[i].index);
			pDC->DrawText(str, &rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

			if (dlg->m_RoiQue[i].bDetect)
			{
				pDC->Ellipse(dlg->m_RoiQue[i].centerx-dlg->m_RoiQue[i].radius, dlg->m_RoiQue[i].centery-dlg->m_RoiQue[i].radius,
					dlg->m_RoiQue[i].centerx+dlg->m_RoiQue[i].radius, dlg->m_RoiQue[i].centery+dlg->m_RoiQue[i].radius);
			}
			
		}

		if (dlg->m_CurrRoi.index >= 0)
		{
			pDC->SelectObject(&focuspen);
			rect = CRect(dlg->m_CurrRoi.tl, dlg->m_CurrRoi.br);
			pDC->SelectStockObject(NULL_BRUSH);
			pDC->Rectangle(rect);
		}

		pen.DeleteObject();
		focuspen.DeleteObject();
	}
}
