//
// Created by Sofia Catalano on 04/01/22.
//

#include "Devices.h"

#ifdef __linux__
result listdev_linux(char *devname)

{
    result r;
    char** hints;
    int    err;
    char** n;
    char*  name;
    char*  desc;
    char*  ioid;

    /* Enumerate sound devices */
    r.n=0;
    err = snd_device_name_hint(-1, devname, (void***)&hints);
    if (err != 0) {

        cout<< "*** Cannot get device names"<<endl;
        exit(1);

    }

    n = hints;
    while (*n != NULL) {

        name = snd_device_name_get_hint(*n, "NAME");
        desc = snd_device_name_get_hint(*n, "DESC");
        ioid = snd_device_name_get_hint(*n, "IOID");

        char* aux = strstr(name, ":");

        if(aux != NULL && strncmp(name,"hw",2)==0) {
            char *prop = aux + 1;
            aux = strtok(prop, ",");

            int cardIndex = snd_card_get_index(aux + 5);
            aux = strtok(NULL, ",");
            if (aux != NULL) {

                /*printf( "hw:%d,%s\n", cardIndex, aux + 4);
                printf("Name of device: %s\n", name);
                printf("Description of device: %s\n", desc);*/
                //printf("I/O type of device: %s\n", ioid);
                //printf("\n");

                std::string x = "hw:";
                std::string y = std::to_string(cardIndex);
                std::string z = x+y+","+(aux+4)+"-"+desc;
                //std::cout<<z<<std::endl;
                r.devices.push_back(z);
                r.n++;

            }
        }



        //

        //}

        if (name && strcmp("null", name)) free(name);
        if (desc && strcmp("null", desc)) free(desc);
        if (ioid && strcmp("null", ioid)) free(ioid);
        n++;

    }

    //Free hint buffer too
    snd_device_name_free_hint((void**)hints);

    return r;

}

#elif _WIN32


HRESULT EnumerateDevices(REFGUID category, IEnumMoniker** ppEnum)
{
    // Create the System Device Enumerator.
    ICreateDevEnum* pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                                  CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
        pDevEnum->Release();
    }
    return hr;
}

result listdev_windows(IEnumMoniker *pEnum)
{
    IMoniker *pMoniker = NULL;
    result r;
    r.n=0;

    while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
    {
        IPropertyBag *pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr))
        {
            pMoniker->Release();
            continue;
        }

        VARIANT var;
        VariantInit(&var);

        // Get description or friendly name.
        hr = pPropBag->Read(L"Description", &var, 0);
        if (FAILED(hr))
        {
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
        }
        if (SUCCEEDED(hr))
        {
            //printf("%S\n", var.bstrVal);
            std::wstring ws(var.bstrVal, SysStringLen(var.bstrVal));
            std::string device(ws.begin(), ws.end());
            r.devices.push_back(device);
            r.n++;
            VariantClear(&var);
        }

        hr = pPropBag->Write(L"FriendlyName", &var);

        // WaveInID applies only to audio capture devices.
        hr = pPropBag->Read(L"WaveInID", &var, 0);
        if (SUCCEEDED(hr))
        {
            //printf("WaveIn ID: %d\n", var.lVal);
            VariantClear(&var);
        }

        hr = pPropBag->Read(L"DevicePath", &var, 0);
        if (SUCCEEDED(hr))
        {
            // The device path is not intended for display.
            //printf("Device path: %S\n", var.bstrVal);
            VariantClear(&var);
        }

        pPropBag->Release();
        pMoniker->Release();
    }
    return r;
}
#endif
result getAudioDevices(){
    result r;
#ifdef __linux__
    r=listdev_linux("pcm");
#elif _WIN32
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        IEnumMoniker* pEnum;

        hr = EnumerateDevices(CLSID_AudioInputDeviceCategory, &pEnum);
        if (SUCCEEDED(hr))
        {
            r=listdev_windows(pEnum);
            pEnum->Release();
        }
        CoUninitialize();
    }

#endif
    return r;
}

