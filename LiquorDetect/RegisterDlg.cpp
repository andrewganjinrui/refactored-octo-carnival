// RegisterDlg.cpp : implementation file

#include "stdafx.h"
#include "LiquorDetect.h"
#include "RegisterDlg.h"


//宏定义编译 
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define IDS_REG_KEY_PATH _T( "Software\\Target" )
#define IDS_REG_KEY_VALUE _T("Value")

#define REGISTER_SUBKEY_PATH "Software\\Target"
/////////////////////////////////////////////////////////////////////////////

// CAboutDlg dialog used for App About



// CRegisterDlg dialog
CRegisterDlg::CRegisterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegisterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRegisterDlg)
	//}}AFX_DATA_INIT
	m_StrRegValue = _T("");
	m_StrDiskSerial = _T("");
}


void CRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegisterDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegisterDlg, CDialog)
	//{{AFX_MSG_MAP(CRegisterDlg)
	ON_BN_CLICKED(IDC_REGISTER_BUTTON, OnRegisterButton)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CRegisterDlg::OnBnClickedCancel)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegisterDlg message handlers
/**************************注册软件*************************/
void CRegisterDlg::OnRegisterButton() 
{
	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_REGVALUE, m_StrRegValue);

	if (m_StrRegValue == _T(""))
	{
		MessageBox("请输入正确注册序列号!", "提示");
		return;
	}

	//将用户输入的注册序列号和硬件序列号比对
	if (RsaRegValve(m_StrRegValue, m_StrDiskSerial))
	{
		CRegKey reg;
		//创建注册表KEY键值
		if (ERROR_SUCCESS != reg.Create(HKEY_LOCAL_MACHINE, IDS_REG_KEY_PATH, 
			REG_NONE, REG_OPTION_NON_VOLATILE,KEY_READ | KEY_WRITE, NULL, NULL))
		{
			MessageBox("注册KEY创建失败!", "提示");
			if (reg.m_hKey != NULL)
			{
				RegCloseKey(reg.m_hKey);
			}
			return ;
		}
		
		//方法1：创建对应的VALIUE，和VALIUE的内容
		if (ERROR_SUCCESS != reg.SetValue(m_StrRegValue, NULL))
		{
			MessageBox("注册Valve键创建失败!", "提示");
			
			if (reg.m_hKey != NULL)
			{
				RegCloseKey(reg.m_hKey);
			}
			
			return ;
		}
		
		//注册成功,关闭注册文件，退出对话框
		if (reg.m_hKey != NULL)
		{
			RegCloseKey(reg.m_hKey);
		}
		EndDialog(10);//结束对话框,10为对话框DoModal的人工返回值
		
	}
	else
	{
		MessageBox("注册序列号错误,请重新输入!", "提示");
		GetDlgItem(IDC_REGVALUE)->SetFocus();
		return ;
	}

			
}


BOOL CRegisterDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
       switch(pMsg->wParam)
       {
       case VK_ESCAPE: return FALSE;
       	    break;

       case VK_RETURN: return FALSE;
            break;

       default:
           break;
       }
	   
	}
	
	
	return CDialog::PreTranslateMessage(pMsg);
}

/*************************************************************************
*
* 函数名称：ChangeByteOrder
*
* 参  数:   PCHAR szString         //字符数组指针
USHORT uscStrSize      //字符数组长度
*
* 返回值:   void       
*
* 说  明:   该函数用来将前后两个字符顺序调换

************************************************************************/
void CRegisterDlg::ChangeByteOrder(PCHAR szString, USHORT uscStrSize)
{  
    USHORT i;
    CHAR temp;
    
    for (i = 0; i < uscStrSize; i+=2)
    {
        temp = szString[i];
        szString[i] = szString[i+1];
        szString[i+1] = temp;
    }
}


/*************************************************************************
*
* 函数名称：HardSerialNT
*
* 参  数:   无
*
* 返回值:   BOOL       
*
* 说  明:   该函数用来获取系统的硬盘序列号

 ************************************************************************/
BOOL CRegisterDlg::HardSerialNT()
{
    char hd[80];
    PIDSECTOR phdinfo;
    char s[41];

    ZeroMemory(&m_struct_vers,sizeof(m_struct_vers));

    //开始查询NT/Win2000系统下的硬盘序列号
    for (m_bj=0; m_bj<4; m_bj++)
	{
        sprintf(hd,"\\\\.\\PhysicalDrive%d",m_bj);
        m_handle = CreateFile(hd, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        if (!m_handle)
		{
            return FALSE;
        }
        if (!DeviceIoControl(m_handle, DFP_GET_VERSION, 0, 0, &m_struct_vers, sizeof(m_struct_vers), &m_di, 0))
		{
            CloseHandle(m_handle);
            continue;
        }
        //如果IDE硬盘命令识别功能不支持，则退出
        if (!(m_struct_vers.fCapabilities&1))
		{
            MessageBox("IDE硬盘命令识别功能不支持!", "提示");
            CloseHandle(m_handle);
            return FALSE;
        }

        //识别IDE硬盘
        ZeroMemory(&m_struct_inparam,sizeof(m_struct_inparam));
        ZeroMemory(&m_struct_outparam,sizeof(m_struct_outparam));
        if (m_bj&1)
		{
            m_struct_inparam.irDriveRegs.bDriveHeadReg=0xb0;
        }
		else
		{
            m_struct_inparam.irDriveRegs.bDriveHeadReg=0xa0;
        }
        if (m_struct_vers.fCapabilities&(16>>m_bj))
		{
            //不检测SATA类型硬盘
			m_strbuf.Format("Drive %d is a ATAPI device, we don't detect it!",(int)(m_bj+1));
			MessageBox(m_strbuf, "提示");
            continue;
        }
		else
		{
            m_struct_inparam.irDriveRegs.bCommandReg=0xec;
        }
        m_struct_inparam.bDriveNumber=m_bj;
        m_struct_inparam.irDriveRegs.bSectorCountReg=1;
        m_struct_inparam.irDriveRegs.bSectorNumberReg=1;
        m_struct_inparam.cBufferSize=512;
        if (!DeviceIoControl(m_handle, DFP_RECEIVE_DRIVE_DATA, &m_struct_inparam,sizeof(m_struct_inparam), &m_struct_outparam, sizeof(m_struct_outparam), &m_di, 0))
		{
            AfxMessageBox("DeviceIoControl failed:DFP_RECEIVE_DRIVE_DATA!");
            CloseHandle(m_handle);
            return FALSE;
        }


		phdinfo = (PIDSECTOR)m_struct_outparam.bBuffer;
        memcpy(s,phdinfo->sSerialNumber,20);
        s[20]=0;
        ChangeByteOrder(s,20);
		m_StrDiskSerial.Format("%s",(LPCSTR)s);
        //去掉表达式中的空格符
		
		int pos = m_StrDiskSerial.Find(" ");
		while(pos!=-1) 
		{
			m_StrDiskSerial.Delete(pos);
			pos=m_StrDiskSerial.Find(" ");
		}

        CloseHandle(m_handle);
    }
	return TRUE;
    
}

/*************************************************************************
*
* 函数名称：HardSerialNT
*
* 参  数:   无
*
* 返回值:   BOOL       
*
* 说  明:   该函数用来获取9X系统的硬盘序列号

************************************************************************/
BOOL CRegisterDlg::HardSerial9X()
{
	PIDSECTOR phdinfo;
	char s[41];

	ZeroMemory(&m_struct_vers,sizeof(m_struct_vers));

	//Identify the IDE drives

	for (m_bj=0; m_bj<4; m_bj++)
	{

		//We start in 95/98/Me
		m_handle = CreateFile("\\\\.\\Smartvsd",0,0,0,CREATE_NEW,0,0);
		if (!m_handle)
		{
			return FALSE;
		}

		if (!DeviceIoControl(m_handle, DFP_GET_VERSION, 0, 0, &m_struct_vers, sizeof(m_struct_vers), &m_di, 0))
		{
			CloseHandle(m_handle);
			continue;
		}
		//If IDE identify command not supported, fails
		if (!(m_struct_vers.fCapabilities&1))
		{
			AfxMessageBox("IDE硬盘命令识别功能不支持!");
			CloseHandle(m_handle);
			return FALSE;
		}


		//识别IDE硬盘
		ZeroMemory(&m_struct_inparam,sizeof(m_struct_inparam));
		ZeroMemory(&m_struct_outparam,sizeof(m_struct_outparam));
		if (m_bj&1)
		{
			m_struct_inparam.irDriveRegs.bDriveHeadReg=0xb0;
		}
		else
		{
			m_struct_inparam.irDriveRegs.bDriveHeadReg=0xa0;
		}
		if (m_struct_vers.fCapabilities&(16>>m_bj))
		{ 
			//不检测SATA类型硬盘
			m_strbuf.Format("Drive %d is a ATAPI device, we don't detect it!",(int)(m_bj+1));
			AfxMessageBox(m_strbuf);
		}
		else
		{
			m_struct_inparam.irDriveRegs.bCommandReg=0xec;
		}
		m_struct_inparam.bDriveNumber=m_bj;
		m_struct_inparam.irDriveRegs.bSectorCountReg=1;
		m_struct_inparam.irDriveRegs.bSectorNumberReg=1;
		m_struct_inparam.cBufferSize=512;
		if (!DeviceIoControl(m_handle, DFP_RECEIVE_DRIVE_DATA,&m_struct_inparam,sizeof(m_struct_inparam),&m_struct_outparam,sizeof(m_struct_outparam),&m_di,0))
		{
			AfxMessageBox("DeviceIoControl failed:DFP_RECEIVE_DRIVE_DATA!");
			CloseHandle(m_handle);
			return FALSE;
		}
		phdinfo=(PIDSECTOR)m_struct_outparam.bBuffer;
		memcpy(s,phdinfo->sSerialNumber,20);
		s[20]=0;
		ChangeByteOrder(s,20);
		m_StrDiskSerial.Format(_T("%s"), (LPCSTR)s);
		//去掉表达式中的空格符

		int pos = m_StrDiskSerial.Find(" ");
		while(pos!=-1) 
		{
			m_StrDiskSerial.Delete(pos);
			pos=m_StrDiskSerial.Find(" ");
		}

		//Close handle before quit
		CloseHandle(m_handle);
	}

	return TRUE;
}


/*************************************************************************
*
* 函数名称：GetHardDiskSerial
*
* 参  数:   无
*
* 返回值:   BOOL       
*
* 说  明:   该函数用来获取系统类型，并调用相应的硬盘识别函数

 ************************************************************************/
BOOL CRegisterDlg::GetHardDiskSerial()
{

   //得到操作系统版本类型
   OSVERSIONINFO VersionInfo;
   ZeroMemory(&VersionInfo,sizeof(VersionInfo));
   VersionInfo.dwOSVersionInfoSize=sizeof(VersionInfo);
   GetVersionEx(&VersionInfo);


   m_StrDiskSerial = _T("");
   //通过操作系统版本类型的不同调用不同的函数得到硬盘信息信息
   switch (VersionInfo.dwPlatformId)
   {
   case VER_PLATFORM_WIN32s:
	    this->SetWindowText("Win32s is not supported by this programm!");
		m_StrDiskSerial = _T("");
		return FALSE;
	    break;

   case VER_PLATFORM_WIN32_WINDOWS:
	    this->SetWindowText("当前系统为:Windows XP");
	    if (!HardSerial9X())
		{
		   m_StrDiskSerial = _T("");
		   return FALSE;
		}
	    break;

   case VER_PLATFORM_WIN32_NT:
	    //this->SetWindowText("当前系统为:Windows NT");
	    if (!HardSerialNT())
		{
		   m_StrDiskSerial = _T("");
		   return FALSE;
		}
	    break;
   }

   return TRUE;
}

BOOL CRegisterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
    if (!GetHardDiskSerial())
    {
		MessageBox("用户注册码获取失败!", "提示");
		return FALSE;
    }
	//如果生成肯定知道硬盘信息，所以显示出来
    SetDlgItemText(IDC_DISKSERIAL, m_StrDiskSerial);

	//SetWindowText("请输入注册序列号!");
	MakeBitmap();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CRegisterDlg::RsaRegValve(CString& m_StrRegValue , CString& m_StrDiskSerial)
{
	// TODO: Add your control notification handler code here
	/* 	KeyGen 1.0 版
	*	版权所有 (C) 2009 Target
	*	2009.4.1
	*	http://cust.edu.cn
	*	http://china.Target.com
	*	本程序调用Miracl ver 4.82大数运算库。*/
	//////////////////////////////////////////////////////////
	
	//P(HEX)=E34436F5F48A227B
	//Q(HEX)=A92FA24467C4E3E3
	//N(HEX)=963251DC5A9C90D9F203A03C363BA411
	//D(HEX)=56157D29A89D77BF2F669A8F0B123CC9
	//E(HEX)=10001
	//Keysize(Bits)=128

	miracl *mip=mirsys(100,0);
	mip->IOBASE=16;	//16进制模式
	
	//定义并初始化变量
	big m=mirvar(0);	//m 放明文：注册码SN
	big c=mirvar(0);	//c 放密文：用户名Name
	big n=mirvar(0);	//n 模数
	big e=mirvar(0);	//e 公钥
	TCHAR Name[256]={0};
	TCHAR SN[256]={0};
	TCHAR temp[256]={0};
	int len=0;
	int i,j;
	
	//m_StrDiskSerial
	len = m_StrDiskSerial.GetLength();


	for (int k=0; k<len; k++)
	{
		Name[k] = m_StrDiskSerial.GetAt(k);		
	}
	
	//m_StrRegValue
	len = m_StrRegValue.GetLength();
	for (int l=0; l<len; l++)
	{
		SN[l] = m_StrRegValue.GetAt(l);		
	}

	
	//检查SN是否为16进制
	for (i=0,j=0;i<len;i++)
	{
		if(isxdigit(SN[i])==0)
		{
			j=1;
			break;
		}
	}
	
	//如果输入的SN为16进制且长度不为0
	if (j!=1&&len!=0)
	{
		
		cinstr(m,SN);									//初始化明文m
		cinstr(n,"963251DC5A9C90D9F203A03C363BA411");	//初始化模数n	 
		cinstr(e,"10001");								//初始化公钥e
		
		//当m<n时
		if(compare(m,n)==-1)
		{
			powmod(m,e,n,c);//计算c=m^e mod n
			big_to_bytes(256,c,temp,FALSE);//将c转换成数组写入temp
			
			//释放内存
			mirkill(m);
			mirkill(c);
			mirkill(n);
			mirkill(e);
			mirexit();
		}
		else
			j=1;
		
	}	
	//对Name、temp， m、n， SN的长度进行检查
	if(lstrcmp(Name,temp)!=0||j==1||len==0)
		return FALSE;
	else
 		return TRUE;

}

CString CRegisterDlg::GetRasValue(CString &m_StrDiskSerial)
{

	CString strRegValue = _T("");

	int namelen = m_StrDiskSerial.GetLength();	//获取硬件序列号的长度

	if (namelen != 0 && namelen<=16)//如果不为空且<=128/8则开始计算SN
	{
		miracl *mip=mirsys(100,0);
		mip->IOBASE=16;	//16进制模式
		
		//定义并初始化变量
		big m=mirvar(0);	//m 放明文：注册码SN
		big c=mirvar(0);	//c 放密文：用户名Name
		big n=mirvar(0);	//n 模数
		big d=mirvar(0);	//d 私钥
		TCHAR Name[256]={0};
		TCHAR SN[256]={0};
		
		//获取硬盘序列号内容
		for (int k=0; k<namelen; k++)
		{
			Name[k] = m_StrDiskSerial.GetAt(k);
		}

		bytes_to_big(namelen,Name,c);					//转换成16进制
		cinstr(n,"963251DC5A9C90D9F203A03C363BA411");	//初始化模数n	 
		cinstr(d,"56157D29A89D77BF2F669A8F0B123CC9");	//初始化私钥d
		powmod(c,d,n,m);								//计算m=c^d mod n
		cotstr(m,SN);									//将m以16进制串写入SN

		//得到加密后字符串
		strRegValue.Format("%s",(LPCSTR)SN);
	    	
		//释放内存
		mirkill(m);
		mirkill(c);
		mirkill(n);
		mirkill(d);
		mirexit();
		return strRegValue;
	}
	return _T("");

}


/***********************************************************************
* 
* 函数名称：CheckRegister
*
* 参数：无
*      
* 功能：访问注册表，并且判断注册内容是否符合要求
*
* 返回值：BOOL (TRUE:注册内容满足要求) , 否则exit(0):退出程序
*	   
************************************************************************/
CString CRegisterDlg::CheckRegister()
{
	//注册KEY定义
	HKEY hKey = NULL;

	//注册表Value定义
	DWORD lpType = REG_EXPAND_SZ;
	
	BYTE  bData[40] = {0};//装值的地方
	
	DWORD cbData = 40;   //指定装的地方大小

	CString strValue;
	
	//打开注册表的键值文件
    if (::RegOpenKey(HKEY_LOCAL_MACHINE, REGISTER_SUBKEY_PATH, &hKey) != ERROR_SUCCESS )
	{
		//AfxMessageBox("注册KEY打开失败!");
		if (NULL != hKey)
		{
			RegCloseKey(hKey);
		}

		return _T("");
		
	}
	//打开键值文件，获得Value
	if(RegQueryValueEx(hKey, NULL, NULL, &lpType, bData, &cbData) != ERROR_SUCCESS)
	{
		if (NULL != hKey)
		{
			RegCloseKey(hKey);
		}
		return _T("");	
	}
	
	strValue = (LPSTR)bData;

	return strValue;

}


void CRegisterDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}

//背景的渐变色
void CRegisterDlg::MakeBitmap()
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
HBRUSH CRegisterDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CRegisterDlg::OnEraseBkgnd(CDC* pDC)
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
