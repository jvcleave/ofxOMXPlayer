#pragma once


#include "BaseVideoPlayer.h"
#include "VideoDecoderDirect.h"

class VideoPlayerDirect : public BaseVideoPlayer
{
public:
    VideoPlayerDirect();
    ~VideoPlayerDirect();
            
    VideoDecoderDirect* directDecoder;
    
    bool open(StreamInfo& hints, OMXClock *av_clock, ofxOMXPlayerSettings& settings_);
    bool openDecoder();
    void close();
    
    bool doUpdate;
};
