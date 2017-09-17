// CameraSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LiquorDetect.h"
#include "CameraSetDlg.h"
#include "LiquorDetectDlg.h"

// CCameraSetDlg 对话框

IMPLEMENT_DYNAMIC(CCameraSetDlg, CDialog)

CCameraSetDlg::CCameraSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCameraSetDlg::IDD, pParent)
{
	m_Exposure = 500;
	m_Gain = 350;
}

CCameraSetDlg::~CCameraSetDlg()
{
}

void CCameraSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPOSURE_SLIDER, m_ExposureSlider);
	DDX_Control(pDX, IDC_GAIN_SLIDER, m_GainSlider);
}


BEGIN_MESSAGE_MAP(CCameraSetDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_HSCROLL()
//	ON_WM_ERASEBKGND()
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CCameraSetDlg 消息处理程序

BOOL CCameraSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
    CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();
	// 参数设置
	m_ExposureSlider.SetRange(5, 4095);
	m_GainSlider.SetRange(350, 1023);

	m_Exposure = dlg->m_Exposure;
	m_Gain = dlg->m_Gain;

	m_ExposureSlider.SetPos(m_Exposure);
	m_GainSlider.SetPos(m_Gain);

	CString str;
	str.Format("%d", m_Exposure);
	SetDlgItemText(IDC_EXPOSURE_TEXT, str);
	str.Format("%d", m_Gain);
	SetDlgItemText(IDC_GAIN_TEXT, str);
	MakeBitmap();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

HBRUSH CCameraSetDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性
	if (nCtlColor != CTLCOLOR_EDIT)
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0, 0, 0));

		hbr = m_hBrush;
		CRect rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		pDC->SetBrushOrg(-rect.left, -rect.top);
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CCameraSetDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CString str;
	m_Exposure = m_ExposureSlider.GetPos();
	m_Gain = m_GainSlider.GetPos();

	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();

	dlg->m_Camera.SetExposureRaw(m_Exposure);
	dlg->m_Camera.SetGainRaw(m_Gain);

	str.Format("%d", m_Exposure);
	SetDlgItemText(IDC_EXPOSURE_TEXT, str);
	str.Format("%d", m_Gain);
	SetDlgItemText(IDC_GAIN_TEXT, str);

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
//背景的渐变色
void CCameraSetDlg::MakeBitmap()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);

	int r1=245,g1=190,b1=240;
	int r2=130,g2=0,b2=0;

	int x1=0,y1=0;
	int x2=0,y2=0;

	CDC dc2;
	dc2.CreateCompatibleDC(&dc);

	if(m_bitmap.m_hObject)
		m_bitmap.DeleteObject();
	m_bitmap.CreateCompatibleBitmap(&dc,rect.Width(),
		rect.Height());

	CBitmap *oldbmap=dc2.SelectObject(&m_bitmap);

	while(x1 < rect.Width() && y1 < rect.Height())
	{
		if(y1 < rect.Height()-1)
			y1++;
		else
			x1++;

		if(x2 < rect.Width()-1)
			x2++;
		else
			y2++;

		int r,g,b;
		int i = x1+y1;
		r = r1 + (i * (r2-r1) / (rect.Width()+rect.Height()));
		g = g1 + (i * (g2-g1) / (rect.Width()+rect.Height()));
		b = b1 + (i * (b2-b1) / (rect.Width()+rect.Height()));

		CPen p(PS_SOLID,1,RGB(r,g,b));
		CPen *oldpen = dc2.SelectObject(&p); 

		dc2.MoveTo(x1,y1);
		dc2.LineTo(x2,y2);

		dc2.SelectObject(oldpen);
	} 

	m_hBrush = ::CreatePatternBrush((HBITMAP)m_bitmap.GetSafeHandle());
	dc2.SelectObject(oldbmap);

}

BOOL CCameraSetDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect;
	GetClientRect(&rect);

	CDC dc2;
	dc2.CreateCompatibleDC(pDC);
	CBitmap *oldbmap=dc2.SelectObject(&m_bitmap);

	/*We copy the bitmap into the DC*/ 
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc2, 0, 0, SRCCOPY);
	dc2.SelectObject(oldbmap);
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}
