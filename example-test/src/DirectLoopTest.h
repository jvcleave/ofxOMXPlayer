#pragma once
#include "BaseTest.h"

class DirectLoopTest : public BaseTest
{
public:
    
    int loopCount;
    DirectLoopTest()
    {
        loopCount = 0;
    }
    void close()
    {
        delete omxPlayer;
        omxPlayer = NULL;
        listener = NULL;
        loopCount = 0;
    }
    void setup(string name_ = "UNDEFINED")
    {
        name = name_;        
    }
    void start()
    {
        
        ofDirectory videos(ofToDataPath("/home/pi/videos/current", true));
        videos.sort();
        string videoPath = videos.getFiles()[0].path();
        
        
        ofxOMXPlayerSettings settings;
        settings.videoPath = videoPath;
        settings.useHDMIForAudio = true;	//default true
        settings.enableTexture = false;		//default true
        settings.enableLooping = true;		//default true
        settings.enableAudio = true;		//default true, save resources by disabling
        //settings.doFlipTexture = true;		//default false
        
        settings.listener = this;
        //so either pass in the settings
        omxPlayer = new ofxOMXPlayer();
        omxPlayer->setup(settings);
    }
    void update()
    {
        
    }
    
    void draw()
    {
        return;
    }
    
    void onVideoEnd(ofxOMXPlayer* player)
    {
        loopCount++;
        ofLogVerbose(__func__)  << "loopCount: " << loopCount;
        
        if(loopCount == 2)
        {
            if(listener)
            {
                listener->onTestComplete(this);
            } 
        }
    }
    
    /*
    void onVideoLoop(ofxOMXPlayerListenerEventData& e)
    {
        
        
    }*/
    
    void onKeyPressed(int key)
    {
        ofLogVerbose(__func__) << "key: " << key;
        
    }
};
