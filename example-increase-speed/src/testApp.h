#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "TerminalListener.h"

class testApp : public ofBaseApp, public KeyListener{

	public:

		void setup();
		void update();
		void draw();
		void keyPressed(int key);	
		ofxOMXPlayer omxPlayer;
	
	void onCharacterReceived(KeyListenerEventData& e);
	TerminalListener consoleListener;
};

