#pragma once

#include "ofMain.h"
#include "ofAppEGLWindow.h"
#include "BaseVideoDecoder.h"

class VideoDecoderTextured : public BaseVideoDecoder
{
public:
    VideoDecoderTextured();
    ~VideoDecoderTextured(){};
    
    bool open(StreamInfo& hints, OMXClock *clock, EGLImageKHR eglImage);
    
    bool  decode(uint8_t *pData, int iSize, double pts);
    
    int getCurrentFrame();
    void resetFrameCounter();
    static OMX_ERRORTYPE onFillBufferDone(OMX_HANDLETYPE hComponent,
                                          OMX_PTR pAppData,
                                          OMX_BUFFERHEADERTYPE* pBuffer);
    
private:
    int frameCounter;
    int frameOffset;
};
