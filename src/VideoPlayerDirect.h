#pragma once


#include "BaseVideoPlayer.h"
#include "VideoDecoderDirect.h"

class VideoPlayerDirect : public BaseVideoPlayer
{
public:
    VideoPlayerDirect();
    ~VideoPlayerDirect();
        
    ofxOMXPlayerSettings settings;
    
    VideoDecoderDirect* directDecoder;
    
    bool open(StreamInfo& hints, OMXClock *av_clock, ofxOMXPlayerSettings& settings_);
    bool openDecoder();
    bool close();
    
    bool doUpdate;
};
