#pragma once


// CSetLiquor 对话框

class CSetLiquor : public CDialog
{
	DECLARE_DYNAMIC(CSetLiquor)

public:
	CSetLiquor(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSetLiquor();

// 对话框数据
	enum { IDD = IDD_SET_LIQUOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	double m_Liquor;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	virtual BOOL OnInitDialog();
};
