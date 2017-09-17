#include "StdAfx.h"
#include "ImData.h"

#define  SAVE_IMAGE_WIDTH 1392
#define  SAVE_IMAGE_HEIGHT 1040
CImData::CImData(void)
{
	m_lpImage = NULL;

	BITMAPINFO* lpBMIH = (BITMAPINFO*)m_BMIH;
	memset(lpBMIH, 0, sizeof(BITMAPINFOHEADER));
	
	lpBMIH->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	for(int i = 0; i < 256; i ++)
	{
		lpBMIH->bmiColors[i].rgbBlue = i;
		lpBMIH->bmiColors[i].rgbGreen = i;
		lpBMIH->bmiColors[i].rgbRed = i;
		lpBMIH->bmiColors[i].rgbReserved = 0;
	}

	lpBMIH->bmiHeader.biBitCount = 8;
	lpBMIH->bmiHeader.biClrUsed = 0;
	lpBMIH->bmiHeader.biCompression = BI_RGB;
	lpBMIH->bmiHeader.biPlanes = 1;
}

CImData::~CImData(void)
{
	Empty();
}

BOOL CImData::Draw(CDC* pDC, CPoint origin, CSize size)
{
	// 尚未有数据，返回FALSE
	if(m_lpImage == NULL) return FALSE;

	// 设置显示模式
	pDC->SetStretchBltMode(COLORONCOLOR);

	SetInfo();
	// 在设备的origin位置上画出大小为size的图象
	::StretchDIBits(pDC->GetSafeHdc(), origin.x, origin.y, size.cx,size.cy,
		0, 0, m_Width, m_Height,
		m_lpImage, (LPBITMAPINFO)m_BMIH, DIB_RGB_COLORS, SRCCOPY);

	// 返回
	return TRUE;
}
void CImData::SetInfo(void)
{
	LPBITMAPINFOHEADER lpBMIH = (LPBITMAPINFOHEADER)m_BMIH;
	lpBMIH->biWidth = m_Width;
	lpBMIH->biHeight = m_Height;
	lpBMIH->biSizeImage = m_Width*m_Height;
}

BOOL CImData::Read(CFile* pFile)
{
	// 释放已经分配的内存
	Empty();
	HaFileInfoHeader haFileInfo;
	UINT nBytesRead;
	nBytesRead = pFile->Read(&haFileInfo, sizeof(HaFileInfoHeader));
	if(nBytesRead != sizeof(HaFileInfoHeader)) 
	{
		throw new CFileException;
		return FALSE;
	}

	if (0 !=memcmp(haFileInfo.tpye, _T("TARGET"), 6))
	{
		AfxMessageBox(_T("未知文件，打开失败！"));
		return FALSE;
	}

	m_Height = haFileInfo.height;
	m_Width  = haFileInfo.width;

	double para[3];
	pFile->Read(para, 3 * sizeof(double));
	m_low      = para[0];
	m_high     = para[1];
	m_circular = para[2];

	m_lpImage = new BYTE[m_Height*m_Width];
    
	nBytesRead = pFile->Read(m_lpImage, m_Height*m_Width);

	if(nBytesRead != m_Height*m_Width) 
	{
		throw new CFileException;
		return FALSE;
	}

	return TRUE;
}

BOOL CImData::Write(CFile* pFile, LPBYTE lpImage, double low, double high, double circular)
{
//	CLiquorDetectDlg* dlg = (CLiquorDetectDlg*)AfxGetMainWnd();
    HaFileInfoHeader haFileInfo;
	sprintf(haFileInfo.tpye, _T("%s"), _T("TARGET"));

	haFileInfo.height = SAVE_IMAGE_HEIGHT;
	haFileInfo.width  = SAVE_IMAGE_WIDTH;

	pFile->Write(&haFileInfo, sizeof(HaFileInfoHeader));
	pFile->Write(&low, sizeof(double));
	pFile->Write(&high, sizeof(double));
	pFile->Write(&circular, sizeof(double));
	pFile->Write(lpImage, SAVE_IMAGE_WIDTH * SAVE_IMAGE_HEIGHT);

	return TRUE;
}
void CImData::Empty(void)
{
	if (m_lpImage)
	{
		delete m_lpImage;
		m_lpImage = NULL;
	}
}

BOOL CImData::IsEmpty(void)
{
	if (m_lpImage)
		return FALSE;
	return TRUE;
}
