#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "SplitterController.h"
#include "TerminalListener.h"

class ofApp : public ofBaseApp, public KeyListener
{
    
public:
    
    void setup();
    void update();
    void draw();
    
    ofxOMXPlayer omxPlayer;
    SplitterController splitterController;
    TerminalListener consoleListener;
    void onCharacterReceived(KeyListenerEventData& e)
    {
        keyPressed((int)e.character);
    }
    
    void keyPressed  (int key);
    
};


