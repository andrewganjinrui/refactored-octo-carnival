#pragma once
#include "stdafx.h"

//�ļ���Ϣͷ
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
	BYTE m_BMIH[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];              // ��Ϣͷָ��
	LPBYTE m_lpImage;				// ���ݿ��ַ

	WORD m_Height;                   // ͼ���С
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
