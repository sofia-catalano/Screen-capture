//
// Created by parallels on 12/27/21.
//

#ifndef GUI_INTERFACE_WITH_THREAD_MAIN_H
#define GUI_INTERFACE_WITH_THREAD_MAIN_H

#endif //GUI_INTERFACE_WITH_THREAD_MAIN_H

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_THREADS
#error "This sample.xpm requires thread support!"
#endif // wxUSE_THREADS

#include "wx/thread.h"
#include "wx/dynarray.h"
#include <wx/tglbtn.h>

// define this to use wxExecute in the exec tests, otherwise just use system
#define USE_EXECUTE

#ifdef USE_EXECUTE
#define EXEC(cmd) wxExecute((cmd), wxEXEC_SYNC)
#else
#define EXEC(cmd) system(cmd)
#endif

using namespace std;


class MyThread;
WX_DEFINE_ARRAY_PTR(wxThread *, wxArrayThread);

// ----------------------------------------------------------------------------
// the application object
// ----------------------------------------------------------------------------

class MyApp : public wxApp
{
public:
    MyApp();
    virtual ~MyApp(){};

    virtual bool OnInit();

    wxCriticalSection m_critsect;
    wxArrayThread m_threads;
    wxSemaphore m_semAllDone;
    bool m_shuttingDown;
};

// ----------------------------------------------------------------------------
// the main application frame
// ----------------------------------------------------------------------------


class MyFrame : public wxFrame
{
public:
    // ctor

    wxBitmapToggleButton *m_playPauseb;
    wxBitmapToggleButton *m_stopb;
    wxBitmapToggleButton *m_micb;



    MyFrame(const wxString& title);
    virtual ~MyFrame();

    // event handlers
    // --------------
    void OnPlayPause(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnMic(wxCommandEvent& event);
    //TODO evento su chiusura finestra

    // helper function - creates a new thread (but doesn't run it)
    MyThread *CreateThread();

    // remember the number of running threads and total number of threads
    size_t m_nRunning,
            m_nCount;
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// ID for the menu commands
enum
{
    THREAD_START_THREAD  = 201,
};

// ----------------------------------------------------------------------------
// a simple thread
// ----------------------------------------------------------------------------

class MyThread : public wxThread
{
public:
    MyThread();
    virtual ~MyThread();

    // thread execution starts here
    virtual void *Entry();

public:
    unsigned m_count;
};

const int ID_PLAY_PAUSE   = 1;
const int ID_STOP         = 2;
const int ID_MIC          = 3;
