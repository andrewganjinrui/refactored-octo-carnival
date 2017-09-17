#pragma once
#include "afxcmn.h"


// CCameraSetDlg 对话框

class CCameraSetDlg : public CDialog
{
	DECLARE_DYNAMIC(CCameraSetDlg)

public:
	CCameraSetDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCameraSetDlg();
	void MakeBitmap();

	//use for bk gradient
	CBitmap m_bitmap;
	HBRUSH  m_hBrush;

// 对话框数据
	enum { IDD = IDD_CAMERASET_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	CSliderCtrl m_ExposureSlider;
	CSliderCtrl m_GainSlider;

	int m_Exposure;
	int m_Gain;

public:
	virtual BOOL OnInitDialog();
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
