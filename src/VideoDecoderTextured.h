#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "BaseVideoDecoder.h"

class VideoDecoderTextured : public BaseVideoDecoder
{
public:
    VideoDecoderTextured();
    ~VideoDecoderTextured(){};
    
    bool open(StreamInfo&, OMXClock*, EGLImageKHR);
        
    int getCurrentFrame();
    void resetFrameCounter();
    static OMX_ERRORTYPE onFillBufferDone(OMX_HANDLETYPE, OMX_PTR, OMX_BUFFERHEADERTYPE*);
    
private:
    int frameCounter;
    int frameOffset;
};
