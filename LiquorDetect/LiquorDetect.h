// LiquorDetect.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CLiquorDetectApp:
// �йش����ʵ�֣������ LiquorDetect.cpp
//

class CLiquorDetectApp : public CWinApp
{
public:
	CLiquorDetectApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CLiquorDetectApp theApp;