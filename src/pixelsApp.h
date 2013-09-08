#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

#include "ConsoleListener.h"
//#include "GlobalEGLContainer.h"	
class pixelsApp : public ofBaseApp, public SSHKeyListener{
	
public:
	
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);

	ofxOMXPlayer omxPlayer;
	bool doSaveImage;
	
	ConsoleListener consoleListener;
	void onCharacterReceived(SSHKeyListenerEventData& e);
	ofImage image;
	bool doUpdatePixels;
};

