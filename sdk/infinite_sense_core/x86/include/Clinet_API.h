/*--------------------------------------------------------------------
                                                                                      
                                                                                      
                        COPYRIGHT (C) 2008,  Corporation
                        ALL RIGHTS RESERVED
                                                                                      
 This source code has been made available to you by  on an
 AS-IS basis. Anyone receiving this source code is licensed under 
 copyrights to use it in any way he or she deems fit, including copying it,
 modifying it, compiling it, and redistributing it either with or without
 modifications. Any person who transfers this source code or any derivative
 work must include the  copyright notice and this paragraph in
 the transferred software.
 1.0    Initial net software                       CCL   2009.02.10
                                                                                      
-----------------------------------------------------------------------*/
#ifndef _CLINET_API_H_
#define _CLINET_API_H_

//#define TIRNETCLIENT_EXPORTS

//#ifdef TIRNETCLIENT_EXPORTS
//#define TIRNETCLIENT_API extern "C" __declspec(dllexport)
#define TIRNETCLIENT_API
//#else
//#define TIRNETCLIENT_API __declspec(dllimport)
//#endif

//#define TIRNETCLIENT_API __declspec(dllexport)

#ifdef SDK_LIB
#include "NetSocket.h"
#else
#include "netdefine.h"
#endif

#ifdef TRACE
#undef TRACE
#define TRACE printf
#endif
// 定义宏来替代 printf  
//#define TRACE(fmt, ...) do {         \
////    printf(fmt, ##__VA_ARGS__);     \
//    fflush(stdout);                 \
//} while (0)  

typedef enum { CAMERA_640 = 0, CAMERA_336 = 1, CAMERA_324 = 2, CAMERA_384 = 3, CAMERA_640_480 = 4, CAMERA_384_2 = 5, CAMERA_640_2 = 6, CAMERA_1024_768 = 7, CAMERA_1280_1024 = 8, CAMERA_320_256 = 9, CAMERA_ADJUSTABLE = 10}CamType;
#define SERIALNO_LEN 		32
#define IPSTRING_LEN 		30

#define CAMERA_ACTION_DO_FFC			0x01   
#define CAMERA_ACTION_STOP_AUTOFFC		0x02  
#define CAMERA_ACTION_START_AUTOFFC		0x03   
#define FRAME_HEAD_FLAG 0x5A5AA5A5  





typedef struct {
	SINT8 sSerialNumber[SERIALNO_LEN]; 
	UINT8 byAlarmInPortNum;		    
	UINT8 byAlarmOutPortNum;		
	UINT8 camType;				//相机类型
	UINT8 byCamResolution;		//分辨率， CAMERA_640 = 0 , CAMERA_336=1 , CAMERA_324 = 2 , CAMERA_384 = 3, CAMERA_640_480=4, CAMERA_384_2 =5, CAMERA_640_2 =6
	UINT8 frameRate;			//帧率
	UINT8 byStartChan;	
	UINT8 reserver1;				//为了8个字节对齐
	UINT8 reserver2;				//为了8个字节对齐
	UINT16 firmwareVersion;         //底层软件的版本号
	UINT16 resolutionWidth;         //图像分辨率的宽
	UINT16 resolutionHeight;        //图像分辨率的高
	UINT8  bytesPerPixel;			//一个像素多少个字节
	UINT8 userId;              
	UINT8 login;
	SINT8 sIPaddr[IPSTRING_LEN];  
	UINT16 uPort;                  
	UINT32 frameLen;              
	SOCKET playSocket;           

}NET_SERVER_DEVICEINFO, *LPNET_SERVER_DEVICEINFO;

TIRNETCLIENT_API BOOL   NET_VIDEO_Init();
TIRNETCLIENT_API INT32  NET_VIDEO_Login(char *sDVRIP,UINT32 wDVRPort,LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_GetDevinfo(char *sDVRIP, UINT32 wDVRPort, LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_Logout(LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_RealPlay(char *sServerIP, UINT32 wServerPort, LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_StopRealPlay(LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_RevData(char *pDataBuf, UINT32 buffersize, int *cnt, LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_RebootDevice(LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_SetIPConfig(LPNET_SERVER_DEVICEINFO lpDeviceInfo, void*  lpInBuffer);
TIRNETCLIENT_API INT32  NET_VIDEO_SetFrameRate(LPNET_SERVER_DEVICEINFO lpDeviceInfo, UINT32 framRate);
TIRNETCLIENT_API INT32  NET_VIDEO_StartStoreRAW(LPNET_SERVER_DEVICEINFO lpDeviceInfo, UINT32 framRate);
TIRNETCLIENT_API INT32  NET_VIDEO_StopStoreRAW(LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_CAMERA_ACTION(LPNET_SERVER_DEVICEINFO lpDeviceInfo, UINT32 commandflag);
TIRNETCLIENT_API INT32  NET_VIDEO_AutoFocus(LPNET_SERVER_DEVICEINFO lpDeviceInfo, int Command);
TIRNETCLIENT_API INT32  NET_VIDEO_AdjustZoom(LPNET_SERVER_DEVICEINFO lpDeviceInfo, int Direction, int zoomValue);
TIRNETCLIENT_API INT32  NET_VIDEO_StopZoomFocus(LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_GetZoomInfo(LPNET_SERVER_DEVICEINFO lpDeviceInfo, int* Value, int Num);
TIRNETCLIENT_API INT32  NET_VIDEO_SendSensorCommand(LPNET_SERVER_DEVICEINFO lpDeviceInfo,  char *cmd, int cmd_len,  char *respond, int *resp_len);
TIRNETCLIENT_API INT32  NET_VIDEO_SetCirclePosition(LPNET_SERVER_DEVICEINFO lpDeviceInfo, unsigned short position);
TIRNETCLIENT_API INT32  NET_VIDEO_SendCircleCommand(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* cmd, int cmd_len, char* respond, int* resp_len);
TIRNETCLIENT_API INT32  NET_VIDEO_SendLENCommand(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* cmd, int cmd_len, int waitting_times_mS, char* respond, int* resp_len);
TIRNETCLIENT_API INT32  NET_VIDEO_SetGPIO(LPNET_SERVER_DEVICEINFO lpDeviceInfo, int port, int value);
TIRNETCLIENT_API INT32  NET_VIDEO_SettingTime(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* TimeString);
TIRNETCLIENT_API INT32  NET_VIDEO_GetRunningTime(LPNET_SERVER_DEVICEINFO lpDeviceInfo);
TIRNETCLIENT_API INT32  NET_VIDEO_SET_TRIGGER(LPNET_SERVER_DEVICEINFO lpDeviceInfo, INT32 commandflag);
TIRNETCLIENT_API INT32  NET_VIDEO_SendSensorCommand_WithoutReply(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* cmd, int cmd_len);
TIRNETCLIENT_API INT32  NET_VIDEO_SendUartCommand(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* cmd, int cmd_len, char* respond, int* resp_len);
TIRNETCLIENT_API INT32  NET_VIDEO_GetEnvironmentTemp(LPNET_SERVER_DEVICEINFO lpDeviceInfo, short* temper);
TIRNETCLIENT_API INT32  NET_VIDEO_DownLoadFile(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* file_name, int file_len, char* file_data);
TIRNETCLIENT_API INT32  NET_VIDEO_GetFile(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* file_name, int* file_len, char* file_data);
#endif
