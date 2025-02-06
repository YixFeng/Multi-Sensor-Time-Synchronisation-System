#include "customised_camera.h"
#include <opencv2/opencv.hpp>  
#include <opencv2/imgproc.hpp>
#include "data_manager.h"

NET_SERVER_DEVICEINFO CustCamManger::deviceInfo;

CustCamManger::CustCamManger() = default;

CustCamManger::~CustCamManger() { Stop(); }

bool CustCamManger::Initialization() {
    // 自定义相机初始化
    bInit = NET_VIDEO_Init();
    return bInit;
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
    printf("Receive frame over\n\r");
    int ret = NET_VIDEO_StopRealPlay(&deviceInfo);
    if (ret == 0)
    {
        printf("Stop play ok\n\r");
    }
    else
    {
        printf("Stop play fail\n\r");
    }
    ret = NET_VIDEO_Logout(&deviceInfo);
    if (ret == 0)
    {
        printf("Logout ok\n\r");
    }
    else
    {
        printf("Logout fail\n\r");
    }
}

void CustCamManger::Receive(const std::string &name) const {
    ImgData img_data;
    if (!bInit) {
        printf("Camera initialization failed\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    char buffer_8BitImg[BUFFER_SIZE/2];
    int width = 640;
    int height = 512;

    // 使用NET_VIDEO_Login登录相机
    char *ip = "192.168.1.201";
    int port = 8000;
    int ret = NET_VIDEO_Login(ip, port, &deviceInfo);
    if (ret != 0) {
        printf("Camera login failed for %s\n", name.c_str());
        return;
    }

    ret = NET_VIDEO_RealPlay(ip, port, &deviceInfo);
    if (ret != 0) {
        printf("Camera real play failed for %s\n", name.c_str());
        return;
    }

    while (is_running_) {
        // 1. 自定义相机获取图像数据
        int cnt;
        int bResult = NET_VIDEO_RevData((char*)buffer, BUFFER_SIZE, &cnt, &deviceInfo);
        if (bResult != 0)
        {
            printf("no data ret =%d\n\r", bResult);
            continue;
        }
        printf("Received data: %d bytes for %s\n", cnt, name.c_str());
        // bResult整型变量存储是否错误，并使用PlatEqual_2将buffer转化为buffer_8BitImg
        bResult = PlatEqual_2(640, 512, buffer, buffer_8BitImg, 14);

        // 将转化后的8-bit 图像数据转换为 mat
        cv::Mat gray_img(height, width, CV_8UC1, buffer_8BitImg);  
        cv::Mat flipped_img;
        cv::flip(gray_img, flipped_img, 0);  // 0--垂直翻转
        // 2. 将图像数据存入img_data
        img_data.image = flipped_img;
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

