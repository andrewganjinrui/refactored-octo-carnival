// LiquorDetectDlg.h : ͷ�ļ�
//

#pragma once
#include "capimage.h"
#include "curverfit.h"
#include "afxcmn.h"
#include "ImageGrab.h"
#include "ImData.h"
#include "BCMenu.h"
#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

#include <vector>
using namespace std;

#define PI 3.141592654
#define SQR2 0.707106



// CLiquorDetectDlg �Ի���
class CLiquorDetectDlg : public CDialog
{
// ����
public:
	CLiquorDetectDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_LIQUORDETECT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// ��ʾ�ؼ�
	CCapImage m_ctrImage;

	// ������Ͽؼ�
	CCurverFit m_ctrCurverFit;

	// ������ʾ�ؼ�
	CListCtrl m_ctrData;

	//
	CImageGrab m_Camera;

	// ��������
	CSliderCtrl m_LowSlider;
	CSliderCtrl m_HighSlider;
	CSliderCtrl m_PrecisionSlider;

	//batch detect progress
	CProgressCtrl m_DetectProgress;

	// ��ʾ����
	int m_MinR;
	int m_MaxR;
	double m_lthreRatio;  // ����ֵ
	double m_hthreRatio;  // ����ֵ
	double m_precision;   // ����
	
	CImData m_Image;
	BOOL m_NoEquipment;

	LPBYTE m_SrcImage;   // ����ԭʼͼ��
	LPBYTE m_CannyImage; // �������Ӵ����ı�Եͼ��
	double m_Scale;      // ���ű���

	//image size
	int m_Width;
	int m_Height;

	//circle Roi 
	CRoi m_CurrRoi;
	vector<CRoi> m_RoiQue;

	int m_curItem;
	int m_curSubItem;

	//camera settings
	int m_Exposure;
	int m_Gain;

	//����������
	CStringArray m_StdLiqData;
	CStringArray m_TestLiqData;

	//������
	CCommandLineInfo cmdInfo;

	//use for bk gradient
	CBitmap m_bitmap;
	HBRUSH  m_hBrush;

	BCMenu m_Menu;
	//message
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMClickListCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	afx_msg void OnBnClickedLive();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCapImage();
	afx_msg void OnBnClickedExit();
	afx_msg void OnExit();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedEdge();
	afx_msg void OnBnClickedNeginCheck();
	afx_msg void OnNMRclickListCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDelete();
	afx_msg void OnCamerasetMenu();
	afx_msg void OnCameralinkMenu();
	afx_msg void OnAboutMenu();
	afx_msg void OnBnClickedFit();
	afx_msg void OnReport();
	afx_msg void OnNMDblclkListCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRegisterMenu();
	afx_msg void OnFileOpen();
	afx_msg void OnSavedataMenu();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedWaitTestCalculate();
	afx_msg void OnBnClickedBtnProductreport();
	afx_msg void OnBnClickedBtnEmptydata();
	afx_msg void OnEnKillfocusEditListctrl();

public:
	CPoint ImageToClient(CPoint point);
	double ImageToClient(double x);
	CPoint ClientToImage(CPoint point);
	double ClientToImage(double x);
	void   SetRidusMaxMin(int RoiWidth, int RoiHeight);	
	
	//��ȡͼ������
	void   CapImageAndCurve(void); 
	
	//������Һ����
	void   CreateLiquorData(void); 

    //dialog's background color gradient
	void   MakeBitmap();           

	// ���������ļ���Ӳ��
	void SaveImgToDisk(CString PathName, BOOL isSave);
};
