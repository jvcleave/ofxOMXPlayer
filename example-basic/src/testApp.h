#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

class testApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
			
		ofxOMXPlayer omxPlayer;
	
};

