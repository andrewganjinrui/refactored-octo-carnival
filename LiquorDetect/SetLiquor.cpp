// SetLiquor.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "LiquorDetect.h"
#include "SetLiquor.h"


// CSetLiquor �Ի���

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


// CSetLiquor ��Ϣ�������

HBRUSH CSetLiquor::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if (pWnd->GetDlgCtrlID() != IDC_LIQUOR)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = CreateSolidBrush(RGB(200,245,235));;
	}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

BOOL CSetLiquor::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	GetDlgItem(IDC_LIQUOR)->SetFocus();
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
