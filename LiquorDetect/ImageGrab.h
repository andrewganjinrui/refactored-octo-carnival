#pragma once
// CImageGrab
//Basler camera 相机包含文件 --------------------
#include <pylon/PylonIncludes.h>

using namespace Pylon;
using namespace GenApi;
using namespace GenICam;
using namespace std;
//Basler camera --------------------

class CImageGrab : public CStatic
{
	DECLARE_DYNAMIC(CImageGrab)

public:
	CImageGrab();
	virtual ~CImageGrab();

	//启动相机连接
	BOOL  StartCameraLink();

	//关闭相机连接
	void  CutCameraLink();

	//获取图像尺寸
	inline WORD GetImgWidth()	{ return (WORD)m_Width; }
	inline WORD GetImgHeight()  { return (WORD)m_Height; }
	inline WORD GetImgSize()  { return (WORD)m_ImageSize; }

	//获取相机参数
	 int GetCameraExposureRaw();
	 int GetCameraGainRaw();
	 int GetDeviceSerialNumber();


	//调整相机参数
	void SetGainRaw(int gainRaw);
	void SetExposureRaw(int exposureRaw);

	//获取图像的数据
	LPBYTE GetImgData();

	BOOL m_isLink;

private:
	//图像宽度
	LONG m_Width;
	
	//图像高度
	LONG m_Height;

	//图像增益
	int  m_GainRaw;

	//图像曝光时间
	int  m_ExposureRaw;

	//图像数据
	LPBYTE m_lpImg;

	//Basler camera --------------------
	// device list
	DeviceInfoList_t m_devices;

	// The camera
	IPylonDevice *m_pCamera;

	// The StreamGrabber
	IStreamGrabber *m_pStreamGrabber;

	// Camera Node Map
	GenApi::INodeMap *m_pCameraNodeMap;

	// The TransportLayer
	CTlFactory *m_pTlFactory;

	//图像大小
	size_t m_ImageSize;

	//感兴趣区域大小
	CSize m_AoiSize;

    //数据流缓存
	StreamBufferHandle m_hBuffer;

	uchar8_t *m_pBuffer;

	//Basler camera --------------------

protected:
	DECLARE_MESSAGE_MAP()
};


