// CurverFit.cpp : 实现文件
//

#include "stdafx.h"
#include "LiquorDetect.h"
#include "CurverFit.h"

#include "ImData.h"
#include "LiquorDetectDlg.h"

extern BOOL g_bLive;


// CCurverFit

IMPLEMENT_DYNAMIC(CCurverFit, CStatic)

CCurverFit::CCurverFit()
{
	m_xMax = 1.0f;
	m_xMin = 0.0f;
	m_yMax = 100.0f;
	m_yMin = 0.0f;

	m_x = NULL;
	m_y = NULL;
	m_theat = 0;
	m_b = 0;
	m_bFit = FALSE;
	m_Count = 0;
	m_pBitmap = NULL;
}

CCurverFit::~CCurverFit()
{
	if (m_x != NULL)
	{
		delete m_x;
		delete m_y;
	}

	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
}


BEGIN_MESSAGE_MAP(CCurverFit, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CCurverFit 消息处理程序



void CCurverFit::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	CDC MemDC;
	CBrush brush;
	CRect rect;
	CPen BluePen, RedPen, *oldPen;
	GetClientRect(&rect);
	MemDC.CreateCompatibleDC(&dc);

	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}

	m_pBitmap = new CBitmap;
	m_pBitmap->CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	m_pBitmap->SetBitmapDimension(rect.Width(), rect.Height());
    MemDC.SelectObject(m_pBitmap);

	brush.CreateSolidBrush(RGB(255,255,255));
	MemDC.FillRect(rect, &brush);

	BluePen.CreatePen(PS_SOLID, 1, RGB(0,0,255));
	RedPen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
	
	// 绘制坐标系
	MemDC.MoveTo(rect.right-20, rect.bottom - 20);
	MemDC.LineTo(rect.left+20, rect.bottom - 20);
	MemDC.LineTo(rect.left+20, rect.top + 20);
	MemDC.LineTo(rect.left+17, rect.top + 30);
	MemDC.MoveTo(rect.left+20, rect.top + 20);
	MemDC.LineTo(rect.left+23, rect.top + 30);

	MemDC.MoveTo(rect.right-30, rect.bottom - 17);
	MemDC.LineTo(rect.right-20, rect.bottom - 20);
	MemDC.LineTo(rect.right-30, rect.bottom - 23);

	// 标记刻度
	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();
	
	int dx, dy;
	float fdx, fdy;
	CString strText;
	m_xMax = dlg->m_MaxR;
	m_xMin = dlg->m_MinR;

	if (m_bFit && m_Count!= 0)
		m_yMin = (int)(m_theat*m_xMin+m_b);
	
	dx = (rect.Width()-60)/5;  fdx = (m_xMax - m_xMin)/5;
	dy = (rect.Height()-60)/5; fdy = (m_yMax - m_yMin)/5;

	MemDC.SetBkMode(TRANSPARENT);  // 设置背景模式
	CFont font;
	font.CreateFont(10,6,0,0,0,0,0,0,0,0,0,0,0,_T("宋体"));
	MemDC.SelectObject(&font);// 设置字体
	
 
	strText.Format("%.2f", m_xMin);
	MemDC.TextOut(rect.left+18, rect.bottom - 19, strText);
	strText.Format("%.2f", m_yMin);
	MemDC.TextOut(rect.left+2, rect.bottom - 26, strText);
	for (int i = 1; i < 6; i++)
	{
		MemDC.MoveTo(rect.left+20 + i*dx, rect.bottom - 20);
		MemDC.LineTo(rect.left+20 + i*dx, rect.bottom - 25);
		strText.Format("%.2f", m_xMin+i*fdx);
		MemDC.TextOut(rect.left+18+i*dx, rect.bottom - 19, strText);

		MemDC.MoveTo(rect.left+20, rect.bottom - 20 - i*dy);
		MemDC.LineTo(rect.left+25, rect.bottom - 20 - i*dy);
		strText.Format("%.2f", m_yMin+i*fdy);
		MemDC.TextOut(rect.left+2, rect.bottom - 26 - i*dy, strText);
	}

	//单位
	MemDC.TextOut(rect.right-65, rect.bottom-40, _T("半径(pixel)"));
	MemDC.TextOut(rect.left, rect.top+5, _T("浓度(ug/ml)"));

	// 绘制曲线
	int i;
	float xstep = (rect.Width()-60)/(m_xMax - m_xMin)*fdx;
	float ystep = (rect.Height()-60)/(m_yMax - m_yMin)*fdy;
	if (m_bFit && m_Count!= 0 )
	{
		int x1,y1,x2,y2;

		// 数据点
		oldPen = MemDC.SelectObject(&RedPen);
		MemDC.SelectStockObject(BLACK_BRUSH);

		for ( i = 0; i < m_Count; i++)
		{
			x1 = (int)(xstep/fdx*(m_x[i]-m_xMin));
			y1 = (int)(ystep/fdy*(m_y[i]-m_yMin));	
//			MemDC.LineTo(rect.left + 20 + x1, rect.bottom - 20 - y1);
			x1 = rect.left + 20 + x1;
			y1 = rect.bottom - 20 - y1;
			MemDC.Ellipse(x1-3, y1-3, x1+3, y1+3);
		}
		MemDC.SelectStockObject(NULL_BRUSH);

		// 数据线
		x1 = 0;
		x2 = (int)(xstep/fdx*(m_xMax-m_xMin));
		y1 = (int)(ystep/fdy*(m_theat*m_xMin+m_b-m_yMin));
		y2 = (int)(ystep/fdy*(m_theat*m_xMax+m_b-m_yMin));

		MemDC.SelectObject(&BluePen);
		MemDC.MoveTo(rect.left + 20 + x1, rect.bottom - 20 - y1);
		MemDC.LineTo(rect.left + 20 + x2, rect.bottom - 20 - y2);

		MemDC.SelectObject(oldPen);

		CFont fontTxt;
		fontTxt.CreateFont(14,7,0,0,FW_HEAVY,0,0,0,0,0,0,0,0,_T("Arial"));
		MemDC.SelectObject(&fontTxt);// 设置字体
		

		// 显示相关系数
		if (m_b < 0.0)
		{
			strText.Format("\ny = %gx - %g\nR  = %.6f",  m_theat, -m_b, m_r*m_r);
					}
		else
		    strText.Format("\ny = %gx + %g\nR  = %.6f",  m_theat, m_b, m_r*m_r);

        uchar r2[2]= {0xb2};
		::TextOutW(MemDC.GetSafeHdc(), 128, 26, (LPCWSTR)r2, 1);
		MemDC.DrawText(strText, rect, DT_CENTER);
	}

	// 拷贝
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);

	BluePen.DeleteObject();
	RedPen.DeleteObject();
	// 不为绘图消息调用 CStatic::OnPaint()
}

BOOL CCurverFit::Fit(void)
{
	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();

	CListCtrl& ListCtrl = dlg->m_ctrData;

	int i, k = 0;
	m_Count = 0;
	for (i = 0; i < ListCtrl.GetItemCount(); i++)
	{
		if (ListCtrl.GetCheck(i) )
			m_Count++;
	}
    if (m_Count < 2)
    {
		return FALSE;
    }
	if (m_x != NULL)
	{
		delete m_x;
		delete m_y;
	}

	m_x = new double[m_Count];
	m_y = new double[m_Count];

	CString str;
	for (i = 0; i < ListCtrl.GetItemCount(); i++)
	{
		if (ListCtrl.GetCheck(i) )
		{
			str = ListCtrl.GetItemText(i, 1);
			m_x[k] = atof(str);
			str = ListCtrl.GetItemText(i, 2);
			m_y[k] = atof(str);
			k++;
		}
		else
			ListCtrl.SetItemText(i, 2, _T(""));
	}
	MedFit(m_x, m_y, m_Count, m_b, m_theat);
	m_r = calculateR(m_x, m_y, m_Count);
	m_bFit = TRUE;
	Invalidate();
	return TRUE;
}

void CCurverFit::MedFit(double x[], double y[], int size, double& a, double& b)
{
	int j;
	double    del;
	double sx = 0.0, sy = 0.0, sxy = 0.0, sxx = 0.0, chisq = 0.0;

	for (j = 0; j < size; j++)
	{
		sx += x[j];
		sy += y[j];
		sxy += x[j]*y[j];
		sxx += x[j]*x[j];
	}

	del = size*sxx - sx*sx;

	a = (sxx*sy - sx* sxy)/del;
	b = (size*sxy - sx*sy)/del;
}

double CCurverFit::calculateR(double x[], double y[], int size)
{
	int j;
	double cov = 0.0, sigma_x = 0.0, sigma_y = 0.0;
	double sx = 0.0, sy = 0.0, aver_x, aver_y;

	for (j = 0; j < size; j++)
	{
		sx += x[j];
		sy += y[j];
	}

	aver_x = sx / size;
	aver_y = sy / size;

	for (j = 0; j < size; j++)
	{
		cov += (x[j]-aver_x)*(y[j]-aver_y);
		sigma_x += (x[j]-aver_x)*(x[j]-aver_x);
		sigma_y += (y[j]-aver_y)*(y[j]-aver_y);
	}
	sigma_x = sqrt(sigma_x);
	sigma_y = sqrt(sigma_y);

	return cov/sigma_x/sigma_y;
}