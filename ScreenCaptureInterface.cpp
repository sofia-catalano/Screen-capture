#include "ScreenCaptureInterface.h"
#include "ScreenRecorder.h"

#include "Devices.h"


static int playpause = 0;
static int audio = 0;
static string output_file = "./prova.mp4";
static int off_x=0, off_y=0;
static int width=600, height=600;
ScreenRecorder *sc;
string recordAudio = "none";
static int n = 0;
result r;

static bool check = false;



// Create a new application object
IMPLEMENT_APP(MyApp)

// ----------------------------------------------------------------------------
// MyFrame
// ----------------------------------------------------------------------------

MyFrame::MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title)
{
    r = getAudioDevices();

    wxPanel     *panel      = new wxPanel(this, -1);
    wxBoxSizer  *sizer      = new wxBoxSizer(wxHORIZONTAL);

    m_playPauseb  = new wxToggleButton(this, ID_PLAY_PAUSE, "PLAY_PAUSE", wxDefaultPosition,  wxSize(100, 25));
    m_stopb       = new wxButton(      this, ID_STOP,       "STOP",       wxDefaultPosition, wxSize(100, 25));
    m_micb        = new wxToggleButton(this, ID_MIC,        "AUDIO",      wxDefaultPosition, wxSize(100, 25));

    screen_portion_b  = new wxToggleButton(this, ID_SCREEN_PORTION, "SCREEN PORTION", wxPoint(1, 200), wxSize(150, 25), 0, wxDefaultValidator, wxButtonNameStr);
    full_screen_b     = new wxToggleButton(this, ID_FULL_SCREEN,    "FULL SCREEN",    wxPoint(151, 200), wxSize(150, 25), 0, wxDefaultValidator, wxButtonNameStr);
    //---------------------------------------------------------
    //  SELECT AUDIO DEVICE
    //---------------------------------------------------------


    wxStaticText *st1 = new wxStaticText(this, -1, wxString("Device audio: "), wxPoint(1, 83),  wxDefaultSize, 0, "1");
    listAudioDevices   = new wxComboBox(  this, 10, wxEmptyString, wxPoint(101, 80), wxSize(200, 25), 0, NULL, wxCB_READONLY, wxDefaultValidator, wxComboBoxNameStr);

    listAudioDevices->Append("none");
    for(int i=0; i<r.n; i++){
        //ciclo su result
        listAudioDevices->Append(r.desc[i]);
    }

    listAudioDevices->Select(0);
    listAudioDevices->Bind(wxEVT_COMBOBOX, &MyFrame::OnSelectionAudio, this);

    listAudioDevices->Enable(false);

    //----------------------------------------------------------
    //  END SELECT AUDIO DEVICE
    //----------------------------------------------------------

    //----------------------------------------------------------
    //  TEXTBOX PATH
    //----------------------------------------------------------

    wxStaticText *st2 = new wxStaticText(this, -1, wxString("Path: "), wxPoint(1, 135),  wxDefaultSize, 0, "1");
    path = new wxTextCtrl(this, 30, output_file, wxPoint(101, 130), wxSize(200, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);

/*
    char l1[15] = "Frame rate";
    wxString str_fps;
    str_fps << l;

    wxStaticText *st3 = new wxStaticText(this, -1, "Frame Rate", wxPoint(20, 180),  wxDefaultSize, 0, "1");
    wxStaticText *controlFps = new wxStaticText(this, -1, "(min:1, max:60)", wxPoint(20, 200),  wxDefaultSize, 0, "1");



    wxFont myFont(7, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

*/
    //controlFps->Show(false);
    //char ss[10];

    path->Bind(wxEVT_TEXT, [&](wxCommandEvent& event) {
            output_file = path->GetValue();
    });
/*
    fps->Bind(wxEVT_TEXT, [&](wxCommandEvent& event) {
        static int tmp = 30;
        frame_rate = fps->GetValue();

        try {
            tmp = stoi(frame_rate);
        }
        catch(std::invalid_argument& e){
            // if no conversion could be performed
        }
        if(tmp > 60 || tmp < 25){
            m_playPauseb->Enable(false);
            fps->SetBackgroundColour(wxColour(0xFF,0xA0,0xA0));
        }else {
            m_playPauseb->Enable(true);
            fps->SetBackgroundColour(wxColour(0xFF, 0xFF, 0xFF));
        }
    });
*/
    //----------------------------------------------------------
    //  END TEXTBOX PATH
    //----------------------------------------------------------

    m_stopb->Enable(false);

    Connect(ID_PLAY_PAUSE, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyFrame::OnPlayPause) );
    Connect(ID_STOP, wxEVT_BUTTON,
            wxCommandEventHandler(MyFrame::OnStop) );
    Connect(ID_MIC, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyFrame::OnMic) );
    Connect(ID_SCREEN_PORTION, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyFrame::OnScreenPortion) );
    Connect(ID_FULL_SCREEN, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyFrame::OnFullScreen) );


    sizer->Add(-1, 20);
    sizer->Add(m_playPauseb,  0, wxTOP, 5);
    sizer->Add(m_stopb,  0, wxTOP, 5);
    sizer->Add(m_micb,  0, wxTOP, 5);


    SetSizer(sizer);

#if wxUSE_STATUSBAR
    CreateStatusBar(2);
#endif // wxUSE_STATUSBAR

    Center();
    SetSize(305, 300);
    Show();
}



MyFrame1::MyFrame1(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title) {

    static string str_width  = "";
    static string str_height = "";

    static string str_off_x = "";
    static string str_off_y = "";

    wxPanel *panel = new wxPanel(this, -1);
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);


    wxStaticText *st1 = new wxStaticText(this, -1, "Widht", wxPoint(20, 20), wxDefaultSize, 0, "1");
    //wxStaticText *controlWidth = new wxStaticText(this, -1, "(min:1, max:60)", wxPoint(20, 200),  wxDefaultSize, 0, "1");
    //wxFont myFont(7, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    //controlWidth->SetFont(myFont);
    //wxTextCtrl *width = new wxTextCtrl(this, 30, frame_rate, wxPoint(70, 20), wxSize(40, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);

    wxStaticText *st2 = new wxStaticText(this, -1, "Height", wxPoint(20, 50), wxDefaultSize, 0, "1");
    //wxTextCtrl *height = new wxTextCtrl(this, 30, frame_rate, wxPoint(70, 50), wxSize(40, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);

    wxStaticText *label_off_x = new wxStaticText(this, -1, "Offset X", wxPoint(20, 80), wxDefaultSize, 0, "1");
    wxStaticText *label_off_y = new wxStaticText(this, -1, "Offset Y", wxPoint(20, 110), wxDefaultSize, 0, "1");

    str_width = to_string(width);
    str_height = to_string(height);
    str_off_x = to_string(off_x);
    str_off_y = to_string(off_y);

    w = new wxTextCtrl(this, ID_WIDTH, str_width, wxPoint(100, 20), wxSize(150, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);
    h = new wxTextCtrl(this, ID_HEIGHT, str_height, wxPoint(100, 50), wxSize(150, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);

    x = new wxTextCtrl(this, ID_OFF_X, str_off_x, wxPoint(100, 80), wxSize(150, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);
    y = new wxTextCtrl(this, ID_OFF_Y, str_off_y, wxPoint(100, 110), wxSize(150, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);

    submit_b = new wxButton( this, ID_CONFIRM, "Confirm", wxPoint(125, 150), wxSize(100 , 25));

    Connect(ID_CONFIRM, wxEVT_BUTTON,
            wxCommandEventHandler(MyFrame1::OnConfirm) );


    SetSizer(sizer);


    Center();
    SetSize(300, 230);


    w->Bind(wxEVT_TEXT, [&](wxCommandEvent& event) {
        static int tmp = 0;
        str_width = w->GetValue();

        try {
            tmp = stoi(str_width);
            cout<<"Conversione to int"<<tmp<<endl;

            if(tmp <= getScreenSize().x && tmp > 0){
                width = tmp;
                submit_b->Enable(true);
            }
            else{
                submit_b->Enable(false);
            }
        }
        catch(std::invalid_argument& e){
            cout<<"Invalid width"<<endl;
            submit_b->Enable(false);
        }
    });


    h->Bind(wxEVT_TEXT, [&](wxCommandEvent& event) {
        static int tmp = 0;
        str_height = h->GetValue();
        cout<<"Height-->"<<str_height<<endl;

        try {
            tmp = stoi(str_height);
            if(tmp <= getScreenSize().y && tmp > 0){
                height = tmp;
                submit_b->Enable(true);
            }
            else{
                submit_b->Enable(false);
            }
        }
        catch(std::invalid_argument& e){
            // if no conversion could be performed
            cout<<"Invalid height"<<endl;
            submit_b->Enable(false);
        }
    });

    x->Bind(wxEVT_TEXT, [&](wxCommandEvent& event) {
        static int tmp = 0;
        str_off_x = x->GetValue();
        cout<<"OFF_X-->"<<str_off_x<<endl;

        try {
            tmp = stoi(str_off_x);
            cout<<"Conversione to int"<<tmp<<endl;
            if(tmp <= getScreenSize().y && tmp >= 0){
                off_x = tmp;
                submit_b->Enable(true);
            }
            else{
                submit_b->Enable(false);
            }

        }
        catch(std::invalid_argument& e){
            // if no conversion could be performed
            cout<<"Invalid off_x"<<endl;
            submit_b->Enable(false);
        }
    });

    y->Bind(wxEVT_TEXT, [&](wxCommandEvent& event) {
        static int tmp = 0;
        str_off_y = y->GetValue();
        cout<<"OFF_X-->"<<str_off_y<<endl;

        try {
            tmp = stoi(str_off_y);
            cout<<"Conversione to int"<<tmp<<endl;
            if(tmp <= getScreenSize().y && tmp >= 0){
                off_y = tmp;
                submit_b->Enable(true);
            }
            else{
                submit_b->Enable(false);
            }
        }
        catch(std::invalid_argument& e){
            // if no conversion could be performed
            cout<<"Invalid off_y"<<endl;
            submit_b->Enable(false);
        }
    });


}

MyFrame::~MyFrame()
{
    {
        wxCriticalSectionLocker locker(wxGetApp().m_critsect);

        // check if we have any threads running first
        const wxArrayThread& threads = wxGetApp().m_threads;
        size_t count = threads.GetCount();

        if ( !count )
            return;

        // set the flag indicating that all threads should exit
        wxGetApp().m_shuttingDown = true;
    }

    // now wait for them to really terminate
    wxGetApp().m_semAllDone.Wait();
}

MyFrame1::~MyFrame1()
{
    {
        wxCriticalSectionLocker locker(wxGetApp().m_critsect);

        // check if we have any threads running first
        const wxArrayThread& threads = wxGetApp().m_threads;
        size_t count = threads.GetCount();

        if ( !count )
            return;

        // set the flag indicating that all threads should exit
        wxGetApp().m_shuttingDown = true;
    }

    // now wait for them to really terminate
    wxGetApp().m_semAllDone.Wait();
}

MyThread *MyFrame::CreateThread()
{
    MyThread *thread = new MyThread;

    if ( thread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogError(wxT("Can't create thread!"));
    }

    wxCriticalSectionLocker enter(wxGetApp().m_critsect);
    wxGetApp().m_threads.Add(thread);

    return thread;
}

// ----------------------------------------------------------------------------
// MyFrame - event handlers
// ----------------------------------------------------------------------------
void MyFrame::OnSelectionAudio(wxCommandEvent& e){
    //prova->SetSelection(static_cast<wxComboBox*>(e.GetEventObject())->GetSelection());
    if(listAudioDevices->GetCurrentSelection() > 0 ){
        recordAudio = r.devices[listAudioDevices->GetCurrentSelection()-1];
    }
    else if(listAudioDevices->GetCurrentSelection() == 0 ){
        recordAudio = "none";
    }
}

void MyFrame::OnPlayPause(wxCommandEvent& WXUNUSED(event) )
{
    if(!playpause){
        playpause = 1;

        MyThread *thread = CreateThread();

        if ( thread->Run() != wxTHREAD_NO_ERROR )
        {
            wxLogError(wxT("Can't start thread!"));
        }
        path->Enable(false);
        m_stopb->Enable(true);
        m_micb->Enable(false);
    }
    else if(playpause == 1){
        sc->pause();
        playpause = 2;
    }
    else if(playpause == 2){
        sc->recording();
        playpause = 1;
    }

#if wxUSE_STATUSBAR
    SetStatusText(wxT("..."), 1);
#endif // wxUSE_STATUSBAR
}

void MyFrame::OnStop(wxCommandEvent& WXUNUSED(event) )
{
    sc->stop_recording();

    m_playPauseb->SetValue(false);
    m_stopb->Enable(false);

    m_micb->SetValue(false);
    m_micb->Enable(true);

    playpause = 0;
    cout<<"Stop recording"<<endl;
}

void MyFrame::OnMic(wxCommandEvent& WXUNUSED(event) )
{
    static int pressed = 1;
    if(pressed){
        pressed = 0;
        listAudioDevices->Enable(true);
    }
    else{
        pressed = 1;
        listAudioDevices->Enable(false);
    }
    //API set up recording from mic too
    cout<<"Recording mic"<<endl;
}

void MyFrame::OnScreenPortion(wxCommandEvent& WXUNUSED(event) )
{
    MyFrame1 *screenPortionFrame = new MyFrame1("ScreenPortion");
    screenPortionFrame->Show();
    full_screen_b->SetValue(false);

}

void MyFrame::OnFullScreen(wxCommandEvent& WXUNUSED(event) )
{
    screen_size size = getScreenSize();
    width = size.x;
    height = size.y;
    off_x = 0;
    off_y = 0;
    screen_portion_b->SetValue(false);
}

void MyFrame1::OnConfirm(wxCommandEvent& event){
    //check input altrimenti non va avanti
    Close(true);
}


// ----------------------------------------------------------------------------
// MyThread
// ----------------------------------------------------------------------------

MyThread::MyThread()
        : wxThread()
{
    //m_count = 0;
}

MyThread::~MyThread()
{
    wxCriticalSectionLocker locker(wxGetApp().m_critsect);

    wxArrayThread& threads = wxGetApp().m_threads;
    threads.Remove(this);

    if ( threads.IsEmpty() )
    {
        // signal the main thread that there are no more threads left if it is
        // waiting for us
        if ( wxGetApp().m_shuttingDown )
        {
            wxGetApp().m_shuttingDown = false;
            wxGetApp().m_semAllDone.Post();
        }
    }
}

wxThread::ExitCode MyThread::Entry()
{
    //wxLogMessage("Thread started (priority = %u).", GetPriority());

    VideoInfo vi;
    //prendere valori da GUI altrimenti default
    cout<<endl<<"--------------------Parametri per creazione costruttore-------------------"<<endl;
    cout<<"Width-->"<<width<<endl;
    cout<<"Heigth-->"<<height<<endl;

    cout<<"OFF_X-->"<<off_x<<endl;
    cout<<"OFF_Y-->"<<off_y<<endl;
    cout<<"--------------------------------------------------------------------------"<<endl;

    vi.width = width;
    vi.height = height;
    vi.offset_x = off_x;
    vi.offset_y = off_y;

#ifdef _WIN32
    vi.framerate = 30;
#elif __linux__
    vi.framerate = 35;
#endif

    vi.output_file = output_file;

    cout<<"Fine costruttore"<<endl;
    ScreenRecorder screen_recorder{vi, recordAudio};

    sc = &screen_recorder;
    sc->recording();

    return NULL;
}
