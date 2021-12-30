//
// Created by Sofia Catalano on 08/10/21.
//

#include "ScreenRecorder.h"
#include <exception>
#include <iostream>

#include <condition_variable>

using namespace std;

bool pauseThread=false;
std::condition_variable cv;
std::mutex m;


ScreenRecorder::ScreenRecorder(VideoInfo vi) : vi(vi){
    try{
        initializeVideoResources();

    }catch(exception &err){
        cout << "All required functions are registered not successfully" << endl;
        cout << (err.what()) << endl;
    }
}

ScreenRecorder::~ScreenRecorder(){
    t_reading_video->join();
    t_converting_video->join();


    av_write_trailer(out_format_context);

    av_packet_free(&outPacket);
    //av_freep(video_buffer);
    cout<<"ciao"<<endl;
    avformat_close_input(&in_format_context);
    avio_close(out_format_context->pb);
    avcodec_free_context(&codec_context);
    avcodec_free_context(&out_codec_context);

    cout << "Distruttore Screen Recorder" << endl;
}

void ScreenRecorder::initializeVideoResources() {
    initializeVideoInput();
    cout << "End initializeInputSource" << endl;

    initializeVideoOutput();
    cout << "End initializeOutputSource" << endl;

    initializeVideoCapture();
    cout << "End initializeCaptureResources" << endl;
}

void ScreenRecorder::initializeVideoInput(){
    //initialize the library: registers all available file formats and codecs with the library
    // so they will be used automatically when a file with the corresponding format/codec is opened.
    //need to be call once
    avdevice_register_all();


    options = NULL ;
    string desktop_str;

    //allocate memory to the component AVFormatContext that will hold information about the format
    in_format_context = NULL;
    in_format_context = avformat_alloc_context();

#ifdef _WIN32
    input_format = av_find_input_format("gdigrab");
    if (input_format == NULL) {
        throw logic_error{"av_find_input_format not found..."};
    }

#elif __linux__
    input_format = av_find_input_format("x11grab");
    if (input_format == NULL) {
        throw logic_error{"av_find_input_format not found..."};
    }



#elif __APPLE__
    input_format = av_find_input_format("avfoundation");
    if (input_format == NULL) {
        throw logic_error{"av_find_input_format not found..."};
    }
#endif

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

#ifdef _WIN32
    desktop_str = "desktop";
    // open the file, read its header and fill the format_context (AVFormatContext) with information about the format
    if(avformat_open_input(&in_format_context, desktop_str.c_str(), input_format, &options) != 0){
        throw logic_error{"Error in opening input stream"};
    }

#elif __linux__
    // TODO risolvere il fullscreen
    //https://unix.stackexchange.com/questions/573121/get-current-screen-dimensions-via-xlib-using-c
    //https://www.py4u.net/discuss/81858

    desktop_str=":0.0+"+ to_string(vi.offset_x)+","+ to_string(vi.offset_y);

    // open the file, read its header and fill the format_context (AVFormatContext) with information about the format
    if(avformat_open_input(&in_format_context, desktop_str.c_str(), input_format, &options) != 0){
        throw logic_error{"Error in opening input stream"};
    }

#elif __APPLE__
    //video:audio
    desktop_str="1:none";
    if(avformat_open_input(&in_format_context, desktop_str.c_str(), input_format, &options) != 0){
        throw logic_error{"Error in opening input stream"};
    }
#endif

    //avformat_open_input: only looks at the header, so next we need to check out the stream information in the file
    // avformat_find_stream_info populates the format_context->streams with proper information
    // format_context->nb_streams will hold the size of the array streams (number of streams)
    // format_context->streams[i] will give us the i stream (an AVStream)
    if (avformat_find_stream_info(in_format_context, &options) < 0) {
        throw logic_error{"Error in finding stream information"};
    }

    video_index = -1;
    // loop through all the streams until we find the video stream position/index
    for (int i = 0; i < in_format_context->nb_streams; i++){
        if( in_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ){
            video_index = i;
	        break;
	    }
    }

    if (video_index == -1) {
        throw logic_error{"Error in finding a video stream"};
    }

    // Get the properties of a codec used by the stream video found
    codec_parameters = in_format_context->streams[video_index]->codecpar;

    //with the parameters, look up the proper codec (with the function avcodec_find_decoder)
    //it will find the registered decoder for the codec id and return an AVCodec
    //AVCodec is the component that knows how to enCode and DECode the stream
    av_decodec = avcodec_find_decoder(codec_parameters->codec_id);
    if(av_decodec == NULL){
        throw logic_error{"Error in finding the decoder"};
    }


    //allocate memory for the AVCodecContext that will hold the context for the decode/encode process
    /* This AVCodecContext contains all the information about the codec that the stream is using,
    and now we have a pointer to it*/
    codec_context = avcodec_alloc_context3(NULL); //TODO oppure av_decodec


    //now fill this codec_context with CODEC parameters
     avcodec_parameters_to_context(codec_context, codec_parameters);

    //open the codec
    //Initialize the AVCodecContext to use the given AVCodec
    //now the codec can be used
    if (avcodec_open2(codec_context, av_decodec, NULL) < 0) {
        throw logic_error{"Error in opening the av codec"};
    }
}

void ScreenRecorder::initializeVideoOutput() {
    /* Returns the output format in the list of registered output formats
    which best matches the provided parameters, or returns NULL if there is no match. */

    output_format = NULL;
    output_format = av_guess_format(NULL, vi.output_file.c_str(), NULL);



    //TODO capire null e nullptr
    if (output_format == NULL) {
        throw runtime_error{"Error in matching the video format"};
    }

    //allocate memory to the component AVFormatContext that will contain information about the output format
    avformat_alloc_output_context2( &out_format_context, output_format, output_format->name, vi.output_file.c_str());

    av_encodec = avcodec_find_encoder(AV_CODEC_ID_H264); //Abdullah: AV_CODEC_ID_MPEG4
    if (!av_encodec) {
        throw logic_error{"Error in allocating av format output context"}; // TODO non mi pare l'errore giusto
    }

    //we need to create new out stream into the output format context
    video_st = avformat_new_stream(out_format_context, av_encodec); // TODO cheina av_encodec
    if( !video_st ){
        throw runtime_error{"Error creating a av format new stream"};
    }

    out_codec_context = avcodec_alloc_context3(av_encodec); // TODO oppure av_encodec
    if( !out_codec_context){
        throw runtime_error{"Error in allocating the codec contexts"};
    }

    //avcodec_parameters_to_context: fill the output codec context with CODEC parameters
    avcodec_parameters_to_context(out_codec_context, video_st->codecpar);


    // set property of the video file
    out_codec_context->codec_id = AV_CODEC_ID_H264; // AV_CODEC_ID_MPEG4
    out_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
    out_codec_context->pix_fmt  = AV_PIX_FMT_YUV420P;
#ifdef _WIN32
    out_codec_context->bit_rate = 4000000; // 80000
#else
    out_codec_context->bit_rate = 400000;
#endif
    out_codec_context->width = vi.width; // (int)(rrs.width * vs.quality) / 32 * 32;
    out_codec_context->height = vi.height; // (int)(rrs.height * vs.quality) / 2 * 2;
    out_codec_context->gop_size = 3; //50
    out_codec_context->max_b_frames = 2;
    out_codec_context->time_base.num = 1;
    out_codec_context->time_base.den = vi.framerate; //framerate
    /*
      out_codec_context->qmin = 5;
        out_codec_context->qmax = 10;

    */

    av_opt_set(out_codec_context, "preset", "slow", 0); // encoding speed to compression ratio
    av_opt_set(out_codec_context, "tune", "stillimage", 0);
    av_opt_set(out_codec_context, "crf", "18.0", 0);

    /* Some container formats like MP4 require global headers to be present.
	   Mark the encoder so that it behaves accordingly. */
    if ( out_format_context->oformat->flags & AVFMT_GLOBALHEADER){
        out_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    //open the codec (or better init the encoder)
    //Initialize the AVCodecContext to use the given AVCodec.
    if (avcodec_open2(out_codec_context, av_encodec, NULL) < 0) {
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

    avcodec_parameters_from_context(out_format_context->streams[out_video_index]->codecpar, out_codec_context);


    /* create empty video file */
    if ( !(out_format_context->flags & AVFMT_NOFILE) ){
        if( avio_open2(&out_format_context->pb , vi.output_file.c_str() , AVIO_FLAG_WRITE,NULL,NULL) < 0 ){
            throw logic_error{"Error creating the context for accessing the resource indicated by url"};
        }
    }

    /* imp: mp4 container or some advanced container file required header information*/
    if(avformat_write_header(out_format_context , &options) < 0){
        throw logic_error{"Error in writing the header context"};

    }

    /**/
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
    if(av_image_alloc(outFrame->data, outFrame->linesize, out_codec_context->width, out_codec_context->height, out_codec_context->pix_fmt, 32 ) < 0){
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
    sws_ctx = sws_getContext(codec_context->width,
                             codec_context->height,
                             codec_context->pix_fmt,
                             out_codec_context->width,
                             out_codec_context->height,
                             out_codec_context->pix_fmt, //the native format of the frame
                             SWS_BICUBIC, NULL, NULL, NULL); //Color conversion and scaling. possibilities: SWS_FAST_BILINEAR, SWS_BILINEAR, SWS_BICUBIC

    outFrame->width=codec_context->width;
    outFrame->height=codec_context->height;
    outFrame->format = AV_PIX_FMT_UYVY422;
}

void ScreenRecorder::recording(){
    end_reading = false;
    t_reading_video = make_unique<thread>([this]() { this->read_packets(); });
    cout<<"ookk"<<endl;
    t_converting_video = make_unique<thread>([this]() { this->convert_video_format(); });
}

void ScreenRecorder::read_packets(){
    //int nFrame = 1500;
    int i = 0;

    while (true){
         while(pauseThread){
             std::unique_lock<std::mutex> lk(m);
             if(!pauseThread)
                 break;
             cv.wait(lk);
             lk.unlock();
         }

         i++;
         cout<<i<<endl;

         if(/*i++ == nFrame ||*/ *vi.status == -1){
             *vi.status = 0;
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
         if(av_read_frame(in_format_context, inPacket) < 0){
             throw logic_error{"Error in getting inPacket"};
         }
         inPacket_video_mutex.lock();
         inPacket_video_queue.push(inPacket);
         inPacket_video_mutex.unlock();


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
    int j = 0, i=0;
    int value = 0;
    int got_picture = 0;

    //We allocate memory for AVPacket in order to extract the packets from the queue and decode and encode it
    /*inPacket2 = av_packet_alloc(); //allocate memory to a packet and set its fields to default values
    if( !inPacket2 ){
        throw logic_error{"Error in allocate memory to AVPacket"};
    }*/

    while(!end_reading || !inPacket_video_queue.empty()){

        inPacket_video_mutex.lock();

        //da valutare se mettere in pause anche qui
        /*
        while(pauseThread){
            std::unique_lock<std::mutex> lk(m);
            if(!pauseThread)
                break;
            cv.wait(lk);
            lk.unlock();
        }
         */

        if(!inPacket_video_queue.empty()) {
            j++;
            cout<<"decoding : "<<j<<endl;

            inPacket2 = inPacket_video_queue.front();
            inPacket_video_queue.pop();
            inPacket_video_mutex.unlock();
            if (inPacket2->stream_index == video_index) {
                //decode video frame
                //let's send the raw data packet (compressed frame) to the decoder, through the codec context
                value = avcodec_send_packet(codec_context, inPacket2);
                if (value < 0) {
                    throw runtime_error("Error in decoding video (send_packet)");
                }
                av_packet_free(&inPacket2);//Free the packet and its pointer will be set to null

                //-------------------------------------------------
                //let's receive the raw data frame (uncompressed frame) from the decoder, through the same codec context, using the function avcodec_receive_frame
                if(avcodec_receive_frame(codec_context, inFrame) == 0){ // frame successfully decoded
                    //Fine decode

                    // Convert the image from its native format to YUV
                    sws_scale(sws_ctx, inFrame->data, inFrame->linesize, 0,
                              codec_context->height, outFrame->data, outFrame->linesize);

                    outFrame->pts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (int64_t)vi.framerate;;



                    //encode video frame
                    value = avcodec_send_frame(out_codec_context, outFrame);

                    //avcodec_receive_packet fa internamente l'av_packet_unref dell'outPacket prima di copiarci dentro il contenuto
                    //del nuovo pacchetto ricevuto dall'encoder : quindi resetta il contenuto e libera i buffer interni
                    got_picture = avcodec_receive_packet(out_codec_context, outPacket);
                    if(value == 0 ){
                        if(got_picture == 0){
                            //if got_picture is 0, we have enough packets to have a frame



                            if(outPacket->pts != AV_NOPTS_VALUE) {
                                outPacket->pts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (int64_t)vi.framerate;
                                cout<<" OUT_PACKET PTS :"<< outPacket->pts<<endl;
                            }
                            if(outPacket->dts != AV_NOPTS_VALUE) {
                                outPacket->dts = (int64_t)i  * (int64_t)30 * (int64_t)30 * (int64_t)100 / (int64_t)vi.framerate;
                                cout << " OUT_PACKET DTS :" << outPacket->pts << endl;
                            }

                            /*outPacket->pts = av_rescale_q(outPacket->pts, format_context->streams[video_index]->time_base, out_format_context->streams[out_video_index]->time_base);
                            outPacket->dts = av_rescale_q(outPacket->dts, format_context->streams[video_index]->time_base, out_format_context->streams[out_video_index]->time_base);
                            */


                            /*outPacket->pts = av_rescale_q_rnd(outPacket->pts, format_context->streams[video_index]->time_base, out_format_context->streams[out_video_index]->time_base,(AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                            outPacket->dts = av_rescale_q_rnd(outPacket->dts, format_context->streams[video_index]->time_base, out_format_context->streams[out_video_index]->time_base, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                            outPacket->duration = av_rescale_q(outPacket->duration, format_context->streams[video_index]->time_base, out_format_context->streams[out_video_index]->time_base);
                            */

                            //TODO vedere se ci va un lock per la scrittura :
                            //TODO ci va per la sincronizzazione dei thread audio e video che scriveranno sullo stesso file di output
                            if(av_write_frame(out_format_context , outPacket) != 0){
                                throw runtime_error("Error in writing video frame");
                            }

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
            i++;
        }else{
            inPacket_video_mutex.unlock();
        }
    }

}

//-----------------------------------------------------
//  API to manage threads
//-----------------------------------------------------

void ScreenRecorder::pause(){
    //pause
    std::lock_guard<std::mutex> lk(m);
    pauseThread=true;
}

void ScreenRecorder::resume(){
    std::lock_guard<std::mutex> lk(m);
    pauseThread=false;
    cv.notify_one();
}

void ScreenRecorder::stop(){
    resume();
    *vi.status = -1;
}







