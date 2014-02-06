#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

#include "ConsoleListener.h"

class developApp : public ofBaseApp, public SSHKeyListener, public ofxOMXPlayerListener{
	
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	void keyPressed(int key);

	
	ofxOMXPlayer omxPlayer;
	ofShader shader;
	ofFbo fbo;
	void updateFbo();
	void loadShader();
	bool doShader;
	bool doTextures;
		
	//allows key commands via Shell
	void onCharacterReceived(SSHKeyListenerEventData& e);
	ConsoleListener consoleListener;
		
	
	ofxOMXPlayerSettings settings;
	

	
	bool doWriteImage;
	void onVideoEnd(ofxOMXPlayerListenerEventData& e);
	void onVideoLoop(ofxOMXPlayerListenerEventData& e);
};

