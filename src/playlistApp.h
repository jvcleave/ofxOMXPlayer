#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "GlobalEGLContainer.h"
#include "ConsoleListener.h"


class playlistApp : public ofBaseApp, public ofxOMXPlayerListener, public SSHKeyListener{

	public:

		void setup();
		void update();
		void draw();
	
	
		void keyPressed(int key);
		void createPlayer();
		ofxOMXPlayer omxPlayer;
		void onVideoEnd(ofxOMXPlayerListenerEventData& e);
		
		vector<ofFile> files;
		int videoCounter;
	
		void onCharacterReceived(SSHKeyListenerEventData& e);
		ConsoleListener consoleListener;
};

