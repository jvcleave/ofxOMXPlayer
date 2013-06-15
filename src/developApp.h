#pragma once

#include "ofMain.h"
#include "ofxOMXPlayer.h"
#include "ofxOMXVideoPlayer.h"

#include "ConsoleListener.h"

class developApp : public ofBaseApp, public SSHKeyListener{
	
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);	
	
	ofxOMXPlayer omxPlayer;
	ofxOMXVideoPlayer omxVideoPlayer;
	ofShader shader;
	ofFbo fbo;
	void updateFbo();
	
	bool doShader;
	bool doPause;
	bool doTextures;
	
	void initOMXPlayer();
	void updateTexturePlayer();
	
	//allows key commands via Shell
	void onCharacterReceived(SSHKeyListenerEventData& e);
	ConsoleListener consoleListener;
	
	void initVideoPlayer();
	string videoPath;
};

