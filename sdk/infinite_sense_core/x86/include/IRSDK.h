#ifndef _IRSDK_H
#define _IRSDK_H

#include <stdio.h>


#ifdef _WIN32
#define IR_SDK_API  extern "C" __declspec(dllexport)
#else
#define IR_SDK_API extern "C"
#endif

#define DEVICE_MAX				(32)							//最多支持例化32个设备的实例句柄
#define OBJ_MAX					(32)							//最多支持例化32个对象，上层只能少于32，不能超过

typedef int (*CBF_IR)(void * lData, void * lParam);				//SDK中回调函数的格式声明

//温度转换
#define CALTEMP(x,y)		((x-10000)/(float)y)				//Y16数据与温度数据的转换关系

//文件操作
#define OPEN_FILE				(1)
#define CLOSE_FILE				(2)
#define WR_FRAME 				(3)

//聚焦参数
#define  STOPFOCUS				(0)
#define  FARFOCUS				(1)
#define  NEARFOCUS				(2)
#define  AUTOFOCUS				(3)
#define  FOCUSSTATUS			(4)

#define  MAX_W					(1280)
#define  MAX_H					(1024)

//帧格式,数据前面32Byte帧头
typedef struct tagFrame
{
    unsigned short width;				//图像宽度
    unsigned short height;				//图像高度
    unsigned short u16FpaTemp;			//焦面温度
    unsigned short u16EnvTemp;			//环境温度
    unsigned char  u8TempDiv;			//数据转换成温度时需要除以该数据，文档具体说明
    unsigned char  u8DeviceType;		//unused
    unsigned char  u8SensorType;		//unused
    unsigned char  u8MeasureSel;		//unused
    unsigned char  u8Lens;				//unused
    unsigned char  u8Fps;				//unused
    unsigned char  u8TriggerFrame;		//unused
    unsigned char  u8Reversed2;			//unused
    unsigned int  u32FrameIndex;		//unused
    unsigned char  Reversed[12];		//unused
	unsigned short buffer[MAX_W*MAX_H];		//图像数据，按行存储，最大支持 1280x1024，每个像素是一个 ushort 类型
} Frame;


//点， （x,y)为坐标
typedef struct t_point
{
    unsigned short x;
    unsigned short y;
}T_POINT;

//线，P1,P2为两端点
typedef struct t_line
{
    T_POINT P1;
    T_POINT P2;
}T_LINE;

//圆（支持椭圆），Pc表示圆心坐标，a表示横半轴，b表示竖半轴，对于圆形a=b，表示半径
typedef struct t_circle
{
    T_POINT Pc;
    unsigned short a;
    unsigned short b;
}T_CIRCLE;

//矩形， P1为矩形左上顶点坐标，P2为右下顶点坐标
typedef struct t_rect
{
    T_POINT P1;
    T_POINT P2;
}T_RECT;

//任意多边形，最多支持16个顶点，Pt_num为顶点个数，Pt为每个点的坐标
typedef struct t_polygon
{
    unsigned int Pt_num;
    T_POINT Pt[16];
}T_POLYGON;

//雷达图，最多支持64个顶点，Pt_num为交点个数，Pt为每个点的坐标
typedef struct t_radar
{
    unsigned int Pt_num;
    T_POINT Pt[64];
}T_RADAR;

//最大最小平均温度及最值对应坐标
typedef struct stat_temper
{
    float maxTemper;
    float minTemper;
    float avgTemper;

    T_POINT maxTemperPT;
    T_POINT minTemperPT;
}STAT_TEMPER;

//颜色
typedef struct t_color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
}T_COLOR;

//参数配置类型
enum T_ALARMTYPE
{
    OverHigh	= 0,			//高于高门限
    UnderLow	= 1,			//低于低门限
    BetweenHL	= 2,			//区间
    DeBetweenHL = 3,			//反选区间
};

//告警
typedef struct t_alarm
{
    unsigned char alarmType;	//告警类型
    unsigned char isDraw;		//是否屏显
    unsigned char isVioce;		//是否声音告警
    unsigned char isVideo;		//是否录像
    float		  HighThresh;	//高门限
    float		  LowThresh;	//低门限
    T_COLOR		  colorAlarm; 	//告警颜色
}T_ALARM;

//全局温度
typedef struct stat_global
{
	stat_global()
	{
		inputEmiss = 0.98;
		inputReflect = 20.0;
		inputDis = 2.0;
		inputOffset = 0;
	}
	STAT_TEMPER sTemp;
	unsigned int  LableEx[32];		//对象名
	unsigned char Lable[32];		//对象名
	float		  inputEmiss;		//输入辐射率
	float		  inputReflect;		//输入反射温度
	float		  inputDis;			//输入距离
	float         inputOffset;      //温度修正
	float		  Area;				//计算面积
	unsigned char pal;				//调色板
	unsigned char reserved2;
	unsigned char reserved3;
	unsigned char reserved4;
	T_COLOR	 color;						//对象颜色
	T_ALARM  sAlarm;
}STAT_GLOBAL;

//点以及点温度
typedef struct stat_point
{
    stat_point()
    {
		inputEmiss = 0.98;
        inputReflect = 20.0;
        inputDis = 2.0;
		inputOffset = 0;
    }
    T_POINT sPoint;
    STAT_TEMPER sTemp;
    unsigned int  LableEx[32];		//对象名
    unsigned char Lable[32];		//对象名
    float		  inputEmiss;		//输入辐射率
    float		  inputReflect;		//输入反射温度
    float		  inputDis;			//输入距离
	float         inputOffset;      //温度修正
    float		  Area;				//计算面积
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
    unsigned char reserved4;
    T_COLOR	 color;						//对象颜色
    T_ALARM  sAlarm;
}STAT_POINT;

//线以及线上温度统计
typedef struct stat_line
{
    stat_line()
    {
		inputEmiss = 0.98;
        inputReflect = 20.0;
        inputDis = 2.0;
		inputOffset = 0;
    }
    T_LINE sLine;
    STAT_TEMPER sTemp;
    unsigned int  LableEx[32];		//对象名
    unsigned char Lable[32];		//对象名
    float		  inputEmiss;		//输入辐射率
    float		  inputReflect;		//输入反射温度
    float		  inputDis;			//输入距离
	float         inputOffset;      //温度修正
    float		  Area;				//计算面积
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
    unsigned char reserved4;
    T_COLOR	 color;					//对象颜色
    T_ALARM  sAlarm;
}STAT_LINE;

//圆（椭圆)以及圆内温度统计
typedef struct stat_circle
{
    stat_circle()
    {
		inputEmiss = 0.98;
        inputReflect = 20.0;
        inputDis = 2.0;
		inputOffset = 0;
    }
    T_CIRCLE sCircle;
    STAT_TEMPER sTemp;
    unsigned int  LableEx[32];		//对象名
    unsigned char Lable[32];		//对象名
    float		  inputEmiss;		//输入辐射率
    float		  inputReflect;		//输入反射温度
    float		  inputDis;			//输入距离
	float         inputOffset;      //温度修正
    float		  Area;				//计算面积
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
    unsigned char reserved4;
    T_COLOR		 color;					//对象颜色
    T_ALARM  sAlarm;
}STAT_CIRCLE;

//矩形以及矩形内温度统计
typedef struct stat_rect
{
    stat_rect()
    {
		inputEmiss = 0.98;
        inputReflect = 20.0;
        inputDis = 2.0;
		inputOffset = 0;
    }
    T_RECT sRect;
    STAT_TEMPER sTemp;
    unsigned int  LableEx[32];		//对象名
    unsigned char Lable[32];		//对象名
    float		  inputEmiss;		//输入辐射率
    float		  inputReflect;		//输入反射温度
    float		  inputDis;			//输入距离
	float         inputOffset;      //温度修正
    float		  Area;				//计算面积
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
    unsigned char reserved4;
    T_COLOR	 color;					//对象颜色
    T_ALARM  sAlarm;
}STAT_RECT;

//任意多边形以及多边形内温度统计
typedef struct stat_polygon
{
    stat_polygon()
    {
		inputEmiss = 0.98;
        inputReflect = 20.0;
        inputDis = 2.0;
		inputOffset = 0;
    }
    T_POLYGON sPolygon;
    STAT_TEMPER sTemp;

    unsigned int  LableEx[32];		//对象名
    unsigned char Lable[32];		//对象名
    float		  inputEmiss;		//输入辐射率
    float		  inputReflect;		//输入反射温度
    float		  inputDis;			//输入距离
	float         inputOffset;      //温度修正
    float		  Area;				//计算面积
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
    unsigned char reserved4;
    T_COLOR	 color;
    T_ALARM  sAlarm;
}STAT_POLYGON;

//雷达图温度统计
typedef struct stat_radar
{
    stat_radar()
    {
		inputEmiss = 0.98;
        inputReflect = 20.0;
        inputDis = 2.0;
		inputOffset = 0;
    }
    T_RADAR sRadar;
    STAT_TEMPER sTemp[64];

    unsigned int  LableEx[32];		//对象名
    unsigned char Lable[32];		//对象名
    float		  inputEmiss;		//输入辐射率
    float		  inputReflect;		//输入反射温度
    float		  inputDis;			//输入距离
	float         inputOffset;      //温度修正
    float		  Area;				//计算面积
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned char reserved3;
    unsigned char reserved4;
    T_COLOR	 color;
    T_ALARM  sAlarm;
}STAT_RADAR;

//所有对象，该结构体用来存放所有测温对象，num表示该类型对象的个数，对应的指针，需要指向测温对象，该空间需要在使用前开辟
typedef struct stat_obj
{
    unsigned char numPt;
    unsigned char numLine;
    unsigned char numCircle;
    unsigned char numRect;
    unsigned char numPolygon;
    unsigned char Reserved1;
    unsigned char Reserved2;
    unsigned char Reserved3;

    T_ALARM sGlobalAlarm;		//全局告警信息

    STAT_POINT	sPt[OBJ_MAX];
    STAT_LINE	sLine[OBJ_MAX];
    STAT_CIRCLE sCircle[OBJ_MAX];
    STAT_RECT	sRect[OBJ_MAX];
    STAT_POLYGON sPolygon[OBJ_MAX];
    STAT_RADAR Reserved[1];
}STAT_OBJ;


//128byte，存储原始视频时，该头信息会保存到视频文件中，读取时，会读出该头信息以供使用
typedef struct tagSAVEHead
{
    unsigned char  Head[32];
    unsigned short width;
    unsigned short height;
    unsigned int   totalFrames;
    unsigned short Freq;
    unsigned char  Reserved0;
    unsigned char  version;
    unsigned int   timelen;
    unsigned char  timestamp[20];
    unsigned char  Reserved[60];
} T_SAVE_HEAD;

typedef struct tagTIME {
    unsigned short year;
    unsigned short month;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short sencond;
    unsigned short millisecond;
} T_TIME;


//512byte，设备信息，均以字符串形式存储，该空间已经开辟，不需要再申请空间
typedef struct tagDeviceID
{
    unsigned char  Name[32];			//设备名称
    unsigned char  Model[32];		//设备型号
    unsigned char  SerialNum[32];	//设备序列号
    unsigned char  Lens[32];			//镜头规格
    unsigned char  FactoryTime[32];	//出厂时间
    unsigned char  WorkTime[32];		//工作时间
    unsigned char  Mac[32];			//MAC地址
    unsigned char  IP[32];			//IP地址
    unsigned char  TempRange[32];   //温度范围
    unsigned char  Reserved2[32];
    unsigned char  Reserved3[32];
    unsigned char  Reserved4[32];
    unsigned char  Reserved5[32];
    unsigned char  Reserved6[32];
    unsigned char  Reserved7[32];
    unsigned char  Reserved8[32];
}T_DEVICE_INFO;

typedef struct tagIPADDR
{
    char IPAddr[32];			//IP
    unsigned char Reserved[32]; //保留
    unsigned int DataPort;		//Port
    unsigned char isValid;		//是否在线
    unsigned char totalOnline;  //在线个数
    unsigned char Index;        //在列表中的索引
}T_IPADDR;

//参数配置类型
enum T_PARAMTYPE
{
    paramDevice			= 0,			//设备类型
    paramDownSample		= 1,			//降采样
    paramDecCoef		= 2,			//校正系数
    paramFPS		    = 3,			//获取帧频
	paramreserved		= 4,			//图像镜像
    paramSpaceFilter	= 5,			//空域滤波
    paramEnhanceImage	= 6,			//增强
    paramTempSegSel		= 7,			//温度段选择
};


#ifndef _T_CTRLPROTOCOL			//防止重复定义
#define _T_CTRLPROTOCOL
//运动控制类型
enum T_PARAMCTRL
{
    //协议选择
    paramPelcod 		= 0,		   //pelco-d
    paramUserDef1		= 1,		   //自定义协议1（升降杆）
    paramUserDef2		= 2,		   //自定义协议2（舵机）
    paramUserDef3		= 3,		   //自定义协议3（转台）
	paramUserDef4		= 21,		   //自定义协议4（舵机马达模式）

    //控制
    paramCtrlUp			= 4,			//上
    paramCtrlDown		= 5,			//下
    paramCtrlLeft		= 6,			//左
    paramCtrlRight		= 7,			//右
    paramCtrlStop		= 8,			//停止
    paramCtrlBaudRate	= 9,			//波特率

    paramCtrlPreSet		= 10,			//预置位
    paramCtrlPreGo		= 11,			//调用预置位

    paramMtGetUDPos		= 21,			//获取位置
    paramMtGetLRPos		= 22,			//获取位置
    paramMtSetUDPos		= 23,			//设置初始位置
    paramMtSetLRPos		= 24,			//设置初始位置

};
#endif

/*============================================================================
function:	IRSDK_GetVersion:获取sdk版本信息,格式：v2.2.3.0
parameter:	char *version：接收版本的字符串指针，调用前开辟空间，大于16Byte
return:		int:-1：异常
0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetVersion(char *version);

/*============================================================================
function:	IRSDK_Init:初始化sdk,实际是创建广播端口，用于接收广播的ip信息
parameter:	void
return:		int: 1：初始化完成
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Init(void);

/*============================================================================
function:	IRSDK_Quit:退出sdk,释放最后一个句柄
parameter:	void
return:		int: -1：未初始化，最后一个句柄不存在
				0： 释放成功
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Quit(void);

/*============================================================================
function:	IRSDK_SetIPAddrArray:设置IP地址的接收地址，指针内容由SDK维护
parameter:	void * pIpInfo：定义一个T_IPADDR数组，将数组的指针传入，数组内容由sdk维护
return:		int: 0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SetIPAddrArray(void * pIpInfo);

/*============================================================================
function:	IRSDK_Create:创建红外句柄
parameter:	int handle：句柄值，不同的红外给不同的值（0~31)
			T_IPADDR sIPAddr: 查询到的红外IP地址信息
			CBF_IR cbf_stm：数据端口回调函数
			CBF_IR cbf_cmd：命令端口回调函数，一般设置为NULL
			CBF_IR cbf_comm：通信端口回调函数，一般设置为NULL
			void * param：回调函数参数
return:		int: 0：IP地址无效或者句柄无效
				1：创建成功
				2：创建成功（句柄重复，销毁之前的句柄后，重新创建句柄）
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Create(int handle, T_IPADDR sIPAddr, CBF_IR cbf_stm, CBF_IR cbf_cmd, CBF_IR cbf_comm, void * param = 0);

/*============================================================================
function:	IRSDK_Connect:连接红外相机
parameter:	int handle：红外句柄
return:		int: 0：命令发送成功（命令发送成功，不表示相机连接正常）
				-1：无效句柄
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Connect(int handle);

/*============================================================================
function:	IRSDK_Destroy:停止连接，销毁句柄
parameter:	int handle：红外句柄
return:		int: 0：销毁成功
				-1：无效句柄
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Destroy(int handle);

/*============================================================================
function:	IRSDK_Play:发送播放命令
parameter:	int handle：红外句柄
return:		int: 0：命令发送成功
				-1：无效句柄
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Play(int handle);

/*============================================================================
function:	IRSDK_Stop:发送停止命令，停止后数据回调将会无数据
parameter:	int handle：红外句柄
return:		int: 0：命令发送成功
				-1：无效句柄
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Stop(int handle);

/*============================================================================
function:	IRSDK_SetIP:发送停止命令，停止后数据回调将会无数据
parameter:	int handle：红外句柄
return:		int: 0：命令发送成功
				-1：无效句柄
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SetIP(int handle, char * pIp);

/*============================================================================
function:	IRSDK_Command:命令端口发送数据，命令+参数
parameter:	int handle：红外句柄
			int cmd：命令
			int param：参数
return:		int: 0：命令发送成功
				-1：无效句柄
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Command(int handle, int command, int param);

/*============================================================================
function:	IRSDK_Calibration:快门校正命令
parameter:	int handle：红外句柄
return:		int: 0：命令发送成功
				-1：无效句柄
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Calibration(int handle);

/*============================================================================
function:	IRSDK_NearFarFocus:电动镜头控制
parameter:	int handle：红外句柄
			unsigned int param：控制方式, 见头文件定义
return:		int: 0：命令发送成功
				-1：句柄错误
				当参数为FOCUSSTATUS,返回值为聚焦状态，0表示自动聚焦完成，1表示正在进行自动聚焦
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_NearFarFocus(int handle, unsigned int param);

/*============================================================================
function:	IRSDK_InquireIP:查询IP命令，该函数需要放在定时执行，定时时间要求小于1s
parameter:	void * pIpInfo：定义一个T_IPADDR*指针，用于接收IP地址信息，指针内容由sdk维护
			unsigned int TimerInterval：定时器间隔时间（单位us)
return:		int: 0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_InquireIP(void * pIpInfo, unsigned int TimerInterval);


/*============================================================================
function:	IRSDK_IsConnected:查询相机是否连接正常（使用此函数时，IRSDK_InquireIP必须要定时执行）
parameter:	void * pIpInfo：定义一个T_IPADDR*指针，用于接收IP地址信息，指针内容由sdk维护
return:		int: 0：句柄错误或者相机连接异常，回调无数据
				1：相机连接正常，回调有数据
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_IsConnected(int handle);

/*============================================================================
function:	IRSDK_InquireDeviceInfo:查询设备信息
parameter:	int handle：红外句柄
			T_DEVICE_INFO* pDevInfo：传入指针，接收信息（调用函数前开辟空间）
return:		int: 0：命令发送成功
				-1：句柄错误或者指针为空
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_InquireDeviceInfo(int handle, T_DEVICE_INFO* pDevInfo);

/*============================================================================
function:	IRSDK_ParamCfg:设置参数
parameter:	int handle：红外句柄
			T_PARAMTYPE mParamType：参数类型，见头文件
			float f32Param:参数值
return:		int: 0：命令发送成功
				-1：句柄错误
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_ParamCfg(int handle, T_PARAMTYPE mParamType, float f32Param);

/*============================================================================
function:	IRSDK_Frame2Gray:自动调光（原始数据转灰度数据）
parameter:	Frame *pFrame：帧结构指针
			unsigned short *pGray: 灰度结果（0~255），类型是ushort
			float f32Constrast:对比度
			float f32Bright：亮度
			unsigned short u16TFilterCoef：时域滤波（0~100，0是关闭，100时域滤波效果最弱，1时效果最好）
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Frame2Gray(Frame *pFrame, unsigned short *pGray, float f32Constrast, float f32Bright, unsigned short u16TFilterCoef);

/*============================================================================
function:	IRSDK_Frame2GrayDDE:带DDE功能的自动调光（原始数据转灰度数据）
parameter:	Frame *pFrame：帧结构指针
unsigned short *pGray: 灰度结果（0~255），类型是ushort
float f32Constrast:对比度
float f32Bright：亮度
unsigned short u16TFilterCoef：时域滤波（0~100，0是关闭，100时域滤波效果最弱，1时效果最好）
unsigned char u8DDEcoef：DDE系数（0~100，0是关闭）
unsigned char u8Gamma：gamma校正（0~10，0是关闭）
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Frame2GrayDDE(Frame *pFrame, unsigned short *pGray, float f32Constrast, float f32Bright, unsigned short u16TFilterCoef, unsigned char u8DDEcoef, unsigned char u8Gamma);

/*============================================================================
function:	IRSDK_FrameAgc:区间调光（原始数据转灰度数据）
parameter:	Frame *pFrame：帧结构指针
			unsigned short *pGray: 灰度结果（0~255），为了兼容，类型是ushort
			float f32Constrast:对比度
			float f32Bright：亮度
			float f32MinT : 调光范围最低温阈值；
			float f32MaxT : 调光范围最高温阈值（当f32MinT与f32MaxT相等时，该函数为自动调光）；
			STAT_RECT *pRect：根据提供的矩形区域的最低最高温设定调光范围，当使用这个时，f32MinT和f32MaxT 设置无效，不需要使用时设置为NULL
			unsigned char u8Method：调光方法，设置为0；
			unsigned short u16TFilterCoef：时域滤波（0~100，0是关闭，100时域滤波效果最弱，1时效果最好）
			unsigned char u8DDEcoef：DDE系数（0~100，0是关闭）
			unsigned char u8Gamma：gamma校正（0~10，0是关闭）
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_FrameAgc(Frame *pFrame, unsigned short *pGray, float f32Constrast, float f32Bright, float f32MinT, float f32MaxT, STAT_RECT *pRect, unsigned char u8Method, unsigned short u16TFilterCoef);

/*============================================================================
function:	IRSDK_FrameAgcDDE:带DDE功能的区间调光（原始数据转灰度数据）
parameter:	Frame *pFrame：帧结构指针
			unsigned short *pGray: 灰度结果（0~255），为了兼容，类型是ushort
			float f32Constrast:对比度
			float f32Bright：亮度
			float f32MinT : 调光范围最低温阈值；
			float f32MaxT : 调光范围最高温阈值（当f32MinT与f32MaxT相等时，该函数为自动调光）；
			STAT_RECT *pRect：根据提供的矩形区域的最低最高温设定调光范围，当使用这个时，f32MinT和f32MaxT 设置无效，不需要使用时设置为NULL
			unsigned short u16TFilterCoef：时域滤波（0~100，0是关闭，100时域滤波效果最弱，1时效果最好）
			unsigned char u8DDEcoef：DDE系数（0~100，0是关闭）
			unsigned char u8Gamma：gamma校正（0~10，0是关闭）
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_FrameAgcDDE(Frame *pFrame, unsigned short *pGray, float f32Constrast, float f32Bright, float f32MinT, float f32MaxT, STAT_RECT *pRect, unsigned char u8Method, unsigned short u16TFilterCoef, unsigned char u8DDECoef, unsigned char u8Gamma);

/*============================================================================
function:	IRSDK_Gray2Rgb:灰度图像转rgb
parameter:	unsigned short* pGray：输入灰度指针，调用前开辟空间，建议大小(54+4*256)Byte
			unsigned char* pRgb：输出rgb，在调用前开辟空间，格式为bgr888
			unsigned short Width:图像宽度
			unsigned short Height:图像高度
			int PalType：调色板类型（0工业1医疗）
			int Pal: 调色板索引（工业时0~14，医疗时0~6）
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Gray2Rgb(unsigned short* pGray, unsigned char* pRgb, unsigned short Width, unsigned short Height, int PalType, int Pal);

/*============================================================================
function:	IRSDK_GetPaletteJpeg:获取调色板jpg图像
parameter:	unsigned char* pPaletteJpeg：输出jpeg图像，调用前开辟空间，建议大小（256*3)Byte
			unsigned int *pJpegLen：输出对应的有效长度
			unsigned char Method:配置为0
			int PalType：调色板类型（0工业1医疗）
			int Pal: 调色板索引（工业时0~14，医疗时0~6）
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetPaletteJpeg(unsigned char* pPaletteJpeg, unsigned int *pJpegLen, unsigned char Method, int PalType, int Pal);

/*============================================================================
function:	IRSDK_GetPaletteBmp:获取调色板bmp图像
parameter:	unsigned char* pPaletteBmp：输出jpeg图像，调用前开辟空间，建议大小(54+4*256)Byte
			unsigned int *pBmpLen：输出对应的有效长度
			unsigned char Method:配置为0
			int PalType：调色板类型（0工业1医疗）
			int Pal: 调色板索引（工业时0~14，医疗时0~6）
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetPaletteBmp(unsigned char* pPaletteBmp, unsigned int *pBmpLen, unsigned char Method, int PalType, int Pal);

/*============================================================================
function:	IRSDK_GetAnyPointTemp:获取任意点的温度，可用于获取鼠标处的温度
parameter:	Frame *pFrame：输入帧结构指针
STAT_POINT *pPointStat：输入点信息
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetAnyPointTemp(Frame *pFrame, STAT_POINT *pPointStat);

/*============================================================================
function:	IRSDK_GetGlobalTemp:获取全局最高最低温度
parameter:	Frame *pFrame：输入帧结构指针
			STAT_GLOBAL* pGlobalStat：输入全局信息
return:		int:-1：指针异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetGlobalTemp(Frame *pFrame, STAT_GLOBAL* pGlobalStat);

/*============================================================================
function:	IRSDK_GetPointTemp:获取点温度
parameter:	Frame *pFrame：输入帧结构指针
			STAT_POINT *pPointStat：输入点信息
			unsigned char statIndex:索引，当时域滤波不开时，此参数无用，时域滤波一般不需要开
return:		int:-1：指针异常
                0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetPointTemp(Frame *pFrame, STAT_POINT *pPointStat, unsigned char index);

/*============================================================================
function:	IRSDK_GetLineTemp:获取线温度
parameter:	Frame *pFrame：输入帧结构指针
			STAT_LINE *pLineStat：输入线信息
			unsigned char statIndex:索引，当时域滤波不开时，此参数无用，时域滤波一般不需要开
return:		int:-1：指针异常
				0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetLineTemp(Frame *pFrame, STAT_LINE *pLineStat, unsigned char index);

/*============================================================================
function:	IRSDK_GetCircleTemp:获取圆形温度
parameter:	Frame *pFrame：输入帧结构指针
			STAT_CIRCLE *pCircleStat：输入圆信息
			unsigned char statIndex:索引，当时域滤波不开时，此参数无用，时域滤波一般不需要开
return:		int:-1：指针异常
				0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetCircleTemp(Frame *pFrame, STAT_CIRCLE *pCircleStat, unsigned char index);

/*============================================================================
function:	IRSDK_GetRectTemp:获取矩形温度
parameter:	Frame *pFrame：输入帧结构指针
			STAT_RECT *pRectStat：输入矩形信息
			unsigned char statIndex:索引，当时域滤波不开时，此参数无用，时域滤波一般不需要开
return:		int:-1：指针异常
				0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetRectTemp(Frame *pFrame, STAT_RECT *pRectStat, unsigned char index);

/*============================================================================
function:	IRSDK_GetPolygonTemp:获取多边形温度
parameter:	Frame *pFrame：输入帧结构指针
			STAT_POLYGON *pPolygonStat：输入多边形信息
			unsigned char statIndex:索引，当时域滤波不开时，此参数无用，时域滤波一般不需要开
return:		int:-1：指针异常
				0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetPolygonTemp(Frame *pFrame, STAT_POLYGON *pPolygonStat, unsigned char index);

/*============================================================================
function:	IRSDK_GetPointTemp:获取调色板jpg图像
parameter:	Frame *pFrame：输入帧结构指针
			STAT_OBJ *pObjStat：输入测温对象信息
return:		int:-1：指针异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_GetObjTemp(Frame *pFrame, STAT_OBJ *pObjStat);

/*============================================================================
function:	IRSDK_Rgb2Bmp:rgb转bmp图像
parameter:	unsigned char * pBmpData：输出转换后对应的BMP图像缓存，调用前开辟，不小于(54+w * h * 3) Byte
			unsigned int *pLen：输出bmp有效长度
			unsigned char* pRgb：输入rgb指针
			unsigned short Width:图像宽度
			unsigned short Height:图像高度
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Rgb2Bmp(unsigned char * pBmpData, unsigned int *pLen, unsigned char* pRgb, unsigned short Width, unsigned short Height);

/*============================================================================
function:	IRSDK_Rgb2Jpeg:rgb转jpg图像
parameter:	unsigned char * pJpegout：输出转换后对应的JPG图像缓存，调用前开辟，不小于(w*h*3)Byte
			unsigned int *pLen：输出bmp有效长度
			int quality：压缩质量，建议80以上（10~100）
			unsigned char* pRgb：输入rgb指针
			unsigned short Width:图像宽度
			unsigned short Height:图像高度
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_Rgb2Jpeg(unsigned char * pJpegout, unsigned int *pLen, int quality, unsigned char * pRgb, unsigned short Width, unsigned short Height);

/*============================================================================
function:	IRSDK_SaveFrame2Jpeg:存储rgb和温度为jpeg图像
parameter:	char* pFile：文件路径和文件名，不能超过512byte
			Frame *pFrame：帧结构
			unsigned char *pRgb：rgb数据
			unsigned char isSaveObj：是否存储obj
			STAT_OBJ *pObj：obj指针
return:		int:-2：指针异常
				-1：文件操作异常
				0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveFrame2Jpeg(char* pFile, Frame *pFrame, unsigned char* pRgb, unsigned char isSaveObj, STAT_OBJ *pObj);

/*============================================================================
function:	IRSDK_ReadJpeg2Frame: 解析Jpeg图像，获取帧和测温对象
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧
			unsigned char isLoadObj：是否加载测温对象
			STAT_OBJ *pObj：传入测温对象指针
return:		int:-2：指针异常
				-1：文件操作异常
				0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_ReadJpeg2Frame(char *pFile, Frame *pFrame, unsigned char isLoadObj, STAT_OBJ *pObj);

/*============================================================================
function:	IRSDK_SaveFrame2Video: 保存gcv视频
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧
			unsigned char Op：操作
			unsigned char isSaveObj：是否保存测温对象
			STAT_OBJ *pObj：传入测温对象
			unsigned char * pThreadBuf：缓存空间，调用前开辟的空间，需要1024byte，保存文件期间地址保持不变
return:		int:-2：指针异常
				-1：文件异常
				0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveFrame2Video(char *pFile, Frame *pFrame, unsigned char Op, unsigned char isSaveObj, STAT_OBJ *pObj, unsigned char * pThreadBuf);

/*============================================================================
function:	IRSDK_ReadVideo2Frame: 解析gcv视频
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧
			unsigned int Index：帧索引，可以从中间开始
			unsigned char Op：操作方法
			T_SAVE_HEAD *pVideoHead：解析的头信息
			STAT_OBJ *pObj：传入测温对象
			unsigned char * pThreadBuf：缓存空间，调用前开辟的空间，需要1024byte，读文件期间地址保持不变
return :	int:-2：文件打开错误
				-1：格式错误
				0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_ReadVideo2Frame(char *pFile, Frame *pFrame, unsigned int Index, unsigned char Op, T_SAVE_HEAD *pVideoHead, STAT_OBJ *pObj, unsigned char * pThreadBuf);

/*============================================================================
function:	IRSDK_SaveRgb2AVI:rgb图转avi图像
parameter:	char* pFile：文件路径和文件名，不能超过512byte
			unsigned char *pRgb：rgb指针
			unsigned char Op：操作
			int PalType：调色板类型（0工业1医疗）
			int Pal: 调色板索引（工业时0~14，医疗时0~6）
			unsigned char * pThreadBuf：缓存空间，调用前开辟的空间，需要1024byte，保存文件期间地址保持不变
return:		int:0
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveRgb2AVI(char* pFile, unsigned char *pRgb, unsigned short Width, unsigned short Height, unsigned char Op, int quality, unsigned char * pThreadBuf);

/*============================================================================
function:	IRSDK_SaveObj2CSV: 存储温度曲线
parameter:	char *pFile：保存的文件名
			unsigned char Op：操作方法
			STAT_OBJ *pObj：传入测温对象
			STAT_TEMPER *pGlobalTemper：全局温度
			unsigned char * pThreadBuf：用于多线程开辟的空间，至少1024byte，保存文件期间地址保持不变
return:		-2：指针异常
			-1：文件异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveObj2CSV(char *pFile, unsigned char Op, STAT_OBJ *pObj, unsigned char * pThreadBuf);

/*============================================================================
function:	IRSDK_SaveFrame2CSV: 保存帧温度信息为csv
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧结构
			float emit：辐射率，默认0.98
			float dis：距离，默认2
			float reflect：反射温度，默认20.0
return:		-2：指针异常
			-1：文件异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveFrame2CSV(char* pFile, Frame *pFrame, float emit, float dis, float reflect, float tempoffset);

/*============================================================================
function:	IRSDK_SaveMultiFrame2CSV: 保存连续多帧温度信息为csv
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧结构
			float emit：辐射率，默认0.98
			float dis：距离，默认2
			float reflect：反射温度，默认20.0
return:		-2：指针异常
			-1：文件异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveMultiFrame2CSV(char* pFile, unsigned char Op, Frame *pFrame, float emit, float dis, float reflect, float tempoffset, unsigned char * pThreadBuf);

/*============================================================================
function:	IRSDK_SaveLine2CSV: 保存线温度信息为csv
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧结构
			STAT_LINE sLine：线信息
return:		-2：指针异常
			-1：文件异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveLine2CSV(char* pFile, Frame *pFrame, STAT_LINE sLine, unsigned char u8Format);

/*============================================================================
function:	IRSDK_SaveRect2CSV: 保存矩形温度信息为csv
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧结构
			STAT_RECT sRect：矩形信息
return:		-2：指针异常
			-1：文件异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveRect2CSV(char* pFile, Frame *pFrame, STAT_RECT sRect);

/*============================================================================
function:	IRSDK_SaveCircle2CSV: 保存圆温度信息为csv
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧结构
			STAT_CIRCLE sCircle：圆信息
return:		-2：指针异常
			-1：文件异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SaveCircle2CSV(char* pFile, Frame *pFrame, STAT_CIRCLE sCircle);

/*============================================================================
function:	IRSDK_SavePolygon2CSV: 保存行温度信息为csv
parameter:	char *pFile：保存的文件名
			Frame *pFrame：输入帧结构
			STAT_POLYGON sPolygon：多边形信息
return:		-2：指针异常
			-1：文件异常
			0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_SavePolygon2CSV(char* pFile, Frame *pFrame, STAT_POLYGON sPolygon);

/*============================================================================
function:	IRSDK_TempPntCorrect:对某一温度值进行校正，提供的最灵活的一种校正方式
parameter:	float f32Emiss：辐射率(0.01~1.00)
			float f32Reflect：反射温度
			float f32Dis:距离
			float temp：待校正温度值
return:		float: 校正后的温度值
history:	null
==============================================================================*/
IR_SDK_API float IRSDK_TempPntCorrect(float f32Emiss, float f32Reflect, float f32Dis, float temp);


/*============================================================================
function:	IRSDK_MoveCtrl: 运动控制，需要硬件支持
parameter:	int handle：句柄
T_PARAMCTRL mProtocol：协议选择
T_PARAMCTRL mType：控制方式
unsigned int u32Param：控制参数
return:		-1：句柄异常
0：正常
history:	null
==============================================================================*/
IR_SDK_API int IRSDK_MoveCtrl(int handle, T_PARAMCTRL mProtocol, T_PARAMCTRL mType, unsigned int u32Param);

#endif
