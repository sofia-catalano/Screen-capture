//
// Created by Sofia Catalano on 08/10/21.
//

#include "ScreenRecorder.h"
#include <exception>
#include <iostream>

#include "Devices.h"

using namespace std;


ScreenRecorder::ScreenRecorder(VideoInfo v, string ad) {

    cout<<"vi->"<<v.output_file<<endl<<"recordAudio->"<<ad<<endl;

#ifdef _WIN32
    vi.framerate = 15;
#elif __linux__
    vi.framerate = 35;
#elif __APPLE__
    vi.framerate = 30;
#endif

    try{
        /*check if audio device is  correct*/
        bool audio = false;
        if(audio_device != "none"){
            result r = getAudioDevices();
            for(int i = 0; i< r.n; i++) {
                if (r.devices[i] == audio_device) {
                    audio = true;
                    break;
                }
            }
            if(!audio){
                //throw eccezione TODO ECCEZIONE DA DEFINIRE
                /*nella gestione dell'eccezione nel MAIN.cpp:
                cout<<"Inserire dispositivo audio tra quelli elencati :"<<endl;
                funzione_che_stampa_i_devices();
                cin<<stringa_audio;*/
            }
        }

        /*check if screen size are correct*/
        screen_size size = getScreenSize();
        cout<<"Size:"<<size.x<<","<<size.y<<endl;
        vi.width = vi.width - vi.offset_x;
        vi.height = vi.height - vi.offset_y;
        if(vi.width%2 != 0) vi.width-=1;
        if(vi.height%2 != 0) vi.height-=1;
        if(vi.width <= 0 || vi.height <= 0 || vi.offset_x < 0 || vi.offset_y < 0 ||
                (vi.offset_x + vi.width) > size.x || (vi.offset_y + vi.height) > size.y){
            //throw eccezione TODO ECCEZIONE DA DEFINIRE
            /*nella gestione dell'eccezione nel MAIN.cpp:
           cout<<Reinserire parametri schermo corretti, con limite massimo dimensioni_schermo.x dimensioni_schermo.y
                      width height starting point x and y<<endl;
            cin<<vi.width vi.height vi.offset_x vi.offset_y;
             */
        }
        //SETTARE SE DATI CORRETTI
        vi = v;
        audio_device = ad;

        program_state=START;
        initializeOutputContext();
        initializeVideoResources();

        if( audio_device != "none"){
            initializeAudioResources();
        }
        initializeOutputMedia();

    }catch(exception &err){
        cout << "All required functions are registered not successfully" << endl;
        cout << (err.what()) << endl;
    }
}

ScreenRecorder::~ScreenRecorder(){
    try {
        cout << "Inizio distruttore" << endl;
        if (t_converting_video.joinable())
            t_converting_video.join();
        if (t_converting_audio.joinable())
            t_converting_audio.join();

        //free video resources
        av_write_trailer(out_format_context);
        avformat_close_input(&video_in_format_context);
        avformat_free_context(video_in_format_context);
        avio_close(out_format_context->pb);
        avcodec_close(video_in_codec_context);
        avcodec_free_context(&video_in_codec_context);
        avcodec_close(video_out_codec_context);
        avcodec_free_context(&video_out_codec_context);

        av_packet_free(&outVideoPacket);
        av_frame_free(&inVideoFrame);
        av_frame_free(&outVideoFrame);

        cout << "Fine blocco try" << endl;
        //fre audio resources
        if( audio_device != "none") {
            //free_audio_resources();
            //av_freep(audio_buffer);

            //input
            avformat_close_input(&in_audio_format_context);
            avformat_free_context(in_audio_format_context);
            avcodec_close(audio_in_codec_context);
            avcodec_free_context(&audio_in_codec_context);

            //output
            av_audio_fifo_free(audio_buffer);
            avcodec_close(audio_out_codec_context);
            avcodec_free_context(&audio_out_codec_context);
            avformat_free_context(in_audio_format_context);


        }
        //avformat_close_inpuft(&out_format_context);
        //avformat_free_context(out_format_context);

        cout << "Distruttore Screen Recorder " << endl;
    }catch(exception e){
        cout << "Distruttore Screen Recorder exception" << endl;
        cout<<e.what()<<endl;
    }
    cout<<"fine distruttore";
}

void ScreenRecorder::initializeOutputContext(){
    /**
     * av_guess_format:
     * @return {AVOutputFormat} the output format in the list of registered output formats which best matches the provided parameters
     * @return {null} if there is no match
     *
     * avformat_alloc_output_context2: allocate memory to the component AVFormatContext that will contain
     * information about the output format
     */
    output_format = nullptr;
    output_format = av_guess_format(nullptr, vi.output_file.c_str(), nullptr);

    if (output_format == nullptr) {
        throw runtime_error{"Error in matching the video format"};
    }

    avformat_alloc_output_context2( &out_format_context, output_format, output_format->name, vi.output_file.c_str());
}

void ScreenRecorder::initializeVideoResources() {
    initializeVideoInput();
    cout << "End initializeVideoInput" << endl;

    initializeVideoOutput();
    cout << "End initializeVideoOutput" << endl;

    initializeVideoCapture();
    cout << "End initializeVideoCapture" << endl;
}

void ScreenRecorder::initializeAudioResources(){
    initializeAudioInput();
    cout << "End initializeAudioInput" << endl;

    initializeAudioOutput();
    cout << "End initializeAudioOutput" << endl;
}

void ScreenRecorder::initializeVideoInput(){
    /**
    * avdevice_register_all(): registers all available file formats and codecs with the library
    * so they will be used automatically when a file with the corresponding format/codec is opened
    * need to be call once
    *
    * avformat_alloc_context(): allocate memory to the component AVFormatContext that will contain
    * information about the output format
    */
    try{
        avdevice_register_all();

        video_options = nullptr;
        string desktop_str, video_format;

        video_in_format_context = nullptr;
        video_in_format_context = avformat_alloc_context();


/* video_format needed for av_find_input_format*/
#ifdef _WIN32
        video_format = "gdigrab";
#elif __linux__
        video_format = "x11grab";
#elif __APPLE__
        video_format = "avfoundation";
#endif
        video_input_format = av_find_input_format(video_format.c_str());
        if (video_input_format == nullptr) {
            throw logic_error{"av_find_input_format not found..."};
        }

        /**
        * av_dict_set: populate the variable "video_options" that is a reference to an AVDictionary object
        * This component is used to inform avformat_open_input and avformat_find_stream_info about all settings
        */
        if(av_dict_set(&video_options, "framerate", to_string(vi.framerate).c_str(), 0) < 0){
            throw logic_error{"Error in setting dictionary value"};
        }
        if( av_dict_set(&video_options, "video_size", (to_string(vi.width) + "*" + to_string(vi.height)).c_str(), 0) < 0){
            throw logic_error{"Error in setting dictionary value"};
        }
        if( av_dict_set(&video_options, "probesize", "30M", 0) <0){  //forse serve al demuxer
            throw logic_error{"Error in setting dictionary value"};
        }
/* desktop_str needed for avformat_open_input*/
#ifdef _WIN32
        desktop_str = "desktop";
        av_dict_set(&video_options, "offset_x", to_string(vi.offset_x).c_str(), 0);
        av_dict_set(&video_options, "offset_y", to_string(vi.offset_y).c_str(), 0);

#elif __linux__
        desktop_str=":0.0+" + to_string(vi.offset_x)+ "," + to_string(vi.offset_y);

#elif __APPLE__
        desktop_str="1:none"; //video:audio
        string video_format_string = "crop=" + to_string(vi.width) + ":" + to_string(vi.height) + ":" + to_string(vi.offset_x) + ":" + to_string(vi.offset_y);
        if( av_dict_set(&video_options, "pixel_format", "uyvy422", 0) < 0){
            throw logic_error{"Error in setting dictionary value"};
        }
        if( av_dict_set(&video_options, "capture_cursor", "1", 0) < 0){
            throw logic_error{"Error in setting dictionary value"};
        }
        if( av_dict_set(&video_options, "vf", video_format_string.c_str(), 0) < 0){
            throw logic_error{"Error in setting dictionary value"};
        }
#endif

        /**
         * avformat_open_input: open the file, read its header and fill the format_context (AVFormatContext)
         * with information about the format
         */
        if(avformat_open_input(&video_in_format_context, desktop_str.c_str(), video_input_format, &video_options) != 0){
            throw logic_error{"Error in opening input stream"};
        }

        /**
         * avformat_find_stream_info: populates the format_context->streams with proper information
         *
         *  format_context->nb_streams will hold the size of the array streams (number of streams)
         *  format_context->streams[i] will give us the i stream (an AVStream)
         *
         * we will loop through all the streams until we find the video stream position/index
         */
        if (avformat_find_stream_info(video_in_format_context, &video_options) < 0) {
            throw logic_error{"Error in finding stream information"};
        }

        in_video_index = -1;
        for (int i = 0; i < video_in_format_context->nb_streams; i++){
            if( video_in_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ){
                in_video_index = i;
                break;
            }
        }

        if (in_video_index == -1) {
            throw logic_error{"Error in finding a video stream"};
        }

        /**
         * avcodec_find_decoder: with the parameters of the codec, look up the proper codec.
         *
         * @return {AVCodec} for the provided codec_id.
         * It is the component that knows how to enCode and DECode the stream
         */
        video_codec_parameters = video_in_format_context->streams[in_video_index]->codecpar;
        video_decodec = avcodec_find_decoder(video_codec_parameters->codec_id);
        if(video_decodec == nullptr){
            throw logic_error{"Error in finding the decoder"};
        }

        /**
         * avcodec_alloc_context3(): allocate memory to the component AVCodecContext.
         * It will contain the context for the decode/encode process,
         * and all the information about the codec that the stream is using
         *
         * avcodec_parameters_to_context(): fill the AVCodecContext with CODEC parameters
         *
         * avcodec_open2(): initialize the AVCodecContext to use the given AVCodec,
         * and open the codec so that it can be used
         */
        video_in_codec_context = avcodec_alloc_context3(nullptr);
        if(video_in_codec_context == nullptr){
            throw logic_error{"Error in alloc video_in_codec_context"};
        }

        avcodec_parameters_to_context(video_in_codec_context, video_codec_parameters);

        if (avcodec_open2(video_in_codec_context, video_decodec, nullptr) < 0) {
            throw logic_error{"Error in opening the av codec"};
        }
    }catch(exception &err){
        // TODO vedere se liberare delle risorse
        //av_dict_free(&video_options);
        cout << (err.what()) << endl;
    }
}

void ScreenRecorder::initializeVideoOutput() {
    video_encodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!video_encodec) {
        throw logic_error{"Error in finding encoder"};
    }

    /**
     * avformat_new_stream(): create the output stream into the output formatContext
     *
     * avcodec_alloc_context3(): allocate memory to the component AVCodecContext.
     * It will contain the context for the decode/encode process,
     * and all the information about the codec that the stream is using
     *
     * avcodec_parameters_to_context(): fill the AVCodecContext with CODEC parameters
     *
     * avcodec_open2(): initialize the AVCodecContext to use the given AVCodec,
     * and open the codec so that it can be used
     */
    video_st = avformat_new_stream(out_format_context, video_encodec);
    if( !video_st ){
        throw runtime_error{"Error creating a av format new stream"};
    }

    video_out_codec_context = avcodec_alloc_context3(video_encodec);
    if( !video_out_codec_context){
        throw runtime_error{"Error in allocating the codec contexts"};
    }

    avcodec_parameters_to_context(video_out_codec_context, video_st->codecpar);

    // set property of the video file
    video_out_codec_context->codec_id = AV_CODEC_ID_H264;
    video_out_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
    video_out_codec_context->pix_fmt  = AV_PIX_FMT_YUV420P;
    video_out_codec_context->bit_rate = 4000000;
    video_out_codec_context->width = vi.width;
    video_out_codec_context->height = vi.height;
    video_out_codec_context->gop_size = 3;
    video_out_codec_context->max_b_frames = 2;
    video_out_codec_context->time_base.num = 1;
    video_out_codec_context->time_base.den = vi.framerate; //framerate


    // Some container formats like MP4 require global headers to be present
    if ( out_format_context->oformat->flags & AVFMT_GLOBALHEADER){
        video_out_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(video_out_codec_context, video_encodec, nullptr) < 0) {
        throw logic_error{"Error in opening the av codec"};
    }

    if(!out_format_context->nb_streams){
        throw logic_error{"Error output file does not contain any stream"};
    }

    out_video_index = -1;
    for (int i = 0; i < out_format_context->nb_streams; i++) {
        if (out_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            out_video_index = i;
        }
    }

    if(out_video_index == -1){
        throw logic_error{"Error in finding a stream for the output file"};
    }

    avcodec_parameters_from_context(out_format_context->streams[out_video_index]->codecpar, video_out_codec_context); // TODO aggiungere if < 0 error
}

void ScreenRecorder::initializeVideoCapture(){
    /**
     * av_frame_alloc(): allocate memory for AVFrame
     * these components are used for reading the packets from the stream and decode them into frames
     *
     * av_image_alloc(): allocate an image with size w, h and pixel format pix_fmt
     * and fill pointers and linesizes accordingly
     *
     * sws_getContext: initialize SWS context for software scaling.
     * It is used later with 'sws_scale' in order to made the conversion
     * from the suorce width, height and format, to the desired width, height and format.
     */

    inVideoFrame = av_frame_alloc();
    if( !inVideoFrame ){
        throw logic_error{"Error in allocate memory to AVFrame"};
    }

    outVideoFrame = av_frame_alloc();
    if( !outVideoFrame ){
        throw logic_error{"Error in allocate memory to AVFrame"};
    }
    outVideoPacket = av_packet_alloc();
    if( !outVideoPacket ){
        throw logic_error{"Error in allocate memory to AVPacket"};
    }

    if(av_image_alloc(outVideoFrame->data, outVideoFrame->linesize, video_out_codec_context->width, video_out_codec_context->height, video_out_codec_context->pix_fmt, 32 ) < 0){
        throw logic_error{"Error in filling image array"};
    };

    sws_ctx = sws_getContext(video_in_codec_context->width,
                             video_in_codec_context->height,
                             video_in_codec_context->pix_fmt,
                             video_out_codec_context->width,
                             video_out_codec_context->height,
                             video_out_codec_context->pix_fmt, //the native format of the frame
                             SWS_BICUBIC, nullptr, nullptr, nullptr); //color conversion and scaling
    if(sws_ctx==nullptr)
        throw runtime_error{"Error in allocating sws_ctx"};

    outVideoFrame->width=video_in_codec_context->width;
    outVideoFrame->height=video_in_codec_context->height;
    outVideoFrame->format = AV_PIX_FMT_YUV420P;
}

void ScreenRecorder::recording(){
    current_video_state.lock();
    current_audio_state.lock();
    if(program_state == RECORDING ){
        current_video_state.unlock();
        current_audio_state.unlock();
        throw runtime_error("Error during recording : recording function invoked was still running");
    }else if(program_state==PAUSE ) {
        reset_video();
        if(audio_device != "none")
            reset_audio();
        program_state = RECORDING;
        current_video_state.unlock();
        current_audio_state.unlock();
        video_cv.notify_one();
        audio_cv.notify_one();
    }else if(program_state == START || program_state == STOP){
        if(program_state == STOP) {
            reset_video();
            current_audio_pts=0;
            current_video_pts=0;
            if (audio_device != "none")
                reset_audio();
        }
        program_state = RECORDING;
        current_video_state.unlock();
        current_audio_state.unlock();

        t_converting_video = thread([this]() { this->convert_video_format(); });
        if (audio_device != "none"){
            t_converting_audio = thread([this]() { this->convert_audio_format(); });
        }
    }else {
        current_video_state.unlock();
        current_audio_state.unlock();
        throw runtime_error("WRONG STATE");
    }
}

void ScreenRecorder::pause(){
    lock_guard<mutex> lg_video(current_video_state);
    lock_guard<mutex> lg_audio(current_audio_state);
    if(program_state == RECORDING) {
        program_state = PAUSE;
        cout<<"RECORDING PAUSED"<<endl;
    }
    else{
        throw runtime_error("Error in pause the recording : recording was not running");
    }
}


void ScreenRecorder::stop_recording() {
    unique_lock<mutex> ul_video(current_video_state);
    unique_lock<mutex> ul_audio(current_audio_state);
    if(program_state==RECORDING || program_state==PAUSE){
        program_state=STOP;
        ul_video.unlock();
        ul_audio.unlock();
        video_cv.notify_one();
        audio_cv.notify_one();
    }else if(program_state==STOP || program_state==START) {
        ul_video.unlock();
        ul_audio.unlock();
    }else{
        throw runtime_error("WRONG STATE");
    }
}

void ScreenRecorder::convert_video_format() {
    int value = 0;
    int got_picture = 0;

    while(true ){
        unique_lock<mutex> ul_video(current_video_state);

        video_cv.wait(ul_video, [this](){return program_state!=PAUSE;} );

        if(program_state==STOP){
            ul_video.unlock();
            break;
        }

        ul_video.unlock();

        /**
         * av_packet_alloc(): allocate memory for AVPacket in order to read the packets from the stream
         * these components are used for reading the packets from the stream and decode them into frames
         *
         * av_read_frame(): split what is stored in the file/context into frames and return one frame for each call
         * This function does not validate if they are valid frames for the decoder.
         * @return {AVFrame} On success, the returned packet is reference-counted (pkt->buf is set) and valid indefinitely.
         *
         * pkt->pts, pkt->dts and pkt->duration are always set to correct values in AVStream.time_base units
         */
        inVideoPacket = av_packet_alloc();
        if( !inVideoPacket ){
            throw logic_error{"Error in allocate memory to AVPacket"};
        }

        if(av_read_frame(video_in_format_context, inVideoPacket) >= 0 && inVideoPacket->stream_index==in_video_index){
            av_packet_rescale_ts(inVideoPacket,video_in_format_context->streams[in_video_index]->time_base,video_in_codec_context->time_base);
        }

        /**
         * INIT DECODE VIDEO FRAME
         * avcodec_send_packet(): send the raw data packet (compressed frame) to the decoder, through the codec context
         *
         * av_packet_free(): Free the packet and its pointer will be set to null
         *
         * avcodec_receive_frame(): receive the raw data frame (uncompressed frame) from the decoder, through the same codec context
         */
        if (inVideoPacket->stream_index == in_video_index) {
            value = avcodec_send_packet(video_in_codec_context, inVideoPacket);
            if (value < 0) {
                throw runtime_error("Error in decoding video (send_packet)");
            }
            av_packet_free(&inVideoPacket);

            if(avcodec_receive_frame(video_in_codec_context, inVideoFrame) == 0){ // frame successfully decoded
                //Fine decode
                current_pts_synch.lock();
                inVideoFrame->pts=current_video_pts++;
                current_pts_synch.unlock();

                if(out_format_context->streams[out_video_index]->start_time<=0){
                    out_format_context->streams[out_video_index]->start_time=inVideoFrame->pts;
                }

                // Convert the image from its native format to YUV
                sws_scale(sws_ctx, inVideoFrame->data, inVideoFrame->linesize, 0,
                          video_in_codec_context->height, outVideoFrame->data, outVideoFrame->linesize);

                outVideoFrame->pts = inVideoFrame->pts;;

                /**
                  * INIT ENCODE VIDEO FRAME
                  * avcodec_send_frame(): aupply a raw video frame to the encoder
                  *
                  * avcodec_receive_packet(): read encoded data from the encoder.
                  * This function internally execute the av_packet_unref of the outPacket, before copying the content
                  * of the new packet (received from the encoder).
                  *
                  * avcodec_receive_frame(): receive the raw data frame (uncompressed frame) from the decoder, through the same codec context
                  */
                value = avcodec_send_frame(video_out_codec_context, outVideoFrame);
                got_picture = avcodec_receive_packet(video_out_codec_context, outVideoPacket);
                if(value == 0 ){
                    if(got_picture == 0){ //if got_picture is 0, we have enough packets to have a frame
                        av_packet_rescale_ts(outVideoPacket,video_out_codec_context->time_base,out_format_context->streams[out_video_index]->time_base);

                        lockwrite.lock();
                        if(av_write_frame(out_format_context , outVideoPacket) != 0){
                            throw runtime_error("Error in writing video frame");
                        }
                        lockwrite.unlock();
                    }
                }else{
                    throw runtime_error("Error in encoding video send_frame");
                }

            }else{
                throw runtime_error("Error in decoding video (receive_frame)");
            }

        }
    }
    cout<<"FINE THREAD VIDEO CONVERTER"<<endl;
}

void ScreenRecorder::initializeAudioInput(){
    string audio_str;
    audio_options = nullptr;
    in_audio_format_context = nullptr;
    in_audio_format_context = avformat_alloc_context();
    if(!in_audio_format_context)
        throw logic_error{"Error in allocating input audio format context"};

#ifdef _WIN32
    audio_input_format = av_find_input_format("dshow");
        if (audio_input_format == nullptr) {
            throw logic_error{"Error in finding this audio input device"};
        }
#elif __linux__
    audio_input_format = av_find_input_format("alsa");
    if (audio_input_format == nullptr){
        throw logic_error{"Error in opening ALSA driver"};
    }
#elif __APPLE__
    audio_input_format = av_find_input_format("avfoundation");
    if (audio_input_format == nullptr){
        throw logic_error{"Error in opening AVFoundation driver"};
    }
#endif

    // TODO CAPIRE SE SERVONO le options
    /**
     * av_dict_set: populate the variable "audio_options" that is a reference to an AVDictionary object
     * This component is used to inform avformat_open_input and avformat_find_stream_info about all settings
     */
    if( av_dict_set(&audio_options, "sample_rate", "44100", 0)){
        throw logic_error{"Error in setting dictionary value"};
    }
    if( av_dict_set(&audio_options, "async", "25", 0) ){
        throw logic_error{"Error in setting dictionary value"};
    }

    /**
    * avformat_open_input: open the file, read its header and fill the format_context (AVFormatContext)
    * with information about the format
    */
#ifdef _WIN32
    //TYPE=NAME where TYPE can be either audio or video, and NAME is the device’s name
        audio_str = "audio=Microphone (Realtek(R) Audio)";
        if(avformat_open_input(&in_audio_format_context, audio_str.c_str(), audio_input_format, &audio_options) != 0){
            throw logic_error{"Error in opening input stream"};
        }
#elif __linux__
    audio_str = audio_device;
    if (avformat_open_input(&in_audio_format_context, audio_str.c_str(), audio_input_format, &audio_options) < 0){
        throw runtime_error("Error in opening input stream");
    }
#elif __APPLE__
    //[[VIDEO]:[AUDIO]]
    // none do not record the corresponding media type
    audio_str = "none:0";
    if (avformat_open_input(&in_audio_format_context, audio_str.c_str(), audio_input_format, &audio_options) < 0){
        throw logic_error("Error in opening input stream");
    }
#endif

    /**
     * avformat_find_stream_info: populates the format_context->streams with proper information
     * we will loop through all the streams until we find the audio stream position/index
     */
    if (avformat_find_stream_info(in_audio_format_context,nullptr ) < 0) {
        throw logic_error{"Error in finding audio stream information"};
    }

    in_audio_index = -1;
    for (int i = 0; i < in_audio_format_context->nb_streams; i++){
        if( in_audio_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO ){
            in_audio_index = i;
            break;
        }
    }

    if (in_audio_index == -1) {
        throw logic_error{"Error in finding an audio stream"};
    }

    /**
     * avcodec_find_decoder: with the parameters of the codec, look up the proper codec.
     *
     * @return {AVCodec} for the provided codec_id.
     * It is the component that knows how to enCode and DECode the stream
     */
    audio_codec_parameters = in_audio_format_context->streams[in_audio_index]->codecpar;
    audio_decodec = avcodec_find_decoder(audio_codec_parameters->codec_id);
    if(audio_decodec == nullptr){
        throw logic_error{"Error in finding the audio decoder"};
    }

    /**
     * avcodec_alloc_context3(): allocate memory to the component AVCodecContext.
     * It will contain the context for the decode/encode process,
     * and all the information about the codec that the stream is using
     *
     * avcodec_parameters_to_context(): fill the AVCodecContext with CODEC parameters
     *
     * avcodec_open2(): initialize the AVCodecContext to use the given AVCodec,
     * and open the codec so that it can be used
     */
    audio_in_codec_context = avcodec_alloc_context3(audio_decodec);
    if(avcodec_parameters_to_context(audio_in_codec_context, audio_codec_parameters)< 0) {
        throw logic_error("Cannot create codec context for audio input");
    }
    if (avcodec_open2(audio_in_codec_context, audio_decodec, nullptr) < 0) {
        throw logic_error{"Error in opening the av decodec"};
    }
}

void ScreenRecorder::initializeAudioOutput(){
    audio_encodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!audio_encodec) {
        throw logic_error{"Error in finding encoder"};
    }

    /**
     * avformat_new_stream(): create the output stream into the output formatContext
     *
     * avcodec_alloc_context3(): allocate memory to the component AVCodecContext.
     * It will contain the context for the decode/encode process,
     * and all the information about the codec that the stream is using
     *
     * avcodec_open2(): initialize the AVCodecContext to use the given AVCodec,
     * and open the codec so that it can be used
     */
    audio_st = avformat_new_stream(out_format_context, nullptr);
    if( !audio_st ){
        throw runtime_error{"Error creating a av format new stream"};
    }

    audio_out_codec_context = avcodec_alloc_context3(audio_encodec);
    if( !audio_out_codec_context){
        throw runtime_error{"Error in allocating the codec contexts"};
    }

    if((audio_encodec)->supported_samplerates){
        audio_out_codec_context->sample_rate = (audio_encodec)->supported_samplerates[0];
        for(int i=0;(audio_encodec)->supported_samplerates[i];i++){
            if((audio_encodec)->supported_samplerates[i] == audio_in_codec_context->sample_rate)
                audio_out_codec_context->sample_rate = audio_in_codec_context->sample_rate;
        }
    }

    // set property of the audio file
    audio_out_codec_context->codec_id = AV_CODEC_ID_AAC;
    audio_out_codec_context->channels = audio_in_codec_context->channels;
    audio_out_codec_context->channel_layout = av_get_default_channel_layout(audio_out_codec_context->channels);
    audio_out_codec_context->bit_rate = 64000;
    audio_out_codec_context->time_base.num = 1;
    audio_out_codec_context->time_base.den = audio_in_codec_context->sample_rate;
    audio_out_codec_context->sample_fmt = audio_encodec->sample_fmts ? audio_encodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    audio_out_codec_context->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;


    // Some container formats like MP4 require global headers to be present
    if ( out_format_context->oformat->flags & AVFMT_GLOBALHEADER){
        audio_out_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(audio_out_codec_context, audio_encodec, nullptr) < 0) {
        throw logic_error{"Error in opening the av encodec"};
    }

    if(!out_format_context->nb_streams){
        throw logic_error{"Error output file does not contain any stream"};
    }

    out_audio_index = -1;
    for (int i = 0; i < out_format_context->nb_streams; i++) {
        if (out_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            out_audio_index = i;
        }
    }

    if(out_audio_index==-1){
        throw logic_error{"Error in finding a stream for the output file"};
    }

    avcodec_parameters_from_context(out_format_context->streams[out_audio_index]->codecpar, audio_out_codec_context);
}

void ScreenRecorder::initializeOutputMedia(){
    // create empty video file
    if ( !(out_format_context->flags & AVFMT_NOFILE) ){
        if( avio_open2(&out_format_context->pb , vi.output_file.c_str() , AVIO_FLAG_WRITE,nullptr,nullptr) < 0 ){
            throw logic_error{"Error in creating the video file"};
        }
    }

    //mp4 container file required header information
    if(avformat_write_header(out_format_context , &video_options) < 0){
        throw logic_error{"Error in writing the header context"};

    }
    cout<<"End initialize output file"<<endl;
}

void ScreenRecorder::convert_audio_format() {
    int ret;
    AVPacket *inPacket, *outPacket;
    AVFrame *inFrame, *outFrame;
    int temp_pts;

    /**
     * av_audio_fifo_alloc(): Create the FIFO buffer based on the specified output sample format
     *
     * av_packet_alloc(): allocate memory for AVPacket in order to read the packets from the stream
     *
     * av_read_frame(): split what is stored in the file/context into frames and return one frame for each call
     * @return {AVFrame} On success, the returned packet is reference-counted (pkt->buf is set) and valid indefinitely.
     */
    if (!(audio_buffer = av_audio_fifo_alloc(audio_out_codec_context->sample_fmt, audio_out_codec_context->channels, 1))) {
        throw logic_error("Error in allocation fifo buffer");
    }
    inPacket = av_packet_alloc();
    if( !inPacket ){
        throw logic_error{"Error in allocate memory to AVPacket"};
    }

    outPacket = av_packet_alloc();
    if( !outPacket ){
        throw logic_error{"Error in allocate memory to AVPacket"};
    }

    inFrame = av_frame_alloc();
    if( !inFrame ){
        throw logic_error{"Error in allocate memory to AVFrame"};
    }

    //init the resampler
    SwrContext *swrContext = nullptr;
    swrContext = swr_alloc_set_opts(swrContext,
                                    av_get_default_channel_layout(audio_out_codec_context->channels),
                                    audio_out_codec_context->sample_fmt,
                                    audio_out_codec_context->sample_rate,
                                    av_get_default_channel_layout(audio_in_codec_context->channels),
                                    audio_in_codec_context->sample_fmt,
                                    audio_in_codec_context->sample_rate,
                                    0,
                                    nullptr);
    if (!swrContext) {
        throw logic_error("Error in allocating the resample context");
    }
    if ((swr_init(swrContext)) < 0) {
        swr_free(&swrContext);
        throw logic_error("Error in opening resample context");
    }

    while(true){
        unique_lock<mutex> ul_audio(current_audio_state);

        audio_cv.wait(ul_audio, [this](){return program_state!=PAUSE;} );

        if(program_state==STOP){
            ul_audio.unlock();
            break;
        }
        ul_audio.unlock();

        current_pts_synch.lock();
        if( av_compare_ts(current_video_pts, video_out_codec_context->time_base, current_audio_pts,audio_out_codec_context->time_base) <= 0) {
            current_pts_synch.unlock();
            continue;
        }
        current_pts_synch.unlock();

        if (av_read_frame(in_audio_format_context,inPacket) < 0){
            throw std::runtime_error("can not read frame");
        }

        /**
         * INIT DECODE AUDIO FRAME
         */
        if (avcodec_send_packet(audio_in_codec_context, inPacket) < 0){
            throw std::runtime_error("can not send pkt in decoding");
        }
        if(avcodec_receive_frame(audio_in_codec_context, inFrame) < 0){
            throw std::runtime_error("can not receive frame in decoding");
        }

        uint8_t **cSamples = nullptr;
        if (av_samples_alloc_array_and_samples(&cSamples, nullptr, audio_out_codec_context->channels, inFrame->nb_samples, AV_SAMPLE_FMT_FLTP, 0) < 0){
            throw std::runtime_error("Fail to alloc samples by av_samples_alloc_array_and_samples.");
        }
        if (swr_convert(swrContext, cSamples, inFrame->nb_samples, (const uint8_t**)inFrame->extended_data, inFrame->nb_samples) < 0){
            throw std::runtime_error("Fail to swr_convert.");
        }

        //if (av_audio_fifo_space(audio_buffer) < inFrame->nb_samples) {
        //throw std::runtime_error("audio buffer is too small.");
        if ((av_audio_fifo_realloc(audio_buffer, av_audio_fifo_size(audio_buffer) + inFrame->nb_samples)) < 0) {
            throw std::runtime_error{"Could not reallocate FIFO\n"};
        }
        //}

        if( av_audio_fifo_write(audio_buffer, (void**)cSamples, inFrame->nb_samples) < 0){
            throw std::runtime_error("Fail to write fifo");
        }

        av_freep(&cSamples[0]);
        av_frame_unref(inFrame);
        av_packet_unref(inPacket);

        while (av_audio_fifo_size(audio_buffer) >= audio_out_codec_context->frame_size) {
            outFrame = av_frame_alloc();
            if (!outFrame) {
                throw logic_error{"Error in allocate memory to AVFrame"};
            }
            outFrame->nb_samples = audio_out_codec_context->frame_size;
            outFrame->channels = audio_in_codec_context->channels;
            outFrame->channel_layout = av_get_default_channel_layout(audio_in_codec_context->channels);
            outFrame->format = AV_SAMPLE_FMT_FLTP;
            outFrame->sample_rate = audio_out_codec_context->sample_rate;

            if (av_frame_get_buffer(outFrame, 0) < 0) {
                throw logic_error("Error in getting audio buffer");
            }
            if (av_audio_fifo_read(audio_buffer, (void **) outFrame->data, audio_out_codec_context->frame_size) <
                0) {
                throw runtime_error("Error in getting audio buffer");
            }

            outFrame->pts = current_audio_pts;
            temp_pts = outFrame->pts;
            current_audio_pts += outFrame->nb_samples;

            /**
             * INIT ENCODE AUDIO FRAME
             */
            if (avcodec_send_frame(audio_out_codec_context, outFrame) < 0) {
                throw std::runtime_error("Fail to send frame in encoding");
            }

            av_frame_free(&outFrame);
            ret = avcodec_receive_packet(audio_out_codec_context, outPacket);
            if (ret == AVERROR(EAGAIN)) {
                continue;
            } else if (ret < 0) {
                throw std::runtime_error("Fail to receive packet in encoding");
            }

            outPacket->stream_index = audio_st->index;
            outPacket->duration = audio_st->time_base.den * 1024 / audio_out_codec_context->sample_rate;
            outPacket->dts = outPacket->pts = temp_pts;//frameCount * audio_st->time_base.den * 1024 / audio_out_codec_context->sample_rate;

            lockwrite.lock();
            ret = av_interleaved_write_frame(out_format_context, outPacket);
            if (ret)
                throw runtime_error{"Problem in write audio packet"};
            lockwrite.unlock();

        }
    }
    av_packet_free(&inPacket);
    av_packet_free(&outPacket);
    av_frame_free(&inFrame);
    swr_free(&swrContext);
    cout<<"End thread audio"<<endl;
}

void ScreenRecorder::reset_audio() {
    av_audio_fifo_reset(audio_buffer);
    avformat_close_input(&in_audio_format_context);
    if (in_audio_format_context != nullptr) {
        throw runtime_error("Error in closing in_audio_format_context for pausing");
    }
    avformat_free_context(in_audio_format_context);
    avcodec_close(audio_in_codec_context);
    avcodec_free_context(&audio_in_codec_context);
    audio_in_codec_context = nullptr;
    initializeAudioInput();
}

void ScreenRecorder::reset_video(){
    avformat_close_input(&video_in_format_context);
    if (video_in_format_context != nullptr) {
        throw runtime_error("Error in closing video_in_format_context for pausing");
    }
    avformat_free_context(video_in_format_context);
    avcodec_close(video_in_codec_context);
    avcodec_free_context(&video_in_codec_context);
    video_in_codec_context = nullptr;
    initializeVideoInput();
    sws_ctx = sws_getContext(video_in_codec_context->width,
                             video_in_codec_context->height,
                             video_in_codec_context->pix_fmt,
                             video_out_codec_context->width,
                             video_out_codec_context->height,
                             video_out_codec_context->pix_fmt, //the native format of the frame
                             SWS_BICUBIC, nullptr, nullptr, nullptr); //Color conversion and scaling. possibilities: SWS_FAST_BILINEAR, SWS_BILINEAR, SWS_BICUBIC

    outVideoFrame->width=video_in_codec_context->width;
    outVideoFrame->height=video_in_codec_context->height;
    outVideoFrame->format = AV_PIX_FMT_YUV420P;
}