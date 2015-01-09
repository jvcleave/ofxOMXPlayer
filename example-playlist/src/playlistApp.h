#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "TerminalListener.h"


class playlistApp : public ofBaseApp, public ofxOMXPlayerListener, public KeyListener{

	public:

		void setup();
		void update();
		void draw();
	
	
		void keyPressed(int key);
		ofxOMXPlayer omxPlayer;
	
		void onVideoEnd(ofxOMXPlayerListenerEventData& e);
		void onVideoLoop(ofxOMXPlayerListenerEventData& e){ /*empty*/ };

		
		vector<ofFile> files;
		int videoCounter;
	
		void onCharacterReceived(KeyListenerEventData& e);
		TerminalListener consoleListener;
		ofxOMXPlayerSettings settings;
	
		void loadNextMovie();
	
};

