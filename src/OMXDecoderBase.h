#pragma once

#include "ofMain.h"
#include "ofxOMXPlayerSettings.h"

#include "OMXClock.h"
#include "Tunnel.h"

#include "OMXStreamInfo.h"

#include <IL/OMX_Video.h>

#include "OMXReader.h"
#include "SingleLock.h"




class OMXDecoderBase
{
public:
    OMXDecoderBase();
    virtual ~OMXDecoderBase();
    OMX_VIDEO_CODINGTYPE m_codingType;
    
    Tunnel clockTunnel;
    Tunnel schedulerTunnel;
    Tunnel decoderTunnel;
    
    Component decoderComponent;
    Component renderComponent;
    Component schedulerComponent;
    
    Component* clockComponent;
    OMXClock* omxClock;
    
    bool isOpen;
    
    bool doPause;
    bool doSetStartTime;
    
    unsigned int videoWidth;
    unsigned int videoHeight;
    
    uint8_t* extraData;
    int extraSize;
    
    
    bool isFirstFrame;
    uint32_t validHistoryPTS;
    
    
    
    virtual bool decode(uint8_t *pData, int iSize, double pts)=0;
    
    
    void submitEOS();
    bool EOS();
    
    bool resume();
    bool pause();
    
    bool sendDecoderConfig();
    bool NaluFormatStartCodes(enum AVCodecID codec, uint8_t *in_extradata, int in_extrasize);
    
    unsigned int getFreeSpace();
    unsigned int getSize();
    void Reset();
    
    void processCodec(OMXStreamInfo& hints);
    static unsigned count_bits(int32_t value)
    {
        unsigned bits = 0;
        for(; value; ++bits)
        {
            value &= value - 1;
        }
        return bits;
    }
    
    
    virtual int getCurrentFrame() = 0;
    virtual void resetFrameCounter() = 0;
    CriticalSection  m_critSection;
};