cmake_minimum_required(VERSION 3.16)
project(udp_demo)

set(CMAKE_CXX_STANDARD 14)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O3  -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3  -Wall")

if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
  set(SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/infinite_sense_core/arm)
else (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
  set(SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../sdk/infinite_sense_core/x86)
endif ()


find_package(catkin QUIET)
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV Version: "${OpenCV_VERSION})

if (catkin_FOUND)
  message(STATUS "ROS found, Compiling ROS nodes")

  find_package(catkin REQUIRED COMPONENTS
      roscpp
      sensor_msgs
      cv_bridge
      image_transport
      geometry_msgs
  )

  file(GLOB_RECURSE SOURCE_FILES
      main_ros.cpp
  )

  set(IR_CAM_LIB_PATH ${SDK_PATH}/lib/libImgProc.a)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SDK_PATH}/lib)

  add_library(ir_depends SHARED
    ${SDK_PATH}/src/Clinet_API.cpp
    ${SDK_PATH}/src/NetSocket.cpp
    ${SDK_PATH}/src/customised_camera.cpp
  )
  target_include_directories(ir_depends PRIVATE
    ${SDK_PATH}/include
    ${OpenCV_INCLUDE_DIRS}
  )
  target_link_libraries(ir_depends PRIVATE ${IR_CAM_LIB_PATH})

  add_executable(${PROJECT_NAME}_ros_node ${SOURCE_FILES})
  target_link_directories(${PROJECT_NAME}_ros_node PRIVATE
      BEFORE ${SDK_PATH}/lib
  )
  target_link_libraries(${PROJECT_NAME}_ros_node
      MvCameraControl
      fusion
      infinite_sense_core
      ir_depends
      ${OpenCV_LIBS}
      ${catkin_LIBRARIES}
  )
  target_include_directories(${PROJECT_NAME}_ros_node PRIVATE
      ${catkin_INCLUDE_DIRS}
      ${SDK_PATH}/include
      ${OpenCV_INCLUDE_DIRS}
  )
  target_compile_definitions(${PROJECT_NAME}_ros_node PUBLIC -DROS_PLATFORM)
endif ()

