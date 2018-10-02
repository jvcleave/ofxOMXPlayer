#pragma once
#include "BaseTest.h"

class TexturedStreamTest : public BaseTest
{
public:
    
    TexturedStreamTest()
    {
        
    }
    void close()
    {
        delete omxPlayer;
        omxPlayer = NULL;
        listener = NULL;
    }
    
    void setup(string name_ = "UNDEFINED")
    {
        name = name_;
    }
    void start()
    {
    
        ofxOMXPlayerSettings settings;
        settings.videoPath = "https://devimages.apple.com.edgekey.net/samplecode/avfoundationMedia/AVFoundationQueuePlayer_HLS2/master.m3u8";
        settings.useHDMIForAudio = true;	//default true
        settings.enableTexture = true;		//default true
        settings.enableLooping = false;		//default true
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
        if(!omxPlayer)
        {
            return;
        }
        if(!omxPlayer->isTextureEnabled())
        {
            return;
        }
        
        omxPlayer->draw(0, 0, ofGetWidth(), ofGetHeight());
        
        //draw a smaller version in the lower right
        int scaledHeight	= omxPlayer->getHeight()/4;
        int scaledWidth		= omxPlayer->getWidth()/4;
        omxPlayer->draw(ofGetWidth()-scaledWidth, ofGetHeight()-scaledHeight, scaledWidth, scaledHeight);
        
        ofDrawBitmapStringHighlight(omxPlayer->getInfo(), 60, 60, ofColor(ofColor::black, 90), ofColor::yellow);
    }
    
    void onVideoEnd(ofxOMXPlayer* player)
    {
        if(listener)
        {
            listener->onTestComplete(this);
        }
    }
    
    
    void onVideoLoop(ofxOMXPlayer* player)
    {
        
    }
    
    void onKeyPressed(int key)
    {
        ofLogVerbose(__func__) << "key: " << key;
        
    }
};
