#ifndef __Q_IMG_PROC__
#define __Q_IMG_PROC__

#define TRUE	1
#define FALSE	0

//#include "../IpCam/cam_SDK/Clinet_API.h"


#ifdef __cplusplus  
extern "C" {  
#endif  



//extern "C" __declspec(dllexport)  BOOL PlatEqual_2(int Width, int Height, void *pInData, void *pOutData, int wBpps);

//wBpps = 14   14bit 有效
int PlatEqual_2(int Width, int Height, void *pInData, void *pOutData, int wBpps);

 int linearcs(int Width, int Height, void *pInData, void *pOutData, int nMin, int nMax, int wBpps);


#ifdef __cplusplus  
}  
#endif  

#endif
