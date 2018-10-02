#pragma once
#include "BaseTest.h"


class PlayerTest
{
public:
    ofxOMXPlayerSettings settings;
    ofxOMXPlayer* omxPlayer;
    int loopCount;
    PlayerTest()
    {
        loopCount=0;
        omxPlayer = NULL;
    }
    
    void setup(string videoPath, ofxOMXPlayerListener* listener)
    {
        settings.videoPath = videoPath;
        settings.useHDMIForAudio = true;	//default true
        settings.enableTexture = true;		//default true
        settings.enableLooping = true;		//default true
        settings.enableAudio = true;		//default true, save resources by disabling
        //settings.doFlipTexture = true;		//default false
        settings.listener = listener;
    }
  
    void start()
    {
        omxPlayer = new ofxOMXPlayer();
        omxPlayer->setup(settings);
    }
    
    void close()
    {
        
        delete omxPlayer;
        omxPlayer = NULL;
    }
    
    
};


class PlaylistTest : public BaseTest
{
public:
    
    vector<PlayerTest> playerTests;
    PlayerTest* currentPlayerTest;    
    bool allTestsComplete;
    bool doNextVideo;
    int currentTestID;
    PlaylistTest()
    {
        currentTestID = 0;
        allTestsComplete = false;
        doNextVideo = false;
    }
    
    void close()
    {
        if(currentPlayerTest)
        {
            currentPlayerTest->close();
            delete currentPlayerTest;
            currentPlayerTest = NULL;
        }
        omxPlayer = NULL;
        listener = NULL;
    }
    
    void setup(string name_ = "UNDEFINED")
    {
        name = name_;        
    }
    
    void start()
    {
        
        ofDirectory folder(ofToDataPath("/home/pi/videos", true));
        folder.sort();
        for(size_t i=0; i<folder.getFiles().size(); i++)
        {
            if(!folder.getFiles()[i].isDirectory())
            {
                PlayerTest playerTest;
                playerTest.setup(folder.getFiles()[i].path(), this);
                playerTests.push_back(playerTest);
            }
        }

        currentPlayerTest = &playerTests[currentTestID];
        currentPlayerTest->start();
        omxPlayer = currentPlayerTest->omxPlayer;
    }
    void update()
    {
        if(doNextVideo)
        {
            doNextVideo = false;
            if(currentTestID+1 < playerTests.size())
            {
                currentTestID++;
                currentPlayerTest->close();
                currentPlayerTest = &playerTests[currentTestID];
                currentPlayerTest->start();
                omxPlayer = currentPlayerTest->omxPlayer;

            }else
            {
                currentPlayerTest->close();
                allTestsComplete = true;
            }
        }
        if(allTestsComplete)
        {
            currentPlayerTest = NULL;
            omxPlayer = NULL;
            playerTests.clear();
            if(listener)
            {
                listener->onTestComplete(this);
            }
        }
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
        
    }
    
    
    void onVideoLoop(ofxOMXPlayer* player)
    {
        currentPlayerTest->loopCount++;
        ofLogVerbose(__func__)  << "currentPlayerTest->.loopCount: " << currentPlayerTest->loopCount;
        if(currentPlayerTest->loopCount == 2)
        {
            doNextVideo = true;
            
        }
    }
    
    void onKeyPressed(int key)
    {
        ofLogVerbose(__func__) << "key: " << key;
        if(key == 'x')
        {
            doNextVideo = true;
        }
        
    }
};
