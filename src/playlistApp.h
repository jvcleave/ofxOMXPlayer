#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

class playlistApp : public ofBaseApp, public ofxOMXPlayerListener{

	public:

		void setup();
		void update();
		void draw();
	
		void keyPressed(int key);
		void createPlayer();
		ofxOMXPlayer* omxPlayer;
		void onVideoEnd(ofxOMXPlayerListenerEventData& e);
		vector<ofFile> files;
		int videoCounter;
};

