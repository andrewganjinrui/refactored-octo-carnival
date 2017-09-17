#if !defined(AFX_REGISTERDLG_H__A70B2EAF_3F64_46C7_ABB4_D495619B001C__INCLUDED_)
#define AFX_REGISTERDLG_H__A70B2EAF_3F64_46C7_ABB4_D495619B001C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RegisterDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegisterDlg dialog

class CRegisterDlg : public CDialog
{
// Construction
public:
	CRegisterDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRegisterDlg)
	enum { IDD = IDD_REGISTER_DLG };
	//}}AFX_DATA

public:

	CString m_StrRegValue;  //注册序列号
	   
    CString m_StrDiskSerial;//存硬盘序列号

	GETVERSIONOUTPARAMS m_struct_vers;

	SENDCMDINPARAMS  m_struct_inparam;

	SENDCMDOUTPARAMS m_struct_outparam;

	CString m_strbuf;

	HANDLE m_handle;

	DWORD m_di;

	BYTE m_bj;

	//use for bk gradient
	CBitmap m_bitmap;
	HBRUSH  m_hBrush;

public:
	CString GetRasValue(CString& m_StrDiskSerial);                      //通过硬盘序列号得到注册号

	BOOL HardSerialNT();                                                //获取NT系统硬盘序列号

	BOOL HardSerial9X();                                                //获取9X系统硬盘序列号 

	BOOL GetHardDiskSerial();                                           //识别系统，调用识别函数

	CString CheckRegister();                                            //获得注册表键值

	BOOL RsaRegValve(CString& m_StrRegValue , CString& m_StrDiskSerial);//RSA算法判断

	void ChangeByteOrder(PCHAR szString, USHORT uscStrSize);
	void MakeBitmap();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegisterDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:

	// Generated message map functions
	//{{AFX_MSG(CRegisterDlg)
	afx_msg void OnRegisterButton();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGISTERDLG_H__A70B2EAF_3F64_46C7_ABB4_D495619B001C__INCLUDED_)
