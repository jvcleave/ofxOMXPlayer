#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"

#include "ConsoleListener.h"
#include "FPSLogger.h"

class developApp : public ofBaseApp, public SSHKeyListener{
	
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
	
	bool doShader;
	bool doTextures;
		
	//allows key commands via Shell
	void onCharacterReceived(SSHKeyListenerEventData& e);
	ConsoleListener consoleListener;
	
	string videoPath;
	bool isClosing;
	bool usingTexturePlayer;
	
	void createNonTexturePlayer();
	void createTexturePlayer();
	bool isShaderEnabled;
	
	ofxOMXPlayerSettings settings;
	vector<ofFile> files;
	bool doRandomSelect;
	
	FPSLogger fpsLogger;
};

