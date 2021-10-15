cmake_minimum_required(VERSION 3.17)
project(Screen-capture)

set(CMAKE_CXX_STANDARD 14)

add_executable(Screen-capture main.cpp ScreenRecorder.cpp ScreenRecorder.h)

link_libraries(
        avcodec
        avformat
        avfilter
        avdevice
        swresample
        swscale
        avutil
)


target_include_directories(Screen-capture PRIVATE
        ${AVCODEC_INCLUDE_DIR}
        ${AVFORMAT_INCLUDE_DIR}
        ${AVUTIL_INCLUDE_DIR}
        ${AVDEVICE_INCLUDE_DIR})

target_link_libraries(Screen-capture PRIVATE
        ${AVCODEC_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${AVDEVICE_LIBRARY})

