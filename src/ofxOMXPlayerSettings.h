#pragma once

#include "ofMain.h"
#include <IL/OMX_Core.h>
#include <IL/OMX_Component.h>
#include <IL/OMX_Index.h>
#include <IL/OMX_Image.h>
#include <IL/OMX_Video.h>
#include <IL/OMX_Broadcom.h>
#include "utils/log.h"
#define __func__ __PRETTY_FUNCTION__

class ofxOMXPlayerListener;
class ofxOMXPlayerSettings
{
public:
    ofxOMXPlayerSettings()
    {
        videoPath = "";
        autoStart = true;
        useHDMIForAudio = true;
        enableTexture = true;
        enableLooping = true;
        loopPoint = "0";
        enableAudio   = true;
        initialVolume = 0.3;
        videoWidth  = 0;
        videoHeight = 0;
        enableFilters = false;
        filter = OMX_ImageFilterNone;
        listener = NULL;
        debugLevel = LOG_LEVEL_NONE;
        logDirectory = ofToDataPath("", true);
        logToOF = true;
        setDisplayResolution = false;
        layer = 0;
    }
    bool enableFilters;
    OMX_IMAGEFILTERTYPE filter;
    string videoPath;
    int videoWidth;
    int videoHeight;
    bool enableTexture;
    bool enableAudio;
    float initialVolume; //0.0 - 1.0
    bool useHDMIForAudio;
    bool enableLooping;
    string loopPoint;
    bool autoStart;
    int debugLevel;
    string logDirectory;
    bool logToOF;
    uint layer;
    ofxOMXPlayerListener* listener;
    
    bool setDisplayResolution; //direct only
    ofRectangle directDrawRectangle;
    
    
    //PlayerDirectDisplayOptions directDisplayOptions;
    /*
     To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot
     
     hdmi_drive=2
     
     see http://elinux.org/RPiconfig for more details
     */
    
};
