// ImageGrab.cpp : 实现文件
//

#include "stdafx.h"
#include "ImageGrab.h"


// CImageGrab

IMPLEMENT_DYNAMIC(CImageGrab, CStatic)

CImageGrab::CImageGrab()
{
    //指针初始化
	m_pCamera = NULL;
	m_pStreamGrabber = NULL;
	m_pCameraNodeMap = NULL;
	m_pTlFactory = NULL;
	m_lpImg = NULL;
	m_pBuffer = NULL;

    //相机参数初始化
	m_GainRaw = 350;
	m_ExposureRaw = 2000;

	m_isLink = FALSE;

}

CImageGrab::~CImageGrab()
{
}

BOOL CImageGrab::StartCameraLink()
{
	m_pTlFactory = &CTlFactory::GetInstance();

	m_devices.clear();
	if (0 != m_pTlFactory->EnumerateDevices(m_devices))
	{
		// The camera object is used to set and get all avaliable
		// camera features.

		m_pCamera = m_pTlFactory->CreateDevice ( m_devices[0] );


		// Get the first stream grabber object of the selected camera
		m_pStreamGrabber = m_pCamera->GetStreamGrabber (0);

		// Open the camera
		m_pCamera->Open();

		// Open the stream grabber
		m_pStreamGrabber->Open();

		// get Node Map for device configuration
		m_pCameraNodeMap = m_pCamera->GetNodeMap();

		// set colour mode
		CEnumerationPtr ptrPixelFormat = m_pCameraNodeMap->GetNode("PixelFormat");
		ptrPixelFormat->FromString("Mono8");

		// Set AOI
		CIntegerPtr ptrOffsetX = m_pCameraNodeMap->GetNode ("OffsetX");
		CIntegerPtr ptrOffsetY = m_pCameraNodeMap->GetNode ("OffsetY");
		CIntegerPtr ptrAoiWidth = m_pCameraNodeMap->GetNode ("Width");
		CIntegerPtr ptrAoiHeight = m_pCameraNodeMap->GetNode ("Height");

		m_AoiSize.cx = (LONG) ptrAoiWidth->GetMax ();
		m_AoiSize.cy = (LONG) ptrAoiHeight->GetMax ();

		// Beware : Windows Bitmaps have to have a DWORD alligned width :-(
		m_AoiSize.cx = m_AoiSize.cx & ~3;

		m_Width = m_AoiSize.cx;
		m_Height = m_AoiSize.cy;

		ptrOffsetX->SetValue (0);
		ptrOffsetY->SetValue (0);
		ptrAoiWidth->SetValue (m_AoiSize.cx);
		ptrAoiHeight->SetValue (m_AoiSize.cy);

		// Set the camera to continuous shot mode
		CEnumerationPtr ptrTriggerSelector = m_pCameraNodeMap->GetNode ("TriggerSelector");
		ptrTriggerSelector->FromString ("AcquisitionStart");

		CEnumerationPtr ptrTriggerMode = m_pCameraNodeMap->GetNode ("TriggerMode");
		ptrTriggerMode->FromString ("Off");

		CEnumerationPtr ptrAcquisitionMode = m_pCameraNodeMap->GetNode ("AcquisitionMode");
		ptrAcquisitionMode->FromString ("Continuous");

		CEnumerationPtr ptrExposureMode = m_pCameraNodeMap->GetNode("ExposureMode");
		ptrExposureMode->FromString("Timed");

		//ExposureTimeRaw
		CIntegerPtr ptrExposure = m_pCameraNodeMap->GetNode("ExposureTimeRaw");
		ptrExposure->SetValue(m_ExposureRaw);

		//GainRaw
		CIntegerPtr ptrGainRaw = m_pCameraNodeMap->GetNode("GainRaw");
		ptrGainRaw->SetValue(m_GainRaw);


		// Get the image buffer size
		CIntegerPtr ptrPayloadSize = m_pCameraNodeMap->GetNode("PayloadSize");
		m_ImageSize = (size_t) (ptrPayloadSize->GetValue ());

		// get Node Map for streamgrabber configuration
		GenApi::INodeMap *pGrabberNodeMap = m_pStreamGrabber->GetNodeMap();

		// We won't use image buffers greater than ImageSize
		CIntegerPtr ptrMaxBufferSize = pGrabberNodeMap->GetNode("MaxBufferSize");
		ptrMaxBufferSize->SetValue (m_ImageSize);

		// We won't queue more than c_nBuffers image buffer at a time
		CIntegerPtr ptrMaxNumBuffer = pGrabberNodeMap->GetNode("MaxNumBuffer");
		ptrMaxNumBuffer->SetValue(1);

		// Allocate all resources for grabbing. Critical parameters like image
		// size now must not be changed until FinishGrab() is called.
		m_pStreamGrabber->PrepareGrab();

		m_lpImg = new BYTE[m_ImageSize];

		m_pBuffer = new uchar8_t[ m_ImageSize ];

		//m_hBuffer =	m_pStreamGrabber->RegisterBuffer( m_pBuffer, m_ImageSize );
        m_isLink = TRUE;
		return TRUE;
	}
	else 
	{
		m_isLink = FALSE;
		return FALSE;
	}

}

void  CImageGrab::CutCameraLink()
{

	if(m_lpImg != NULL)
	{
		delete [] m_lpImg;
		m_lpImg = NULL;
	}

	if (m_pBuffer != NULL)
	{
		delete [] m_pBuffer;
		m_pBuffer = NULL;
	}

    // Free all resources used for grabbing
	m_pStreamGrabber->CancelGrab();
	
    // Free all resources used for grabbing
	m_pStreamGrabber->DeregisterBuffer( m_hBuffer );
	
	m_pStreamGrabber->FinishGrab();

	// Close stream grabber
	m_pStreamGrabber->Close();

	// Close camera
	m_pCamera->Close();

}

void  CImageGrab::SetGainRaw(int gainRaw)
{
  m_GainRaw = gainRaw;

  //调整增益
  CIntegerPtr ptrGainRaw = m_pCameraNodeMap->GetNode ("GainRaw");
  ptrGainRaw->SetValue(m_GainRaw);

}

void  CImageGrab::SetExposureRaw(int exposureRaw)
{
  m_ExposureRaw = exposureRaw;

  //调整积分时间
  CIntegerPtr ptrExposure = m_pCameraNodeMap->GetNode ("ExposureTimeRaw");
  ptrExposure->SetValue(m_ExposureRaw);

}

//获取图像的数据
LPBYTE CImageGrab::GetImgData()
{
	// Buffers used for grabbing must be registered at the stream grabber.
	m_hBuffer =	m_pStreamGrabber->RegisterBuffer( m_pBuffer, m_ImageSize );

	// Put buffer into the grab queue for grabbing
	m_pStreamGrabber->QueueBuffer( m_hBuffer, NULL );

	CCommandPtr ptrAcquisitionStart = m_pCameraNodeMap->GetNode ("AcquisitionStart");
	ptrAcquisitionStart->Execute ();

	GrabResult result;	

	if ( m_pStreamGrabber->GetWaitObject().Wait( 1000 ) )
	{	
		m_pStreamGrabber->RetrieveResult( result );

		if ( result.Succeeded() )
		{
			const uint8_t *pImageBuffer = (uint8_t *) result.Buffer();
			memcpy(m_lpImg, pImageBuffer, m_ImageSize);
		
		} 
	}
	else 
	{
//		MessageBox(_T("相机捕获图像超时！"));
		for ( GrabResult r; m_pStreamGrabber->RetrieveResult( r ););
		return NULL;
	}

	CCommandPtr ptrAcquisitionStop = m_pCameraNodeMap->GetNode ("AcquisitionStop");
	ptrAcquisitionStop->Execute();

	return m_lpImg;

}

int CImageGrab::GetCameraExposureRaw()
{
	CIntegerPtr ptrExpRaw = m_pCameraNodeMap->GetNode ("ExposureTimeRaw");
	return (int)ptrExpRaw->GetValue();
}
int CImageGrab::GetDeviceSerialNumber()
{
	CIntegerPtr ptrExpRaw = m_pCameraNodeMap->GetNode ("DeviceSerialNumber");
	return (int)ptrExpRaw->GetValue();
}
int CImageGrab::GetCameraGainRaw()
{
	CIntegerPtr ptrGainRaw = m_pCameraNodeMap->GetNode ("GainRaw");
	return (int)ptrGainRaw->GetValue();

}
BEGIN_MESSAGE_MAP(CImageGrab, CStatic)
END_MESSAGE_MAP()

// CImageGrab 消息处理程序

