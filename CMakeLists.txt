cmake_minimum_required(VERSION 3.17)
project(Screen-capture)

set(CMAKE_CXX_STANDARD 14)

add_executable(Screen-capture main.cpp ScreenRecorder.cpp ScreenRecorder.h)

if(_WIN32)
  #blocco per windows /Linux
else()              # optional block
    link_libraries(
            avcodec
            avformat
            avfilter
            avdevice
            swresample
            swscale
            avutil
    )
endif()

