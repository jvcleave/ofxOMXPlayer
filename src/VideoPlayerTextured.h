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
    
    bool open(StreamInfo&, OMXClock*, ofxOMXPlayerSettings&, EGLImageKHR);
    bool openDecoder();
    void close();
    
    VideoDecoderTextured* textureDecoder;
    EGLImageKHR eglImage;
};

