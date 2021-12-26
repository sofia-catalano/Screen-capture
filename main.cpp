#include <iostream>

using namespace std;

#include "main.h"
#include "ScreenCaptureInterface.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{

    ScreenCaptureInterface *scGui = new ScreenCaptureInterface(wxT("Screen Capture"));
    scGui->Show(true);

    return true;
}

/*int main(int argc, char const* argv[]) {

    VideoInfo vi;
    vi.width = 800;
    vi.height = 800;
    vi.offset_x = 0;
    vi.offset_y = 0;
    vi.framerate = 30;
    vi.output_file = "./output1.mp4"; //TODO: sostituire con un argomento passato, serve per av_guess_format
    ScreenRecorder screen_recorder{vi};
    cout << "Fine costruttore" << endl;
    screen_recorder.recording();
    cout << "Costruito oggetto Screen Recorder" << endl;
    return 0;
}
 */
