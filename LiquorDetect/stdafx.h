// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#endif

// ���������ʹ��������ָ����ƽ̨֮ǰ��ƽ̨�����޸�����Ķ��塣
// �йز�ͬƽ̨����Ӧֵ��������Ϣ����ο� MSDN��
#ifndef WINVER				// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define WINVER 0x0501		// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif						

#ifndef _WIN32_WINDOWS		// ����ʹ���ض��� Windows 98 ����߰汾�Ĺ��ܡ�
#define _WIN32_WINDOWS 0x0410 // ��������Ϊ�ʺ� Windows Me ����߰汾����Ӧֵ��
#endif

#ifndef _WIN32_IE			// ����ʹ���ض��� IE 6.0 ����߰汾�Ĺ��ܡ�
#define _WIN32_IE 0x0600	// ����ֵ����Ϊ��Ӧ��ֵ���������� IE �������汾��ֵ��
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

typedef struct RECT_ROI
{
	int index;
	BOOL bDetect;
	CPoint tl;
	CPoint br;
	CPoint itl;
	CPoint ibr;
	int centerx;
	int centery;
	double radius;
	double density;
	//CRoi()
	//{
	//	index = -1;
	//	bCheck = FALSE;
	//	tl = CPoint(-1,-1);
	//	br= CPoint(-1,-1);
	//	itl= CPoint(-1,-1);
	//	ibr= CPoint(-1,-1);
	//	centerx = -1;
	//	centery =-1;
	//	radius = 0;
	//	density = 0;
	//}
}CRoi;



//ע��---------------------------------------------------------------------------------------------------------
/*******************************����������ͷ�ļ���.Lib***********************************************/
extern "C"
{
#include "miracl.h"
#include "mirdef.h"
}
//#pragma comment( lib, "MS32.LIB" ) //�ظ�����

/**********************************ע�������Ҫ��ͷ�ļ�**************************************************/
#include <atlbase.h>       
#include <WINERROR.H>

/*************************Ӳ����Ϣ�������ַ��Ϣ��Ҫ��ͷ�ļ���.lib�ļ�***********************************/
//���ͷ�ļ�
//#include <windows.h>
//#include <stdio.h>
//#include <Nb30.h> //����ļ������ˣ��ĸ����緽��Ĺ���ͺ�����NCB��LANA_ENUM�� NAME_BUFFER��ADAPTER_STATUS�� 

//���.Lib�ļ�
//#pragma comment( lib, "Netapi32" )//����

//�궨��
#define DFP_GET_VERSION        0x00074080
#define DFP_SEND_DRIVE_COMMAND 0x0007c084
#define DFP_RECEIVE_DRIVE_DATA 0x0007c088

//�õ�������������ṹ��
typedef struct _GETVERSIONOUTPARAMS {
	BYTE bVersion; // Binary driver version.
	BYTE bRevision; // Binary driver revision.
	BYTE bReserved; // Not used.
	BYTE bIDEDeviceMap; // Bit map of IDE devices.
	DWORD fCapabilities; // Bit mask of driver capabilities.
	DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

//Ӳ��ע��ṹ��
typedef struct _IDEREGS {
	BYTE bFeaturesReg; // Used for specifying SMART "commands".
	BYTE bSectorCountReg; // IDE sector count register
	BYTE bSectorNumberReg; // IDE sector number register
	BYTE bCylLowReg; // IDE low order cylinder value
	BYTE bCylHighReg; // IDE high order cylinder value
	BYTE bDriveHeadReg; // IDE drive/head register
	BYTE bCommandReg; // Actual IDE command.
	BYTE bReserved; // reserved for future use. Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;

//ָ����������ṹ��
typedef struct _SENDCMDINPARAMS {
	DWORD cBufferSize; // Buffer size in bytes
	IDEREGS irDriveRegs; // Structure with drive register values.
	BYTE bDriveNumber; // Physical drive number to send
	// command to (0,1,2,3).
	BYTE bReserved[3]; // Reserved for future expansion.
	DWORD dwReserved[4]; // For future use.
	//BYTE bBuffer[1]; // Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;

//Ӳ��״̬������
typedef struct _DRIVERSTATUS {
	BYTE bDriverError; // Error code from driver,
	// or 0 if no error.
	BYTE bIDEStatus; // Contents of IDE Error register.
	// Only valid when bDriverError
	// is SMART_IDE_ERROR.
	BYTE bReserved[2]; // Reserved for future expansion.
	DWORD dwReserved[2]; // Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;

//ָ������ṹ��
typedef struct _SENDCMDOUTPARAMS {
	DWORD cBufferSize; // Size of bBuffer in bytes
	DRIVERSTATUS DriverStatus; // Driver status structure.
	BYTE bBuffer[512]; // Buffer of arbitrary length
	// in which to store the data read from the drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;

//������Ϣ�ṹ��
typedef struct _IDSECTOR {
	USHORT wGenConfig;
	USHORT wNumCyls;
	USHORT wReserved;
	USHORT wNumHeads;
	USHORT wBytesPerTrack;
	USHORT wBytesPerSector;
	USHORT wSectorsPerTrack;
	USHORT wVendorUnique[3];
	CHAR sSerialNumber[20];
	USHORT wBufferType;
	USHORT wBufferSize;
	USHORT wECCSize;
	CHAR sFirmwareRev[8];
	CHAR sModelNumber[40];
	USHORT wMoreVendorUnique;
	USHORT wDoubleWordIO;
	USHORT wCapabilities;
	USHORT wReserved1;
	USHORT wPIOTiming;
	USHORT wDMATiming;
	USHORT wBS;
	USHORT wNumCurrentCyls;
	USHORT wNumCurrentHeads;
	USHORT wNumCurrentSectorsPerTrack;
	ULONG ulCurrentSectorCapacity;
	USHORT wMultSectorStuff;
	ULONG ulTotalAddressableSectors;
	USHORT wSingleWordDMA;
	USHORT wMultiWordDMA;
	BYTE bReserved[128];
} IDSECTOR, *PIDSECTOR;
//ע��---------------------------------------------------------------------------------------------------------


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


