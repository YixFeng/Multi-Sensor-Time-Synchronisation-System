/*----------------------------------------------------------------------------                                                                                      
                                                                                      
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
//#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include "Clinet_API.h"
//#include "TirNetInterface.h"
//#include "netserver.h"
#include "NetSocket.h"
#include <iostream>
#include <unistd.h>
//#include <WinSock2.h>

using namespace std;

#define CAMERA_IS_COOL 0  //制冷相机1， 非制冷相机0；

#define NET_SERVER_LOGIN			0x84
#define NET_SERVER_GET_DEVINFO		0x85
#define NET_SERVER_GET_REALPLAY		0xab
#define NET_SERVER_GET_STOPPLAY		0xad
#define NET_SERVER_GET_LOGOUT		0xaa

#define	NET_SERVER_SETGPIO_V20	    0xB8
#define NET_SERVER_REBOOT_DEV		0xbe
#define NET_SERVER_SET_IP			0xbd
#define	NET_SERVER_SENDCAMERACMD	0xBC 
#define NET_SERVER_SENDCAMERACMD_WITHOUT_REPLY   0x10BC   //给机芯发命令，不等机芯返回数据，直接应答上位机已经收到命令了。

#define NET_SERVER_SENDUARTCMD      0xC1   //给串口发送命令，数据通过串口转发出去， 有些设备没有串口
#define NET_SERVER_SETFRAMERATE     0xC2   // 设置帧率，根据数值设置发送多少帧。

#define NET_SERVER_CAMERA_ACTION	0xCF   // 控制机芯做一次动作，特定机芯有这个功能
#define NET_SERVER_STARTSTORING     0xC3   // 启动保存图像 到SSD
#define NET_SERVER_STOPSTORING      0xC4   // 停止保存图像
#define NET_SERVER_SETTINGTIME      0xC5   // 设置前端时间
#define NET_SERVER_GETRUNNINGTIME   0xC6   // 获取机器这次开机运行的时长
#define NET_SERVER_SETTRIGGERCMD    0x02B0  //动态控制是否使用外触发功能。 //HiNet
#define NET_SERVER_GETDATAFILE		0xB5
#define DVR_SERVER_DownLoadDATAFILE		0x20

#ifdef CAMERA_IS_COOL
#define	NET_SERVER_SENDCIRCLECMD	0xE0    //给虑片轮发命令
#define NET_SERVER_AUTOFOCUS        0x10E0    //自动聚焦
#define NET_SERVER_ADJUSTZOOM       0x10E1   //调焦
#define NET_SERVER_STOPZOOMFOCUS    0x10E2   //停止调焦，对焦
#define NET_SERVER_GETZOOMINFO      0x10E3    //获取zoom的AD值跟范围
#define	NET_SERVER_GETENVIRONMENTTMP	    0x10BD    //获取环境温度
#define	NET_SERVER_SENDCOMMAND_TOLEN	    0x10BE    //发命令给镜头
#else

#define NET_SERVER_AUTOFOCUS        0xE0   //自动聚焦
#define NET_SERVER_ADJUSTZOOM       0xE1   //调焦
#define NET_SERVER_STOPZOOMFOCUS    0xE2   //停止调焦，对焦
#define NET_SERVER_GETZOOMINFO     0xE3    //获取zoom的AD值跟范围
#endif
#define CONFIG_ACTIVE_FLAG          0x5a5aa5a5  
#define CAMERA_ACTION_FLAG          0x5500aa00  

unsigned int gFrameHeadLen = 8;
char recBuffer[1280*1024*2];

void init_socket_dll(void)
{
	static bool init_Socket_flag = false;

	if(init_Socket_flag == false)
	{
#ifdef WIN32
		 WSADATA wsadata;//WSADATA结构体中主要包含了系统所支持的Winsock版本信息
		if( WSAStartup( MAKEWORD(2,2),&wsadata )!=0 )//初始化Winsock 2.2
		{
			 TRACE("WSAStartup�޷���ʼ����");
			 TRACE("WSAStartup�޷���ʼ����");
		}
#endif
		
		init_Socket_flag = true;
	}
}
BOOL  NET_VIDEO_Init()
{
	init_socket_dll();

	return TRUE;
}

INT32  NET_VIDEO_Login(char *sDVRIP, UINT32 wDVRPort, LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{
	unsigned int userId = 0, usedindex = 0;
	char *sUserName = "admin";
	char *sPassword = "123456";
	SOCKET Socket;
	unsigned char cmdbuffer[84];
	unsigned char cmdreceive[1024];

	int cmdlen = 84;
	memset(cmdbuffer, 0, 84);

	//the command length
	((unsigned int *)cmdbuffer)[0] = 84;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_LOGIN;

	((unsigned short *)cmdbuffer)[40] = 0x84;

	memcpy(cmdbuffer + 16, sUserName, 32);
	memcpy(cmdbuffer + 48, sPassword, 16);
	memcpy(cmdbuffer + 64, sDVRIP, 16);

	
	if (!ConnectSever(&Socket, sDVRIP, wDVRPort))
	{
		TRACE("Login Connect %s faile\n", sDVRIP);
		return -1;
	}

	TRACE("Login Connect %s OK\n", sDVRIP);

	if (SendPactket(&Socket,(void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	TRACE("Login send %d bytes\n", cmdlen);

	cmdlen = 76;
	if (ReceivePacket(Socket, &cmdreceive, cmdlen) != 76)
	{
		TRACE("Login rece  faile\n");

		return -1;
	}
	TRACE("Login rece  %d bytes\n", cmdlen);


	lpDeviceInfo->uPort = wDVRPort;
	strcpy(lpDeviceInfo->sIPaddr, sDVRIP);

	userId = ((unsigned int *)cmdreceive)[2];
	lpDeviceInfo->userId = userId;

	lpDeviceInfo->login = TRUE;

	NET_VIDEO_GetDevinfo(sDVRIP, wDVRPort, lpDeviceInfo);

	return 0;
}




//Get device information: 84 bytes,command type is 0x85.
// --------------------------------------------------------
// | 4 length | 4 command type | 8 userID | 68  reserverd |
// --------------------------------------------------------
/***************************************************************
Description:
Get device information.

Parameters:
sDVRIP: the IP address.
wDVRPort: the connect port.
userId: user id.
lpDeviceInfo: device information.

Returns:
TRUE: login success.
****************************************************************/
INT32 NET_VIDEO_GetDevinfo(char *sDVRIP, UINT32 wDVRPort, LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{

	unsigned char cmdbuffer[84];
	unsigned char cmdreceive[1024];
	int cmdlen = 84;
	int usedindex = 0;
	int GetBottomUserId = lpDeviceInfo->userId;
	//GetBottomUserId = cServer.GetBottomUserId(usedindex);
	//if(GetBottomUserId==-1)
	//	return FALSE;

	memset(cmdbuffer, 0, 84);
	//the command length
	((unsigned int *)cmdbuffer)[0] = 84;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_GET_DEVINFO;

	((unsigned int *)cmdbuffer)[2] = GetBottomUserId;

	SOCKET Socket;


	if (!ConnectSever(&Socket, sDVRIP, wDVRPort))
	{
		TRACE("GetDevinfo %s faile\n", sDVRIP);
		return -1;
	}

	TRACE("GetDevinfo %s OK\n", sDVRIP);

	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	TRACE("GetDevinfo %d bytes\n", cmdlen);

	cmdlen = 76;
	if (ReceivePacket(Socket, &cmdreceive, cmdlen) != 76)
	{
		TRACE("Login rece  faile\n");

		return -1;
	}



	unsigned int cpylen = 38;//(unsigned int)((unsigned int)&lpDeviceInfo->userId - (unsigned int)lpDeviceInfo);

	if (((unsigned int *)cmdreceive)[1] == 0x06)
	{
		memcpy(lpDeviceInfo, cmdreceive + 20, cpylen);
	}

	return 0;
}



//图像预览
INT32  NET_VIDEO_RealPlay(char *sServerIP, UINT32 wServerPort, LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{
	unsigned char cmdbuffer[120];
	int cmdlen = 36;
	unsigned char cmdreceive[1024];
	int usedindex = 0;
	int GetBottomUserId = lpDeviceInfo->userId;

	SOCKET Socket;
	memset(cmdbuffer, 0, 36);
	//the command length
	((unsigned int *)cmdbuffer)[0] = 36;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_GET_REALPLAY;

	((unsigned int *)cmdbuffer)[2] = GetBottomUserId;
	

	if (!ConnectSever(&Socket, sServerIP, wServerPort))
	{
		return -1;
	}


	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen )
	{
		return -2;
	}
	cmdlen = 20;

	if (ReceivePacket(Socket, &cmdreceive, cmdlen) != 20)
	{
		return -3;
	}

	TRACE("%d,%d\n", ((unsigned int *)cmdreceive)[0], ((unsigned int *)cmdreceive)[3]);
	if (((unsigned int *)cmdreceive)[0] == 0x1A)
	{
		if (((unsigned int *)cmdreceive)[3] != 0)
		{
			return -4;
		}
	}

	lpDeviceInfo->playSocket = Socket;

	UINT32 m_nDataLen;
	CamType cam_type = (CamType)lpDeviceInfo->byCamResolution;
	if (cam_type == CAMERA_640)
	{
		m_nDataLen = 640 * 512 * 2;
	}
	else if (cam_type == CAMERA_336)
	{
		m_nDataLen = 336 * 256 * 2;
	}
	else if (cam_type == CAMERA_324)
	{
		m_nDataLen = 324 * 256 * 2;
	}
	else if (cam_type == CAMERA_384)
	{
		m_nDataLen = 384 * 288 * 2;
	}
	else if (cam_type == CAMERA_640_480)
	{
		m_nDataLen = 640 * 480 * 2;
	}
	else if (cam_type == CAMERA_384_2)
	{
		m_nDataLen = 384 * 288 * 2 * 2;
	}
	else if (cam_type == CAMERA_640_2)
	{
		m_nDataLen = 640 * 512 * 2 * 2;
	}
	else if (cam_type == CAMERA_1024_768)
	{
		m_nDataLen = 1024 * 768 * 2;
	}
	else if (cam_type == CAMERA_1280_1024)
	{
		m_nDataLen = 1280 * 1024 * 2;
	}
	else if (cam_type == CAMERA_ADJUSTABLE)
	{
		m_nDataLen = lpDeviceInfo->resolutionWidth * lpDeviceInfo->resolutionHeight * lpDeviceInfo->bytesPerPixel;
	}
	else
	{
		m_nDataLen = 0;
	}
	lpDeviceInfo->frameLen = m_nDataLen;

	if (lpDeviceInfo->byStartChan == 0x5A)
		gFrameHeadLen = 8;
	else if(lpDeviceInfo->byStartChan == 0x5B)
		gFrameHeadLen = 24;
	else if (lpDeviceInfo->byStartChan == 0x5C)
		gFrameHeadLen = 12;
	else if (lpDeviceInfo->byStartChan == 0x5D)
		gFrameHeadLen = 12;
	else if (lpDeviceInfo->byStartChan == 0x5E)
		gFrameHeadLen = 40;
	TRACE("usedindex = %d,lUserID = %d.\n", usedindex, 0);

	return 0;
}


//cnt ： 收到的字节长度,不包括前面4个int型， type，收到字符的类型
//pDataBuf   buffer指针
//int buffersize  buffer大小
// int *cnt 获去数据大小
//int *type  数据类型
INT32  NET_VIDEO_RevData(char *pDataBuf, UINT32 buffersize, int *cnt, LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{

	int usedindex = 0;
	unsigned int *ptr;
	int nRef = 0;
	int compressFrameLen;
	int uncompressLen;
	if (pDataBuf == NULL)
	{
		return FALSE;
	}

		SOCKET playSocket = lpDeviceInfo->playSocket;

		UINT32 m_nDataLen = lpDeviceInfo->frameLen;
	if (lpDeviceInfo->byStartChan == 0x20)
	{
		if (m_nDataLen>0)
		{
			if (m_nDataLen > buffersize)  
			{
				TRACE("Buffer 不够大 ### %d， %d\n\r", m_nDataLen, buffersize);
				return -4;
			}
			else
			{
				nRef = ReceivePacket(playSocket, pDataBuf, 8);
				ptr = (unsigned int *)pDataBuf;
				if (ptr[0] == 0x7E5AA57E)
				{
					m_nDataLen = ptr[1] -8;
					nRef = ReceivePacket(playSocket, pDataBuf+8, m_nDataLen);
					nRef += 8;
				}
				//	TRACE("Rece data %d,  need %d,  %d, %d,\n\r",nRef,  m_nDataLen, tmp1, tmp2);
			}
			if (nRef == -1)
			{
				return nRef;
			}
			*cnt = nRef;
			return 0;
		}
	}
	if (lpDeviceInfo->byStartChan == 0x5A || lpDeviceInfo->byStartChan == 0x5B || lpDeviceInfo->byStartChan == 0x5C || lpDeviceInfo->byStartChan == 0x5E)
	{
		if (m_nDataLen > 0)
		{
			if (m_nDataLen > buffersize)  //传进来的buffer 不够大
			{
				TRACE("Buffer 不够大 ### %d， %d\n\r", m_nDataLen, buffersize);
				return -4;
			}
			else
			{
				nRef = ReceivePacket(playSocket, pDataBuf, gFrameHeadLen);
				ptr = (unsigned int*)pDataBuf;
				if ((ptr[0] == m_nDataLen+ gFrameHeadLen) &&(ptr[1] == FRAME_HEAD_FLAG ))    
				{
					TRACE("Secnd:%d, mS:%d\n", ptr[2], ptr[3]);
				nRef = ReceivePacket(playSocket, pDataBuf, m_nDataLen);
				}
				else   //帧头丢失
				{
					
					INT32 offsetLen = -1;

					TRACE("Paser the frame head.\n\r");

					nRef = ReceivePacket(playSocket, pDataBuf, m_nDataLen);
					unsigned char* ucPtr = (unsigned char*)pDataBuf;
					for (int i = 0; i <= (nRef - 4); i++)
					{
						if (ucPtr[i] == 0xA5 && ucPtr[i + 1] == 0xA5 && ucPtr[i + 2] == 0x5A && ucPtr[i + 3] == 0x5A)
						{
							offsetLen = nRef - (gFrameHeadLen-4)  - i;  //应该扣掉长度这4个字节
							if (offsetLen > 0)
							{
								memcpy(&pDataBuf[0], &ucPtr[i + (gFrameHeadLen - 4)], offsetLen);
							}
							TRACE("Found the frame head.offsetLen =%d\n\r", offsetLen);
							break;
						}
					}
					if (offsetLen >= 0)
					{
						nRef = ReceivePacket(playSocket, &pDataBuf[offsetLen], m_nDataLen - offsetLen);
						nRef += offsetLen;
					}
					else
					{
						nRef = -1;
						TRACE("Can't find the frame head.\n\r");
					}

				}
				//	TRACE("Rece data %d,  need %d,  %d, %d,\n\r",nRef,  m_nDataLen, tmp1, tmp2);
			}
			if (nRef == -1)
			{
				return nRef;
			}
			*cnt = nRef;

			return 0;
		}
	}
	else if (lpDeviceInfo->byStartChan == 0x5D)  //帧变长的情况
	{
		if (m_nDataLen > 0)
		{
			if (m_nDataLen > buffersize)  //传进来的buffer 不够大
			{
				TRACE("Buffer 不够大 ### %d， %d\n\r", m_nDataLen, buffersize);
				return -4;
			}
			else
			{
				nRef = ReceivePacket(playSocket, recBuffer, gFrameHeadLen);
				ptr = (unsigned int*)recBuffer;
				if (ptr[1] == FRAME_HEAD_FLAG)
				{
					compressFrameLen = ptr[0] - gFrameHeadLen;
					nRef = ReceivePacket(playSocket, recBuffer, compressFrameLen);
					if (nRef == compressFrameLen)
					{
						
						//nRef = uncompressLen;
						TRACE("decompress Unused\n\r");
					}
				}
				else   //帧头丢失
				{

					INT32 offsetLen = -1;

					TRACE("Paser the frame head.\n\r");

					nRef = ReceivePacket(playSocket, recBuffer, m_nDataLen/2);
					unsigned char* ucPtr = (unsigned char*)recBuffer;
					unsigned int Variable_frame_length = 0;
					for (int i = 0; i <= (nRef - 4); i++)
					{
						if (ucPtr[i] == 0xA5 && ucPtr[i + 1] == 0xA5 && ucPtr[i + 2] == 0x5A && ucPtr[i + 3] == 0x5A)
						{	
							if (i >= 4)
							{
								ptr = (unsigned int*)&ucPtr[i-4];
								Variable_frame_length = ptr[0] - gFrameHeadLen;  //获取长度
							}

							offsetLen = nRef - (gFrameHeadLen - 4) - i;   //应该扣掉长度这4个字节
							if (offsetLen > 0)
							{
								memcpy(recBuffer, &ucPtr[i + (gFrameHeadLen - 4)], offsetLen);
							}
							TRACE("Found the frame head.offsetLen =%d\n\r", offsetLen);
							break;
						}
					}
					if (Variable_frame_length > 0 && Variable_frame_length <= m_nDataLen)  //实际一帧图像解压后的数据有效长度
					{
						nRef = ReceivePacket(playSocket, &recBuffer[offsetLen], Variable_frame_length - offsetLen);
						nRef += offsetLen;
						if (nRef == Variable_frame_length)
						{
							
							TRACE("decompress Unused\n\r");

						}
					}
					else
					{
						nRef = -1;
						TRACE("Can't find the frame head.\n\r");
					}

				}
				//	TRACE("Rece data %d,  need %d,  %d, %d,\n\r",nRef,  m_nDataLen, tmp1, tmp2);
			}
			if (nRef == -1)
			{
				return nRef;
			}
			*cnt = nRef;

			return 0;
		}
	}
	else  //原来流程
	{

		if (m_nDataLen > 0)
		{
			if (m_nDataLen > buffersize)  //传进来的buffer 不够大
			{
				TRACE("Buffer 不够大 ### %d， %d\n\r", m_nDataLen, buffersize);
				return -4;
			}
			else
			{
				nRef = ReceivePacket(playSocket, pDataBuf, m_nDataLen);
				//	TRACE("Rece data %d,  need %d,  %d, %d,\n\r",nRef,  m_nDataLen, tmp1, tmp2);
			}
			if (nRef == -1)
			{
				return nRef;
			}
			*cnt = nRef;
			return 0;
		}
	}
		return -3;

}


INT32  NET_VIDEO_StopRealPlay(LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{
	unsigned char cmdbuffer[32];

	int cmdlen = 32;
	int usedindex = 0;
	int GetBottomUserId = lpDeviceInfo->userId;

	if (usedindex == -1)
		return FALSE;
	SOCKET Socket;


	memset(cmdbuffer, 0, 32);
	//the command length
	((unsigned int *)cmdbuffer)[0] = 32;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_GET_STOPPLAY;

	((unsigned int *)cmdbuffer)[2] = GetBottomUserId;

	((unsigned int *)cmdbuffer)[3] = 0;  //channel = 0;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket,(void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	return 0;
}


INT32  NET_VIDEO_Logout(LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{
	unsigned char cmdbuffer[32];
	int cmdlen = 32;

	int usedindex = 0;
	int GetBottomUserId = lpDeviceInfo->userId;

	SOCKET Socket;

	memset(cmdbuffer, 0, 32);
	//the command length
	((unsigned int *)cmdbuffer)[0] = 32;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_GET_LOGOUT;

	((unsigned int *)cmdbuffer)[2] = GetBottomUserId;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if ( SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	cmdlen = 20;
	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

#ifdef WIN32
	Sleep(200);
#else
	usleep(200*1000);
#endif

	Disconnect(&Socket); 

	return 0;
}

// 控制设备重启
INT32  NET_VIDEO_RebootDevice(LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{
	unsigned char cmdbuffer[256];

	int cmdlen = 16;
	int usedindex = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, 256);
	SOCKET Socket;

	//the command length
	((unsigned int *)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_REBOOT_DEV;

	((unsigned int *)cmdbuffer)[2] = usedindex;

	((unsigned int *)cmdbuffer)[3] = CONFIG_ACTIVE_FLAG;


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	cmdlen = 20;
	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}


	if (((unsigned int *)cmdbuffer)[1] == NET_SERVER_REBOOT_DEV)
	{
		if (((unsigned int *)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}


//设置ip命令
INT32  NET_VIDEO_SetIPConfig(LPNET_SERVER_DEVICEINFO lpDeviceInfo, void*  lpInBuffer)
{
	unsigned char cmdbuffer[268];

	int cmdlen = 12 + sizeof(APP_CONFIG);
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;

	memset(cmdbuffer, 0, 268);
	//the command length
	((unsigned int *)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_SET_IP;

	((unsigned int *)cmdbuffer)[2] = usedindex;
	memcpy(cmdbuffer + 12, lpInBuffer, sizeof(APP_CONFIG));


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}
	///	TirNetClient.m_cmdSock.Exit();

	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	cmdlen = 20;
	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int *)cmdbuffer)[1] == NET_SERVER_SET_IP)
	{
		if (((unsigned int *)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;

}

//framRate 帧率，一秒多少帧
INT32  NET_VIDEO_SetFrameRate(LPNET_SERVER_DEVICEINFO lpDeviceInfo, UINT32 framRate)
{
	unsigned char cmdbuffer[32];
	SOCKET Socket;

	int cmdlen = 32;
	int GetBottomUserId = lpDeviceInfo->userId;


	memset(cmdbuffer, 0, 32);

	((unsigned int *)cmdbuffer)[0] = 32;

	((unsigned int *)cmdbuffer)[1] = NET_SERVER_SETFRAMERATE;

	((unsigned int *)cmdbuffer)[2] = GetBottomUserId;

	((unsigned int *)cmdbuffer)[3] = framRate;  //channel = 0;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int *)cmdbuffer)[1] == NET_SERVER_SETFRAMERATE)
	{
		if (((unsigned int *)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}


//启动保存图像 framRate 帧率， 一秒钟存的图像数。这个要求相机输出的帧率要大于这个设置值， framRate =0 表示所有图像都存下来。 不是所有相机都支持这个 功能。
INT32  NET_VIDEO_StartStoreRAW(LPNET_SERVER_DEVICEINFO lpDeviceInfo, UINT32 framRate)
{
	unsigned char cmdbuffer[32];
	SOCKET Socket;

	int cmdlen = 32;
	int GetBottomUserId = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, 32);

	((unsigned int*)cmdbuffer)[0] = 32;

	((unsigned int*)cmdbuffer)[1] = NET_SERVER_STARTSTORING;

	((unsigned int*)cmdbuffer)[2] = GetBottomUserId;

	((unsigned int*)cmdbuffer)[3] = framRate;  //channel = 0;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int*)cmdbuffer)[1] == NET_SERVER_STARTSTORING)
	{
		if (((unsigned int*)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}

//停止保存图像
INT32  NET_VIDEO_StopStoreRAW(LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{
	unsigned char cmdbuffer[32];
	SOCKET Socket;

	int cmdlen = 32;
	int GetBottomUserId = lpDeviceInfo->userId;


	memset(cmdbuffer, 0, 32);

	((unsigned int *)cmdbuffer)[0] = 32;

	((unsigned int*)cmdbuffer)[1] = NET_SERVER_STOPSTORING;

	((unsigned int *)cmdbuffer)[2] = GetBottomUserId;

	((unsigned int*)cmdbuffer)[3] = 0;


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int*)cmdbuffer)[1] == NET_SERVER_STOPSTORING)
	{
		if (((unsigned int*)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}


////获取这次上电开机时长  
INT32  NET_VIDEO_GetRunningTime(LPNET_SERVER_DEVICEINFO lpDeviceInfo)
{
	unsigned char cmdbuffer[32];
	SOCKET Socket;

	int cmdlen = 32;
	int GetBottomUserId = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, 32);

	((unsigned int*)cmdbuffer)[0] = 32;

	((unsigned int*)cmdbuffer)[1] = NET_SERVER_GETRUNNINGTIME;

	((unsigned int*)cmdbuffer)[2] = GetBottomUserId;

	((unsigned int*)cmdbuffer)[3] = 0;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int*)cmdbuffer)[1] == NET_SERVER_GETRUNNINGTIME)
	{
		if (((unsigned int*)cmdbuffer)[3] == 0)
			return	((int*)cmdbuffer)[4];
		else
			return -4;
	}
	else
		return -5;
}

//设置前端系统时间
INT32  NET_VIDEO_SettingTime(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char *TimeString)
{
	unsigned char cmdbuffer[36];
	SOCKET Socket;

	unsigned int len;
	int cmdlen = 32;
	int GetBottomUserId = lpDeviceInfo->userId;

	len = strlen(TimeString);
	if (len != 19)
	{
		return -6;
	}

	memset(cmdbuffer, 0, 32);

	((unsigned int*)cmdbuffer)[0] = 32;

	((unsigned int*)cmdbuffer)[1] = NET_SERVER_SETTINGTIME;

	((unsigned int*)cmdbuffer)[2] = GetBottomUserId;

	strncpy((char *) & cmdbuffer[12], TimeString, 20);

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int*)cmdbuffer)[1] == NET_SERVER_SETTINGTIME)
	{
		if (((unsigned int*)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}

//自动聚焦
//Command == 10  //对焦焦距，往远处微调
//Command == 20  //对焦焦距，往近处微调
//Command == 44  //对焦焦距，往近处持续调整，直到停止命令
//Command == 45  //对焦焦距，往近处持续调整，直到停止命令
//Command  == 30 //自动对焦
INT32  NET_VIDEO_AutoFocus(LPNET_SERVER_DEVICEINFO lpDeviceInfo, int Command)
{
	unsigned char cmdbuffer[32];
	SOCKET Socket;

	int cmdlen = 32;
	int GetBottomUserId = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, 32);

	((unsigned int *)cmdbuffer)[0] = 32;

	((unsigned int *)cmdbuffer)[1] = NET_SERVER_AUTOFOCUS;

	((unsigned int *)cmdbuffer)[2] = GetBottomUserId;

	((unsigned int *)cmdbuffer)[3] = Command;  //channel = 0;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int *)cmdbuffer)[1] == NET_SERVER_AUTOFOCUS)
	{
		if (((unsigned int *)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}


//调焦
//Direction > 0 : 聚焦+  ;  Direction < 0 : 聚焦- ; Direction = 0 聚焦到zoomValue这个个焦距 
INT32  NET_VIDEO_AdjustZoom(LPNET_SERVER_DEVICEINFO lpDeviceInfo, int Direction, int zoomValue)
{
	unsigned char cmdbuffer[32];
	SOCKET Socket;

	int cmdlen = 32;
	int GetBottomUserId = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, 32);

	((unsigned int*)cmdbuffer)[0] = 32;

	((unsigned int*)cmdbuffer)[1] = NET_SERVER_ADJUSTZOOM;

	((int*)cmdbuffer)[2] = GetBottomUserId;

	(( int*)cmdbuffer)[3] = Direction;

	(( int*)cmdbuffer)[4] = zoomValue;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int*)cmdbuffer)[1] == NET_SERVER_ADJUSTZOOM)
	{
		if (((unsigned int*)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}

//停止调焦，聚焦
INT32  NET_VIDEO_StopZoomFocus(LPNET_SERVER_DEVICEINFO lpDeviceInfo )
{
	unsigned char cmdbuffer[32];
	SOCKET Socket;

	int cmdlen = 32;
	int GetBottomUserId = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, 32);

	((unsigned int*)cmdbuffer)[0] = 32;

	((unsigned int*)cmdbuffer)[1] = NET_SERVER_STOPZOOMFOCUS;

	((int*)cmdbuffer)[2] = GetBottomUserId;

	((int*)cmdbuffer)[3] = 0;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int*)cmdbuffer)[1] == NET_SERVER_STOPZOOMFOCUS)
	{
		if (((unsigned int*)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}

//获取焦距信息
//Value[0]: 当前焦距值 单位mm
//Value[1]: 最小AD值
//Value[2]: 最大AD值
//Value[3]: 镜头最小焦距 单位mm
//Value[4]: 镜头最大焦距 单位mm
// Nun = 5, 是这个int型数组指针的长度
INT32  NET_VIDEO_GetZoomInfo(LPNET_SERVER_DEVICEINFO lpDeviceInfo, int *Value, int Num)
{
	unsigned char cmdbuffer[36];
	SOCKET Socket;

	int cmdlen = 36;
	int GetBottomUserId = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, 36);

	((unsigned int*)cmdbuffer)[0] = 36;

	((unsigned int*)cmdbuffer)[1] = NET_SERVER_GETZOOMINFO;

	((int*)cmdbuffer)[2] = GetBottomUserId;

	((int*)cmdbuffer)[3] = 0;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int*)cmdbuffer)[1] == NET_SERVER_GETZOOMINFO)
	{
		if (((unsigned int*)cmdbuffer)[3] == 0)
		{	
			if (Num >= 5)
			{
				memcpy(Value, &((unsigned int*)cmdbuffer)[4], sizeof(unsigned int) * 5);
				return	0;
			}
			else
			{
				return -4;
			}
		}
		else
			return -5;
	}
	else
		return -6;
}

// 控制相机动作
INT32  NET_VIDEO_CAMERA_ACTION(LPNET_SERVER_DEVICEINFO lpDeviceInfo, UINT32 commandflag)
{
	unsigned char cmdbuffer[32];

	int cmdlen = 32;
	int usedindex = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	SOCKET Socket;

	//the command length
	((unsigned int *)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_CAMERA_ACTION;

	((unsigned int *)cmdbuffer)[2] = usedindex;

	((unsigned int *)cmdbuffer)[3] = CAMERA_ACTION_FLAG;

	((unsigned int *)cmdbuffer)[4] = commandflag;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}
	///	TirNetClient.m_cmdSock.Exit();

	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	cmdlen = 20;
	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int *)cmdbuffer)[1] == NET_SERVER_CAMERA_ACTION)
	{
		if (((unsigned int *)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}

//设置是否启动外触发发图像数据功能
//commandflag > 0,允许外触发功能； commandflag=0 不允许外触发；
INT32  NET_VIDEO_SET_TRIGGER(LPNET_SERVER_DEVICEINFO lpDeviceInfo, INT32 commandflag)
{
	unsigned char cmdbuffer[32];

	int cmdlen = 32;
	int usedindex = lpDeviceInfo->userId;

	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	SOCKET Socket;

	//the command length
	((unsigned int*)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_SETTRIGGERCMD;

	((unsigned int*)cmdbuffer)[2] = usedindex;

	((unsigned int*)cmdbuffer)[3] = commandflag;


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}
	///	TirNetClient.m_cmdSock.Exit();

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	cmdlen = 32;
	if (ReceivePacket(Socket, cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}

	if (((unsigned int*)cmdbuffer)[1] == NET_SERVER_SETTRIGGERCMD)
	{
		if (((unsigned int*)cmdbuffer)[3] == 0)
			return	0;
		else
			return -4;
	}
	else
		return -5;
}


// 发送机芯的控制命令，并获取返回数据
/// cmd 命令的buffer， 
//respond 机芯返回的应答， 
//resp_len 传进去表示respond的命令buffer空间大小，跟返回的命令的长度
INT32  NET_VIDEO_SendSensorCommand(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* cmd, int cmd_len, char* respond, int* resp_len)
{
	unsigned char cmdbuffer[512];
	unsigned char cmdreceive[512];
	int reclen;
	int cmdlen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;

	if (cmd == NULL || respond == NULL || cmd_len == 0)
	{
		TRACE("SendSensorCommand arg err.\r\n");
		return -1;
	}

	cmdlen = 12 + cmd_len;
	if (cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}
	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	//the command length
	((unsigned int*)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_SENDCAMERACMD; 

	((unsigned int*)cmdbuffer)[2] = usedindex;

	memcpy(&cmdbuffer[12], cmd, cmd_len);

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive));   
																	
	if (reclen <= 0)/// SOCKET_ERROR)
	{
		*resp_len = 0;
		return -3;
	}
	if (reclen > 12)
	{
		reclen -= 12;
		reclen = reclen <= *resp_len ? reclen : *resp_len;
		memcpy(respond, &cmdreceive[12], reclen);
		*resp_len = reclen;

		return 0;
	}

	*resp_len = 0;

	return -4;
}

//给机芯发命令， 不等机芯应答
//cmd 要发送的命令
//cmd_len 命令的长度
INT32  NET_VIDEO_SendSensorCommand_WithoutReply(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* cmd, int cmd_len)
{
	unsigned char cmdbuffer[512];
	unsigned char cmdreceive[24];
	int reclen;
	int cmdlen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;

	if (cmd == NULL ||  cmd_len == 0)
	{
		TRACE("SendSensorCommand without reply arg err.\r\n");
		return -1;
	}

	cmdlen = 12 + cmd_len;
	if (cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}
	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	//the command length
	((unsigned int*)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_SENDCAMERACMD_WITHOUT_REPLY; 

	((unsigned int*)cmdbuffer)[2] = usedindex;

	memcpy(&cmdbuffer[12], cmd, cmd_len);


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}
	///	TirNetClient.m_cmdSock.Exit();

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive));  

	if (reclen <= 0)/// SOCKET_ERROR)
	{
		return -3;
	}

		return 0;
	}	



// 发送虑片轮的控制命令，并获取返回数据
/// cmd 命令的buffer， 
//respond 虑片轮返回的应答， 
//resp_len 传进去表示respond的命令buffer空间大小，跟返回的命令的长度
INT32  NET_VIDEO_SendCircleCommand(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char *cmd, int cmd_len, char *respond, int *resp_len)
{
	char cmdbuffer[128];
	char cmdreceive[128];
	int reclen;
	int cmdlen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;

	if (cmd == NULL || respond == NULL || cmd_len == 0 || resp_len == NULL)
	{
		TRACE("SendCircleCommand arg err.\r\n");
		return -1;
	}

	cmdlen = 12 + cmd_len;
	if (cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}
	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	//the command length
	((unsigned int *)cmdbuffer)[0]=cmdlen;

	//the command type
	((unsigned int *)cmdbuffer)[1] = NET_SERVER_SENDCIRCLECMD;  

	((unsigned int *)cmdbuffer)[2] = usedindex;

	memcpy(&cmdbuffer[12], cmd, cmd_len);


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive));  

	if (reclen <= 0)/// SOCKET_ERROR)
	{
		*resp_len = 0;
		return -3;
	}
	if (reclen > 12)
	{
		reclen -= 12;
		reclen = reclen <= *resp_len ? reclen : *resp_len;
		memcpy(respond, &cmdreceive[12], reclen);
		*resp_len = reclen;

		return 0;
	}

	*resp_len = 0;

	return -4;
	}
// 设置虑片轮的位置
//position 虑片轮位置值
//resp_len 传进去表示respond的命令buffer空间大小，跟返回的命令的长度
INT32  NET_VIDEO_SetCirclePosition(LPNET_SERVER_DEVICEINFO lpDeviceInfo, unsigned short position)
{
	char cmdbuffer[128];
	char cmdreceive[128];
	unsigned char cmd[] = { 0xFF, 0xFF, 0x01, 0x09, 0x03, 0x2A, 0x22, 0x06, 0x00, 0x00, 0x00, 0x00, 0xA0};
	unsigned char checkSum =0;
	int i;
	int reclen;
	int cmdlen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;


	cmdlen = 12 + sizeof(cmd);
	if (cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}

	cmd[6] = position & 0xff;
	cmd[7] = (position>>8) & 0xff;
	for (i = 2; i < (sizeof(cmd) - 1); i++)
	{
		checkSum += cmd[i];
	}
	checkSum = ~checkSum;
	cmd[sizeof(cmd) - 1] = checkSum;

	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	//the command length
	((unsigned int*)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_SENDCIRCLECMD;

	((unsigned int*)cmdbuffer)[2] = usedindex;

	memcpy(&cmdbuffer[12], cmd, sizeof(cmd));


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}	

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
}

	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive));


	if (reclen >= 12)
	{
		return 0;
	}
	return -3;
}

// 发送镜头的控制命令，并获取返回数据
// cmd 命令的数据buffer
// cmd_len 命令得长度
// waitting_times_mS 等待命令最长时间，如果等于0，表示不等待返回命令发送完命令就返回
//respond 镜头返回的应答， 
//resp_len 传进去表示respond的命令buffer空间大小，跟返回的命令的长度
INT32  NET_VIDEO_SendLENCommand(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* cmd, int cmd_len, int waitting_times_mS, char* respond, int* resp_len)
{
	char cmdbuffer[128];
	char cmdreceive[128];
    int reclen;
	int cmdlen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;
	
	if (cmd == NULL || (respond == NULL && waitting_times_mS > 0) || cmd_len == 0 || resp_len == NULL)
	{
		TRACE("SendCircleCommand arg err.\r\n");
		return -1;
	}

	cmdlen = 12 + 4+ cmd_len;
	if (cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}
	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	//the command length
	((unsigned int *)cmdbuffer)[0]=cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_SENDCOMMAND_TOLEN;

	((unsigned int*)cmdbuffer)[2] = usedindex;

	((int*)cmdbuffer)[3] = waitting_times_mS;
	

	memcpy(&cmdbuffer[12+4], cmd, cmd_len);


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}
	
	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive));

	if (reclen <= 0)/// SOCKET_ERROR)
	{
		*resp_len = 0;
		return -3;
	}
	if (reclen > 12)
	{
		reclen -= 12;
		reclen = reclen <= *resp_len ? reclen : *resp_len;
		memcpy(respond, &cmdreceive[12], reclen);
		*resp_len = reclen;

		return 0;
	}
	else if(reclen == 12)
	{
		*resp_len = 0;
		return 0;
	}	

	*resp_len = 0;

	return -4;
}

//给串口发送命令，数据通过串口转发出去， 有些设备没有串口
// cmd 要转发出去的命令
// cmd_len 命令的长度
//respond 收到串口返回的命令
//resp_len 串口返回命令的长度
INT32  NET_VIDEO_SendUartCommand(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* cmd, int cmd_len, char* respond, int* resp_len)
{
	unsigned char cmdbuffer[512];
	unsigned char cmdreceive[512];
    int reclen;
	int cmdlen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;

	if(cmd == NULL || respond == NULL || cmd_len == 0)
	{
		TRACE("SendUartCommand arg err.\r\n");
		return -1;
	}

	cmdlen = 12+cmd_len;
	if(cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}
	memset(cmdbuffer,0,sizeof(cmdbuffer));
	//the command length
	((unsigned int *)cmdbuffer)[0]=cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_SENDUARTCMD; // #define	NET_SERVER_SENDCAMERACMD	0xC1    //����о����������������յ���о���ص�������ص��������ݲ��̶�����

	((unsigned int*)cmdbuffer)[2] = usedindex;

	memcpy(&cmdbuffer[12], cmd, cmd_len);

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}
	
	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive)); 
	
	if(reclen <= 0)/// SOCKET_ERROR)
	{
			*resp_len = 0;
			return -3;
	}
	if(reclen > 12)
	{
		reclen -= 12;
		reclen = reclen <= *resp_len? reclen:*resp_len;
		memcpy(respond, &cmdreceive[12],reclen);
		*resp_len = reclen;

		return 0;
	}
	*resp_len = 0;
	return -4;
}

//获取环境温度，制冷机芯专有命令
//temper 返回的温度，单位是0.1度，有符号。注意判断返回值为0才是有效
INT32  NET_VIDEO_GetEnvironmentTemp(LPNET_SERVER_DEVICEINFO lpDeviceInfo,  short *temper)
{
	unsigned char cmdbuffer[512];
	unsigned char cmdreceive[512];
	int reclen;
	int cmdlen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;
	 
	if (temper == NULL )
	{
		TRACE("GetEnvironmentTempData arg err.\r\n");
		return -1;
	}

	cmdlen = 12 ;
	if (cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}
	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	//the command length
	((unsigned int*)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_GETENVIRONMENTTMP; 

	((unsigned int*)cmdbuffer)[2] = usedindex;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive)); 

	if (reclen <= 0)/// SOCKET_ERROR)
	{
		return -3;
	}

	if (reclen >= (12 + 7))   //12是网络帧头的字节长度
	{
		if (cmdreceive[12] == 1 && cmdreceive[13] == 3 && cmdreceive[14] == 2)
		{
			*temper = cmdreceive[15];
			*temper <<= 8;
			*temper |= cmdreceive[16];

			return 0;
		}
	return -4;
	}

	return -5;
}



//设置端口号的值
//port =1，或 =2  对应GPIO1 , GPIO2  value = 0 或 1
//返回0 设置成功
INT32  NET_VIDEO_SetGPIO(LPNET_SERVER_DEVICEINFO lpDeviceInfo, int port, int value)
{
	unsigned char cmdbuffer[84];
	unsigned char cmdreceive[84];
	int reclen;
	int cmdlen = 20;
	int GetBottomUserId = 0;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;

	memset(cmdbuffer, 0, 84);
	//the command length
	((unsigned int*)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_SETGPIO_V20; 

	((unsigned int*)cmdbuffer)[2] = GetBottomUserId;

	((unsigned int*)cmdbuffer)[3] = port;
	((unsigned int*)cmdbuffer)[4] = value;

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}
	cmdlen = 24;
	memset(cmdreceive, 0, cmdlen);
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive)); 
	if (reclen <= 0)
	{
		return -3;
	}
	if (reclen >= 24)
	{
		if ((((unsigned int*)cmdreceive)[1] == NET_SERVER_SETGPIO_V20) && (((unsigned int*)cmdreceive)[3] == 0)) 
		{
			return 0;
		}
	}

	return -5;
}


//上位机给下面发送文件
//*file_name  存储的文件名称
//file_len 文件数据长度，单位字节
//*file_data 文件内容
INT32  NET_VIDEO_DownLoadFile(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* file_name, int file_len, char* file_data)
{
	char cmdbuffer[128];
	char cmdreceive[128];
    int reclen;
	int cmdlen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;

	if (file_name == NULL || file_data == NULL || file_len == 0)
	{
		TRACE("DownLoadFile arg err.\r\n");
		return -1;
	}

	cmdlen = strlen(file_name);
	if (cmdlen > 64)
	{
		TRACE("DownLoadFile name is too long %d.\r\n", cmdlen);
		return -2;
	}

	cmdlen += 16 + cmdlen + 1;
	if (cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}
	memset(cmdbuffer, 0, sizeof(cmdbuffer));
	//the command length
	((unsigned int*)cmdbuffer)[0] = cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = DVR_SERVER_DownLoadDATAFILE;  

	((unsigned int*)cmdbuffer)[2] = usedindex;

	((unsigned int*)cmdbuffer)[3] = file_len;

	strncpy(&cmdbuffer[16], file_name, sizeof(cmdbuffer)-16);


	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1; 
	}
	
	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive));   

	if (reclen == 36 && (((int*)cmdreceive)[3] == file_len))
	{
		if (SendPactket(&Socket, (void*)file_data, file_len) != file_len)
		{
			return -2;
		}

		return 0;
	}
	return -4;
}


//获取下位机的文件
//*file_name 要获取文件的名称
//*file_len 文件的长度
//*file_data 文件的存放的buffer
INT32  NET_VIDEO_GetFile(LPNET_SERVER_DEVICEINFO lpDeviceInfo, char* file_name, int *file_len, char* file_data)
{
	char cmdbuffer[128];
	char cmdreceive[64*1024];
	int reclen;
	int cmdlen;
	int readFilelen;
	int usedindex = lpDeviceInfo->userId;
	SOCKET Socket;

	if (file_name == NULL || file_data == NULL || file_len == NULL)
	{
		TRACE("GetFile arg err.\r\n");
		return -1;
	}

	cmdlen = strlen(file_name);
	if (cmdlen > 64)
	{
		TRACE("GetFile name is too long %d.\r\n", cmdlen);
		return -2;
	}

	cmdlen += 16 + cmdlen + 1;
	if(cmdlen > sizeof(cmdbuffer))
	{
		return -1;
	}
	memset(cmdbuffer,0,sizeof(cmdbuffer));
	//the command length
	((unsigned int *)cmdbuffer)[0]=cmdlen;

	//the command type
	((unsigned int*)cmdbuffer)[1] = NET_SERVER_GETDATAFILE;

	((unsigned int*)cmdbuffer)[2] = usedindex;

	((unsigned int*)cmdbuffer)[3] = 0;

	strncpy(&cmdbuffer[16], file_name, sizeof(cmdbuffer) - 16);

	if (!ConnectSever(&Socket, lpDeviceInfo->sIPaddr, lpDeviceInfo->uPort))
	{
		return -1;
	}

	if (SendPactket(&Socket, (void*)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}

	memset(cmdreceive, 0, sizeof(cmdreceive));
	reclen = ReceiveRespond(Socket, cmdreceive, sizeof(cmdreceive));  

	if (reclen == ((int*)cmdreceive)[3] + 16)
	{
		readFilelen = ((int*)cmdreceive)[3];
		if (readFilelen > *file_len)
		{
			readFilelen = *file_len;
		}
 
		memcpy(file_data, &cmdreceive[16], readFilelen);
		*file_len = readFilelen;
		return 0;
	}
	return -4;
}

#if 0

//设置端口号的值
//port =1，或 =2  对应GPIO1 , GPIO2  value = 0 或 1
//返回0 设置成功
LONG  NET_VIDEO_SetGPIO(int port,int value)
{
	unsigned char cmdbuffer[84];
	unsigned char cmdreceive[1024];
//	CString sIP;
	int cmdlen = 20;
	int usedindex = 0;
	int GetBottomUserId = 0;
	//GetBottomUserId = cServer.GetBottomUserId(usedindex);
	//if(GetBottomUserId==-1)
	//	return FALSE;

	if(TirNetClient.m_pCmdPort == 0)
	{
		TRACE("Not login the device\r\n");
		return -1;
	}


	memset(cmdbuffer,0,84);
	//the command length
	((unsigned int *)cmdbuffer)[0]=20;

	//the command type
	((unsigned int *)cmdbuffer)[1]=0xB9; //#define	NET_SERVER_SETGPIO	0xB9    //����GPIO,Ϊ�������ƾ�ͷ������

	((unsigned int *)cmdbuffer)[2]=GetBottomUserId;

     ((unsigned int *)cmdbuffer)[3]=port;
     ((unsigned int *)cmdbuffer)[4]=value;

	if(!TirNetClient.ConnectControl(TirNetClient.m_sIP,TirNetClient.m_pCmdPort))
	{
		return -2;
	}

	if(TirNetClient.m_ctlSock.SendPactket((void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -3;
	}
	cmdlen = 24;

	if(!TirNetClient.m_ctlSock.ReceivePacket(&cmdreceive,cmdlen))
	{
		return -4;
	}

	if(((unsigned int *)cmdreceive)[3] == 0)  //flag ��׼�� =0 ��ʾִ����ȷ
	{
		return 0;
	}	


	return -5;
}

//设置ip命令
BOOL  NET_VIDEO_SetIPConfig(void*  lpInBuffer)
{
	unsigned char cmdbuffer[268];
	unsigned char cmdreceive[1024];

	int cmdlen = 12+sizeof(APP_CONFIG);
	int usedindex=0;

	memset(cmdbuffer,0,268);
	//the command length
	((unsigned int *)cmdbuffer)[0]=cmdlen;

	//the command type
	((unsigned int *)cmdbuffer)[1]=0xBD;

	((unsigned int *)cmdbuffer)[2]=0;
	 memcpy(cmdbuffer+12,lpInBuffer,sizeof(APP_CONFIG));

	if(!TirNetClient.ConnectCommand(TirNetClient.m_sIP,TirNetClient.m_pCmdPort))
	{
		return FALSE;
	}

	if(TirNetClient.m_cmdSock.SendPactket((void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return FALSE;
	}
	cmdlen = 20;

	if(!TirNetClient.m_cmdSock.ReceivePacket(&cmdreceive,cmdlen))
	{
		return FALSE;
	}

	if(((unsigned int *)cmdreceive)[1] == 0xBD)
	{
		if(((unsigned int *)cmdreceive)[4]==0)
			return	TRUE;
		else
			return FALSE;
	}	
	else
		return FALSE;
	return TRUE;
}


// 控制设备重启
BOOL  NET_VIDEO_RebootDevice(void)
{
	unsigned char cmdbuffer[256];
	unsigned char cmdreceive[256];

	int cmdlen = 16;
	int usedindex=0;

	memset(cmdbuffer,0,256);
	//the command length
	((unsigned int *)cmdbuffer)[0]=cmdlen;

	//the command type
	((unsigned int *)cmdbuffer)[1]=0xBE;

	((unsigned int *)cmdbuffer)[2]=0;

	((unsigned int *)cmdbuffer)[3]=CONFIG_ACTIVE_FLAG;
	

	if(!TirNetClient.ConnectCommand(TirNetClient.m_sIP,TirNetClient.m_pCmdPort))
	{
		return FALSE;
	}

	if(TirNetClient.m_cmdSock.SendPactket((void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return FALSE;
	}
	cmdlen = 20;

	if(!TirNetClient.m_cmdSock.ReceivePacket(&cmdreceive,cmdlen))
	{
		return FALSE;
	}

	if(((unsigned int *)cmdreceive)[1] == 0xBE)
	{
		if(((unsigned int *)cmdreceive)[4]==0)
			return	TRUE;
		else
			return FALSE;
	}	
	else
		return FALSE;
	return TRUE;
}



//获取传感器的温度
//返回0 成功
LONG  NET_VIDEO_GetSensorTemp(int *value)
{
	unsigned char cmdbuffer[84];
	unsigned char cmdreceive[512];
	const unsigned char cmdData[] = {0x6E, 0x00, 0x00, 0x20, 0x00, 0x02, 0x79, 0x3F, 0x00, 0x00, 0x00, 0x00};  //6E 00 00 20 00 02 79 3F 00 00 00 00 
    int reclen;
	int cmdlen = 84;
	int usedindex = 0;
	int GetBottomUserId = 0;
	//GetBottomUserId = cServer.GetBottomUserId(usedindex);
	//if(GetBottomUserId==-1)
	//	return FALSE;
	
	if(TirNetClient.m_pCmdPort == 0)
	{
		TRACE("Not login the device\r\n");
		return -1;
	}

	cmdlen = 12+sizeof(cmdData);
	memset(cmdbuffer,0,84);
	//the command length
	((unsigned int *)cmdbuffer)[0]=cmdlen;

	//the command type
	((unsigned int *)cmdbuffer)[1]=0xBA; //#define	NET_SERVER_GETSENSORTMP	0xBA   

	((unsigned int *)cmdbuffer)[2]=GetBottomUserId;

	memcpy(&cmdbuffer[12], cmdData,sizeof(cmdData));

	if(!TirNetClient.ConnectControl(TirNetClient.m_sIP,TirNetClient.m_pCmdPort))
	{
		return -1;
	}

	if(TirNetClient.m_ctlSock.SendPactket((void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}
	
	cmdlen = 24;

	reclen = TirNetClient.m_ctlSock.ReceivePacket(&cmdreceive,cmdlen);
	if(reclen < 24 )  //接收的长度不够
	{
			return -3;
	}

	if(cmdreceive[12] == 0x6E && cmdreceive[15] == 0x20  && cmdreceive[17] == 0x02)  //
	{
		*value =  cmdreceive[20]<<8 |  cmdreceive[21];

	return 0;
	}
	 

	return -4;
}




LONG SendHeartCmd(LONG userID,int index)
{
	unsigned char cmdbuffer[84];
	unsigned char cmdreceive[84];

//	CString sIP;
	int cmdlen = 32;
	int usedindex=userID;
	int GetBottomUserId = 0;
	//GetBottomUserId = cServer.GetBottomUserId(usedindex);
	//if(GetBottomUserId==-1)
	//	return FALSE;

	memset(cmdbuffer,0,84);
	//the command length
	((unsigned int *)cmdbuffer)[0]=32;

	//the command type
	((unsigned int *)cmdbuffer)[1]=0xb1;

	((unsigned int *)cmdbuffer)[2]=GetBottomUserId;

	if(!TirNetClient.ConnectHeart(TirNetClient.m_sIP,TirNetClient.m_pCmdPort))
	{
		return -1;
	}

		
	TirNetClient.m_heartSock.SendPactket((void *)cmdbuffer, cmdlen);

	cmdlen = 20;

	if(!TirNetClient.m_heartSock.ReceivePacket(&cmdreceive,cmdlen))
	{
		return -2;
	}

	if(((unsigned int *)cmdreceive)[1] == 0x1A)
	{
		TRACE("the heart return.\n");
		if(((unsigned int *)cmdreceive)[4]==NET_DVR_USERNOTEXIST)
		{
			}

	}	


	return 0;
}


//Get net config: 32 bytes,command type is 0x86.
// --------------------------------------------------------
// | 4 length | 4 command type | 8 userID | 16  reserverd |
// --------------------------------------------------------
/***************************************************************
Description:
		Get net config information.
		
Parameters:
		sDVRIP: the IP address.
		wDVRPort: the connect port.
		userId: user id.
		lpDeviceInfo: device information.
		
Returns:
		TRUE: net config.
****************************************************************/
LONG NET_VIDEO_GetNetinfo(char *sDVRIP,WORD wDVRPort,LONG userId,LPNET_SERVER_NETCFG lpNetcfg )
{

	unsigned char cmdbuffer[32];
	unsigned char cmdreceive[1024];
	int cmdlen = 32;
	int usedindex=userId;
	int GetBottomUserId = 0;
//	GetBottomUserId = cServer.GetBottomUserId(usedindex);
//	if(GetBottomUserId==-1)
//		return FALSE;

	memset(cmdbuffer,0,32);
	//the command length
	((unsigned int *)cmdbuffer)[0]=32;

	//the command type
	((unsigned int *)cmdbuffer)[1]=0x86;

	((unsigned int *)cmdbuffer)[2]=GetBottomUserId;


	if(!TirNetClient.ConnectCommand(TirNetClient.m_sIP,TirNetClient.m_pCmdPort))
	{
		return -1;
	}

	if(TirNetClient.m_cmdSock.SendPactket((void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return -2;
	}
	cmdlen = 272;

	if(!TirNetClient.m_cmdSock.ReceivePacket(&cmdreceive,cmdlen))
	{
		return -3;
	}
	if(((unsigned int *)cmdreceive)[1] == 0x07)
	{
		memcpy(lpNetcfg,cmdreceive+20,sizeof(NET_SERVER_NETCFG));
	}
	
	return 0;
}



//login command: 84 bytes,command type is 0x84.
// ----------------------------------------------------------------------------------------------------------
// | 4 length | 4 command type | 8 userID | 32 user name | 16 passwd | 16 ip address | 2 port | 2 reserverd |
// ----------------------------------------------------------------------------------------------------------
/***************************************************************
Description:
		net user login.
		
Parameters:
		sDVRIP: the IP address.
		wDVRPort: the connect port.
		sUserName: user name.
		sPassword: user password.
		lpDeviceInfo: device information.
		//userindex for top user
		//serverid is for bottom user
		
Returns:
		TRUE: login success.
****************************************************************/











#define NET_SERVER_SETWARNTEMP      0xBF   //设置报警温度
#define NET_SERVER_GETWARNTEMP      0xC0   //获取报警温度
//设置报警温度值,设置的温度值单位是 0.1摄氏度
BOOL  NET_VIDEO_SetWarnTemp(int tmp_01C)
{
	unsigned char cmdbuffer[32];

	int cmdlen = 32;
	int usedindex=0;
	int GetBottomUserId =0;

	if(usedindex==-1)
		return FALSE;

	cServerClient.m_bExitDisplay = FALSE;	


	memset(cmdbuffer,0,32);

	((unsigned int *)cmdbuffer)[0]=32;

	((unsigned int *)cmdbuffer)[1]=NET_SERVER_SETWARNTEMP;

	((unsigned int *)cmdbuffer)[2]=GetBottomUserId;

	((unsigned int *)cmdbuffer)[3]=0;  //channel = 0;

	(( int *)cmdbuffer)[4]=tmp_01C;  //channel = 0;

	if(!TirNetClient.ConnectControl(TirNetClient.m_sIP,TirNetClient.m_pCmdPort))
	{
		return FALSE;
	}

	if(TirNetClient.m_ctlSock.SendPactket((void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return FALSE;
	}

	if(!TirNetClient.m_ctlSock.ReceivePacket(cmdbuffer,cmdlen))
	{
		return FALSE;
	}


///	cServerClient.StopVideoDecodeThread();
///	Sleep(100);

	return TRUE;
}
BOOL  NET_VIDEO_GetWarnTemp(int *tmp_01C)
{
	unsigned char cmdbuffer[32];

	int cmdlen = 32;
	int usedindex=0;
	int GetBottomUserId =0;

	if(usedindex==-1)
		return FALSE;

	cServerClient.m_bExitDisplay = FALSE;	


	memset(cmdbuffer,0,32);

	((unsigned int *)cmdbuffer)[0]=32;

	((unsigned int *)cmdbuffer)[1]=NET_SERVER_GETWARNTEMP;

	((unsigned int *)cmdbuffer)[2]=GetBottomUserId;

	((unsigned int *)cmdbuffer)[3]=0;  //channel = 0;

	

	if(!TirNetClient.ConnectControl(TirNetClient.m_sIP,TirNetClient.m_pCmdPort))
	{
		return FALSE;
	}

	if(TirNetClient.m_ctlSock.SendPactket((void *)cmdbuffer, cmdlen) != cmdlen)
	{
		return FALSE;
	}

	if(!TirNetClient.m_ctlSock.ReceivePacket(cmdbuffer,cmdlen))
	{
		return FALSE;
	}

	*tmp_01C = (( int *)cmdbuffer)[4];

	return TRUE;
}




#endif
