#pragma once


#include "VideoPlayerBase.h"
#include "VideoDecoderNonTextured.h"

class VideoPlayerNonTextured : public VideoPlayerBase
{
public:
    VideoPlayerNonTextured();
    ~VideoPlayerNonTextured();
    
    bool doDeinterlace;
    float displayAspectRatio;
    bool doHDMISync;
    
    VideoDecoderNonTextured* nonTextureDecoder;
    
    bool open(OMXStreamInfo& hints, OMXClock *av_clock, bool deinterlace, bool hdmi_clock_sync, float display_aspect);
    bool openDecoder();
    bool close();
    ofRectangle displayRect;
    void setDisplayRect(ofRectangle& rectangle);
    bool validateDisplayRect(ofRectangle& rectangle);
};
