// LiquorDetectDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LiquorDetect.h"
#include "LiquorDetectDlg.h"
#include "SetLiquor.h"
#include "CameraSetDlg.h"
#include "RegisterDlg.h"
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//报表生成DLL*************************************************************
extern "C" _declspec(dllimport) BOOL ProduceWordReport(CStringArray * stdData, CStringArray * testData, float lowTh, float highTh, float precision, char* ImgSaveName, BOOL & isSave);

BOOL g_bLive = TRUE;
BOOL g_bDetectCir = TRUE;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
/*************************************************************************
* 函数名称：
*   HoughCircle()
*
* 参数:
*   LPSTR lpDIBBits       - 指向源DIB象素的指针
*   LONG  lWidth          - 源图像宽度（象素数，必须是4的倍数）
*   LONG  lHeight         - 源图像高度（象素数）
*   CPoint  topleft       - 区域左上角的点
*   CPoint  bottomright   - 区域右下角的点
*   double factor         - 检测精度
*   float minR            - 最小半径
*   float maxR            - 最大半径
* 返回值:
*   BOOL                  - 运算成功返回TRUE，否则返回FALSE。
*
* 功能:
*      该函数用于检测图像中的圆。
* 
* 要求目标图像为只有0和255两个灰度值的灰度图像。
************************************************************************/

BOOL HoughCircle(LPBYTE pImage, int nWidth, int nHeight,
				 CRoi& roi, double factor, float minR, float maxR)
{
	// 指向源图像的指针
	LPBYTE	lpSrc;

	// 指向缓存图像的指针
	LPBYTE	lpDst;

	// 指向缓存DIB图像的指针
	LPBYTE	lpNewDIBBits;
	HLOCAL	hNewDIBBits;

	//指向变换域的指针
	WORD*	lpTransArea;
	//int *lpTransArea;??heap空间有限
	HLOCAL	hTransArea;

	// 变换域的坐标
	int iX;
	int iY;
	int iR;

	//循环变量
	int x;
	int y;

	//像素值
	BYTE pixel;

	// 区域大小
	int w = roi.ibr.x - roi.itl.x;
	int h = roi.itl.y - roi.ibr.y;

	// 暂时分配内存，以保存新图像
	hNewDIBBits = LocalAlloc(LHND, nWidth * nHeight);

	if (hNewDIBBits == NULL)
		return FALSE;// 分配内存失败

	// 锁定内存
	lpNewDIBBits = (LPBYTE)LocalLock(hNewDIBBits);

	// 初始化新分配的内存，设定初始值为255
	lpDst = lpNewDIBBits;
	memset(lpDst, (BYTE)255, nWidth * nHeight);

	//计算变换域的尺寸
	int Min = w < h ? w : h;
	int iMinR, iMaxR = min(Min/2, maxR);
	if (minR > iMaxR)
		iMinR = max(0, iMaxR - 10);
	else
		iMinR = minR;

	//为变换域分配内存
	hTransArea = LocalAlloc(LHND, w * h * (iMaxR+1) * sizeof(WORD));

	if (hTransArea == NULL)
		return FALSE;// 分配内存失败

	// 锁定内存
	lpTransArea = (WORD * )LocalLock(hTransArea);

	// 初始化新分配的内存，设定初始值为0
	ZeroMemory(lpTransArea, w * h * (iMaxR+1) *  sizeof(WORD));

	// 计算图像每行的字节数
	//	LONG lLineBytes = WIDTHBYTES(nWidth * 8);

	for(y = roi.ibr.y; y < roi.itl.y; y++)
	{
		for(x = roi.itl.x; x < roi.ibr.x; x++)
		{

			// 指向源图像倒数第y行，第x个象素的指针			
			lpSrc = pImage + nWidth * y + x;

			//取得当前指针处的像素值
			pixel = *lpSrc;

			//目标图像中含有0和255外的其它灰度值
			if(pixel != 255 && *lpSrc != 0)
				return FALSE;

			//如果是黑点，则在变换域的对应点上加1
			if(pixel == 0)
			{
				int xx = x - roi.itl.x;
				int yy = y - roi.ibr.y;
				for(iX = max(0, xx-iMaxR); iX < min(w, xx+iMaxR+1); iX++)
				{
					for(iY = max(0, yy-iMaxR); iY < min(h, yy+iMaxR+1); iY++)
					{
						if (abs(iX - xx) < SQR2*iMinR && abs(iY - yy) < SQR2*iMinR)
						{
							continue;
						}

						iR = (int) (sqrt((double)(xx-iX)*(xx-iX)+(yy-iY)*(yy-iY))+0.5);

						if (iR < iMinR || iR > iMaxR)
						{
							continue;
						}
						//变换域的对应点上加1
						(*(lpTransArea+iR*w*h+iY*w+iX))++; 
					}
				}
			}
		}
	}

	//找到变换域中最大值点
	roi.centerx = 0;
	roi.centery = 0;
	roi.radius = 0;
	int maxn = 0;
	//	MV_Circle.Value=0;
	//	g_count = 0;
	float Sum, CurrentIR, PiIR, NextIR;
	NextIR = 0.0;

	for (iR = iMaxR; iR >= iMinR; iR--)
	{
		for(iY = 0; iY < h; iY++)
		{
			for(iX = 0; iX< w; iX++)
			{

				if(*(lpTransArea+iR*w*h+iY*w+iX) > factor*iR*PI/*MV_Circle.Value*/)
				{
					roi.centerx = iX + roi.itl.x;
					roi.centery = iY + roi.ibr.y;
					roi.radius = (double)iR;



					//精检测
					CurrentIR = (float)*(lpTransArea+iR*w*h+iY*w+iX);
					PiIR = (float)*(lpTransArea+(iR-1)*w*h+iY*w+iX);
					if (iR != iMaxR)
					{
						NextIR = (float)*(lpTransArea+(iR+1)*w*h+iY*w+iX);
					}

					Sum = CurrentIR + PiIR + NextIR;

					roi.radius = (iR+1)*NextIR/Sum + (iR)*CurrentIR/Sum + (iR-1)*PiIR/Sum;


					// 释放内存
					LocalUnlock(hNewDIBBits);
					LocalFree(hNewDIBBits);

					// 释放内存
					LocalUnlock(hTransArea);
					LocalFree(hTransArea);

					// 	CString str;
					// 	str.Format("%d",g_count);
					// 	AfxGetMainWnd()->SetWindowText("扩散环检测");
					// 返回
					return TRUE;
				}
			}
		}
	}

	// 释放内存
	LocalUnlock(hNewDIBBits);
	LocalFree(hNewDIBBits);

	// 释放内存
	LocalUnlock(hTransArea);
	LocalFree(hTransArea);

	// 	CString str;
	// 	str.Format("%d",g_count);
	// 	AfxGetMainWnd()->SetWindowText("扩散环检测");
	// 返回
	return FALSE;
}
//相机图像显示监视线程
UINT WatchThread(LPVOID param)
{	
	CLiquorDetectDlg* pDlg = (CLiquorDetectDlg*)param;
	int timeout = 0;
	LPBYTE pCameraData;

	while(g_bLive)
	{
		
		pDlg->m_Image.Empty();
		pDlg->m_Image.m_Width = pDlg->m_Width;
		pDlg->m_Image.m_Height = pDlg->m_Height;	
		pDlg->m_Image.m_lpImage = new BYTE[pDlg->m_Height*pDlg->m_Width];

		
		while ((pCameraData = pDlg->m_Camera.GetImgData()) == NULL)
		{
			
			timeout++;
			CString msg;
			msg.Format(_T("获取数据超时%d次，等待重新获取..."), timeout);
			pDlg->GetDlgItem(IDC_MESSAGE)->SetWindowText(msg);
			Sleep(5000);
			if (timeout == 10)
			{
				pDlg->GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("超时次数达到最大上限，监视已断开，请重新开启监视。"));
				g_bLive = FALSE;
				return 0;
			}
		}

		memcpy(pDlg->m_Image.m_lpImage, pCameraData, pDlg->m_Height*pDlg->m_Width);

		pDlg->m_ctrImage.RedrawWindow();
	}
	return 0;
}

//扩散环检测线程
UINT DetectCircleThread(LPVOID param)
{
	CLiquorDetectDlg* pDlg = (CLiquorDetectDlg*)param;
	g_bDetectCir = FALSE;

	//禁用功能按钮
	pDlg->GetDlgItem(IDC_DETECT_PROGRESS)->ShowWindow(SW_SHOW);
	pDlg->GetDlgItem(IDC_FIT)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_BTN_EMPTYDATA)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_LOW_SLIDER)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_HIG_SLIDER)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_EDGE)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_PER_CIRCLE)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_MIN_R)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_MAX_R)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_BTN_PRODUCTREPORT)->EnableWindow(FALSE);


	pDlg->m_DetectProgress.SetRange(1, pDlg->m_RoiQue.size());

	pDlg->m_Image.Empty();
	pDlg->m_Image.m_Width = pDlg->m_Width;	
	pDlg->m_Image.m_Height = pDlg->m_Height;	
	pDlg->m_Image.m_lpImage = new BYTE[pDlg->m_Height*pDlg->m_Width];
	memcpy(pDlg->m_Image.m_lpImage, pDlg->m_SrcImage, pDlg->m_Width*pDlg->m_Height);

	CString str;
	for (int i = 0; i < pDlg->m_RoiQue.size(); i++)
	{
		pDlg->m_RoiQue[i].itl = pDlg->ClientToImage(pDlg->m_RoiQue[i].tl);
		pDlg->m_RoiQue[i].ibr = pDlg->ClientToImage(pDlg->m_RoiQue[i].br);

		str.Format(" 开始检测感兴趣区域 %d ...", i+1);
		pDlg->GetDlgItem(IDC_MESSAGE)->SetWindowText(str);

		HoughCircle(pDlg->m_CannyImage, pDlg->m_Width, pDlg->m_Height, pDlg->m_RoiQue[i], 
			pDlg->m_precision, pDlg->m_MinR, pDlg->m_MaxR);
		pDlg->m_RoiQue[i].bDetect = TRUE;

		str.Format("完成感兴趣区域 %d 的检测...", i+1);
		pDlg->GetDlgItem(IDC_MESSAGE)->SetWindowText(str);
		pDlg->m_DetectProgress.SetPos(i+1);

		str.Format("%.2f", pDlg->m_RoiQue[i].radius);
		pDlg->m_ctrData.SetItemText(i, 1, str);

		CPoint point = pDlg->ImageToClient(CPoint(pDlg->m_RoiQue[i].centerx, pDlg->m_RoiQue[i].centery));
		pDlg->m_RoiQue[i].centerx = point.x;
		pDlg->m_RoiQue[i].centery = point.y;
		pDlg->m_RoiQue[i].radius = pDlg->ImageToClient(pDlg->m_RoiQue[i].radius);                                                             

		pDlg->m_ctrImage.RedrawWindow();
	}
	pDlg->GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("完成所有感兴趣区域的检测，请进行标准品拟合！"));
	pDlg->GetDlgItem(IDC_DETECT_PROGRESS)->ShowWindow(SW_HIDE);

	//开启功能按钮
	pDlg->GetDlgItem(IDC_FIT)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_BTN_EMPTYDATA)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_LOW_SLIDER)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_HIG_SLIDER)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_EDGE)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_PER_CIRCLE)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_MIN_R)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_MAX_R)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_BTN_PRODUCTREPORT)->EnableWindow(TRUE);

	g_bDetectCir = TRUE; 
	return 0;
}
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLiquorDetectDlg 对话框




CLiquorDetectDlg::CLiquorDetectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLiquorDetectDlg::IDD, pParent)
	, m_MinR(0)
	, m_MaxR(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_MinR = 200;
	m_MaxR = 0;
	m_lthreRatio = 60.0;
	m_hthreRatio = 200.0;
	m_precision = 0.65;
	m_SrcImage = NULL;
	m_CannyImage = NULL;
	m_hBrush = NULL;
}

void CLiquorDetectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMAGE, m_ctrImage);
	DDX_Control(pDX, IDC_CURVER_FIT, m_ctrCurverFit);
	DDX_Control(pDX, IDC_LIST_CTRL, m_ctrData);
	DDX_Control(pDX, IDC_LOW_SLIDER, m_LowSlider);
	DDX_Control(pDX, IDC_HIG_SLIDER, m_HighSlider);
	DDX_Control(pDX, IDC_PER_CIRCLE, m_PrecisionSlider);
	DDX_Text(pDX, IDC_MIN_R, m_MinR);
	DDX_Text(pDX, IDC_MAX_R, m_MaxR);
	DDX_Control(pDX, IDC_DETECT_PROGRESS, m_DetectProgress);
}

BEGIN_MESSAGE_MAP(CLiquorDetectDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_CLICK, IDC_LIST_CTRL, &CLiquorDetectDlg::OnNMClickListCtrl)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_LIVE, &CLiquorDetectDlg::OnBnClickedLive)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CAP_IMAGE, &CLiquorDetectDlg::OnBnClickedCapImage)
	ON_BN_CLICKED(IDC_EXIT, &CLiquorDetectDlg::OnBnClickedExit)
	ON_COMMAND(ID_EXIT, &CLiquorDetectDlg::OnExit)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_EDGE, &CLiquorDetectDlg::OnBnClickedEdge)
	ON_BN_CLICKED(IDC_NEGIN_CHECK, &CLiquorDetectDlg::OnBnClickedNeginCheck)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CTRL, &CLiquorDetectDlg::OnNMRclickListCtrl)
	ON_COMMAND(ID_DELETE, &CLiquorDetectDlg::OnDelete)
	ON_COMMAND(ID_CAMERASET_MENU, &CLiquorDetectDlg::OnCamerasetMenu)
	ON_COMMAND(ID_CAMERALINK_MENU, &CLiquorDetectDlg::OnCameralinkMenu)
	ON_COMMAND(ID_ABOUT_MENU, &CLiquorDetectDlg::OnAboutMenu)
	ON_BN_CLICKED(IDC_FIT, &CLiquorDetectDlg::OnBnClickedFit)
	ON_COMMAND(ID_REPORT, &CLiquorDetectDlg::OnReport)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CTRL, &CLiquorDetectDlg::OnNMDblclkListCtrl)
	ON_COMMAND(ID_REGISTER_MENU, &CLiquorDetectDlg::OnRegisterMenu)
	ON_COMMAND(ID_FILE_OPEN, &CLiquorDetectDlg::OnFileOpen)
	ON_COMMAND(ID_SAVEDATA_MENU, &CLiquorDetectDlg::OnSavedataMenu)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_WAIT_TEST_CALCULATE, &CLiquorDetectDlg::OnBnClickedWaitTestCalculate)
	ON_BN_CLICKED(IDC_BTN_PRODUCTREPORT, &CLiquorDetectDlg::OnBnClickedBtnProductreport)
	ON_BN_CLICKED(IDC_BTN_EMPTYDATA, &CLiquorDetectDlg::OnBnClickedBtnEmptydata)
	ON_EN_KILLFOCUS(IDC_EDIT_LISTCTRL, &CLiquorDetectDlg::OnEnKillfocusEditListctrl)
END_MESSAGE_MAP()


// CLiquorDetectDlg 消息处理程序

BOOL CLiquorDetectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

 	//注册---------------------------------------------------------------------------
	CRegisterDlg dlg;

	//得到硬盘序列号
	dlg.GetHardDiskSerial();
	
	//硬件信息RSA编码
	dlg.m_StrRegValue = dlg.GetRasValue(dlg.m_StrDiskSerial);

	
	//硬件信息编码?=注册码
	if ( dlg.m_StrRegValue.IsEmpty() || dlg.m_StrRegValue != dlg.CheckRegister())
	{
	
		GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_FIT)->EnableWindow(FALSE);		
		GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(FALSE);
		MessageBox("请您先进行注册，否则软件功能受限!", "软件注册提示");

	}
	else
	{
		GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(TRUE);
		GetDlgItem(IDC_FIT)->EnableWindow(TRUE);
	}
    //注册---------------------------------------------------------------------------
    
	

	//
	DWORD dwStyle = m_ctrData.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;               //选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;                   //网格线（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_CHECKBOXES;                  //item前生成checkbox控件
	m_ctrData.SetExtendedStyle(dwStyle);           //设置扩展风格
	m_ctrData.InsertColumn(0, _T("标记"), LVCFMT_CENTER, 40);
	m_ctrData.InsertColumn(1, _T("半径"), LVCFMT_CENTER, 50);
	m_ctrData.InsertColumn(2, _T("稀释浓度"), LVCFMT_CENTER, 62);
	m_ctrData.InsertColumn(3, _T("ROI"), LVCFMT_CENTER, 68);
	m_ctrData.InsertColumn(4, _T("N"), LVCFMT_CENTER, 33);
    m_ctrData.InsertColumn(5, _T("实际浓度"), LVCFMT_CENTER, 61);
	

	g_bLive = FALSE;
	m_NoEquipment = TRUE;

	// SLIDER参数设置
	m_LowSlider.SetRange(0,255);
	m_HighSlider.SetRange(0,255);
	m_PrecisionSlider.SetRange(0,100);
	m_LowSlider.SetPos((int)m_lthreRatio);
	m_HighSlider.SetPos((int)m_hthreRatio);
	m_PrecisionSlider.SetPos((int)((m_precision-0.5)/0.5*100));

	CString str;
	str.Format("%g", m_lthreRatio);
	SetDlgItemText(IDC_LOW_TEXT, str);
	str.Format("%g", m_hthreRatio);
	SetDlgItemText(IDC_HIG_TEXT, str);
	str.Format("%.2f", m_precision);
	SetDlgItemText(IDC_PRECISION, str);

    MakeBitmap();
	m_Menu.LoadMenu(IDR_MENU);
	m_Menu.SetMenuDrawMode(BCMENU_DRAWMODE_XP);
	SetMenu(&m_Menu);

	//-----------写入注册表信息，用来关联文件（图标，双击打开）-------
	//写入文件关联打开项
	//CRegKey reg, reg1; 
	//CString FileExt = ".tha";
	//CString FileType = "THA File";
	//CString MIMEType = "tha/plain";
	//CString DefaultIconStr;
	//char ExeName[MAX_PATH];
	//GetModuleFileName(AfxGetInstanceHandle(), ExeName, MAX_PATH);

	//DefaultIconStr = (LPTSTR) ExeName;

	//int position = DefaultIconStr.ReverseFind('\\');   //反向查找"\"所在的位置 
	//DefaultIconStr=DefaultIconStr.Left(position+1);          //删除文件名，只获得路径

	//strcat(ExeName," %1");

	//reg.Create(HKEY_CLASSES_ROOT,FileExt);
	//reg.SetValue(FileType,""); 
	//reg.SetValue(MIMEType,"Content Type");

	//reg.Create(reg.m_hKey,"shell\\open\\command");
	//reg.SetValue(ExeName,"");
	//reg.Close();

	////写入缺省ICON
	//LONG lRet=reg1.Open(HKEY_CLASSES_ROOT,".tha");
	//if(lRet==ERROR_SUCCESS)
	//{
	//	reg1.Create(reg1.m_hKey,"defaulticon");
	//	reg1.SetValue(DefaultIconStr + _T("Icon\\tha.ico"),"");
	//	reg1.Close();
	//}

	AfxGetApp()->ParseCommandLine(cmdInfo);
	if(cmdInfo.m_strFileName != "") //当前操作为直接播放TCF文件时
	{
		if (m_SrcImage)
		{
			delete m_SrcImage;
			m_SrcImage = NULL;
		}
		CFile file;
		if(file.Open(cmdInfo.m_strFileName, CFile::modeRead | CFile::shareDenyWrite))
		{
			if(m_Image.Read(&file))
			{
				m_Width = m_Image.m_Width;
				m_Height = m_Image.m_Height;
				m_SrcImage = new BYTE[m_Width * m_Height];
				memcpy(m_SrcImage, m_Image.m_lpImage, m_Width * m_Height);
				m_ctrImage.RedrawWindow();

				//更新图像处理参数信息
				m_lthreRatio = m_Image.m_low;
				m_hthreRatio = m_Image.m_high;
				m_precision  = m_Image.m_circular;

				m_LowSlider.SetPos((int)m_lthreRatio);
				m_HighSlider.SetPos((int)m_hthreRatio);
				m_PrecisionSlider.SetPos((int)((m_precision-0.5)/0.005));

				str.Format("%.2f", m_precision);
				SetDlgItemText(IDC_PRECISION, str);

				str.Format("%g", m_lthreRatio);
				SetDlgItemText(IDC_LOW_TEXT, str);

				str.Format("%g", m_hthreRatio);
				SetDlgItemText(IDC_HIG_TEXT, str);

				
			}
			file.Close();
		}
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("导入待测数据."));

	}
	else
		//启动相机连接
		SetTimer(0, 1000, NULL);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLiquorDetectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLiquorDetectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CLiquorDetectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


HBRUSH CLiquorDetectDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
	if (pWnd->GetDlgCtrlID() == IDC_MESSAGE)
	{
		CFont font;
		VERIFY(font.CreateFont(
			20,                        // nHeight
			0,                         // nWidth
			0,                         // nEscapement
			0,                         // nOrientation
			FW_BOLD,                 // nWeight
			TRUE,                     // bItalic
			TRUE,                     // bUnderline
			0,                         // cStrikeOut
			ANSI_CHARSET,              // nCharSet
			OUT_DEFAULT_PRECIS,        // nOutPrecision
			CLIP_DEFAULT_PRECIS,       // nClipPrecision
			DEFAULT_QUALITY,           // nQuality
			DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
			"宋体"));                 // lpszFacename
      pDC->SelectObject(&font);
	  pDC->SetTextColor(RGB(0,0,255));
	  DeleteObject(&font);
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CLiquorDetectDlg::OnNMClickListCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	//----------------------------------------------
	if(pNMListView->iItem != -1)
	{
		//对空间的内容进行编辑--------------------------
		if (pNMListView->iSubItem == 4)
		{
			CRect rectSub;
			m_ctrData.GetSubItemRect(pNMListView->iItem, pNMListView->iSubItem,LVIR_LABEL, rectSub);
			CString str = m_ctrData.GetItemText(pNMListView->iItem, pNMListView->iSubItem);
			
			m_ctrData.ClientToScreen(&rectSub);
			
			ScreenToClient(&rectSub);
			CRect rect = rectSub;
//		GetDlgItem(IDC_EDIT_LISTCTRL)->MoveWindow(rectSub);
			::SetWindowPos(::GetDlgItem(m_hWnd,IDC_EDIT_LISTCTRL),HWND_TOP,rectSub.left,rectSub.top,rectSub.Width(),rect.Height(),NULL);

			GetDlgItem(IDC_EDIT_LISTCTRL)->ShowWindow(SW_SHOW);
		
			::SetWindowText(::GetDlgItem(m_hWnd,IDC_EDIT_LISTCTRL),str);
	        GetDlgItem(IDC_EDIT_LISTCTRL)->SetFocus();
			((CEdit *)GetDlgItem(IDC_EDIT_LISTCTRL))->SetSel(0, -1);

			
			m_curItem = pNMListView->iItem;
			m_curSubItem = pNMListView->iSubItem;

		}

		CString strtemp;
		strtemp.Format("单击的是第%d行第%d列",
			pNMListView->iItem, pNMListView->iSubItem);
		//		MessageBox(strtemp);

		m_CurrRoi = m_RoiQue[pNMListView->iItem];
		DWORD dwPos = GetMessagePos();
		CPoint point( LOWORD(dwPos), HIWORD(dwPos) );

		m_ctrData.ScreenToClient(&point);

	
		//LVHITTESTINFO lvinfo;
		//lvinfo.pt = point;
		//lvinfo.flags = LVHT_ABOVE;

		UINT nFlag;
		int nItem = m_ctrData.HitTest(point, &nFlag);
		//判断是否点在checkbox上
		CString str, strN, strReal;
		int N;
		if(nFlag == LVHT_ONITEMSTATEICON )
		{
			if (!m_ctrData.GetCheck(pNMListView->iItem))
			{
				CSetLiquor dlg;
				if (dlg.DoModal() == IDOK)
				{
					str.Format("%.2f", dlg.m_Liquor);
					m_ctrData.SetItemText(pNMListView->iItem, 2, str);
				    strN = m_ctrData.GetItemText(pNMListView->iItem, 4);
					N = atoi(strN);
					strReal.Format(_T("%.2f"), N*dlg.m_Liquor);
					m_ctrData.SetItemText(pNMListView->iItem, 5, strReal);

				}
			}
			else
				m_ctrData.SetItemText(pNMListView->iItem, 2, "");
		}
		m_ctrImage.RedrawWindow();
	}
	*pResult = 0;
}

void CLiquorDetectDlg::OnNMRclickListCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(pNMListView->iItem != -1)
	{
		CString strtemp;
		strtemp.Format("单击的是第%d行第%d列",
			pNMListView->iItem, pNMListView->iSubItem);

		m_CurrRoi = m_RoiQue[pNMListView->iItem];
		if (pNMListView->iSubItem == 0)
		{
			CPoint point;
			GetCursorPos(&point);
			CMenu popmenu;
			popmenu.LoadMenu(IDR_POP_MENU);
			popmenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
		}
	}
	*pResult = 0;
}

// 删除选中的区域
void CLiquorDetectDlg::OnDelete()
{
	// TODO: 在此添加命令处理程序代码
	int i;
	//感兴趣区域重排
	for (i = m_CurrRoi.index-1; i < m_RoiQue.size()-1; i++)
	{
		m_RoiQue[i] = m_RoiQue[i+1];
		m_RoiQue[i].index--;
	}
	m_RoiQue.pop_back();
	m_ctrData.DeleteItem(m_CurrRoi.index-1);

	//更新ListCtrl
	for (i = m_CurrRoi.index-1; i < m_RoiQue.size(); i++)
	{
		CString str;
		str.Format(_T("%d"), m_RoiQue[i].index);
		m_ctrData.SetItemText(i, 0, str);
	}

	m_CurrRoi.index = -1;
	m_ctrImage.RedrawWindow();
}

void CLiquorDetectDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!g_bDetectCir)
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("正在批量检测中，程序暂不能退出!"));
		return;
	}
	 
	g_bLive = FALSE;
	Sleep(200);
	// 关闭相机链接
	if (!m_NoEquipment)
	{
		m_Camera.GetImgData();
		m_Camera.CutCameraLink();
	}
	
	if (m_SrcImage)
	{
		delete m_SrcImage;
		m_SrcImage = NULL;
	}

	if (m_CannyImage)
	{
		delete m_CannyImage;
		m_CannyImage = NULL;
	}
	
	if (m_hBrush)
	{
		DeleteObject(m_hBrush);
	}
	CDialog::OnClose();
}

void CLiquorDetectDlg::OnBnClickedLive()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!g_bDetectCir)
	{
		return;
	}
	
	CString str;
    GetDlgItemText(IDC_LIVE, str);
	if (str == _T("实时监视"))
	{
		// 没有设备则返回
		if (m_NoEquipment)
		{
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("未发现相机."));

			m_Image.Empty();

			if (m_SrcImage)
			{
				delete m_SrcImage;
				m_SrcImage = NULL;
			}

			if (m_CannyImage)
			{
				delete m_CannyImage;
				m_CannyImage = NULL;
			}
			m_ctrImage.RedrawWindow();
			return;
		}

		// 开启监视线程
		if (!g_bLive)
		{
			//清空LISTCTRL数据
			m_ctrData.DeleteAllItems();

			//清空感兴趣区域
			while (m_RoiQue.size())
			{
				m_RoiQue.pop_back();
			}
			m_CurrRoi.index = -1;

			//清空图像数据
			if (m_SrcImage)
			{
				delete m_SrcImage;
				m_SrcImage = NULL;
			}

			if (m_CannyImage)
			{
				delete m_CannyImage;
				m_CannyImage = NULL;
			}

			g_bLive = TRUE;
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("系统实时监视中..."));
			AfxBeginThread(WatchThread, (LPVOID)this);
		}
		SetDlgItemText(IDC_LIVE, _T("停止监视"));
	}
	else if (str == _T("停止监视"))
	{
		g_bLive = FALSE;
		Sleep(200);
		if (m_SrcImage)
		{
			delete m_SrcImage;
			m_SrcImage = NULL;
		}
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("系统停止监视！"));
		SetDlgItemText(IDC_LIVE, _T("实时监视"));
	}
	
	
	
}

void CLiquorDetectDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	
	BeginWaitCursor();
	int device;
	m_ctrImage.m_Msg = _T("正在初始化相机...");
	m_ctrImage.RedrawWindow();
	GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("正在初始化相机..."));
	// 链接相机
	if(m_Camera.m_isLink) 
	{
		m_Camera.GetImgData();
		m_Camera.CutCameraLink();
	}
	if (m_Camera.StartCameraLink())
	{
		m_ctrImage.m_Msg = _T("相机初始化完成.");
		m_ctrImage.RedrawWindow();
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("相机初始化完成."));
		m_NoEquipment = FALSE;
		if (m_Camera.GetDeviceSerialNumber() !=0x013ee9a9)
		{
			GetDlgItem(IDC_LIVE)->EnableWindow(FALSE);
			GetDlgItem(IDC_CAP_IMAGE)->EnableWindow(FALSE);
			GetDlgItem(IDC_FIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_EMPTYDATA)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_LOW_SLIDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_HIG_SLIDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDGE)->EnableWindow(FALSE);
			GetDlgItem(IDC_PER_CIRCLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_MIN_R)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAX_R)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_PRODUCTREPORT)->EnableWindow(FALSE);
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("使用了非法捕获设别，请用户更换为指定设别！"));
			EndWaitCursor();
			KillTimer(0);
			return;
		}

		m_Width = m_Camera.GetImgWidth();
		m_Height = m_Camera.GetImgHeight();
	}
	else
	{
		m_ctrImage.m_Msg = _T("未发现相机.");
		m_ctrImage.RedrawWindow();
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("未发现相机."));
		m_NoEquipment = TRUE;
	}
	

	EndWaitCursor();
	KillTimer(0);
	CDialog::OnTimer(nIDEvent);
}

void CLiquorDetectDlg::OnBnClickedCapImage()
{
	// TODO: 在此添加控件通知处理程序代码

	// 没有设备则返回
	if (m_NoEquipment)
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("未发现相机."));
		return;
	}

	// 关闭监视线程
	if (g_bLive)
	{
		g_bLive = FALSE;
		Sleep(200);
		if (m_SrcImage)
		{
			delete m_SrcImage;
			m_SrcImage = NULL;
		}
		m_SrcImage = new BYTE[m_Width*m_Height];
		memcpy(m_SrcImage, m_Image.m_lpImage, m_Width*m_Height);
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("成功捕获一帧图像"));
		SetDlgItemText(IDC_LIVE, _T("实时监视"));

	}
	else
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("请先开启监视"));
}

void CLiquorDetectDlg::OnBnClickedExit()
{
	// TODO: 在此添加控件通知处理程序代码
	SendMessage(WM_CLOSE);
}

void CLiquorDetectDlg::OnExit()
{
	// TODO: 在此添加命令处理程序代码
	SendMessage(WM_CLOSE);
}

void CLiquorDetectDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString str;

	if (m_PrecisionSlider.GetSafeHwnd() == pScrollBar->GetSafeHwnd())
	{
		m_precision = 0.005*m_PrecisionSlider.GetPos()+0.5;
		str.Format("%.2f", m_precision);
		SetDlgItemText(IDC_PRECISION, str);
	}
	else if (m_LowSlider.GetSafeHwnd() == pScrollBar->GetSafeHwnd() ||
		m_HighSlider.GetSafeHwnd() == pScrollBar->GetSafeHwnd())
	{
       if (m_SrcImage && g_bDetectCir)
       {
		   m_lthreRatio = (double)m_LowSlider.GetPos();
		   m_hthreRatio = (double)m_HighSlider.GetPos();
		   str.Format("%g", m_lthreRatio);
		   SetDlgItemText(IDC_LOW_TEXT, str);
		   str.Format("%g", m_hthreRatio);
		   SetDlgItemText(IDC_HIG_TEXT, str);

		   if (g_bLive || m_Image.IsEmpty())
			   return;

		   GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("正在进行图像预处理..."));
		   // Canny边缘检测
		   IplImage* edge = cvCreateImage(cvSize(m_Width, m_Height), IPL_DEPTH_8U, 1);
		   IplImage* gray = cvCreateImage(cvSize(m_Width, m_Height), IPL_DEPTH_8U, 1);
		   memcpy(gray->imageData, m_SrcImage, m_Width*m_Height);

		   // 高斯平滑 
		   cvSmooth(gray, edge, CV_BLUR, 3, 3, 0, 0);
		   cvNot(edge, gray);
		   cvCanny(gray, edge, m_lthreRatio, m_hthreRatio, 3);
		   cvNot(edge, edge);

		   // 存储边缘图像
		   if (m_CannyImage)
		   {
			   delete m_CannyImage;
			   m_CannyImage = NULL;
		   }
		   m_CannyImage = new BYTE[m_Width*m_Height];
		   memcpy(m_CannyImage, edge->imageData, m_Width*m_Height);

		   // 采用像素相关性插值算法缩小显示
		   IplImage* scale = cvCreateImage(cvSize(600,448), IPL_DEPTH_8U, 1);
		   cvResize( edge, scale, CV_INTER_AREA);
		   m_Image.Empty();
		   m_Image.m_Width = 600;	
		   m_Image.m_Height = 448;
		   m_Image.m_lpImage = new BYTE[600*448];
		   memcpy(m_Image.m_lpImage, scale->imageData, 600*448);

		   cvReleaseImage(&edge);
		   cvReleaseImage(&gray);
		   cvReleaseImage(&scale);

		   m_ctrImage.RedrawWindow();
       }
		
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CLiquorDetectDlg::OnBnClickedEdge()
{
	// TODO: 在此添加控件通知处理程序代码
	// 实时监视时不处理
	if (g_bLive)
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("请先捕获图像..."));
		return;
	}

	// 无图像源时不处理
	if (!m_SrcImage) 
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("没有图像源..."));
		return;
	}

	//批量检测中
	if (!g_bDetectCir)
	{
		return;
	}

	m_lthreRatio = (double)m_LowSlider.GetPos();
	m_hthreRatio = (double)m_HighSlider.GetPos();

    GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("正在进行图像预处理..."));
	// Canny边缘检测
	IplImage* edge = cvCreateImage(cvSize(m_Width, m_Height), IPL_DEPTH_8U, 1);
	IplImage* gray = cvCreateImage(cvSize(m_Width, m_Height), IPL_DEPTH_8U, 1);
	memcpy(gray->imageData, m_SrcImage, m_Width*m_Height);

	cvSmooth(gray, edge, CV_BLUR, 3, 3, 0, 0);// 高斯平滑
	cvNot(edge, gray);
	cvCanny(gray, edge, m_lthreRatio, m_hthreRatio, 3);
	cvNot(edge, edge);

	// 存储边缘图像
	if (m_CannyImage)
	{
		delete m_CannyImage;
		m_CannyImage = NULL;
	}
	m_CannyImage = new BYTE[m_Width*m_Height];
	memcpy(m_CannyImage, edge->imageData, m_Width*m_Height);

	// 采用像素相关性插值算法缩小显示
	IplImage* scale = cvCreateImage(cvSize(600,448), IPL_DEPTH_8U, 1);
	cvResize( edge, scale, CV_INTER_AREA);
	m_Image.Empty();
	m_Image.m_Width = 600;	
	m_Image.m_Height = 448;
	m_Image.m_lpImage = new BYTE[600*448];
	memcpy(m_Image.m_lpImage, scale->imageData, 600*448);

	cvReleaseImage(&edge);
	cvReleaseImage(&gray);
	cvReleaseImage(&scale);

	m_ctrImage.RedrawWindow();
}

// 坐标变换
CPoint CLiquorDetectDlg::ImageToClient(CPoint point)
{
	// 计算缩放比例
	double scale = m_Width/600.0;
	return CPoint(point.x/scale, (m_Height-1-point.y)/scale+1);
}

CPoint CLiquorDetectDlg::ClientToImage(CPoint point)
{
	// 计算缩放比例
	double scale = m_Width/600.0;
	return CPoint(point.x*scale, m_Height-1 - (point.y-1)*scale);
}

double CLiquorDetectDlg::ImageToClient(double x)
{
	// 计算缩放比例
	double scale = m_Width/600.0;
	return x/scale;
}

double CLiquorDetectDlg::ClientToImage(double x)
{
	// 计算缩放比例
	double scale = m_Width/600.0;
	return x*scale;
}


// 检测圆环
void CLiquorDetectDlg::OnBnClickedNeginCheck()
{
	// TODO: 在此添加控件通知处理程序代码
	//注册---------------------------------------------------------------------------
	CRegisterDlg dlg;

	//得到硬盘序列号
	dlg.GetHardDiskSerial();

	//硬件信息RSA编码
	dlg.m_StrRegValue = dlg.GetRasValue(dlg.m_StrDiskSerial);


	//硬件信息编码?=注册码
	if ( dlg.m_StrRegValue.IsEmpty() || dlg.m_StrRegValue != dlg.CheckRegister())
	{

		GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_FIT)->EnableWindow(FALSE);		
		GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(FALSE);
		MessageBox("请您先进行注册，否则软件功能受限!", "软件注册提示");

	}
	else
	{
		GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(TRUE);
		GetDlgItem(IDC_FIT)->EnableWindow(TRUE);
	}
	//注册---------------------------------------------------------------------------
	// 更新数据
	if (g_bDetectCir && m_CannyImage != NULL && !g_bLive)
	{
		m_ctrCurverFit.m_bFit = FALSE;
		m_ctrCurverFit.m_theat = 0.0;
		m_ctrCurverFit.m_b = 0.0;

		UpdateData();
		AfxBeginThread(DetectCircleThread, (LPVOID)this);
	}
	
}


void CLiquorDetectDlg::OnCamerasetMenu()
{
	// TODO: 在此添加命令处理程序代码
	if (g_bLive)
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("调整相机参数设置..."));
		CCameraSetDlg camSetDlg;
		m_Exposure = m_Camera.GetCameraExposureRaw();
		m_Gain = m_Camera.GetCameraGainRaw();
		camSetDlg.DoModal();
	}
	else
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("请先开启实时监视."));
	}
	
}

void CLiquorDetectDlg::OnCameralinkMenu()
{
	// TODO: 在此添加命令处理程序代码
	if( !g_bLive )
	{
		m_Image.Empty();

		if (m_SrcImage)
		{
			delete m_SrcImage;
			m_SrcImage = NULL;
		}

		if (m_CannyImage)
		{
			delete m_CannyImage;
			m_CannyImage = NULL;
		}

		m_ctrImage.RedrawWindow();
		SetTimer(0, 1000, NULL);	
	}
}

BOOL CLiquorDetectDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
     if(pMsg->message == WM_KEYDOWN )
	 {
		 switch(pMsg->wParam)
		 {
		 case VK_RETURN:
			 return TRUE;

		 case VK_ESCAPE:
			 return TRUE;

		 default:
			 break;
		 }
	 }
	return CDialog::PreTranslateMessage(pMsg);
}

void CLiquorDetectDlg::SetRidusMaxMin(int RoiWidth, int RoiHeight)
{
	(max(RoiWidth, RoiHeight)/2 > m_MaxR) ? m_MaxR = (max(RoiWidth, RoiHeight)/2) : m_MaxR;
	(min(RoiWidth, RoiHeight)/2 < m_MinR) ? m_MinR = (min(RoiWidth, RoiHeight)/2/3) : m_MinR;

	UpdateData(FALSE);
}
void CLiquorDetectDlg::OnAboutMenu()
{
	// TODO: 在此添加命令处理程序代码
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}

void CLiquorDetectDlg::OnBnClickedFit()
{
	// TODO: 在此添加控件通知处理程序代码
	//注册---------------------------------------------------------------------------
	CRegisterDlg dlg;

	//得到硬盘序列号
	dlg.GetHardDiskSerial();

	//硬件信息RSA编码
	dlg.m_StrRegValue = dlg.GetRasValue(dlg.m_StrDiskSerial);

	//硬件信息编码?=注册码
	if ( dlg.m_StrRegValue.IsEmpty() || dlg.m_StrRegValue != dlg.CheckRegister())
	{
		GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_FIT)->EnableWindow(FALSE);		
		GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(FALSE);
		MessageBox("请您先进行注册，否则软件功能受限!", "软件注册提示");
	}

	//注册---------------------------------------------------------------------------
	if (!g_bDetectCir)
	{		
		return;
	}
	if( m_ctrCurverFit.Fit() )
	{
		GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(TRUE);
	}
	else
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("请设定完整的标准品浓度！"));

	
}

void CLiquorDetectDlg::OnReport()
{
	// TODO: 在此添加命令处理程序代码
	if (!g_bDetectCir)
	{
		return;
	}
	//OPenCV 截屏保存图像  
	CapImageAndCurve();

	//获取报表需要的数据
	CreateLiquorData();

	BOOL  isSave;

	char ImgSaveName[MAX_PATH];

	//创建报表
	if( ProduceWordReport(&m_StdLiqData, &m_TestLiqData, m_lthreRatio, m_hthreRatio, m_precision, ImgSaveName, isSave) )
		//存贮图像文件
		SaveImgToDisk(ImgSaveName, isSave);
}

/************************************************************************/
/* OPenCV 截屏保存图像                                                  */
/*
/************************************************************************/
void CLiquorDetectDlg::CapImageAndCurve(void)
{
	int bits = ::GetDeviceCaps(::GetDC(NULL), BITSPIXEL);
	CSize size;
	IplImage* Image;

	//获得当前应用程序所在路径
	CString pathName;
	TCHAR szPath[MAX_PATH + 1];
	GetModuleFileName(AfxGetInstanceHandle(), szPath, MAX_PATH);
	pathName = (LPTSTR) szPath;
	int position = pathName.ReverseFind('\\');   //反向查找"\"所在的位置 
	pathName=pathName.Left(position+1);          //删除文件名，只获得路径

	//取图像数据(带感兴趣区域的)
	if (m_ctrImage.m_pBitmap)
	{
		size = m_ctrImage.m_pBitmap->GetBitmapDimension();
		Image = cvCreateImage(cvSize(size.cx, size.cy), IPL_DEPTH_8U, bits/8);
		m_ctrImage.m_pBitmap->GetBitmapBits(size.cx*size.cy*bits/8, Image->imageData);
		cvSaveImage(pathName + _T("ReportData\\Image.bmp"), Image);
		cvReleaseImage(&Image);
	}

	//原始图像数据
	if (m_SrcImage != NULL)
	{
		IplImage* ImgOri = cvCreateImage(cvSize(m_Width, m_Height), IPL_DEPTH_8U, 1);
		IplImage* scale = cvCreateImage(cvSize(600,448), IPL_DEPTH_8U, 1);
		memcpy(ImgOri->imageData, m_SrcImage, m_Width*m_Height);
        cvResize(ImgOri, scale, CV_INTER_AREA);
		cvFlip(scale, NULL, 0);
		cvSaveImage(pathName + "ReportData\\ImgOri.bmp", scale);
		cvReleaseImage(&ImgOri);
		cvReleaseImage(&scale);
	}
	else
	{
		size = m_ctrImage.m_pBitmap->GetBitmapDimension();
		Image = cvCreateImage(cvSize(size.cx, size.cy), IPL_DEPTH_8U, bits/8);
		m_ctrImage.m_pBitmap->GetBitmapBits(size.cx*size.cy*bits/8, Image->imageData);
		cvSaveImage(pathName + _T("ReportData\\ImgOri.bmp"), Image);
		cvReleaseImage(&Image);

	}

	// 取曲线图数据
	if (m_ctrCurverFit.m_pBitmap)
	{
		size = m_ctrCurverFit.m_pBitmap->GetBitmapDimension();
		Image = cvCreateImage(cvSize(size.cx, size.cy), IPL_DEPTH_8U, bits/8);
		m_ctrCurverFit.m_pBitmap->GetBitmapBits(size.cx*size.cy*bits/8, Image->imageData);
		cvSaveImage(pathName + "ReportData\\Curve.bmp", Image);
		cvReleaseImage(&Image);
	}
}

// 创建标准溶液数据
void CLiquorDetectDlg::CreateLiquorData(void)
{
	m_StdLiqData.RemoveAll();
	m_TestLiqData.RemoveAll();
	for (int i = 0; i < m_ctrData.GetItemCount(); i++)
	{
		if(m_ctrData.GetCheck(i))		
		{   
			m_StdLiqData.Add(m_ctrData.GetItemText(i, 0));
			m_StdLiqData.Add(m_ctrData.GetItemText(i, 1));
			m_StdLiqData.Add(m_ctrData.GetItemText(i, 2));
			m_StdLiqData.Add(m_ctrData.GetItemText(i, 4));
			m_StdLiqData.Add(m_ctrData.GetItemText(i, 5));
		}
		else
		{
			//添加序号
			m_TestLiqData.Add(m_ctrData.GetItemText(i, 0));

			//添加半径
			m_TestLiqData.Add(m_ctrData.GetItemText(i, 1));

			//添加稀释浓度
			m_TestLiqData.Add(m_ctrData.GetItemText(i, 2));

			//添加稀释倍数
			m_TestLiqData.Add(m_ctrData.GetItemText(i, 4));

			//添加实际浓度值
			m_TestLiqData.Add(m_ctrData.GetItemText(i, 5));
		
		}
	
	}

}

void CLiquorDetectDlg::OnNMDblclkListCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_ctrCurverFit.m_bFit)
	{
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("请先进行标准浓度拟合."));
		return;
	}

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if(pNMListView->iItem != -1)
	{
		CString strtemp;
		strtemp.Format("单击的是第%d行第%d列",
			pNMListView->iItem, pNMListView->iSubItem);
		//		MessageBox(strtemp);

		m_CurrRoi = m_RoiQue[pNMListView->iItem];
		DWORD dwPos = GetMessagePos();
		CPoint point( LOWORD(dwPos), HIWORD(dwPos) );

		m_ctrData.ScreenToClient(&point);

		UINT nFlag;
		int nItem = m_ctrData.HitTest(point, &nFlag);

		CString str;
		double  TestDensity = 0.0;

		if (!m_ctrData.GetCheck(pNMListView->iItem))
		{   
			TestDensity = m_ctrCurverFit.m_theat * atof(m_ctrData.GetItemText(pNMListView->iItem, 1)) + m_ctrCurverFit.m_b;
			str.Format("%.2f", TestDensity);
			m_ctrData.SetItemText(pNMListView->iItem, 2, str);
		}

		m_ctrImage.RedrawWindow();
	}
	*pResult = 0;

}

void CLiquorDetectDlg::OnRegisterMenu()
{
	// TODO: 在此添加命令处理程序代码

	//注册
	CRegisterDlg dlg;

	//得到硬盘序列号
	dlg.GetHardDiskSerial();

	//硬件信息RSA编码
	dlg.m_StrRegValue = dlg.GetRasValue(dlg.m_StrDiskSerial);

	//硬件信息编码?=注册码
	if ( dlg.m_StrRegValue.IsEmpty() || dlg.m_StrRegValue != dlg.CheckRegister())
	{
		//程序初始化；
		if ( dlg.DoModal() != 10)
		{
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("请您先进行注册，否则软件功能受限!"));
			GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(FALSE);
			GetDlgItem(IDC_FIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("注册成功, 欢迎使用!"));
			GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(TRUE);
			GetDlgItem(IDC_FIT)->EnableWindow(TRUE);
			GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(TRUE);
		}

	}
	else
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("本软件已成功注册, 欢迎使用!"));
}

void CLiquorDetectDlg::OnFileOpen()
{
	// TODO: 在此添加命令处理程序代码
	if (!g_bDetectCir)
	{
		return;
	}

	//清空LISTCTRL数据
	m_ctrData.DeleteAllItems();

	//清空感兴趣区域
	while (m_RoiQue.size())
	{
		m_RoiQue.pop_back();
	}
	m_CurrRoi.index = -1;

	//清空图像数据
	if (m_SrcImage)
	{
		delete m_SrcImage;
		m_SrcImage = NULL;
	}

	if (m_CannyImage)
	{
		delete m_CannyImage;
		m_CannyImage = NULL;
	}

	CFile file;
	CString pathName, str;
	static char BASED_CODE szFilter[] = "THA Files (*.tha)|*.tha||";
	CFileDialog fileDialog(TRUE, _T(".tha"), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	if(fileDialog.DoModal() == IDOK)
	{
		pathName = fileDialog.GetPathName();
		if(file.Open(pathName, CFile::modeRead | CFile::shareDenyWrite))
		{
			if(m_Image.Read(&file))
			{
				m_Width = m_Image.m_Width;
				m_Height = m_Image.m_Height;
				m_SrcImage = new BYTE[m_Width * m_Height];
				memcpy(m_SrcImage, m_Image.m_lpImage, m_Width * m_Height);
				m_ctrImage.RedrawWindow();

				//更新图像处理参数信息
				m_lthreRatio = m_Image.m_low;
				m_hthreRatio = m_Image.m_high;
				m_precision  = m_Image.m_circular;

				m_LowSlider.SetPos((int)m_lthreRatio);
				m_HighSlider.SetPos((int)m_hthreRatio);
				m_PrecisionSlider.SetPos((int)((m_precision-0.5)/0.005));

				str.Format("%.2f", m_precision);
				SetDlgItemText(IDC_PRECISION, str);

				str.Format("%g", m_lthreRatio);
				SetDlgItemText(IDC_LOW_TEXT, str);

				str.Format("%g", m_hthreRatio);
				SetDlgItemText(IDC_HIG_TEXT, str);

				GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("从文件中载入一帧图像..."));

			}
			file.Close();
		}

	}


}

void CLiquorDetectDlg::OnSavedataMenu()
{
	// TODO: 在此添加命令处理程序代码
	if (!g_bDetectCir)
	{
		return;
	}

	if (m_SrcImage != NULL)
	{
		CFile file;
		CString pathName;
		static char BASED_CODE szFilter[] = "THA Files (*.tha)|*.tha||";

		CFileDialog fileDialog(FALSE, _T("tha"), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);

		if(fileDialog.DoModal() == IDOK)
		{
			pathName = fileDialog.GetPathName();
			if (file.Open(pathName, CFile::modeCreate | CFile::modeWrite))
			{
				if(!m_Image.Write(&file, m_SrcImage, m_lthreRatio, m_hthreRatio, m_precision))
				{
					GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("写入数据失败！"));
					return;
				}
				file.Close();
			}
			else
				GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("保存文件失败！"));
			
		}
	}
	else
    	GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("数据文件无法保存，请先捕获图像数据！"));
	
}
//void CLiquorDetectDlg::MakeBitmap()
//{
//	CPaintDC dc(this);
//	CRect rect;
//	GetClientRect(&rect);
//
//	rect.right += 3;
//
////  int r2=155, g2=206, b2=207;
//// 	int r1=247, g1=252, b1=255;
//
//	//int r2=247, g2=252, b2=255;
//	//int r1=205, g1=230, b1=231;
//
//	int r1=130,g1=0,b1=0;
//	int r2=245,g2=190,b2=240;
//
//	CDC dc2;
//	dc2.CreateCompatibleDC(&dc);
//
//	if(m_bitmap.m_hObject)
//	{
//		m_bitmap.DeleteObject();
//	}
//	m_bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
//
//	CBitmap *oldbmap = dc2.SelectObject(&m_bitmap);
//
//	//将颜色值划分2^8 = 256个等级
//	int nShift = 8;
//	int nSteps = 1 << nShift;
//
//	for (int i = 0; i < nSteps; i++)
//	{
//		//合成每个等级的颜色值，实际上是以nSteps为等级取得首尾二色的均值
//		BYTE bR = (BYTE) ((r1 * (nSteps - i) + r2 * i) >> nShift);
//		BYTE bG = (BYTE) ((g1 * (nSteps - i) + g2 * i) >> nShift);
//		BYTE bB = (BYTE) ((b1 * (nSteps - i) + b2 * i) >> nShift);
//
//		CBrush br (RGB(bR, bG, bB));
//
//		//使用新的画刷填充
//		CRect r2 = rect;
//		//垂直方向渐变填充
//		r2.bottom = rect.bottom - 
//			((i * rect.Height()/2) >> nShift);
//		r2.top = rect.bottom - 
//			(((i + 1) * rect.Height()/2) >> nShift);
//		if (r2.Height() > 0)
//			dc2.FillRect(r2, &br);
//	}
//
//	for (int i = 0; i < nSteps; i++)
//	{
//		//合成每个等级的颜色值，实际上是以nSteps为等级取得首尾二色的均值
//		BYTE bR = (BYTE) ((r2 * (nSteps - i) + r1 * i) >> nShift);
//		BYTE bG = (BYTE) ((g2 * (nSteps - i) + g1 * i) >> nShift);
//		BYTE bB = (BYTE) ((b2 * (nSteps - i) + b1 * i) >> nShift);
//
//		CBrush br (RGB(bR, bG, bB));
//
//		//使用新的画刷填充
//		CRect r2 = rect;
//		//垂直方向渐变填充
//		r2.bottom = (rect.bottom - rect.Height()/2) - 
//			((i * rect.Height()/2) >> nShift);
//		r2.top = (rect.bottom - rect.Height()/2) - 
//			(((i + 1) * rect.Height()/2) >> nShift);
//		if (r2.Height() > 0)
//			dc2.FillRect(r2, &br);
//	}
//
//	m_hBrush = ::CreatePatternBrush((HBITMAP)m_bitmap.GetSafeHandle());
//	dc2.SelectObject(oldbmap);
//	oldbmap->DeleteObject();
//	dc2.DeleteDC();
//}

//背景的渐变色
void CLiquorDetectDlg::MakeBitmap()
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
void CAboutDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnTimer(nIDEvent);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	//SetTimer(0, 40, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
//---------------------------------------------------------
//-功能：添加背景色渐变刷新
//-Date：2009.4.10
//-Ver：1.0
//---------------------------------------------------------
BOOL CLiquorDetectDlg::OnEraseBkgnd(CDC* pDC)
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

//	return CDialog::OnEraseBkgnd(pDC);
}

//---------------------------------------------------------
//-功能：计算待测溶液的浓度值
//-Date：2009.4.17
//-Ver：1.0
//---------------------------------------------------------
void CLiquorDetectDlg::OnBnClickedWaitTestCalculate()
{
	// TODO: 在此添加控件通知处理程序代码
	//注册---------------------------------------------------------------------------
	CRegisterDlg dlg;

	//得到硬盘序列号
	dlg.GetHardDiskSerial();

	//硬件信息RSA编码
	dlg.m_StrRegValue = dlg.GetRasValue(dlg.m_StrDiskSerial);

	//硬件信息编码?=注册码
	if ( dlg.m_StrRegValue.IsEmpty() || dlg.m_StrRegValue != dlg.CheckRegister())
	{

		GetDlgItem(IDC_NEGIN_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_FIT)->EnableWindow(FALSE);		
		GetDlgItem(IDC_WAIT_TEST_CALCULATE)->EnableWindow(FALSE);
		MessageBox("请您先进行注册，否则软件功能受限!", "软件注册提示");

	}

	//注册---------------------------------------------------------------------------
    if (m_ctrCurverFit.m_bFit)
    {
		CString str, strN, strReal;
		float   TestDensity, ConReal;

		for (int i = 0; i < m_ctrData.GetItemCount(); i++)
		{
			if (!m_ctrData.GetCheck(i))
			{   
				//计算测试浓度
				TestDensity = m_ctrCurverFit.m_theat * atof(m_ctrData.GetItemText(i, 1)) + m_ctrCurverFit.m_b;
				str.Format("%.2f", TestDensity);
				m_ctrData.SetItemText(i, 2, str);

				//计算实际浓度
				strN    = m_ctrData.GetItemText(i, 4);
				ConReal = TestDensity*atoi(strN);
				strReal.Format(_T("%.2f"), ConReal);
				m_ctrData.SetItemText(i, 5, strReal);
			}

		}
    }
	

	
}

void CLiquorDetectDlg::OnBnClickedBtnProductreport()
{
	// TODO: 在此添加控件通知处理程序代码
	//检测中不允许生成报表
	if (!g_bDetectCir)
	{
		return;
	}
	//OPenCV 截屏保存图像  
	CapImageAndCurve();

	//获取报表需要的数据
	CreateLiquorData();

	BOOL  isSave = FALSE;
	char ImgSaveName[MAX_PATH];

	//创建报表
	if( ProduceWordReport(&m_StdLiqData, &m_TestLiqData, m_lthreRatio, m_hthreRatio, m_precision, ImgSaveName, isSave) )
    //存贮图像文件
	SaveImgToDisk(ImgSaveName, isSave);
	

}

void CLiquorDetectDlg::OnBnClickedBtnEmptydata()
{
	// TODO: 在此添加控件通知处理程序代码
	//清空LISTCTRL数据
	m_ctrData.DeleteAllItems();

	//清空感兴趣区域
	while (m_RoiQue.size())
	{
		m_RoiQue.pop_back();
	}
	m_CurrRoi.index = -1;
	m_ctrImage.RedrawWindow();
}

void CLiquorDetectDlg::OnEnKillfocusEditListctrl()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str, strTest, strN, strReal;
	float  ConReal;
	GetDlgItem(IDC_EDIT_LISTCTRL)->GetWindowText(str);

	m_ctrData.SetItemText(m_curItem, m_curSubItem, str);
	strTest = m_ctrData.GetItemText(m_curItem, 2);
	if (!strTest.IsEmpty())
	{
		strN    = m_ctrData.GetItemText(m_curItem, 4);
		ConReal = atof(strTest)*atoi(strN);
		strReal.Format(_T("%.2f"), ConReal);
		m_ctrData.SetItemText(m_curItem, 5, strReal);
	}
	GetDlgItem(IDC_EDIT_LISTCTRL)->ShowWindow(SW_HIDE);
}

// 存贮数据文件到硬盘
void CLiquorDetectDlg::SaveImgToDisk(CString PathName, BOOL isSave)
{
	//没有选中
	if (!isSave )
	{
		return;
	}

	if (!g_bDetectCir)
	{
		return;
	}

	if (m_SrcImage != NULL )
	{   
		CFile file;
		if (file.Open(PathName, CFile::modeCreate | CFile::modeWrite))
		{
			if(!m_Image.Write(&file, m_SrcImage, m_lthreRatio, m_hthreRatio, m_precision))
			{
				GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("写入数据失败！"));
				return;
			}
			file.Close();
		}
		
	}
	else
		GetDlgItem(IDC_MESSAGE)->SetWindowText(_T("数据文件无法保存，请先捕获图像数据！"));
}
