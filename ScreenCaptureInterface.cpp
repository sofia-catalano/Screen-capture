#include "ScreenCaptureInterface.h"
#include "ScreenRecorder.h"
#include <wx/textdlg.h>

using namespace std;

static int playpause = 0;
static int mic = false;



ScreenCaptureInterface::ScreenCaptureInterface(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 150))
{

    wxPanel     *panel  = new wxPanel(this, -1);
    wxBoxSizer  *hbox   = new wxBoxSizer(wxHORIZONTAL);
    btnPanel            = new MyPanel(panel);
    hbox->Add(btnPanel, 4, wxEXPAND | wxRIGHT , 5);

    panel->SetSizer(hbox);
    Center();
}


MyPanel::MyPanel(wxPanel * parent)
        : wxPanel(parent, wxID_ANY)
{
    wxBoxSizer *vbox = new wxBoxSizer(wxHORIZONTAL);

    m_playPauseb  = new wxBitmapToggleButton(this, ID_PLAY_PAUSE,wxBitmap("../icons/playpause.png", wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);
    m_stopb       = new wxBitmapButton(      this, ID_STOP,  wxBitmap("../icons/stop.png", wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);
    m_audiob      = new wxBitmapToggleButton(this, ID_AUDIO, wxBitmap("../icons/mic.png",  wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);

    m_stopb->Enable(false);
    m_audiob->Enable(false);


    Connect(ID_PLAY_PAUSE, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyPanel::OnPlayPause) );
    Connect(ID_STOP, wxEVT_BUTTON,
            wxCommandEventHandler(MyPanel::OnStop) );
    Connect(ID_AUDIO, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyPanel::OnAudio) );

    vbox->Add(-1, 20);
    vbox->Add(m_playPauseb,  0, wxTOP, 5);
    vbox->Add(m_stopb,       0, wxTOP, 5);
    vbox->Add(m_audiob,      0, wxTOP, 5);

    SetSizer(vbox);
}

void MyPanel::OnPlayPause(wxCommandEvent& event)
{
    bool tmpMic;

    if(!playpause){

        wxString dimensions;
        dimensions = wxGetTextFromUser(wxT("Dimensions"),
                                       wxT("Dimensions of capture"));
        if (!dimensions.IsEmpty()) {
            m_stopb->Enable(true);
            m_audiob->Enable(true);
            playpause = 1;

            VideoInfo vi;
            vi.width = 800;
            vi.height = 800;
            vi.offset_x = 0;
            vi.offset_y = 0;
            vi.framerate = 30;

            vi.output_file = "./output1.mp4";

            ScreenRecorder screen_recorder{vi};
            cout << "Fine costruttore" << endl;
            screen_recorder.recording();
            cout << "Costruito oggetto Screen Recorder" << endl;
        }
        else{
            m_playPauseb->SetValue(false);
        }
    }
    else if(playpause == 1){
        //pause video ed audio se attivo
        //debug da terminale
        if(mic){
            tmpMic = mic;
            mic = false;
        }
        cout<<"Pause recording..."<<endl;
        playpause = 2;
    }
    else if(playpause == 2){
        //resume video ed audio se stato attivato prima della pausa
        playpause = 1;

        if(tmpMic){
            mic = tmpMic;
        }

        //debug da terminale
        cout<<"Resume recording..."<<endl;
    }
}

void MyPanel::OnStop(wxCommandEvent& event)
{
    //chiamare funzione pause

    wxString path;
    path = wxGetTextFromUser(wxT("Path"), wxT("Saving video"));

    if (!path.IsEmpty()) {
        //richiamare funzione che salva il video/audio
        m_audiob->SetValue(false);
        m_audiob->Enable(false);
        m_stopb->Enable(false);
        playpause = 0;
        m_playPauseb->SetValue(false);

        //debug da terminale
        cout<<"Stop recording and saving into --> "<<path<<endl;
    }
    else{
        playpause = 1;
    }
}


void MyPanel::OnAudio(wxCommandEvent& event)
{
    if(mic)
    {
        mic = false;
        cout<<"Disable recording audio..."<<endl;
    }
    else
    {
        mic = true;
        cout<<"Enable recording audio..."<<endl;
    }
}


