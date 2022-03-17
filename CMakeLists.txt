cmake_minimum_required(VERSION 3.16)
project(Screen-capture)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")

#threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

IF(WIN32)
    include_directories(
            includes
            C:/Users/cmela/Desktop/wx-v3/
            C:/Users/cmela/Desktop/wx-v3/include
            C:/Users/cmela/Desktop/wx-v3/wxMSW-3.1.5_gcc730_x64_Dev/lib/gcc730_x64_dll/mswud
            C:/Users/cmela/Desktop/wx-v3/wxMSW-3.1.5_gcc730_x64_ReleaseDLL/lib/gcc730_x64_dll


    )

    link_directories(
            C:/Users/cmela/Desktop/wx-v3/wxMSW-3.1.5_gcc730_x64_Dev/lib/gcc730_x64_dll
            C:/Users/cmela/Desktop/wx-v3/wxMSW-3.1.5_gcc730_x64_Dev/lib/gcc730_x64_dll/mswud
            C:/Users/cmela/Desktop/wx-v3/wxMSW-3.1.5_gcc730_x64_ReleaseDLL/lib/gcc730_x64_dll
    )

    set(
            WX_LIBS
            libwxmsw31ud_propgrid.a
            libwxmsw31ud_ribbon.a
            libwxmsw31ud_stc.a
            libwxmsw31ud_webview.a
            libwxmsw31ud_gl.a
            libwxmsw31ud_richtext.a
            libwxmsw31ud_xrc.a
            libwxmsw31ud_aui.a
            libwxmsw31ud_html.a
            libwxmsw31ud_adv.a
            libwxmsw31ud_core.a
            libwxbase31ud_xml.a
            libwxbase31ud_net.a
            libwxbase31ud.a
            libwxscintillad.a
            libwxtiffd.a
            libwxjpegd.a
            libwxpngd.a
            libwxzlibd.a
            libwxregexud.a
            libwxexpatd.a
            libwxmsw31ud_media.a
    )



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

ENDIF(WIN32)

IF(UNIX)
    aux_source_directory(. SRC_LIST)
    set(wxWidgets_CONFIGURATION mswu)
    find_package(wxWidgets COMPONENTS core base REQUIRED)
    include(${wxWidgets_USE_FILE})


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

    find_path(ALSA_INCLUDE_DIR  alsa/asoundlib.h)
    find_library(ALSA_LIBRARY asound)
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

    include_directories(
            BEFORE /usr/X11R6/include /usr/X11R6/lib
    )
ENDIF(APPLE)


add_executable(${PROJECT_NAME} ${SRC_LIST} ScreenRecorder.cpp ScreenRecorder.h ScreenCaptureInterface.h ScreenCaptureInterface.cpp main.cpp main.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR}
        ${SWRESAMPLE_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR} ${ALSA_INCLUDE_DIR})



IF(APPLE)
    target_link_libraries(Screen-capture PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} Threads::Threads
            ${SWRESAMPLE_LIBRARY} ${SWSCALE_LIBRARY} -L/usr/X11R6/lib -I/usr/X11R6/include -lX11)
ELSE()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} Threads::Threads
                   ${SWRESAMPLE_LIBRARY} ${SWSCALE_LIBRARY} ${wxWidgets_LIBRARIES} ${ALSA_LIBRARY} "-lX11")
ENDIF(APPLE)

IF(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} Threads::Threads
            ${SWRESAMPLE_LIBRARY} ${SWSCALE_LIBRARY} ${WX_LIBS} )
ENDIF(WIN32)


#add_executable(${PROJECT_NAME} ${SRC_LIST} ScreenCaptureInterface.cpp ScreenCaptureInterface.h main.h)
#target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})