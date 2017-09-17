// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 从 Windows 头中排除极少使用的资料
#endif

// 如果您必须使用下列所指定的平台之前的平台，则修改下面的定义。
// 有关不同平台的相应值的最新信息，请参考 MSDN。
#ifndef WINVER				// 允许使用特定于 Windows XP 或更高版本的功能。
#define WINVER 0x0501		// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif						

#ifndef _WIN32_WINDOWS		// 允许使用特定于 Windows 98 或更高版本的功能。
#define _WIN32_WINDOWS 0x0410 // 将它更改为适合 Windows Me 或更高版本的相应值。
#endif

#ifndef _WIN32_IE			// 允许使用特定于 IE 6.0 或更高版本的功能。
#define _WIN32_IE 0x0600	// 将此值更改为相应的值，以适用于 IE 的其他版本。值。
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC 对 Windows 公共控件的支持
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



//注册---------------------------------------------------------------------------------------------------------
/*******************************大数运算库的头文件和.Lib***********************************************/
extern "C"
{
#include "miracl.h"
#include "mirdef.h"
}
//#pragma comment( lib, "MS32.LIB" ) //重复定义

/**********************************注册表类需要的头文件**************************************************/
#include <atlbase.h>       
#include <WINERROR.H>

/*************************硬盘信息和物理地址信息需要的头文件和.lib文件***********************************/
//添加头文件
//#include <windows.h>
//#include <stdio.h>
//#include <Nb30.h> //这个文件包括了，四个网络方面的构体和函数（NCB，LANA_ENUM， NAME_BUFFER，ADAPTER_STATUS） 

//添加.Lib文件
//#pragma comment( lib, "Netapi32" )//好招

//宏定义
#define DFP_GET_VERSION        0x00074080
#define DFP_SEND_DRIVE_COMMAND 0x0007c084
#define DFP_RECEIVE_DRIVE_DATA 0x0007c088

//得到分区输出参数结构体
typedef struct _GETVERSIONOUTPARAMS {
	BYTE bVersion; // Binary driver version.
	BYTE bRevision; // Binary driver revision.
	BYTE bReserved; // Not used.
	BYTE bIDEDeviceMap; // Bit map of IDE devices.
	DWORD fCapabilities; // Bit mask of driver capabilities.
	DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

//硬盘注册结构体
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

//指令输入参数结构体
typedef struct _SENDCMDINPARAMS {
	DWORD cBufferSize; // Buffer size in bytes
	IDEREGS irDriveRegs; // Structure with drive register values.
	BYTE bDriveNumber; // Physical drive number to send
	// command to (0,1,2,3).
	BYTE bReserved[3]; // Reserved for future expansion.
	DWORD dwReserved[4]; // For future use.
	//BYTE bBuffer[1]; // Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;

//硬盘状态机构体
typedef struct _DRIVERSTATUS {
	BYTE bDriverError; // Error code from driver,
	// or 0 if no error.
	BYTE bIDEStatus; // Contents of IDE Error register.
	// Only valid when bDriverError
	// is SMART_IDE_ERROR.
	BYTE bReserved[2]; // Reserved for future expansion.
	DWORD dwReserved[2]; // Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;

//指令输出结构体
typedef struct _SENDCMDOUTPARAMS {
	DWORD cBufferSize; // Size of bBuffer in bytes
	DRIVERSTATUS DriverStatus; // Driver status structure.
	BYTE bBuffer[512]; // Buffer of arbitrary length
	// in which to store the data read from the drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;

//扇区信息结构体
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
//注册---------------------------------------------------------------------------------------------------------


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


