#pragma once


#include "BaseVideoPlayer.h"
#include "VideoDecoderDirect.h"

class VideoPlayerDirect : public BaseVideoPlayer
{
public:
    VideoPlayerDirect();
    ~VideoPlayerDirect();
    
    bool doDeinterlace;
    bool doHDMISync;
    
    VideoDecoderDirect* nonTextureDecoder;
    
    bool open(StreamInfo& hints, OMXClock *av_clock, bool deinterlace, bool hdmi_clock_sync);
    bool openDecoder();
    bool close();
    void setDisplayRect(ofRectangle& rectangle);

};
