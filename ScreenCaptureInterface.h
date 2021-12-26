//
// Created by parallels on 12/25/21.
//

#ifndef SCREEN_CAPTURE_SCREENRECORDERINTERFACE_H
#define SCREEN_CAPTURE_SCREENRECORDERINTERFACE_H

#endif //SCREEN_CAPTURE_SCREENRECORDERINTERFACE_H

#include <wx/wx.h>
#include <wx/tglbtn.h>

class MyPanel : public wxPanel
{
public:
    MyPanel(wxPanel *parent);

    void OnPlayPause(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnAudio(wxCommandEvent& event);

    wxBitmapToggleButton *m_playPauseb;
    wxBitmapButton       *m_stopb;
    wxBitmapToggleButton *m_audiob;


};

class ScreenCaptureInterface : public wxFrame
{
public:
    ScreenCaptureInterface(const wxString& title);

    MyPanel *btnPanel;
};

const int ID_PLAY_PAUSE   = 1;
const int ID_STOP         = 2;
const int ID_AUDIO        = 3;
