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
#include <condition_variable>
#include <queue>
#include <thread>
#include "Devices.h"

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
    int framerate;
    int width;
    int height;
    int offset_x;
    int offset_y;
    bool fullscreen;
    string output_file;

} VideoInfo;

enum state{
    START,RECORDING,PAUSE,STOP
};


class ScreenRecorder {
public:
    ScreenRecorder(VideoInfo vi,string audio_device);
    ~ScreenRecorder();

    void recording();
    void pause();
    void stop_recording();


private:
    condition_variable video_cv;
    condition_variable audio_cv;
    //COMMON VARIABLE FOR VIDEO AND AUDIO
    AVFormatContext  *out_format_context;

    //VIDEO VARIABLES
    VideoInfo vi;//struct which contains all the video info to be grabbed
    AVFormatContext *video_in_format_context;

    int current_video_pts=0;
    int current_audio_pts=0;



#if defined( __APPLE__) || defined (_WIN32)
    const AVInputFormat *video_input_format;
    const AVOutputFormat *output_format;
    const AVCodec *video_encodec, *video_decodec;//This registers all available file formats and codecs with the library so they will be used automatically when a file with the corresponding format/codec is opened.Vecodec;
#else
    AVInputFormat *video_input_format;
    AVOutputFormat *output_format;
    AVCodec *video_encodec, *video_decodec;//This registers all available file formats and codecs with the library so they will be used automatically when a file with the corresponding format/codec is opened.Vecodec;
#endif
    AVDictionary *video_options;
    int in_video_index, out_video_index;

    AVCodecContext *video_in_codec_context, *video_out_codec_context;
    AVCodecParameters *video_codec_parameters;



    AVStream *video_st;

    AVPacket *inVideoPacket, *outVideoPacket;
    AVFrame *inVideoFrame, *outVideoFrame;
    SwsContext *sws_ctx;
    mutex lockwrite;
    mutex current_video_state;
    mutex current_audio_state;
    mutex current_pts_synch;
    state program_state;

    //thread t_reading_video; da eliminare
    thread t_converting_video;
    thread t_converting_audio;






    void initializeOutputContext();
    void initializeOutputMedia();
    void reset_video();

    //-----video  functions
    void initializeVideoResources();//Invoke all the required methods (below) in order to initialize all the video resources
    void initializeVideoInput();
    void initializeVideoOutput();
    void initializeVideoCapture();
    void free_audio_resources();

    //implementation video grabbing and converting
    void convert_video_format();



/********* AUDIO VARIABLE **********/
    string audio_device;
    AVFormatContext *in_audio_format_context;
    AVDictionary *audio_options;
#if defined( __APPLE__) || defined (_WIN32)
    const AVInputFormat *audio_input_format;
#else
    AVInputFormat *audio_input_format;
#endif
    int in_audio_index, out_audio_index;
    AVCodecParameters *audio_codec_parameters;
    AVCodecContext *audio_in_codec_context, *audio_out_codec_context;
    const AVCodec *audio_encodec, *audio_decodec;
    AVStream *audio_st;
    AVAudioFifo *audio_buffer;

    //-----audio  functions
    void initializeAudioResources();
    void initializeAudioInput();
    void initializeAudioOutput();
    void reset_audio();

    //implementation audio grabbing and converting
    void convert_audio_format();

};


#endif //PROGETTOPDS_SCREENRECORDER_H
