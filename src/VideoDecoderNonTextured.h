#pragma once

#include "ofMain.h"

#include "VideoDecoderBase.h"


class VideoDecoderNonTextured : public VideoDecoderBase
{
public:
  
    VideoDecoderNonTextured();
    ~VideoDecoderNonTextured();
    bool open(OMXStreamInfo& hints,
              OMXClock *clock, 
              float display_aspect = 0.0f, 
              bool deinterlace = false, 
              bool hdmi_clock_sync = false);
    
    bool decode(uint8_t *pData, int iSize, double pts);
  
    void setDisplayRect(ofRectangle& rectangle);
    void configureDisplay();
    void updateFrameCount();
    void onUpdate(ofEventArgs& args);
    
    int getCurrentFrame();
    void resetFrameCounter();
    
    Component m_omx_image_fx;
    Tunnel m_omx_tunnel_image_fx;
    bool doDeinterlace;
    bool doHDMISync;
    ofRectangle displayRect;
    
private:
    int frameCounter;
    int frameOffset;
    
    
};