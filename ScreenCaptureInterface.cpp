#include "ScreenCaptureInterface.h"
#include "ScreenRecorder.h"

static int playpause = 0;
ScreenRecorder *sc;

// Create a new application object
IMPLEMENT_APP(MyApp)

MyApp::MyApp()
{
    m_shuttingDown = false;
}

// `Main program' equivalent, creating windows and returning main app frame
bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;


    new MyFrame("ScreenCapture");

    return true;
}

// ----------------------------------------------------------------------------
// MyFrame
// ----------------------------------------------------------------------------

MyFrame::MyFrame(const wxString& title)
        : wxFrame(NULL, wxID_ANY, title)
{

    wxPanel     *panel      = new wxPanel(this, -1);
    wxBoxSizer  *sizer      = new wxBoxSizer(wxHORIZONTAL);

    m_playPauseb  = new wxBitmapToggleButton(this, ID_PLAY_PAUSE, wxBitmap("../icons/playpause.png", wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);
    m_stopb       = new wxBitmapToggleButton(this, ID_STOP,       wxBitmap("../icons/stop.png",      wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);
    m_micb        = new wxBitmapToggleButton(this, ID_MIC,        wxBitmap("../icons/mic.png",       wxBITMAP_TYPE_PNG), wxDefaultPosition, wxSize(60, 60), wxBORDER_NONE);

    Connect(ID_PLAY_PAUSE, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyFrame::OnPlayPause) );
    Connect(ID_STOP, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyFrame::OnStop) );
    Connect(ID_MIC, wxEVT_TOGGLEBUTTON,
            wxCommandEventHandler(MyFrame::OnMic) );

    sizer->Add(-1, 20);
    sizer->Add(m_playPauseb,  0, wxTOP, 5);
    sizer->Add(m_stopb,  0, wxTOP, 5);
    sizer->Add(m_micb,  0, wxTOP, 5);

    SetSizer(sizer);

#if wxUSE_STATUSBAR
    CreateStatusBar(2);
#endif // wxUSE_STATUSBAR

    Center();
    SetSize(400, 150);
    Show();
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

void MyFrame::OnPlayPause(wxCommandEvent& WXUNUSED(event) )
{
    if(!playpause){
        playpause = 1;

        MyThread *thread = CreateThread();

        if ( thread->Run() != wxTHREAD_NO_ERROR )
        {
            wxLogError(wxT("Can't start thread!"));
        }
    }
    else if(playpause == 1){
        sc->pause();
        //cout<<"Pause recording..."<<endl;
        playpause = 2;
    }
    else if(playpause == 2){
        sc->resume();
        //cout<<"Resume recording..."<<endl;
        playpause = 1;
    }

#if wxUSE_STATUSBAR
    SetStatusText(wxT("..."), 1);
#endif // wxUSE_STATUSBAR
}

void MyFrame::OnStop(wxCommandEvent& WXUNUSED(event) )
{
    //API stop recording thread
    cout<<"Stop recording"<<endl;
}

void MyFrame::OnMic(wxCommandEvent& WXUNUSED(event) )
{
    //API set up recording from mic too
    cout<<"Recording mic"<<endl;
}

// ----------------------------------------------------------------------------
// MyThread
// ----------------------------------------------------------------------------

MyThread::MyThread()
        : wxThread()
{
    m_count = 0;
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
    vi.width = 800;
    vi.height = 800;
    vi.offset_x = 0;
    vi.offset_y = 0;
    vi.framerate = 30;
    vi.status  = &playpause;
    cout<<"0x"<<&playpause<<endl;
    vi.output_file = "./prova.mp4";
    ScreenRecorder screen_recorder{vi};

    sc = &screen_recorder;
    sc->recording();

    //playpause = 0;


    //wxLogMessage("Thread finished.");

    return NULL;
}
