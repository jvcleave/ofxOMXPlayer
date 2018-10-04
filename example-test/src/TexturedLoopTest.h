#pragma once
#include "BaseTest.h"

class TexturedLoopTest : public BaseTest
{
public:
    
    int loopCount;
    TexturedLoopTest()
    {
        loopCount = 0;
    }
    void close()
    {
        omxPlayer.close();
        listener = NULL;
        loopCount = 0;
        isOpen = false;


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
        settings.enableTexture = true;		//default true
        settings.enableLooping = true;		//default true
        settings.enableAudio = true;		//default true, save resources by disabling
        //settings.doFlipTexture = true;		//default false
        settings.enableFilters = true;
        settings.listener = this;
        //so either pass in the settings
        omxPlayer.setup(settings);
        isOpen = true;

    }
    void update()
    {
    
    }
    
    void draw()
    {
        
        if(!omxPlayer.isTextureEnabled())
        {
            return;
        }
        
        omxPlayer.draw(0, 0, ofGetWidth(), ofGetHeight());
        
        //draw a smaller version in the lower right
        int scaledHeight	= omxPlayer.getHeight()/4;
        int scaledWidth		= omxPlayer.getWidth()/4;
        omxPlayer.draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);
        
        ofDrawBitmapStringHighlight(omxPlayer.getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
    }
    
    void onVideoEnd(ofxOMXPlayer* player)
    {
        
    }
    
    void onVideoLoop(ofxOMXPlayer* player)
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

    
    void onKeyPressed(int key)
    {
        ofLogVerbose(__func__) << "key: " << key;
        
    }
};
