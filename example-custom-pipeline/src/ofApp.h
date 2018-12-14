#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "ofxOMXPlayerRecorder.h"
#include "TerminalListener.h"

class ofApp : public ofBaseApp, public KeyListener
{
    
public:
    
    
    vector<string> imageFilterNames;
    void setup();
    void update();
    void draw();
    
    ofxOMXPlayer omxPlayer;
    ofxOMXPlayerRecorder omxPlayerRecorder;
    TerminalListener consoleListener;
    void onCharacterReceived(KeyListenerEventData& e)
    {
        keyPressed((int)e.character);
    }
    
    void keyPressed  (int key);
    
};


