#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "TerminalListener.h"
#include "ImageFilterCollection.h"

class ofApp : public ofBaseApp, public KeyListener{


	public:

		void setup();
		void update();
		void draw();
		void keyPressed(int key);
    
		ofxOMXPlayer omxPlayer;
    
    //allows key commands via Shell
    void onCharacterReceived(KeyListenerEventData& e)
    {
        keyPressed((int)e.character);
    };
    
    TerminalListener consoleListener;
    
    bool doChangeFilter;
    
    ImageFilterCollection filterCollection;
	
};

