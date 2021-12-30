//
// Created by parallels on 12/30/21.
//

#include "ScreenCaptureInterface.h"

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
