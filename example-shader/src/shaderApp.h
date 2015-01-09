#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

#include "TerminalListener.h"

class shaderApp : public ofBaseApp, public KeyListener, public ofxOMXPlayerListener{
	
public:
	
	void setup();
	void update();
	void draw();
	void keyPressed(int key);

	
	ofxOMXPlayer omxPlayer;
	ofShader shader;
	ofFbo fbo;
	void updateFbo();
	void loadShader();
	bool doShader;
	bool doTextures;
	
	
	//allows key commands via Shell
	void onCharacterReceived(KeyListenerEventData& e);
	TerminalListener consoleListener;
	
	void onVideoEnd(ofxOMXPlayerListenerEventData& e);
	void onVideoLoop(ofxOMXPlayerListenerEventData& e);
};

