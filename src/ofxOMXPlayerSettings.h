#pragma once

#include "ofMain.h"
#include "OMX_Maps.h"
#include "ofxOMXPlayerListener.h"



class DirectDisplayOptions
{
public:
    
    DirectDisplayOptions()
    {
        doFullScreen=false;
        noAspectRatio=false;
        doMirror=false;
        rotationIndex=0;
        
        doHDMISync = true;
        alpha = 255;
        doMirror = false;
        rotationIndex = 0;
        rotationDegrees =0; 
        pixelAspectX = 0;
        pixelAspectY = 0;
        doForceFill = false;
    };
    
    //decoder related
    
    bool doHDMISync;
    
    ofRectangle drawRectangle;
    ofRectangle cropRectangle;
    bool doFullScreen;
    bool noAspectRatio;
    bool doMirror;
    int rotationIndex;
    int rotationDegrees;
    int alpha;
    int pixelAspectX; 
    int pixelAspectY;
    bool doForceFill;
};

class ofxOMXPlayerSettings
{
public:
    ofxOMXPlayerSettings()
    {
        videoPath = "";

        useHDMIForAudio = true;
        enableTexture = true;
        doFlipTexture = false; //true for older firmware
        enableLooping = true;
        listener      = NULL;
        enableAudio   = true;
        initialVolume = 0.5;
        videoWidth  = 0;
        videoHeight = 0;
        enableFilters = false;
        filter = OMX_ImageFilterNone;

    }
    bool enableFilters;
    OMX_IMAGEFILTERTYPE filter;
    string videoPath;
    int videoWidth;
    int videoHeight;
    bool enableTexture;
    bool doFlipTexture;
    bool enableAudio;
    float initialVolume;
    bool useHDMIForAudio;
    bool enableLooping;
    ofxOMXPlayerListener* listener;

    DirectDisplayOptions directDisplayOptions;
    /*
     To use HDMI Audio you may need to add the below line to /boot/config.txt and reboot

     hdmi_drive=2

     see http://elinux.org/RPiconfig for more details
     */

};

