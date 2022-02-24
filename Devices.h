//
// Created by Sofia Catalano on 04/01/22.
//

#ifndef SCREEN_CAPTURE_DEVICES_H
#define SCREEN_CAPTURE_DEVICES_H

#include <iostream>

#ifdef __linux__
#include <alsa/asoundlib.h>
#include <alsa/control.h>
#include <alsa/pcm.h>
#elif _WIN32
#include <windows.h>
#include <initguid.h>
#include <dshow.h>
#pragma comment(lib, "strmiids")
#endif

#include <vector>
#include <stdlib.h>
#include <string>

using namespace std;

typedef struct {
    std::vector<std::string> devices;
    std::vector<std::string> desc;
    int n;
    int device_select = -1;
} result;

typedef struct {
    int x,y;
} screen_size;

result getAudioDevices();
screen_size getScreenSize();

#endif //SCREEN_CAPTURE_DEVICES_H
