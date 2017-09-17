#pragma once
#include "stdafx.h"

//文件信息头
typedef struct _FILE_INFOR
{
	char tpye[7];
	WORD width;
	WORD height;
}HaFileInfoHeader;

class CImData :
	public CObject
{
public:
	CImData(void);
public:
	~CImData(void);
	BYTE m_BMIH[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];              // 信息头指针
	LPBYTE m_lpImage;				// 数据块地址

	WORD m_Height;                   // 图像大小
	WORD m_Width;
	double m_low;
	double m_high;
	double m_circular;

	BOOL Read(CFile* pFile);
	BOOL Write(CFile* pFile, LPBYTE lpImage, double low, double high, double circular);
	BOOL Draw(CDC* pDC, CPoint origin, CSize size);
	void SetInfo(void);
public:
	void Empty(void);
public:
	BOOL IsEmpty(void);
};
