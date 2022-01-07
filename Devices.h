//
// Created by Sofia Catalano on 04/01/22.
//

#ifndef SCREEN_CAPTURE_DEVICES_H
#define SCREEN_CAPTURE_DEVICES_H


class Devices {

};



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
    int n;
} result;

result getAudioDevices();



/*int main(void)

{

    printf("PCM devices:\n");
    printf("\n");
    for(auto i : listdev("pcm").devices){
        std::cout<<i<<std::endl;
    }



    return 0;

}*/

#endif //SCREEN_CAPTURE_DEVICES_H
