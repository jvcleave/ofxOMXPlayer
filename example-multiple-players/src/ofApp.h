#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		void keyPressed(int key);
	
	
	vector<ofxOMXPlayer*> omxPlayers; 

};

