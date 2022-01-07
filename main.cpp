#include <iostream>
#include "ScreenRecorder.h"
using namespace std;

int main(int argc, char const* argv[]) {
    VideoInfo vi;
    vi.width = atoi(argv[1]);
    vi.height = atoi(argv[2]);
    vi.offset_x = atoi(argv[3]);
    vi.offset_y = atoi(argv[4]);
    vi.framerate = atoi(argv[5]);
    bool audio = false;//atoi(argv[6]);
    vi.output_file = "./output.mp4"; //TODO: sostituire con un argomento passato, serve per av_guess_format
    ScreenRecorder screen_recorder{vi, audio};
    cout << "Fine costruttore" << endl;
    screen_recorder.recording();
    cout << "Costruito oggetto Screen Recorder" << endl;
    /*for(auto i: getAudioDevices().devices){
        cout<<i<<endl;
    }*/
    return 0;
}
