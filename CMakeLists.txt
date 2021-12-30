cmake_minimum_required(VERSION 3.16)
project(Screen-capture)

aux_source_directory(. SRC_LIST)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})






set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")

#threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#set(CMAKE_THREAD_LIBS_INIT "-lpthread")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#set(CMAKE_HAVE_THREADS_LIBRARY 1)
#set(CMAKE_USE_WIN32_THREADS_INIT 0)
#set(CMAKE_USE_PTHREADS_INIT 1)
#set(THREADS_PREFER_PTHREAD_FLAG ON)

IF(UNIX)
    find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
    find_library(AVCODEC_LIBRARY avcodec)

    find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
    find_library(AVFORMAT_LIBRARY avformat)

    find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
    find_library(AVDEVICE_LIBRARY avdevice)

    find_path(AVFORMAT_INCLUDE_DIR libavformat/avio.h)
    find_library(AVFORMAT_LIBRARY avformat)

    find_path(AVUTIL_INCLUDE_DIR libavutil/audio_fifo.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(AVUTIL_INCLUDE_DIR libavutil/imgutils.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(AVUTIL_INCLUDE_DIR libavutil/opt.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(AVUTIL_INCLUDE_DIR libavutil/audio_fifo.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)
    find_library(SWRESAMPLE_LIBRARY swresample)

    find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
    find_library(SWSCALE_LIBRARY swscale)
ENDIF(UNIX)

IF(APPLE)
    find_path(AVCODEC_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavcodec/avcodec.h)
    find_library(AVCODEC_LIBRARY avcodec)

    find_path(AVFORMAT_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavformat/avformat.h)
    find_library(AVFORMAT_LIBRARY avformat)

    find_path(AVUTIL_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavutil/avutil.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(AVDEVICE_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavdevice/avdevice.h)
    find_library(AVDEVICE_LIBRARY avdevice)

    find_path(AVFORMAT_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavformat/avio.h)
    find_library(AVFORMAT_LIBRARY avformat)

    find_path(AVUTIL_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavutil/audio_fifo.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(AVUTIL_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavutil/imgutils.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(AVUTIL_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavutil/opt.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(AVUTIL_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libavutil/audio_fifo.h)
    find_library(AVUTIL_LIBRARY avutil)

    find_path(SWRESAMPLE_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libswresample/swresample.h)
    find_library(SWRESAMPLE_LIBRARY swresample)

    find_path(SWSCALE_INCLUDE_DIR /opt/homebrew/Cellar/ffmpeg/4.4.1-with-options_2/include/libswscale/swscale.h)
    find_library(SWSCALE_LIBRARY swscale)
ENDIF(APPLE)
add_executable(${PROJECT_NAME} ${SRC_LIST} ScreenRecorder.cpp ScreenRecorder.h ScreenCaptureInterface.h ScreenCaptureInterface.cpp main.cpp main.h)

target_include_directories(${PROJECT_NAME} PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR}
        ${SWRESAMPLE_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} Threads::Threads
        ${SWRESAMPLE_LIBRARY} ${SWSCALE_LIBRARY} ${wxWidgets_LIBRARIES} )

#add_executable(${PROJECT_NAME} ${SRC_LIST} ScreenCaptureInterface.cpp ScreenCaptureInterface.h main.h)
#target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})