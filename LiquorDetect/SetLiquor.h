#pragma once


// CSetLiquor �Ի���

class CSetLiquor : public CDialog
{
	DECLARE_DYNAMIC(CSetLiquor)

public:
	CSetLiquor(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSetLiquor();

// �Ի�������
	enum { IDD = IDD_SET_LIQUOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	double m_Liquor;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	virtual BOOL OnInitDialog();
};
