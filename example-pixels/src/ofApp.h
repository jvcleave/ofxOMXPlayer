#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

#include "TerminalListener.h"
class ofApp : public ofBaseApp, public KeyListener{
	
public:
	
	void setup();
	void update();
	void draw();
	
	void keyPressed(int key);

	ofxOMXPlayer omxPlayer;
	bool doSaveImage;
	
	TerminalListener consoleListener;
	void onCharacterReceived(KeyListenerEventData& e);

	ofTexture pixelOutput;
	bool doUpdatePixels;
};

