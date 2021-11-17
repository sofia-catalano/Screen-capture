//
// Created by Sofia Catalano on 08/10/21.
//

#include "ScreenRecorder.h"
#include <exception>
#include <iostream>

using namespace std;


ScreenRecorder::ScreenRecorder(VideoInfo vi) : vi(vi){
    initializeInputSource();
    cout << "End initializeInputSource" << endl;

    initializeOutputSource();
    cout << "End initializeOutputSource" << endl;

    cout << "All required functions are registered successfully" << endl;
}


void ScreenRecorder::initializeInputSource(){
    avdevice_register_all();

    options = nullptr;

    //allocate memory to the component AVFormatContext that will hold information about the format
    format_context = nullptr;
    format_context = avformat_alloc_context();

    input_format = av_find_input_format("x11grab");
    if (input_format == nullptr) {
        throw logic_error{"av_find_input_format not found..."};
    }

    // TODO risolvere il fullscreen
    //https://unix.stackexchange.com/questions/573121/get-current-screen-dimensions-via-xlib-using-c
    //https://www.py4u.net/discuss/81858


    //AVDictionary to inform avformat_open_input and avformat_find_stream_info about all settings
    //options is a reference to an AVDictionary object that will be populated by av_dict_set
    if(av_dict_set(&options, "framerate", to_string(vi.framerate).c_str(), 0) < 0){
        throw logic_error{"Error in setting dictionary value"};
    }
    if( av_dict_set(&options, "video_size", (to_string(vi.width) + "*" + to_string(vi.height)).c_str(), 0) < 0){
        throw logic_error{"Error in setting dictionary value"};
    }
    if( av_dict_set(&options, "probesize", "30M", 0) <0){  //forse serve al demuxer
        throw logic_error{"Error in setting dictionary value"};
    }

    // open the file, read its header and fill the AVFormatContext format_context with minimal information about the format
    //TODO cambiare con offset_x offeset_y width e height
    if(avformat_open_input(&format_context, ":0.0+10,250", input_format, &options) != 0){
         throw logic_error{"Error in opening input stream"};
    }

    // avformat_find_stream_info: we need to read data from the media to access the stream
    // Now, the format_context->nb_streams will hold the amount of streams
    // and the format_context->streams[i] will give us the i stream (an AVStream).
    if (avformat_find_stream_info(format_context, &options) < 0) {
        throw logic_error{"Error in finding stream information"};
    }


    video_index = -1;
    // loop through all the streams
    for (int i = 0; i < format_context->nb_streams; i++){ // find video stream position/index
        if( format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ){
	        video_index = i;
	        break;
	    }
    }

    if (video_index == -1) {
        throw logic_error{"Error in finding a video stream"};
    }

    //get the properties of a codec used by the stream video_index
    codec_parameters = format_context->streams[video_index]->codecpar;

    //with the parameters, look up the proper codec querying with the function avcodec_find_decoder
    //it will find the registered decoder for the codec id and return an AVCodec
    av_decodec = avcodec_find_decoder(codec_parameters->codec_id);
    if(av_decodec == nullptr){
        throw logic_error{"Error in finding the decoder"};
    }

    //allocate memory for the AVCodecContext
    //it will hold the context for the decode/encode process
    codec_context = avcodec_alloc_context3(nullptr);

    //fill this codec context with CODEC parameters using avcodec_parameters_to_context
     avcodec_parameters_to_context(codec_context, codec_parameters);

    //open the codec
    //Initialize the AVCodecContext to use the given AVCodec.
    if (avcodec_open2(codec_context , av_decodec , NULL) < 0) {
        throw logic_error{"Error in opening the av codec"};
    }



}


void ScreenRecorder::initializeOutputSource() {
    /* Returns the output format in the list of registered output formats
    which best matches the provided parameters, or returns NULL if there is no match. */
    output_format = nullptr;
    output_format = av_guess_format(NULL, vi.output_file.c_str(), NULL);

    //TODO capire null e nullptr
    if (output_format == NULL) {
        throw runtime_error{"Error in matching the video format"};
    }

    //allocate memory to the component AVFormatContext that will hold information about the output format
    avformat_alloc_output_context2(&out_format_context, output_format, output_format->name, vi.output_file.c_str());

    av_encodec = avcodec_find_encoder(AV_CODEC_ID_H264); //Abdullah: AV_CODEC_ID_MPEG4
    if (!av_encodec) {
        throw logic_error{"Error in allocating av format output context"};
    }

    //create new out stream into the output format context
    video_st = avformat_new_stream(out_format_context, NULL);
    if( !video_st )
    {
        throw runtime_error{"Error creating a av format new stream"};
    }

    out_codec_context = avcodec_alloc_context3(outAVCodec);
    if( !out_codec_context){
        throw runtime_error{error in allocating the codec contexts};
    }

    //avcodec_parameters_to_context: fill this codec context with CODEC parameters
    avcodec_parameters_to_context(out_codec_context, video_st->codecpar);

    // set property of the video file
    out_codec_context->codec_id = AV_CODEC_ID_H264; // AV_CODEC_ID_MPEG4
    out_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
    out_codec_context->pix_fmt  = AV_PIX_FMT_YUV420P;
    out_codec_context->bit_rate = 400000; // 80000
    out_codec_context->width = 1920; // (int)(rrs.width * vs.quality) / 32 * 32;
    out_codec_context->height = 1080; // (int)(rrs.height * vs.quality) / 2 * 2;
    out_codec_context->gop_size = 3; //50
    out_codec_context->max_b_frames = 2;
    out_codec_context->time_base.num = 1;
    out_codec_context->time_base.den = 30; // vi.framerate
    /*  out_codec_context->qmin = 5;
        out_codec_context->qmax = 10;
        out_codec_context->max_b_frames = 2;
    */
    if (out_codec_context->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(out_codec_context, "preset", "ultrafast", 0); //slow
       /* av_opt_set(out_codec_context, "tune", "zerolatency", 0);
        av_opt_set(out_codec_context, "cabac", "1", 0);
        av_opt_set(out_codec_context, "ref", "3", 0);
        av_opt_set(out_codec_context, "deblock", "1:0:0", 0);
        av_opt_set(out_codec_context, "analyse", "0x3:0x113", 0);
        av_opt_set(out_codec_context, "subme", "7", 0);
        av_opt_set(out_codec_context, "chroma_qp_offset", "4", 0);
        av_opt_set(out_codec_context, "rc", "crf", 0);
        av_opt_set(out_codec_context, "rc_lookahead", "40", 0);
        av_opt_set(out_codec_context, "crf", "10.0", 0);*/
    }

    /* Some container formats (like MP4) require global headers to be present
	   Mark the encoder so that it behaves accordingly. */
    if ( out_codec_context->oformat->flags & AVFMT_GLOBALHEADER){
        out_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    //open the codec
    //Initialize the AVCodecContext to use the given AVCodec.
    if (avcodec_open2(out_codec_context, av_encodec, NULL) < 0) {
        throw logic_error{"Error in opening the av codec"};
    }


}