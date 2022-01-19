#include "ScreenCaptureInterface.h"
#include "ScreenRecorder.h"

static int playpause = 0;
static int audio = 0;
static string output_file = "./prova.mp4";
static string frame_rate = "30";
ScreenRecorder *sc;
static int n = 0;

// Create a new application object
IMPLEMENT_APP(MyApp)

// ----------------------------------------------------------------------------
// MyFrame
// ----------------------------------------------------------------------------

MyFrame::MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title)
{

    wxPanel     *panel      = new wxPanel(this, -1);
    wxBoxSizer  *sizer      = new wxBoxSizer(wxHORIZONTAL);

    m_playPauseb  = new wxBitmapToggleButton(this, ID_PLAY_PAUSE, wxBitmap("../icons/playpause.png", wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);
    m_stopb       = new wxBitmapButton(      this, ID_STOP,       wxBitmap("../icons/stop.png",      wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);
    m_micb        = new wxBitmapToggleButton(this, ID_MIC,        wxBitmap("../icons/mic.png",       wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);

    wxButton *p  = new wxButton(this, ID_FULL_SCREEN, "Screen portion", wxPoint(200, 20), wxDefaultSize, 0, wxDefaultValidator, wxButtonNameStr);

    //---------------------------------------------------------
    //  SELECT AUDIO DEVICE
    //---------------------------------------------------------
    wxPoint *posLabelAudio = new wxPoint(20, 90);
    wxPoint *posSelectAudio = new wxPoint(120, 80);

    char str_label[15] = "Device audio: ";
    wxString str1;
    str1 << str_label;

    wxStaticText *st1 = new wxStaticText(this, -1, str1,          *posLabelAudio,  wxDefaultSize, 0, "1");
    listAudioDevices   = new wxComboBox(  this, 10, wxEmptyString, *posSelectAudio, wxSize(200, 25), 0, NULL, wxCB_READONLY, wxDefaultValidator, wxComboBoxNameStr);

    listAudioDevices->Append("first item");
    for(int i=0; i<6; i++){
        //ciclo su result
        listAudioDevices->Append("itemhhhhhhhhhhh ");
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
    wxPoint *posPath = new wxPoint(120, 130);
    wxPoint *posLabelPath = new wxPoint(20, 130);

    char l[15] = "Path: ";
    wxString str_path;
    str_path << l;

    wxStaticText *st2 = new wxStaticText(this, -1, str_path, *posLabelPath,  wxDefaultSize, 0, "1");
    path = new wxTextCtrl(this, 30, output_file, *posPath, wxSize(200, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);


    char l1[15] = "Frame rate";
    wxString str_fps;
    str_fps << l;

    wxStaticText *st3 = new wxStaticText(this, -1, "Frame Rate", wxPoint(20, 180),  wxDefaultSize, 0, "1");
    wxStaticText *controlFps = new wxStaticText(this, -1, "(min:1, max:60)", wxPoint(20, 200),  wxDefaultSize, 0, "1");



    wxFont myFont(7, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    controlFps->SetFont(myFont);
    fps = new wxTextCtrl(this, 30, frame_rate, wxPoint(120, 180), wxSize(40, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);
    //controlFps->Show(false);
    //char ss[10];

    path->Bind(wxEVT_TEXT, [&](wxCommandEvent& event) {
            output_file = path->GetValue();
    });

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
    Connect(ID_FULL_SCREEN, wxEVT_BUTTON,
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
    SetSize(400, 400);
    Show();
}



MyFrame1::MyFrame1(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title)
{

    wxPanel     *panel      = new wxPanel(this, -1);
    wxBoxSizer  *sizer      = new wxBoxSizer(wxHORIZONTAL);


    wxStaticText *st1 = new wxStaticText(this, -1, "Widht", wxPoint(20, 20),  wxDefaultSize, 0, "1");
    //wxStaticText *controlWidth = new wxStaticText(this, -1, "(min:1, max:60)", wxPoint(20, 200),  wxDefaultSize, 0, "1");
    //wxFont myFont(7, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

    //controlWidth->SetFont(myFont);
    wxTextCtrl *width = new wxTextCtrl(this, 30, frame_rate, wxPoint(70, 20), wxSize(40, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);

    wxStaticText *st2 = new wxStaticText(this, -1, "Height", wxPoint(20, 50),  wxDefaultSize, 0, "1");
    wxTextCtrl *height = new wxTextCtrl(this, 30, frame_rate, wxPoint(70, 50), wxSize(40, 25), 0, wxDefaultValidator, wxTextCtrlNameStr);


    SetSizer(sizer);

#if wxUSE_STATUSBAR
    CreateStatusBar(2);
#endif // wxUSE_STATUSBAR

    Center();
    SetSize(400, 150);
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
    cout<<"Selezione::"<<listAudioDevices->GetCurrentSelection()<<endl;
    //cout<<path->GetLineText(0);
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
        sc->resume();
        playpause = 1;
    }

#if wxUSE_STATUSBAR
    SetStatusText(wxT("..."), 1);
#endif // wxUSE_STATUSBAR
}

void MyFrame::OnStop(wxCommandEvent& WXUNUSED(event) )
{
    sc->stop();

    m_playPauseb->SetValue(false);
    m_stopb->Enable(false);

    m_micb->SetValue(false);
    m_micb->Enable(true);

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

void MyFrame::OnFullScreen(wxCommandEvent& WXUNUSED(event) )
{
    MyFrame1 *screenFrame = new MyFrame1("Prova");
    screenFrame->Show();
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
    vi.width = 800;
    vi.height = 800;
    vi.offset_x = 0;
    vi.offset_y = 0;

    vi.framerate = stoi(frame_rate);
    vi.status  = &playpause; //for restore m_playb on stop()
    vi.output_file = output_file;

    ScreenRecorder screen_recorder{vi};

    sc = &screen_recorder;
    sc->recording();

    return NULL;
}
