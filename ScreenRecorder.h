//
// Created by Sofia Catalano on 08/10/21.
//

#ifndef PROGETTOPDS_SCREENRECORDER_H
#define PROGETTOPDS_SCREENRECORDER_H
#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <math.h>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;

extern "C"
{
#include "libavformat/avio.h"
#include "libavutil/audio_fifo.h"
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}

typedef struct
{
    int framerate;   //30 ????
    int capturetime_seconds;
    float quality;  //value between 0.1 and 1
    int width;
    int height;
    int offset_x;
    int offset_y;
    int screen_number;
    bool fullscreen; //copiato da vedere
    string output_file;

} VideoInfo;

enum class status{
    record, pause, stop
};


class ScreenRecorder {
public:
    ScreenRecorder(VideoInfo vi, bool audio);
    ~ScreenRecorder();
    void recording();

private:

    /**********VIDEO VARIABLES***********/
    VideoInfo vi;//struct which contains all the video info to be grabbed
    AVFormatContext *video_format_context;


#ifdef __APPLE__
    const AVInputFormat *video_input_format;
    const AVCodec *video_encodec, *video_decodec;//This registers all available file formats and codecs with the library so they will be used automatically when a file with the corresponding format/codec is opened.Vecodec;
#endif
#ifdef __linux__
    AVInputFormat *video_input_format;
    AVCodec *video_encodec, *video_decodec;//This registers all available file formats and codecs with the library so they will be used automatically when a file with the corresponding format/codec is opened.Vecodec;
#endif
#ifdef _WIN32
    const AVInputFormat *video_input_format;
    const AVCodec *video_encodec, *video_decodec;
#endif
    AVDictionary *video_options;
    int video_index, out_video_index;

    AVCodecContext *video_in_codec_context, *video_out_codec_context;
    AVCodecParameters *video_codec_parameters;


    AVStream *video_st;

    AVPacket *inPacket, *inPacket2, *outPacket;//inPacket is needed for the thread which read packets and push it into the queue
    //while inPacket2 is needed for the function which decode and encode, to extract the packet from the queue
    AVFrame *inFrame, *outFrame;
    SwsContext *sws_ctx;
    queue<AVPacket *> inPacket_video_queue;
    mutex inPacket_video_mutex;
    bool end_reading;

    unique_ptr<thread> t_reading_video;
    unique_ptr<thread> t_converting_video;

    //-----video  functions
    void initializeVideoResources();//Invoke all the required methods (below) in order to initialize all the video resources
    void initializeVideoInput();
    void initializeVideoOutput();
    void initializeVideoCapture();

    //implementation video grabbing and converting
    void read_packets();
    void convert_video_format();

/********* AUDIO VARIABLE **********/
    bool audio;
    AVFormatContext *audio_format_context;
    AVDictionary *audio_options;
    const AVInputFormat *audio_input_format;
    int audio_index, out_audio_index;
    AVCodecParameters *audio_codec_parameters;
    AVCodecContext *audio_in_codec_context, *audio_out_codec_context;
    const AVCodec *audio_encodec, *audio_decodec;
    AVStream *audio_st;

    //-----audio functions
    void initializeAudioResources();
    void initializeAudioInput();
    void initializeAudioOutput();

/********* COMMON VARIABLE **********/
    AVFormatContext *out_format_context;
    #if defined (__APPLE__) || defined (_WIN32)
        const AVOutputFormat *output_format;
    #elif __linux__
        AVOutputFormat *output_format;
    #endif

    //-----common functions
    void initializeOutputContext();
    void initializeOutputMedia();
};


#endif //PROGETTOPDS_SCREENRECORDER_H