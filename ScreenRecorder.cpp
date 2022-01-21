//
// Created by Sofia Catalano on 08/10/21.
//

#include "ScreenRecorder.h"
#include <exception>
#include <iostream>

using namespace std;


ScreenRecorder::ScreenRecorder(VideoInfo vi, bool audio) : vi(vi), audio(audio){
    try{
        initializeOutputContext();

        initializeVideoResources();

        if(audio){
            initializeAudioResources();
        }

        initializeOutputMedia();
    }catch(exception &err){
        cout << "All required functions are registered not successfully" << endl;
        cout << (err.what()) << endl;
    }
}

ScreenRecorder::~ScreenRecorder(){
    t_reading_video->join();
    t_converting_video->join();
    t_converting_audio->join();


    av_write_trailer(out_format_context);

    av_packet_free(&outPacket);
    av_freep(audio_buffer);
    avformat_close_input(&video_in_format_context);
    avio_close(out_format_context->pb);
    avcodec_free_context(&video_in_codec_context);
    avcodec_free_context(&video_out_codec_context);

    cout << "Distruttore Screen Recorder" << endl;
}

void ScreenRecorder::initializeOutputContext(){
    /* Returns the output format in the list of registered output formats
which best matches the provided parameters, or returns NULL if there is no match. */

    output_format = NULL;
    output_format = av_guess_format(NULL, vi.output_file.c_str(), NULL);

    if (output_format == NULL) {
        throw runtime_error{"Error in matching the video format"};
    }

    //allocate memory to the component AVFormatContext that will contain information about the output format
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
    //initialize the library: registers all available file formats and codecs with the library
    // so they will be used automatically when a file with the corresponding format/codec is opened.
    //need to be call once
    avdevice_register_all();


    video_options = NULL ;
    string desktop_str;

    //allocate memory to the component AVFormatContext that will hold information about the format
    video_in_format_context = NULL;
    video_in_format_context = avformat_alloc_context();

#ifdef _WIN32
    video_input_format = av_find_input_format("gdigrab");
    if (video_input_format == NULL) {
        throw logic_error{"av_find_input_format not found..."};
    }

#elif __linux__
    video_input_format = av_find_input_format("x11grab");
    if (video_input_format == NULL) {
        throw logic_error{"av_find_input_format not found..."};
    }



#elif __APPLE__
    video_input_format = av_find_input_format("avfoundation");
    if (video_input_format == NULL) {
        throw logic_error{"av_find_input_format not found..."};
    }
#endif

    //AVDictionary to inform avformat_open_input and avformat_find_stream_info about all settings
    //video_options is a reference to an AVDictionary object that will be populated by av_dict_set
    if(av_dict_set(&video_options, "framerate", to_string(vi.framerate).c_str(), 0) < 0){
        throw logic_error{"Error in setting dictionary value"};
    }
    if( av_dict_set(&video_options, "video_size", (to_string(vi.width) + "*" + to_string(vi.height)).c_str(), 0) < 0){
        throw logic_error{"Error in setting dictionary value"};
    }
    if( av_dict_set(&video_options, "probesize", "30M", 0) <0){  //forse serve al demuxer
        throw logic_error{"Error in setting dictionary value"};
    }

//TODO VEDERE SE FUNZIONA COSI SU MAC
#ifdef MACOS
    ret = av_dict_set(&video_options, "pixel_format", "0rgb", 0);
        if(ret<0)
            throw runtime_error("Error in setting dictionary value");

        ret = av_dict_set(&video_options, "capture_cursor", "1", 0);
        if(ret<0)
            throw runtime_error("Error in setting dictionary value");
#endif

#ifdef _WIN32
    desktop_str = "desktop";
    // open the file, read its header and fill the format_context (AVFormatContext) with information about the format
    if(avformat_open_input(&video_in_format_context, desktop_str.c_str(), video_input_format, &video_options) != 0){
        throw logic_error{"Error in opening input stream"};
    }

#elif __linux__
    // TODO risolvere il fullscreen
    //https://unix.stackexchange.com/questions/573121/get-current-screen-dimensions-via-xlib-using-c
    //https://www.py4u.net/discuss/81858

    desktop_str=":0.0+"+ to_string(vi.offset_x)+","+ to_string(vi.offset_y);

    // open the file, read its header and fill the format_context (AVFormatContext) with information about the format
    if(avformat_open_input(&video_in_format_context, desktop_str.c_str(), video_input_format, &video_options) != 0){
        throw logic_error{"Error in opening input stream"};
    }

#elif __APPLE__
    //video:audio
    desktop_str="1:none";
    if(avformat_open_input(&video_format_context, desktop_str.c_str(), video_input_format, &video_options) != 0){
        throw logic_error{"Error in opening input stream"};
    }
#endif

    //avformat_open_input: only looks at the header, so next we need to check out the stream information in the file
    // avformat_find_stream_info populates the format_context->streams with proper information
    // format_context->nb_streams will hold the size of the array streams (number of streams)
    // format_context->streams[i] will give us the i stream (an AVStream)
    if (avformat_find_stream_info(video_in_format_context, &video_options) < 0) {
        throw logic_error{"Error in finding stream information"};
    }

    in_video_index = -1;
    // loop through all the streams until we find the video stream position/index
    for (int i = 0; i < video_in_format_context->nb_streams; i++){
        if( video_in_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ){
            in_video_index = i;
	        break;
	    }
    }



    if (in_video_index == -1) {
        throw logic_error{"Error in finding a video stream"};
    }

    // Get the properties of a codec used by the stream video found
    video_codec_parameters = video_in_format_context->streams[in_video_index]->codecpar;

    //with the parameters, look up the proper codec (with the function avcodec_find_decoder)
    //it will find the registered decoder for the codec id and return an AVCodec
    //AVCodec is the component that knows how to enCode and DECode the stream
    video_decodec = avcodec_find_decoder(video_codec_parameters->codec_id);
    if(video_decodec == NULL){
        throw logic_error{"Error in finding the decoder"};
    }


    //allocate memory for the AVCodecContext that will hold the context for the decode/encode process
    /* This AVCodecContext contains all the information about the codec that the stream is using,
    and now we have a pointer to it*/
    video_in_codec_context = avcodec_alloc_context3(NULL); //TODO oppure video_decodec


    //now fill this video_in_codec_context with CODEC parameters
     avcodec_parameters_to_context(video_in_codec_context, video_codec_parameters);

    //open the codec
    //Initialize the AVCodecContext to use the given AVCodec
    //now the codec can be used
    if (avcodec_open2(video_in_codec_context, video_decodec, NULL) < 0) {
        throw logic_error{"Error in opening the av codec"};
    }
}


void ScreenRecorder::initializeVideoOutput() {
    video_encodec = avcodec_find_encoder(AV_CODEC_ID_H264); //Abdullah: AV_CODEC_ID_MPEG4
    if (!video_encodec) {
        throw logic_error{"Error in finding encoder"};
    }

    //we need to create new out stream into the output format context
    video_st = avformat_new_stream(out_format_context, video_encodec);
    if( !video_st ){
        throw runtime_error{"Error creating a av format new stream"};
    }

    video_out_codec_context = avcodec_alloc_context3(video_encodec);
    if( !video_out_codec_context){
        throw runtime_error{"Error in allocating the codec contexts"};
    }

    //avcodec_parameters_to_context: fill the output codec context with CODEC parameters
    avcodec_parameters_to_context(video_out_codec_context, video_st->codecpar);


    // set property of the video file
    video_out_codec_context->codec_id = AV_CODEC_ID_H264; // AV_CODEC_ID_MPEG4
    video_out_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
    video_out_codec_context->pix_fmt  = AV_PIX_FMT_YUV420P;

    video_out_codec_context->bit_rate = 4000000; // 80000

    video_out_codec_context->width = vi.width; // (int)(rrs.width * vs.quality) / 32 * 32;
    video_out_codec_context->height = vi.height; // (int)(rrs.height * vs.quality) / 2 * 2;
    video_out_codec_context->gop_size = 3; //50
    video_out_codec_context->max_b_frames = 2;
    video_out_codec_context->time_base.num = 1;
    video_out_codec_context->time_base.den = vi.framerate; //framerate
    /*
      out_codec_context->qmin = 5;
        out_codec_context->qmax = 10;

    */

    av_opt_set(video_out_codec_context, "preset", "slow", 0); // encoding speed to compression ratio
    av_opt_set(video_out_codec_context, "tune", "stillimage", 0);
    av_opt_set(video_out_codec_context, "crf", "18.0", 0);

    /* Some container formats like MP4 require global headers to be present.
	   Mark the encoder so that it behaves accordingly. */
    if ( out_format_context->oformat->flags & AVFMT_GLOBALHEADER){
        video_out_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    //open the codec (or better init the encoder)
    //Initialize the AVCodecContext to use the given AVCodec.
    if (avcodec_open2(video_out_codec_context, video_encodec, NULL) < 0) {
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

    if(out_video_index==-1){
        throw logic_error{"Error in finding a stream for the output file"};
    }

    avcodec_parameters_from_context(out_format_context->streams[out_video_index]->codecpar, video_out_codec_context); // TODO aggiungere if < 0 error
}

void ScreenRecorder::initializeVideoCapture(){
    //We allocate memory for AVPacket and AVFrame
    //in order to read the packets from the stream and decode them into frames
    inFrame = av_frame_alloc(); //Allocate an AVFrame and set its fields to default values
    if( !inFrame ){
        throw logic_error{"Error in allocate memory to AVFrame"};
    }

    outFrame = av_frame_alloc();
    if( !outFrame ){
        throw logic_error{"Error in allocate memory to AVFrame"};
    }
    outPacket = av_packet_alloc();
    if( !outPacket ){
        throw logic_error{"Error in allocate memory to AVPacket"};
    }

    //Allocate an image with size w and h and pixel format pix_fmt, and fill pointers and linesizes accordingly.
    if(av_image_alloc(outFrame->data, outFrame->linesize, video_out_codec_context->width, video_out_codec_context->height, video_out_codec_context->pix_fmt, 32 ) < 0){
        throw logic_error{"Error in filling image array"};
    };
    /*
     int v_outbuf_size;
    //Return the size in bytes of the amount of data required to store an image with the given parameters.
    int nbytes =  av_image_get_buffer_size(out_codec_context->pix_fmt, out_codec_context->width, out_codec_context->height,32);

    uint8_t *v_outbuf = (uint8_t*) av_malloc(nbytes);
    if( v_outbuf == NULL ){
        throw logic_error{"Error in allocate memory to buffer"};
    }
    // Setup the data pointers and linesizes based on the specified image parameters and the provided array.
    // returns: the size in bytes required for video_outbuf
    if(av_image_fill_arrays( outFrame->data, outFrame->linesize, v_outbuf, AV_PIX_FMT_YUV420P, out_codec_context->width, out_codec_context->height,1 ) < 0){
        throw logic_error{"Error in filling image array"};
    }
     */


    //setup swsContext: a pointer to an allocated context, or NULL in case of error.
    //This allows us to compile the conversion we want, and then pass that in later to 'sws_scale'
    //which is going to want our source width and height, our desired width and height,
    // the source format and desired format, along with some other options and flags.
    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(video_in_codec_context->width,
                             video_in_codec_context->height,
                             video_in_codec_context->pix_fmt,
                             video_out_codec_context->width,
                             video_out_codec_context->height,
                             video_out_codec_context->pix_fmt, //the native format of the frame
                             SWS_BICUBIC, NULL, NULL, NULL); //Color conversion and scaling. possibilities: SWS_FAST_BILINEAR, SWS_BILINEAR, SWS_BICUBIC

    outFrame->width=video_in_codec_context->width;
    outFrame->height=video_in_codec_context->height;
    outFrame->format = AV_PIX_FMT_YUV420P;
}


void ScreenRecorder::recording(){
    end_reading = false;
    t_reading_video = make_unique<thread>([this]() { this->read_packets(); });
    cout<<"ookk"<<endl;
    t_converting_video = make_unique<thread>([this]() { this->convert_video_format(); });
    if (audio){
        t_converting_audio = make_unique<thread>([this]() { this->convert_audio_format(); });
    }
}

void ScreenRecorder::read_packets(){
    int nFrame = 5000;
    int i = 0;

     while (true){
         cout<<i<<endl;

         if(i++ == nFrame){
             break;
         }
         //We allocate memory for AVPacket in order to read the packets from the stream
         inPacket = av_packet_alloc(); //allocate memory to a packet and set its fields to default values
         if( !inPacket ){
             throw logic_error{"Error in allocate memory to AVPacket"};
         }

         /* av_read_frame() Return the next frame of a stream.
                 This function returns what is stored in the file, and does not validate that what is there are valid frames for the decoder.
                 It will split what is stored in the file/context into frames and return one for each call. It will not omit invalid data between
                 valid frames so as to give the decoder the maximum information possible for decoding.

                 On success, the returned packet is reference-counted (pkt->buf is set) and valid indefinitely. The packet must be freed
                 with av_packet_unref() when it is no longer needed. For video, the packet contains exactly one frame. For audio, it contains
                 an integer number of frames if each frame has a known fixed size (e.g. PCM or ADPCM data). If the audio frames have a
                 variable size (e.g. MPEG audio), then it contains one frame.

                 pkt->pts, pkt->dts and pkt->duration are always set to correct values in AVStream.time_base units*/
         //Let's feed our packets from the streams with the function av_read_frame while it has packets
         if(av_read_frame(video_in_format_context, inPacket) >= 0 && inPacket->stream_index==in_video_index){
             av_packet_rescale_ts(inPacket,video_in_format_context->streams[in_video_index]->time_base,video_in_codec_context->time_base);
             inPacket_video_mutex.lock();
             inPacket_video_queue.push(inPacket);
             inPacket_video_mutex.unlock();
             cout<<"ciao"<<endl;
         }
     }

    inPacket_video_mutex.lock();
    end_reading = true;
    inPacket_video_mutex.unlock();

    /*
     av_packet_unref(inPacket);
    av_packet_free(&inPacket);//Free the packet and its pointer will be set to null
     */
}

void ScreenRecorder::convert_video_format() {
    int j = 0, i = 0;
    int value = 0;
    int got_picture = 0;

    //We allocate memory for AVPacket in order to extract the packets from the queue and decode and encode it
    /*inPacket2 = av_packet_alloc(); //allocate memory to a packet and set its fields to default values
    if( !inPacket2 ){
        throw logic_error{"Error in allocate memory to AVPacket"};
    }*/


    while(!end_reading || !inPacket_video_queue.empty()){

        inPacket_video_mutex.lock();
        if(!inPacket_video_queue.empty()) {
            j++;
            cout<<"decoding : "<<j<<endl;

            inPacket2 = inPacket_video_queue.front();
            inPacket_video_queue.pop();
            inPacket_video_mutex.unlock();
            if (inPacket2->stream_index == in_video_index) {
                //decode video frame
                //let's send the raw data packet (compressed frame) to the decoder, through the codec context
                value = avcodec_send_packet(video_in_codec_context, inPacket2);
                if (value < 0) {
                    throw runtime_error("Error in decoding video (send_packet)");
                }
                av_packet_free(&inPacket2);//Free the packet and its pointer will be set to null

                //-------------------------------------------------
                //let's receive the raw data frame (uncompressed frame) from the decoder, through the same codec context, using the function avcodec_receive_frame
                if(avcodec_receive_frame(video_in_codec_context, inFrame) == 0){ // frame successfully decoded
                    //Fine decode
                    inFrame->pts=i++;

                    if(out_format_context->streams[out_video_index]->start_time<=0){
                        out_format_context->streams[out_video_index]->start_time=inFrame->pts;
                    }

                    // Convert the image from its native format to YUV
                    sws_scale(sws_ctx, inFrame->data, inFrame->linesize, 0,
                              video_in_codec_context->height, outFrame->data, outFrame->linesize);

                    // outFrame->pts = i;
                    outFrame->pts = inFrame->pts;;



                            //encode video frame
                    value = avcodec_send_frame(video_out_codec_context, outFrame);

                    //avcodec_receive_packet fa internamente l'av_packet_unref dell'outPacket prima di copiarci dentro il contenuto
                    //del nuovo pacchetto ricevuto dall'encoder : quindi resetta il contenuto e libera i buffer interni
                    got_picture = avcodec_receive_packet(video_out_codec_context, outPacket);
                    if(value == 0 ){
                        if(got_picture == 0){
                            //if got_picture is 0, we have enough packets to have a frame

                            av_packet_rescale_ts(outPacket,video_out_codec_context->time_base,out_format_context->streams[out_video_index]->time_base);

                            /*if(outPacket->pts != AV_NOPTS_VALUE) {
                                outPacket->pts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (int64_t)vi.framerate;
                                cout<<" OUT_PACKET PTS :"<< outPacket->pts<<endl;
                            }
                            if(outPacket->dts != AV_NOPTS_VALUE) {
                                outPacket->dts = (int64_t)i  * (int64_t)30 * (int64_t)30 * (int64_t)100 / (int64_t)vi.framerate;
                                cout << " OUT_PACKET DTS :" << outPacket->pts << endl;
                            }*/


                            //TODO vedere se ci va un lock per la scrittura :
                            //TODO ci va per la sincronizzazione dei thread audio e video che scriveranno sullo stesso file di output
                            lockwrite.lock();
                            if(av_write_frame(out_format_context , outPacket) != 0){
                                throw runtime_error("Error in writing video frame");
                            }
                            lockwrite.unlock();
                            //TODO qui ci va invece l'unlock
                        }
                    }else{
                        throw runtime_error("Error in encoding video send_frame");
                    }

                }else{

                    throw runtime_error("Error in decoding video (receive_frame)");
                }



                cout << "inPacket: " << inPacket2 << endl;
            }
            //i++;
        }else{
            inPacket_video_mutex.unlock();
        }
    }
    stop=true;
}


void ScreenRecorder::initializeAudioInput(){
    audio_options = NULL;
    in_audio_format_context = NULL;
    in_audio_format_context = avformat_alloc_context();
    string audio_str;

#ifdef _WIN32
    audio_input_format = av_find_input_format("dshow");
    if (audio_input_format == NULL) {
        throw logic_error{"Error in finding this audio input device"};
    }

#elif __linux__
    audio_input_format = av_find_input_format("alsa");
    if (audio_input_format == NULL){
        throw logic_error{"Error in opening ALSA driver"};
    }

#elif __APPLE__
    audio_input_format = av_find_input_format("avfoundation");
    if (audio_input_format == NULL){
        throw logic_error{"Error in opening AVFoundation driver"};
    }
#endif

    // TODO CAPIRE SE SERVONO le options
    if( av_dict_set(&audio_options, "sample_rate", "44100", 0)){
        throw logic_error{"Error in setting dictionary value"};
    }
    if( av_dict_set(&audio_options, "async", "25", 0) ){
        throw logic_error{"Error in setting dictionary value"};
    }

#ifdef _WIN32
    //TYPE=NAME where TYPE can be either audio or video, and NAME is the device’s name
    audio_str = "audio=Microphone (Realtek(R) Audio)";
    if(avformat_open_input(&in_audio_format_context, audio_str.c_str(), audio_input_format, &audio_options) != 0){
        throw logic_error{"Error in opening input stream"};
    }

#elif __linux__
 //TODO get audio string
    audio_str = "hw:0";
    if (avformat_open_input(&audio_format_context, audio_str.c_str(), audio_input_format, &audio_options) < 0){
        throw runtime_error("Error in opening input stream");
    }

#elif __APPLE__
    //[[VIDEO]:[AUDIO]]
    // none do not record the corresponding media type
    audio_str = "none:0"; // TODO oppure 1
    if (avformat_open_input(&audio_format_context, audio_str.c_str(), audio_input_format, &audio_options) < 0){
        throw logic_error("Error in opening input stream");
    }
#endif

    if (avformat_find_stream_info(in_audio_format_context,NULL /* &audio_options*/) < 0) {
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

    // Get the properties of a codec used by the stream audio found
    audio_codec_parameters = in_audio_format_context->streams[in_audio_index]->codecpar;


    audio_decodec = avcodec_find_decoder(audio_codec_parameters->codec_id);
    if(audio_decodec == NULL){
        throw logic_error{"Error in finding the audio decoder"};
    }

    //allocate memory for the AVCodecContext that will hold the context for the decode/encode process
    /* This AVCodecContext contains all the information about the codec that the stream is using,
    and now we have a pointer to it*/
    audio_in_codec_context = avcodec_alloc_context3(audio_decodec); //TODO oppure NULL


    //now fill this video_in_codec_context with CODEC parameters
    if(avcodec_parameters_to_context(audio_in_codec_context, audio_codec_parameters)< 0) {

        throw logic_error("Cannot create codec context for audio input");
    } //TODO aggiungere if < 0 error

    //open the codec
    //Initialize the AVCodecContext to use the given AVCodec
    //now the codec can be use
    if (avcodec_open2(audio_in_codec_context, audio_decodec, NULL) < 0) {
        throw logic_error{"Error in opening the av decodec"};
    }
}

void ScreenRecorder::initializeAudioOutput(){


    //we need to create new out stream into the output format context
    audio_st = avformat_new_stream(out_format_context, NULL);
    if( !audio_st ){
        throw runtime_error{"Error creating a av format new stream"};
    }

    audio_encodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!audio_encodec) {
        throw logic_error{"Error in finding encoder"};
    }

    audio_out_codec_context = avcodec_alloc_context3(audio_encodec);
    if( !audio_out_codec_context){
        throw runtime_error{"Error in allocating the codec contexts"};
    }

    //avcodec_parameters_to_context: fill the output codec context with CODEC parameters
    //avcodec_parameters_to_context(audio_out_codec_context, audio_st->codecpar);

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
    //audio_out_codec_context->sample_rate = 0;
    audio_out_codec_context->bit_rate = 64000;  //TODO 128000 or 9600
    audio_out_codec_context->time_base.num = 1;
    audio_out_codec_context->time_base.den = audio_in_codec_context->sample_rate;//audio_out_codec_context->sample_rate;
    audio_out_codec_context->sample_fmt = audio_encodec->sample_fmts ? audio_encodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP; //TODO ???

    audio_out_codec_context->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    /*//TODO copiato dal cinese
    const enum AVSampleFormat* p = audio_encodec->sample_fmts;
    int ret = 0;
    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == audio_in_codec_context->sample_fmt) {
            ret = 1;
            break;
        }
        p++;
    }
    if (ret <= 0) {
        throw logic_error("error in...");
    }


    //TODO copiato dal cinese
    if (!audio_encodec->supported_samplerates)
        audio_out_codec_context->sample_rate = 44100;
    else {
        const int* p = audio_encodec->supported_samplerates;
        while (*p) {
            if (!audio_out_codec_context->sample_rate || abs(44100 - *p) < abs(44100 - audio_out_codec_context->sample_rate))
                audio_out_codec_context->sample_rate = *p;
            p++;
        }
    }*/


    /* Some container formats like MP4 require global headers to be present.
	   Mark the encoder so that it behaves accordingly. */
    if ( out_format_context->oformat->flags & AVFMT_GLOBALHEADER){
        audio_out_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    //open the codec (or better init the encoder)
    //Initialize the AVCodecContext to use the given AVCodec.
    if (avcodec_open2(audio_out_codec_context, audio_encodec, NULL) < 0) {
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

    avcodec_parameters_from_context(out_format_context->streams[out_audio_index]->codecpar, audio_out_codec_context); // TODO aggiungere if < 0 error
}



void ScreenRecorder::initializeOutputMedia(){
    /* create empty video file */
    if ( !(out_format_context->flags & AVFMT_NOFILE) ){
        if( avio_open2(&out_format_context->pb , vi.output_file.c_str() , AVIO_FLAG_WRITE,NULL,NULL) < 0 ){
            throw logic_error{"Error in creating the video file"};
        }
    }

    /* imp: mp4 container or some advanced container file required header information*/
    if(avformat_write_header(out_format_context , &video_options) < 0){
        throw logic_error{"Error in writing the header context"};

    }
    cout<<"End initialize output file"<<endl;
}

void ScreenRecorder::convert_audio_format() {
    int ret;
    AVPacket* inPacket, * outPacket;
    AVFrame* inFrame, * outFrame;
    uint8_t** resampledData;
    uint64_t frameCount=0;
    int temp_pts;

    // Create the FIFO buffer based on the specified output sample format
    if (!(audio_buffer = av_audio_fifo_alloc(audio_out_codec_context->sample_fmt, audio_out_codec_context->channels, 1))) {
        throw logic_error("Error in allocation fifo buffer");
    }

    //Allocate memory for AVPacket and AVFrame
    // in order to read the packets from the stream and decode them into frames
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

    outFrame = av_frame_alloc();
    if( !outFrame ){
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
                                    NULL);
    if (!swrContext) {
        throw logic_error("Error in allocating the resample context");
    }
    if ((swr_init(swrContext)) < 0) {
        swr_free(&swrContext);
        throw logic_error("Error in opening resample context");
    }
//-----------------------------------------------------
    while( !stop){
        ret = av_read_frame(in_audio_format_context,inPacket);
        if (ret < 0) {
            throw std::runtime_error("can not read frame");
        }
        ret = avcodec_send_packet(audio_in_codec_context, inPacket);
        if (ret < 0) {
            throw std::runtime_error("can not send pkt in decoding");
        }
        ret = avcodec_receive_frame(audio_in_codec_context, inFrame);
        if (ret < 0) {
            throw std::runtime_error("can not receive frame in decoding");
        }

        uint8_t **cSamples = nullptr;
        ret = av_samples_alloc_array_and_samples(&cSamples, NULL, audio_out_codec_context->channels, inFrame->nb_samples, AV_SAMPLE_FMT_FLTP, 0);
        if (ret < 0) {
            throw std::runtime_error("Fail to alloc samples by av_samples_alloc_array_and_samples.");
        }
        ret = swr_convert(swrContext, cSamples, inFrame->nb_samples, (const uint8_t**)inFrame->extended_data, inFrame->nb_samples);
        if (ret < 0) {
            throw std::runtime_error("Fail to swr_convert.");
        }
        if (av_audio_fifo_space(audio_buffer) < inFrame->nb_samples) {
            //throw std::runtime_error("audio buffer is too small.");
            if ((av_audio_fifo_realloc(audio_buffer, av_audio_fifo_size(audio_buffer) + inFrame->nb_samples)) < 0) {
                cout<<"Could not reallocate FIFO\n"<<endl;
            }
        }

        ret = av_audio_fifo_write(audio_buffer, (void**)cSamples, inFrame->nb_samples);
        if (ret < 0) {
            throw std::runtime_error("Fail to write fifo");
        }

        av_freep(&cSamples[0]);

        av_frame_unref(inFrame);
        av_packet_unref(inPacket);

        while (av_audio_fifo_size(audio_buffer) >= audio_out_codec_context->frame_size) {
            AVFrame* outputFrame = av_frame_alloc();
            outputFrame->nb_samples = audio_out_codec_context->frame_size;
            outputFrame->channels = audio_in_codec_context->channels;
            outputFrame->channel_layout = av_get_default_channel_layout(audio_in_codec_context->channels);
            outputFrame->format = AV_SAMPLE_FMT_FLTP;
            outputFrame->sample_rate = audio_out_codec_context->sample_rate;


            if(av_frame_get_buffer(outputFrame, 0)<0)
                throw logic_error("Error in getting audio buffer");
            if(av_audio_fifo_read(audio_buffer, (void**)outputFrame->data, audio_out_codec_context->frame_size)<0)
                throw runtime_error("[RUNTIME_ERROR] cannot get audio buffer");

            outputFrame->pts = frameCount;
            temp_pts = outputFrame->pts;
            frameCount = frameCount + outputFrame->nb_samples;


            ret = avcodec_send_frame(audio_out_codec_context, outputFrame);
            if (ret < 0) {
                throw std::runtime_error("Fail to send frame in encoding");
            }
            av_frame_free(&outputFrame);
            ret = avcodec_receive_packet(audio_out_codec_context, outPacket);
            if (ret == AVERROR(EAGAIN)) {
                continue;
            }
            else if (ret < 0) {
                throw std::runtime_error("Fail to receive packet in encoding");
            }


            outPacket->stream_index = audio_st->index;
            outPacket->duration = audio_st->time_base.den * 1024 / audio_out_codec_context->sample_rate;
            outPacket->dts = outPacket->pts =temp_pts;//frameCount * audio_st->time_base.den * 1024 / audio_out_codec_context->sample_rate;

            //frameCount++;
            lockwrite.lock();
            av_interleaved_write_frame(out_format_context, outPacket);
            lockwrite.unlock();
            cout<<"Pacchetto : "<<frameCount<<endl;
            //av_packet_unref(outPacket);

        }



    }

    av_packet_free(&inPacket);
    av_packet_free(&outPacket);
    av_frame_free(&inFrame);
    }

