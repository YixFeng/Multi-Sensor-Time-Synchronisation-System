/*--------------------------------------------------------------------
                        COPYRIGHT (C) 2008,  Corporation
                        ALL RIGHTS RESERVED
                                                                                      
 This source code has been made available to you by  on an
 AS-IS basis. Anyone receiving this source code is licensed under 
 copyrights to use it in any way he or she deems fit, including copying it,
 modifying it, compiling it, and redistributing it either with or without
 modifications. Any person who transfers this source code or any derivative
 work must include the copyright notice and this paragraph in
 the transferred software.
 1.0    Initial net software                       CCL   2008.12.24
                                                                                      
-----------------------------------------------------------------------*/

#ifndef _NETDEFINE_H_
#define _NETDEFINE_H_
//#include "netdefine.h"
//#include <windows.h>
//typedef char BOOL;
#define TRUE	1
#define FALSE	0

//define type
typedef unsigned char UINT8;
typedef char SINT8;
typedef unsigned short UINT16;
typedef short SINT16;
typedef unsigned int UINT32;
typedef int SINT32;
typedef int INT32;

typedef int SOCKET;
typedef bool BOOL;



#define CONFIG_ACTIVE_FLAG          0x5a5aa5a5  
#define CAMERA_ACTION_FLAG          0x5500aa00  


typedef struct
{
	UINT32 	ip_addr;									//Ӧ�ó�������
	UINT32  gw_addr;
	UINT32  net_mask;
	UINT32  port;           //�˿�Ŀǰ��֧���޸�
	SINT32  reserve1;

	UINT32 reserve2;									//������
	UINT32 stru_flag;									// ���ݽṹ��Ч��־
} APP_CONFIG, *LAPP_CONFIG;



typedef long LONG;

#define TRACE  printf 

#define SOCKET_ERROR (-1)  
#define INVALID_SOCKET  (SOCKET)(~0)
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02




#endif  //_NETDEFINE_H_
