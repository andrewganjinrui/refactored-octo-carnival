#pragma once


// CCurverFit

class CCurverFit : public CStatic
{
	DECLARE_DYNAMIC(CCurverFit)

public:
	CCurverFit();
	virtual ~CCurverFit();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();

	CBitmap* m_pBitmap;
	double m_theat; // 斜率
	double m_b;     // 截距
	BOOL m_bFit;

protected:
	float m_xMax;
	float m_xMin;
	float m_yMax;
	float m_yMin;
	double* m_x;
	double* m_y;
	int m_Count;
	double m_r;

public:
	BOOL Fit(void);
public:
	// 直线拟合
	void MedFit(double x[], double y[], int size, double& a, double& b);
	double calculateR(double x[], double y[], int size); // 计算相关系数
};


