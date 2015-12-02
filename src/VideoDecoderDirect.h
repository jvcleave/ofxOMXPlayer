#pragma once

#include "ofMain.h"

#include "BaseVideoDecoder.h"
#include "OMXDisplay.h"


class VideoDecoderDirect : public BaseVideoDecoder
{
public:
  
    VideoDecoderDirect();
    ~VideoDecoderDirect();
    bool open(StreamInfo&, OMXClock*, ofxOMXPlayerSettings&);
      
    void updateFrameCount();
    void onUpdate(ofEventArgs& args);
    
    int getCurrentFrame();
    void resetFrameCounter();
    
    Component imageFXComponent;
    Tunnel imageFXTunnel;
    bool doDeinterlace;
    bool doHDMISync;    
    OMXDisplay display;
    OMXDisplay* getOMXDisplay()
    {
        return &display;
    }
    ofxOMXPlayerSettings settings;
    bool doUpdate;
private:
    int frameCounter;
    int frameOffset;
    
    
};