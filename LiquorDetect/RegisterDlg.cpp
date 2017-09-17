// RegisterDlg.cpp : implementation file

#include "stdafx.h"
#include "LiquorDetect.h"
#include "RegisterDlg.h"


//�궨����� 
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
/**************************ע�����*************************/
void CRegisterDlg::OnRegisterButton() 
{
	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_REGVALUE, m_StrRegValue);

	if (m_StrRegValue == _T(""))
	{
		MessageBox("��������ȷע�����к�!", "��ʾ");
		return;
	}

	//���û������ע�����кź�Ӳ�����кűȶ�
	if (RsaRegValve(m_StrRegValue, m_StrDiskSerial))
	{
		CRegKey reg;
		//����ע���KEY��ֵ
		if (ERROR_SUCCESS != reg.Create(HKEY_LOCAL_MACHINE, IDS_REG_KEY_PATH, 
			REG_NONE, REG_OPTION_NON_VOLATILE,KEY_READ | KEY_WRITE, NULL, NULL))
		{
			MessageBox("ע��KEY����ʧ��!", "��ʾ");
			if (reg.m_hKey != NULL)
			{
				RegCloseKey(reg.m_hKey);
			}
			return ;
		}
		
		//����1��������Ӧ��VALIUE����VALIUE������
		if (ERROR_SUCCESS != reg.SetValue(m_StrRegValue, NULL))
		{
			MessageBox("ע��Valve������ʧ��!", "��ʾ");
			
			if (reg.m_hKey != NULL)
			{
				RegCloseKey(reg.m_hKey);
			}
			
			return ;
		}
		
		//ע��ɹ�,�ر�ע���ļ����˳��Ի���
		if (reg.m_hKey != NULL)
		{
			RegCloseKey(reg.m_hKey);
		}
		EndDialog(10);//�����Ի���,10Ϊ�Ի���DoModal���˹�����ֵ
		
	}
	else
	{
		MessageBox("ע�����кŴ���,����������!", "��ʾ");
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
* �������ƣ�ChangeByteOrder
*
* ��  ��:   PCHAR szString         //�ַ�����ָ��
USHORT uscStrSize      //�ַ����鳤��
*
* ����ֵ:   void       
*
* ˵  ��:   �ú���������ǰ�������ַ�˳�����

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
* �������ƣ�HardSerialNT
*
* ��  ��:   ��
*
* ����ֵ:   BOOL       
*
* ˵  ��:   �ú���������ȡϵͳ��Ӳ�����к�

 ************************************************************************/
BOOL CRegisterDlg::HardSerialNT()
{
    char hd[80];
    PIDSECTOR phdinfo;
    char s[41];

    ZeroMemory(&m_struct_vers,sizeof(m_struct_vers));

    //��ʼ��ѯNT/Win2000ϵͳ�µ�Ӳ�����к�
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
        //���IDEӲ������ʶ���ܲ�֧�֣����˳�
        if (!(m_struct_vers.fCapabilities&1))
		{
            MessageBox("IDEӲ������ʶ���ܲ�֧��!", "��ʾ");
            CloseHandle(m_handle);
            return FALSE;
        }

        //ʶ��IDEӲ��
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
            //�����SATA����Ӳ��
			m_strbuf.Format("Drive %d is a ATAPI device, we don't detect it!",(int)(m_bj+1));
			MessageBox(m_strbuf, "��ʾ");
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
        //ȥ�����ʽ�еĿո��
		
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
* �������ƣ�HardSerialNT
*
* ��  ��:   ��
*
* ����ֵ:   BOOL       
*
* ˵  ��:   �ú���������ȡ9Xϵͳ��Ӳ�����к�

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
			AfxMessageBox("IDEӲ������ʶ���ܲ�֧��!");
			CloseHandle(m_handle);
			return FALSE;
		}


		//ʶ��IDEӲ��
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
			//�����SATA����Ӳ��
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
		//ȥ�����ʽ�еĿո��

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
* �������ƣ�GetHardDiskSerial
*
* ��  ��:   ��
*
* ����ֵ:   BOOL       
*
* ˵  ��:   �ú���������ȡϵͳ���ͣ���������Ӧ��Ӳ��ʶ����

 ************************************************************************/
BOOL CRegisterDlg::GetHardDiskSerial()
{

   //�õ�����ϵͳ�汾����
   OSVERSIONINFO VersionInfo;
   ZeroMemory(&VersionInfo,sizeof(VersionInfo));
   VersionInfo.dwOSVersionInfoSize=sizeof(VersionInfo);
   GetVersionEx(&VersionInfo);


   m_StrDiskSerial = _T("");
   //ͨ������ϵͳ�汾���͵Ĳ�ͬ���ò�ͬ�ĺ����õ�Ӳ����Ϣ��Ϣ
   switch (VersionInfo.dwPlatformId)
   {
   case VER_PLATFORM_WIN32s:
	    this->SetWindowText("Win32s is not supported by this programm!");
		m_StrDiskSerial = _T("");
		return FALSE;
	    break;

   case VER_PLATFORM_WIN32_WINDOWS:
	    this->SetWindowText("��ǰϵͳΪ:Windows XP");
	    if (!HardSerial9X())
		{
		   m_StrDiskSerial = _T("");
		   return FALSE;
		}
	    break;

   case VER_PLATFORM_WIN32_NT:
	    //this->SetWindowText("��ǰϵͳΪ:Windows NT");
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
		MessageBox("�û�ע�����ȡʧ��!", "��ʾ");
		return FALSE;
    }
	//������ɿ϶�֪��Ӳ����Ϣ��������ʾ����
    SetDlgItemText(IDC_DISKSERIAL, m_StrDiskSerial);

	//SetWindowText("������ע�����к�!");
	MakeBitmap();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CRegisterDlg::RsaRegValve(CString& m_StrRegValue , CString& m_StrDiskSerial)
{
	// TODO: Add your control notification handler code here
	/* 	KeyGen 1.0 ��
	*	��Ȩ���� (C) 2009 Target
	*	2009.4.1
	*	http://cust.edu.cn
	*	http://china.Target.com
	*	���������Miracl ver 4.82��������⡣*/
	//////////////////////////////////////////////////////////
	
	//P(HEX)=E34436F5F48A227B
	//Q(HEX)=A92FA24467C4E3E3
	//N(HEX)=963251DC5A9C90D9F203A03C363BA411
	//D(HEX)=56157D29A89D77BF2F669A8F0B123CC9
	//E(HEX)=10001
	//Keysize(Bits)=128

	miracl *mip=mirsys(100,0);
	mip->IOBASE=16;	//16����ģʽ
	
	//���岢��ʼ������
	big m=mirvar(0);	//m �����ģ�ע����SN
	big c=mirvar(0);	//c �����ģ��û���Name
	big n=mirvar(0);	//n ģ��
	big e=mirvar(0);	//e ��Կ
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

	
	//���SN�Ƿ�Ϊ16����
	for (i=0,j=0;i<len;i++)
	{
		if(isxdigit(SN[i])==0)
		{
			j=1;
			break;
		}
	}
	
	//��������SNΪ16�����ҳ��Ȳ�Ϊ0
	if (j!=1&&len!=0)
	{
		
		cinstr(m,SN);									//��ʼ������m
		cinstr(n,"963251DC5A9C90D9F203A03C363BA411");	//��ʼ��ģ��n	 
		cinstr(e,"10001");								//��ʼ����Կe
		
		//��m<nʱ
		if(compare(m,n)==-1)
		{
			powmod(m,e,n,c);//����c=m^e mod n
			big_to_bytes(256,c,temp,FALSE);//��cת��������д��temp
			
			//�ͷ��ڴ�
			mirkill(m);
			mirkill(c);
			mirkill(n);
			mirkill(e);
			mirexit();
		}
		else
			j=1;
		
	}	
	//��Name��temp�� m��n�� SN�ĳ��Ƚ��м��
	if(lstrcmp(Name,temp)!=0||j==1||len==0)
		return FALSE;
	else
 		return TRUE;

}

CString CRegisterDlg::GetRasValue(CString &m_StrDiskSerial)
{

	CString strRegValue = _T("");

	int namelen = m_StrDiskSerial.GetLength();	//��ȡӲ�����кŵĳ���

	if (namelen != 0 && namelen<=16)//�����Ϊ����<=128/8��ʼ����SN
	{
		miracl *mip=mirsys(100,0);
		mip->IOBASE=16;	//16����ģʽ
		
		//���岢��ʼ������
		big m=mirvar(0);	//m �����ģ�ע����SN
		big c=mirvar(0);	//c �����ģ��û���Name
		big n=mirvar(0);	//n ģ��
		big d=mirvar(0);	//d ˽Կ
		TCHAR Name[256]={0};
		TCHAR SN[256]={0};
		
		//��ȡӲ�����к�����
		for (int k=0; k<namelen; k++)
		{
			Name[k] = m_StrDiskSerial.GetAt(k);
		}

		bytes_to_big(namelen,Name,c);					//ת����16����
		cinstr(n,"963251DC5A9C90D9F203A03C363BA411");	//��ʼ��ģ��n	 
		cinstr(d,"56157D29A89D77BF2F669A8F0B123CC9");	//��ʼ��˽Կd
		powmod(c,d,n,m);								//����m=c^d mod n
		cotstr(m,SN);									//��m��16���ƴ�д��SN

		//�õ����ܺ��ַ���
		strRegValue.Format("%s",(LPCSTR)SN);
	    	
		//�ͷ��ڴ�
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
* �������ƣ�CheckRegister
*
* ��������
*      
* ���ܣ�����ע��������ж�ע�������Ƿ����Ҫ��
*
* ����ֵ��BOOL (TRUE:ע����������Ҫ��) , ����exit(0):�˳�����
*	   
************************************************************************/
CString CRegisterDlg::CheckRegister()
{
	//ע��KEY����
	HKEY hKey = NULL;

	//ע���Value����
	DWORD lpType = REG_EXPAND_SZ;
	
	BYTE  bData[40] = {0};//װֵ�ĵط�
	
	DWORD cbData = 40;   //ָ��װ�ĵط���С

	CString strValue;
	
	//��ע���ļ�ֵ�ļ�
    if (::RegOpenKey(HKEY_LOCAL_MACHINE, REGISTER_SUBKEY_PATH, &hKey) != ERROR_SUCCESS )
	{
		//AfxMessageBox("ע��KEY��ʧ��!");
		if (NULL != hKey)
		{
			RegCloseKey(hKey);
		}

		return _T("");
		
	}
	//�򿪼�ֵ�ļ������Value
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnCancel();
}

//�����Ľ���ɫ
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

	// TODO:  �ڴ˸��� DC ���κ�����
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
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}

BOOL CRegisterDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
