#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "ConsoleListener.h"

class testApp : public ofBaseApp, public SSHKeyListener{

	public:

		void setup();
		void update();
		void draw();
		void keyPressed(int key);	
		ofxOMXPlayer omxPlayer;
		ofFbo fbo;
	
	void onCharacterReceived(SSHKeyListenerEventData& e);
	ConsoleListener consoleListener;
};

