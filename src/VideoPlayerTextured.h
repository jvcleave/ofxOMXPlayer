#pragma once

#include "BaseVideoPlayer.h"
#include "VideoDecoderTextured.h"

#include <deque>
#include <sys/types.h>

class VideoPlayerTextured : public BaseVideoPlayer
{
public:
    VideoPlayerTextured();
    ~VideoPlayerTextured();
    
    bool open(OMXStreamInfo& hints, OMXClock *av_clock, EGLImageKHR eglImage);
    bool openDecoder();
    bool close();
    
    ofxOMXPlayerSettings settings;
    VideoDecoderTextured* eglImageDecoder;
    EGLImageKHR eglImage;
};

