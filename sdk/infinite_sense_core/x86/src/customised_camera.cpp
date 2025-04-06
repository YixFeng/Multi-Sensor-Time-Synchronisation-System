#include "customised_camera.h"
#include <opencv2/opencv.hpp>  
#include <opencv2/imgproc.hpp>
#include "data_manager.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "IRSDK.h"
#include <unistd.h>
NET_SERVER_DEVICEINFO CustCamManger::deviceInfo;

CustCamManger::CustCamManger() = default;

CustCamManger::~CustCamManger() { Stop(); }

unsigned short gray[MAX_W * MAX_H];
unsigned char rgb[MAX_W * MAX_H * 3];
unsigned long recv_index = 0;
unsigned int save_index = 0;
char filename[128];
T_DEVICE_INFO sInfo;
ImgData img_data;

int FrameCallBack(void* lData, void* lParam) {
    Frame* pFrame = (Frame*)lData;
    printf("index=%lu width=%d height=%d center_temp=%.1f\n", 
           recv_index, pFrame->width, pFrame->height, 
           CALTEMP(pFrame->buffer[(pFrame->width + 1) * pFrame->height / 2], pFrame->u8TempDiv));

    // 转换为灰度数据
    IRSDK_Frame2Gray(pFrame, gray, 50, 50, 0);
    // 将16bit灰度数据转换为RGB
    // IRSDK_Gray2Rgb(gray, rgb, pFrame->width, pFrame->height, 0, 0);
    img_data.image = cv::Mat(pFrame->height, pFrame->width, CV_8UC1);

    for(int i = 0; i < pFrame->width * pFrame->height; i++) {
        img_data.image.data[i] = gray[i] & 0xFF;
    }
    // 每隔300帧保存一次图像
    if (recv_index++ % 300 == 0) {
        sprintf(filename, "grab_%03d.jpg", save_index++);
        printf("Saving: %s\n", filename);
        cv::imwrite(filename, img_data.image);
    }

    // 首次运行时打印设备信息
    if (recv_index == 1) {
        int ret = IRSDK_InquireDeviceInfo(0, &sInfo);
        if (!ret) {
            printf("Model = %s\n", sInfo.Model);
            printf("IP = %s\n", sInfo.IP);
            printf("Mac = %s\n", sInfo.Mac);
        }
    }
    return 0;
}

bool CustCamManger::Initialization() {
    // 自定义相机初始化
}

// 根据相机数量，构造对应数量的线程，读取相机
void CustCamManger::Start() {
    // 假设有两个相机
    const std::string cam_1 = "cam1";
    cam_threads_.emplace_back(&CustCamManger::Receive, this, cam_1);
    Enable();
}

void CustCamManger::Stop() {
    // 1. 关闭相机线程
    Disable();
    std::this_thread::sleep_for(std::chrono::milliseconds{500});
    for (auto &cam_thread : cam_threads_) {
        while (cam_thread.joinable()) {
            cam_thread.join();
        }
    }
    cam_threads_.clear();
    cam_threads_.shrink_to_fit();
    // 2. 关闭相机, 自定义相机关闭

}

void CustCamManger::Receive(const std::string &name) const {
    while (is_running_) {
        // 1. 自定义相机获取图像数据
        printf("Run Node Successfully\n");
        T_IPADDR IpInfo[DEVICE_MAX];		//定义接收 ip 信息的数组
        //需要初如化
        IRSDK_Init();
        memset(IpInfo, 0, sizeof(IpInfo)); //清零
        IRSDK_SetIPAddrArray(IpInfo); //设置接收 IP 的空间

    #if 0
    // 模式1: 自动搜索相机
        while (IpInfo[0].totalOnline == 0) sleep(10000);	
    #else
    // 模式2: 手动指定相机IP和端口
        memcpy(IpInfo[0].IPAddr, "192.168.1.13", sizeof("192.168.1.13"));  //手动指定ip
        IpInfo[0].DataPort = 13 * 10 + 30005; //自定义端口
        IpInfo[0].Index = 0;
        IpInfo[0].isValid = 1;
        IpInfo[0].totalOnline = 1;
    #endif
        CBF_IR pCBFframe = &FrameCallBack;	//回调函数使用
        printf("connect ip: %s\n", IpInfo[0].IPAddr);
        int ret =  IRSDK_Create(0, IpInfo[0], pCBFframe, NULL, NULL, NULL);
        printf("ret = %d", ret);
        IRSDK_Connect(0); //连接，连接成功后回调就可以接收到数据

        // 2. 将图像数据存入img_data
        // 在回调函数中实现存入
        // 3. 自定义相机获取曝光时间
        float expose_time_us = 1e4;
        // 4. 将曝光时间存入img_data t = t_trigger + t_expose / 2
        img_data.time_stamp_us = DataManger::GetInstance().GetLastTiggerTime() + static_cast<uint64_t>(expose_time_us / 2.);
        // 5. 设置相机名字
        img_data.camera_name = name;
        // img_data.image 是要存储的图像
        // 6. 将img_data存入DataManger
        DataManger::GetInstance().AddCamData(name, img_data);
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
    }
}


