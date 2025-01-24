#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/Imu.h>

#include "udp_manager.h"
#include "cam_manager.h"
#include "customised_camera.h"
#include "data_manager.h"
#include "Fusion.h"

FusionAhrs ahrs;
inline ros::Time CreateRosTimestamp(const uint64_t mico_sec) {
  uint32_t nsec_per_second = 1e9;
  auto u64 = mico_sec * 1000;
  uint32_t sec = u64 / nsec_per_second;
  uint32_t nsec = u64 - (sec * nsec_per_second);
  return {sec, nsec};
}

void PublishIMUData(const ros::Publisher& pub, const ImuData& imudata) {

  FusionVector gyroscope = {imudata.gx, imudata.gy, imudata.gz};
  FusionVector accelerometer = {imudata.ax, imudata.ay, imudata.az};
  FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer, 0.005f);
  FusionQuaternion q = FusionAhrsGetQuaternion(&ahrs);

  sensor_msgs::Imu imu_msg_data;
  imu_msg_data.header.frame_id = "/base_imu_link";
  imu_msg_data.header.stamp = CreateRosTimestamp(imudata.time_stamp_us);

  imu_msg_data.angular_velocity.x = imudata.gx;
  imu_msg_data.angular_velocity.y = imudata.gy;
  imu_msg_data.angular_velocity.z = imudata.gz;

  imu_msg_data.linear_acceleration.x = imudata.ax;
  imu_msg_data.linear_acceleration.y = imudata.ay;
  imu_msg_data.linear_acceleration.z = imudata.az;

  imu_msg_data.orientation.w = q.array[0];
  imu_msg_data.orientation.x = q.array[1];
  imu_msg_data.orientation.y = q.array[2];
  imu_msg_data.orientation.z = q.array[3];

  pub.publish(imu_msg_data);
}
void SigIntHandler(int sig) {
  ros::shutdown();  // 让ROS节点安全退出
}
int main(int argc, char** argv) {
    ros::init(argc, argv, "udp_demo",ros::init_options::NoSigintHandler);
    ros::NodeHandle node;
    FusionAhrsInitialise(&ahrs);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    /*
      本机IP地址应该在192.168.1.X网段下。
      192.168.1.168是同步板的IP地址
      The host IP address should be in the 192.168.1.X network segment.
      192.168.1.168 is the IP address of the synchronization board.
    */
    auto udp_manager = std::make_shared<UdpManager>("192.168.1.168", 8888);
    udp_manager->Start();

    ros::Publisher imu_pub = node.advertise<sensor_msgs::Imu>("/imu_sync_board", 1000);
    CamManger::GetInstance().Initialization();
    CamManger::GetInstance().Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    image_transport::ImageTransport it(node);
    std::vector<std::string> all_cam_names;
    DataManger::GetInstance().GetAllCamNames(all_cam_names);
    std::cout << "Number of cameras detected " << all_cam_names.size() << std::endl;
    std::map<std::string, image_transport::Publisher> pub_list;
    for (auto& cam : all_cam_names) {
        pub_list[cam] = it.advertise(cam, 30);
    }
    ros::Rate loop_rate(1000);
    ImuData imudata{};
    while (node.ok()) {
        while (DataManger::GetInstance().GetNewImuData(imudata)) {
            PublishIMUData(imu_pub, imudata);
        }
        ImgData img_data{};
        for (auto& cam : all_cam_names) {
            while (DataManger::GetInstance().GetNewCamData(cam, img_data)) {
                sensor_msgs::ImagePtr msg =
                // mono8:灰度类型,bgr8:彩图，具体需要根据相机类型进行修改
                        cv_bridge::CvImage(std_msgs::Header(), "mono8", img_data.image.clone()).toImageMsg();
                msg->header.stamp = CreateRosTimestamp(img_data.time_stamp_us);
                pub_list[cam].publish(msg);
            }
        }
        ros::spinOnce();  // 确保处理ROS回调
        loop_rate.sleep();
    }
    udp_manager->Stop();
    CamManger::GetInstance().Stop();
    return 0;
}
