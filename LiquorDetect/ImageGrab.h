#pragma once
// CImageGrab
//Basler camera ��������ļ� --------------------
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

	//�����������
	BOOL  StartCameraLink();

	//�ر��������
	void  CutCameraLink();

	//��ȡͼ��ߴ�
	inline WORD GetImgWidth()	{ return (WORD)m_Width; }
	inline WORD GetImgHeight()  { return (WORD)m_Height; }
	inline WORD GetImgSize()  { return (WORD)m_ImageSize; }

	//��ȡ�������
	 int GetCameraExposureRaw();
	 int GetCameraGainRaw();
	 int GetDeviceSerialNumber();


	//�����������
	void SetGainRaw(int gainRaw);
	void SetExposureRaw(int exposureRaw);

	//��ȡͼ�������
	LPBYTE GetImgData();

	BOOL m_isLink;

private:
	//ͼ����
	LONG m_Width;
	
	//ͼ��߶�
	LONG m_Height;

	//ͼ������
	int  m_GainRaw;

	//ͼ���ع�ʱ��
	int  m_ExposureRaw;

	//ͼ������
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

	//ͼ���С
	size_t m_ImageSize;

	//����Ȥ�����С
	CSize m_AoiSize;

    //����������
	StreamBufferHandle m_hBuffer;

	uchar8_t *m_pBuffer;

	//Basler camera --------------------

protected:
	DECLARE_MESSAGE_MAP()
};


