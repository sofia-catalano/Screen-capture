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
#include "Devices.h"

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

    wxToggleButton *m_playPauseb;
    wxButton       *m_stopb;
    wxToggleButton *m_micb;

    wxToggleButton *screen_portion_b;
    wxToggleButton *full_screen_b;


    wxComboBox *listAudioDevices;
    wxTextCtrl *path;

    MyFrame(const wxString& title);
    virtual ~MyFrame();

    // event handlers
    // --------------
    void OnPlayPause(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnMic(wxCommandEvent& event);
    void OnScreenPortion(wxCommandEvent& event);
    void OnFullScreen(wxCommandEvent& event);

    void OnSelectionAudio(wxCommandEvent& e);
    //TODO evento su chiusura finestra

    // helper function - creates a new thread (but doesn't run it)
    MyThread *CreateThread();

    // remember the number of running threads and total number of threads
    size_t m_nRunning,
            m_nCount;
};

//-----------------------------------------------
//  WINDOW TO GET AREA TO RECORD
//-----------------------------------------------
class MyFrame1 : public wxFrame{
        public:
        // ctor

        wxButton *submit_b;
        wxTextCtrl *w;
        wxTextCtrl *h;

        wxTextCtrl *x;
        wxTextCtrl *y;

        MyFrame1(const wxString& title);
        virtual ~MyFrame1();

        // event handlers
        // --------------
        void OnConfirm(wxCommandEvent& event);

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
const int ID_SCREEN_PORTION  = 4;
const int ID_FULL_SCREEN  = 5;

const int ID_WIDTH  = 6;
const int ID_HEIGHT = 7;
const int ID_OFF_X  = 8;
const int ID_OFF_Y  = 9;

const int ID_CONFIRM = 10;

