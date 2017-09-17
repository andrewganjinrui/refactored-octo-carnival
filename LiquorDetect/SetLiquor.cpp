// SetLiquor.cpp : 实现文件
//

#include "stdafx.h"
#include "LiquorDetect.h"
#include "SetLiquor.h"


// CSetLiquor 对话框

IMPLEMENT_DYNAMIC(CSetLiquor, CDialog)

CSetLiquor::CSetLiquor(CWnd* pParent /*=NULL*/)
	: CDialog(CSetLiquor::IDD, pParent)
	, m_Liquor(0)
{

}

CSetLiquor::~CSetLiquor()
{
}

void CSetLiquor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LIQUOR, m_Liquor);
	DDV_MinMaxDouble(pDX, m_Liquor, 0, 100);
}


BEGIN_MESSAGE_MAP(CSetLiquor, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSetLiquor 消息处理程序

HBRUSH CSetLiquor::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性
	if (pWnd->GetDlgCtrlID() != IDC_LIQUOR)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = CreateSolidBrush(RGB(200,245,235));;
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

BOOL CSetLiquor::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	GetDlgItem(IDC_LIQUOR)->SetFocus();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
