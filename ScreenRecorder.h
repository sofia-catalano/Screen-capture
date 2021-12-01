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
    ScreenRecorder(VideoInfo vi);
	~ScreenRecorder();


    private:
    //video variables
    VideoInfo vi;
    AVFormatContext *format_context, *out_format_context;
    const AVOutputFormat *output_format;
  
    const AVCodec *av_encodec, *av_decodec;//This registers all available file formats and codecs with the library so they will be used automatically when a file with the corresponding format/codec is opened.Vecodec;
    const AVInputFormat *input_format;
    AVDictionary *options;
    int video_index;

    AVCodecContext *codec_context, *out_codec_context;
    AVCodecParameters *codec_parameters;


    AVStream *video_st;

    AVPacket *pPacket;
    AVFrame *pFrame, *outFrame;
    SwsContext* sws_ctx;


    //functions
    void initializeInputSource();
    void initializeOutputSource();
    int captureVideoFrames();

};


#endif //PROGETTOPDS_SCREENRECORDER_H
