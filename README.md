# Screen-capture

## Our Team 
* [Sofia Catalano](https://github.com/sofia-catalano)
* [Alessio Dongiovanni](https://github.com/alessiodongio)
* [Davide Leone](https://github.com/davide-leone96)

## Our Project
Screen-capture is a system based on two components:
* The Screen-capture library, a multiplatform C++ library capable of capturing the entire screen, or a portion of it, and storing it in mp4 format, with or without audio recorded by the microphone.
* The wxWidgets application, a C++ frontend based on wxWidgets libraries, used to show the proper behavior of the Screen-capture library, and to allow the user to record/pause/resume/stop a video stream.


### Screen-capture library
This is the main part of the project. This library is composed by 2 main files:
* ScreenRecorder.ccp
* ScreenRecorder.h

It uses a third part library, **ffmpeg**. In particular: avformat, avutil, avcodec, avdevice, swscale, swresample.
These libraries allow us to capture audio and video frames, to elaborate them and save the final media in a mp4 file. 

The feature implemented in the library Screen-capture are: 
* Record **Screen Video**
* Eventually record **Audio**
* **Activate and stop** the recording process
* Temporarily **pause** and subsequently **resume** the recording process
* Define the **area** to be recorded
* Define the **file .mp4** that will contain the final recording
* In case of **internal errors**:
  * throws an exception with a description of the error
  * stops the recording process, if it has been activate
  * save in the .mp4 file what has been recorded untill that moment
  * deallocate the resources allocated untill that moment

The functions available in the library are:
