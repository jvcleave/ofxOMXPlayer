#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

class BaseTest;

class TestListener
{
public:
    virtual void onTestComplete(BaseTest*)=0;
};

class BaseTest : public ofxOMXPlayerListener
{
public:
    
    ofxOMXPlayer omxPlayer;
    TestListener* listener;
    string name;
    bool isOpen;
    BaseTest()
    {
        name = "UNDEFINED";
        listener = NULL;
        isOpen = false;
    }
    
    
    virtual void setup(string name_ = "UNDEFINED")=0;
    virtual void start()=0;
    virtual void close()=0;
    virtual void  update()=0;
    virtual void  draw()=0;
    virtual void onKeyPressed(int key)=0;
    
    
    
};
